// Functions belonging to the database manager

#include <cerrno>
#include <csignal>
#include <cstring>
#include <sys/resource.h>

#include "consts.hpp"
#include "fileio.hpp"
#include "fperror.hpp"
#include "io.hpp"
#include "ipc.hpp"
#include "libprotos.hpp"
#include "manager.hpp"
#include "misc.hpp"
#include "objects.hpp"
#include "rooms.hpp"
#include "smugl.hpp"
#include "syslog.hpp"
#include "variables.hpp"

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

// Manager/child indicator. manager == 0 if this is a child process
bool g_manager{ true };

DATA *data;
long roomCount, nounCount;

static inline void incoming_connection();

// Return filesize optimized for memory allocations
static size_t
memsize(const char *file)
{
    const ssize_t size = filesize(datafile(file));
    if (size == -1) {
        sysLog.Write(_FLT, "can't access %s: %s", datafile(file), strerror(errno));
        /*ABORT*/
    }
    // In order to ensure we allow for boundary rationalising,
    // such as on M68k architecture, allow for an additional
    // two long words - that'll allow for boundaries, etc
    return ptr_align((size_t) size);
}

// Evaluate and return the amount of memory required to load the
// complete game into memory
long
memory_required()
{
    long mem = 0L;

    char *file = datafile(statsfn);
    FILE *fp = fopen(file, "rb");
    if (fp == nullptr) {
        sysLog.Write(_FLT, "Missing stats file %s, invalid database", file);
        /*ABORT*/
    }

    if (fscanf(fp, "%ld %ld", &roomCount, &nounCount) < 2)
        throw Smugl::FPReadError(file, errno, fp);

    fclose(fp);

    // Unfortunately I don't store these, so we have to repeat
    // the "experiment" afterwards
    mem = ptr_align(sizeof(DATA));
    mem += memsize(umsgifn) + memsize(umsgfn);
    mem += memsize(ranksfn);
    mem += ptr_align(sizeof(Room) * roomCount);
    mem += memsize(mobfn);
    mem += ptr_align(sizeof(Object) * nounCount);
    mem += memsize(statfn);
    mem += memsize(langfn);
    mem += memsize(ttfn) + memsize(ttpfn);
    mem += memsize(synsifn);
    mem += memsize(vocifn) + memsize(vocfn);
    mem += memsize(bobfn);

    return mem;
}

// Collect notification of dead children
static void
child_reaper(int)
{
    signal(SIGCHLD, child_reaper);
    int pid;
    while ((pid = waitpid(-1, nullptr, WNOHANG)) != -1) {
        if (pid <= 0)
            break;
        for (int i = 0; i < MAXU; i++) {
            if (data->pid[i] == pid) {
                assume_identity(i);
                me->disconnected();
                break;
            }
        }
    }
}

// The overall system shell. This waits for activity on one of the
// primary file handles, and then does the neccesary.
// File handles to be watched:
//  . listen_sock_fd  -- incoming client connections
//  . command_sock_fd -- connections from command-line tools
//  * ipc_pipe_fd     -- [will be] IPC from clients
void
run_the_game()
{
    int n = 0;
    fd_set fds, rd_fds;

    FD_ZERO(&fds);
    FD_SET(listen_sock_fd, &fds);  // incoming client connections
    if (listen_sock_fd > n)
        n = listen_sock_fd;
    FD_SET(command_sock_fd, &fds);  // command socket
    if (command_sock_fd > n)
        n = command_sock_fd;
    FD_SET(ipc_fd, &fds);  // Client->Server IPC pipe
    if (ipc_fd > n)
        n = ipc_fd;

    userbase = data->user;

    // Set the signal handler for dealing with dead-children
    child_reaper(0);

    time(&data->game_start);  // Get current time stamp
    strcpy(data->lastres, ctime(&data->game_start));
    char *cr = strchr(data->lastres, '\n');
    if (cr != nullptr)
        *cr = 0;
    strcpy(data->lastcrt, ctime(&data->compiled));
    cr = strchr(data->lastcrt, '\n');
    if (cr != nullptr)
        *cr = 0;

    if (g_fork_on_load) {
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        switch (fork())
        // Called one, returns twice
        {
            case -1:  // Failed
                sysLog.Perror(_FLF, "can't detach: fork()");
                /*ABORT*/
                break;

            case 0:  // Child
                break;

            default:                  // Parent
                g_fork_on_load = -1;  // So we don't get mistaken for manager
                exit(0);
        }
    }
    sysLog.Write(_FLI, "'%s' restarted on port %d", data->name, data->port);

    while (true) {
        int ret;  // Return value from select

        // Wait for some input
        rd_fds = fds;  // So we don't have to keep constructing it
        ret = select(n + 1, &rd_fds, nullptr, nullptr, nullptr);
        if (ret == -1)
        // If select call fails
        {
            if (errno != EINTR && errno != EAGAIN) {
                sysLog.Perror(_FLT, "run_the_game::select()");
                /*ABORT*/
            }
            continue;
        }

        if (FD_ISSET(listen_sock_fd, &rd_fds)) {  // New Connection arriving
            sysLog.Write(_FLD, "accepting game connection #%ld", data->connections + 1);
            if (accept_connection() == false)
                return;  // Failed to accept() new connection

            incoming_connection();
            if (!g_manager)
                return;
            close(conn_sock_fd);
            conn_sock_fd = -1;
        }

        if (FD_ISSET(ipc_fd, &rd_fds))
            ipc_proc();

        if (FD_ISSET(command_sock_fd, &rd_fds)) {
            sysLog(_FLD, "accepting command connection");
            accept_command();  // Accept a command connection
            exit(0);
        }
    }
    exit(0);
}

// Accept an incoming IP connection.
static inline void
incoming_connection()
{
    int id = -1;
    int assigned_slot = 0;  // slot we get assigned as a client

    // Now find a free slot in the pid table
    sem_lock(sem_DATA);
    data->connections++;
    if (data->connected == MAXU)
        assigned_slot = MAXU;
    else {
        for (; assigned_slot < MAXU; assigned_slot++) {
            if (data->pid[assigned_slot] <= 0)
                break;
        }
    }

    if (assigned_slot == MAXU || assigned_slot < 0) {  // Did we get into the game?
        tx(message(NOSLOTS), '\n');
        sysLog.Write(_FLW, "game full, unable to accept new connection");
        sem_unlock(sem_DATA);
        return;
    }

    // Open a PIPE to talk to this guy
    while ((id = pipe(clifd[assigned_slot])) && (errno == EINTR || errno == EAGAIN))
        ;
    if (id == -1) {  // pipe() failed
        sysLog.Perror(_FLW, "incoming_connect::pipe()");
        sem_unlock(sem_DATA);
        return;
    }
    srand((unsigned int) time(nullptr) + rand());
    id = fork();     // Create a client-child
    if (id == -1) {  // fork() failed
        sysLog.Perror(_FLW, "incoming_connect::fork()");
        sem_unlock(sem_DATA);
        return;
    }
    srand(id + rand());  // Just so we don't share the same seed

    if (!id) {
        close(listen_sock_fd);                  // Dispose of the listener
        listen_sock_fd = -1;                    // We don't need it
        g_manager = false;                      // Flag that we're not the manager
        g_slot = assigned_slot;                 // So we know which slot we're on
        close(clifd[g_slot & 0xff][WRITEfd]);   // Handle for writing to me
        close(servfd[READfd]);                  // Server's read handle
        ipc_fd = clifd[g_slot & 0xff][READfd];  // Listen here for IPC
        me = &data->user[g_slot & 0xff];        // Used throughout the game to refer to self
        me->init_bob();
        me->state = OFFLINE;

        // Sacrifice scheduling points for the benefit of the server
        setpriority(PRIO_PROCESS, 0, getpriority(PRIO_PROCESS, 0) - 5);

        /* We don't own the semaphore on SEM_DATA, the manager does,
         * so we don't sem_unlock it (I think!) */
        return;
    }

    // Otherwise we're back to being the manager
    sysLog(_FLD, "new client (pid=%d) in slot %d", id, assigned_slot);
    data->connected++;
    data->pid[assigned_slot] = id;

    // We have some file handles to modify:
    close(clifd[assigned_slot][READfd]);

    sem_unlock(sem_DATA);
    return;
}
