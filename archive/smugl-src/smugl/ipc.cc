// ipc.C -- Interprocess Communications Handling
//
// Provides:
//  init_ipc()
//   Initialiases the IPC stuff (creates 'data')
//  tidy_ipc()
//   Added to atexit() by init_ipc to tidy away the IPC stuff
//  sem_lock(table)
//   Lock a table by semaphore
//  sem_unlock(table)
//   Unlock a table by semaphore

static const char rcsid[] = "$Id: ipc.cc,v 1.13 1999/05/25 13:08:12 oliver Exp $";

#include <cerrno>
#include <cstring>

#include "fileio.hpp"
#include "io.hpp"
#include "ipc.hpp"
#include "manager.hpp"
#include "rooms.hpp"
#include "smugl.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/types.h>

// Private stuff
static int shmid = -1;          // Shared memory handle
static char will_tidy_ipc = 0;  // Have we set atexit(&tidy_ipc) yet?
static int ipc_nest = 0;        // To detect io_proc nesting

static struct sembuf sop = { 0, 0, 0 };

// Socket stuff
#define SOCK struct sockaddr_in  // I'm lazy, OK?
int listen_sock_fd = -1;         // FD of socket to listen on
int conn_sock_fd = -1;           // FD of other half of socket
int command_sock_fd = -1;        // FD of command-port socket
SOCK listen_sock;                // sockaddr_in of listener
SOCK conn_sock;                  // sockaddr_in of incoming connection
SOCK command_sock;               // sockaddr_in of command sock
int servfd[2];                   // Client->Server pipe
int clifd[MAXU][2];              // Server->Client pipes
int ipc_fd;                      // Which fd should I listen to for IPC?

// Other public stuff
int forced = FALSE;  // True when we get 'forced'

char sems[num_SEMS];  // Which semaphores we're holding

// Protos
static void tidy_ipc();
void ipc_init();
void sem_lock(int n), sem_unlock(int n);
static void *shmalloc(size_t memory);

// Clean up the IPC and shared memory (presumably for reset or exit)
// We need the arguments because we're also a signal handler, and
// in many compiler environments, signal handlers need these arguments
static void
tidy_ipc()
{
    if (!manager)  // Client - close the handles down
        tx("\n\nNO CARRIER\n\n");
    if (debug && !manager)
        syslog(LOG_INFO, "disconnected #%d", slot);
    else if (debug && !fork_on_load)
        syslog(LOG_INFO, "game shut down");

    // Close the sockets down
    if (conn_sock_fd != -1)
        close(conn_sock_fd);
    if (manager && fork_on_load != -1) {
        if (command_sock_fd != -1)
            close(command_sock_fd);
        if (listen_sock_fd != -1)
            close(listen_sock_fd);
    }

    if (data) {
        if (manager && fork_on_load != -1) {
            semctl(data->semid, IPC_RMID, 0, nullptr);
            data->semid = -1;
        }

        shmdt((char *) data);  // Detatch from the data segment
        data = NULL;
    }

    // Delete the shared memory segment if it exists and we're the parent
    if (manager && fork_on_load != -1 && shmid != -1)
        shmctl(shmid, IPC_RMID, NULL);

    shmid = -1;
}

// Initialise the IPC and shared memory
void
init_ipc(long memory)
{
    /* Unless we've already done this, set the tidy_ipc routine as an
     * 'on exit' function, so it's forcibly called on the way out. */
    if (will_tidy_ipc == 0)
        atexit(&tidy_ipc);
    // Remember that we've already done this
    will_tidy_ipc = 1;

    // Try and allocate the main shared memory segment
    data = (struct DATA *) shmalloc(memory);
    data->errors = 0;
    data->semid = -1;
    // Increment and round-up shmbase
    data->shmbase = ptr_align(data + 1);

    // Now setup the sempahores used for locking control
    data->semid = semget(IPC_PRIVATE, num_SEMS, (IPC_CREAT | IPC_EXCL | 0666));
    if (data->semid == -1) {
        error(LOG_ERR, "can't initialise semaphores: %s", strerror(errno));
        exit(1);
    }
    // Clear the locks
    for (int i = 0; i < num_SEMS; i++) {
        sems[i] = 1;  // So sem_unlock can
        sem_unlock(i);
    }

    // Now create the child->server pipes
    if (pipe(servfd) == -1) {
        syslog_perror("pipe");
        exit(1);
    }
    ipc_fd = servfd[READfd];  // By default, listen here for IPC

    // Set the signal handlers
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, exit);
    signal(SIGQUIT, exit);
    signal(SIGHUP, exit);
}

/* shmalloc - Shared Memory Malloc
 * Creates a suitable shared memory segment, attaches to it, and then
 * returns the pointer to it.
 */
void *
shmalloc(size_t memory)
{
    void *new_shm = (char *) -1;
    // Convert to a multiple of page size with a uintptr guard either side.
    memory = (memory + sizeof(uintptr_t) * 2 + 4095) & ~4095;
    shmid = shmget(IPC_PRIVATE, memory, (IPC_CREAT | IPC_EXCL | 0666));
    if (shmid != -1)
        new_shm = shmat(shmid, NULL, 0);
    if (shmid == -1 || new_shm == (char *) -1) {
        error(LOG_ERR, "can't initialise shared memory: %s", strerror(errno));
        exit(1);
    }
    return new_shm;
}

/* sem_lock - lock a sempahore
 * 'take's a sempahore, waiting until it can be aquired or is deleted
 */
void
sem_lock(int semnum)
{
    if (data && semnum >= 0)  // No point if data isn't assigned
    {
        if (!sems[semnum])  // Don't actually perform multiple locks
        {
            sop.sem_num = semnum;
            sop.sem_op = -1;  // Decrement waits until semaphore is free
            while (semop(data->semid, &sop, 1) == -1 && (errno == EINTR || errno == EAGAIN))
                ;
        }
        sems[semnum]++;  // Count the locks
    }
}

/* sem_unlock - release a semaphore
 * 'give's a semaphore back to the queue
 */
void
sem_unlock(int semnum)
{
    if (data && semnum >= 0 && sems[semnum] > 0) {
        if (--sems[semnum])  // Do we have multiple locks here?
            return;
        sop.sem_num = semnum;
        sop.sem_op = 1;  // Return the semaphore
        while (semop(data->semid, &sop, 1) == -1 && (errno == EINTR || errno == EAGAIN))
            ;
    }
}

// Setup and initialise a listening socket
static int
setup_socket(short port, long addr, SOCK *sock, int listen_to)
{
    int sock_fd;
    int ret;
    int tmp = 1;

    bzero(sock, sizeof(SOCK));
    sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == -1) {
        syslog_perror("socket");
        exit(1);
    }
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(tmp));
    // XXX: Not sure if we need SO_RCVBUF, think I was just being
    // stupid and couldn't get FNBIO to work... ;-)
    setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, &tmp, sizeof(tmp));

    // Set port and address to listen on
    sock->sin_port = htons(port);
    sock->sin_addr.s_addr = htonl(addr);

    // Now bind the socket
    ret = bind(sock_fd, (struct sockaddr *) sock, sizeof(SOCK));
    if (ret == -1) {
        if (errno == EADDRINUSE)
            error(LOG_ERR,
                  "Port %s:%d already in use, unable to bind",
                  inet_ntoa(sock->sin_addr),
                  port);
        else
            error(LOG_ERR, "bind(%s:%d): %s", inet_ntoa(sock->sin_addr), port, strerror(errno));
        exit(1);
    }

    // Make the socket a listener
    ret = listen(sock_fd, listen_to);
    if (ret == -1) {
        error(LOG_ERR, "listen(%s:%d): %s", inet_ntoa(sock->sin_addr), port, strerror(errno));
        close(sock_fd);
        exit(1);
    }

    // Make the socket non-blocking
#if defined(sun) || defined(M_UNIX) || defined(NeXT)
    if (ioctl(sock_fd, FIONBIO, &l) == -1) {
        error(LOG_ERR, "ioctl socket FIONBIO: %s", strerror(errno));
        abort();
    }
#else   // sun
    if (fcntl(sock_fd, F_SETFL, FNDELAY) == -1) {
        error(LOG_ERR, "ioctl socket FNDELAY: %s", strerror(errno));
        abort();
    }
#endif  // sun

    return sock_fd;
}

// Setup the listen sockets - wrapper for setup_socket
void
init_sockets()
{
    bzero(&conn_sock, sizeof(SOCK));

    // Create the inbound half of our socket
    listen_sock_fd = setup_socket(data->port, INADDR_ANY, &listen_sock, 4);
    command_sock_fd = setup_socket(data->port + 1, COMMAND_ADDR, &command_sock, 1);
}

// receive an incoming socket connection
int
accept_connection()
{
    static unsigned int addrlen = sizeof(listen_sock);
    bzero(&conn_sock, addrlen);
    while ((conn_sock_fd = accept(listen_sock_fd, (struct sockaddr *) &conn_sock, &addrlen)) ==
                   -1 &&
           errno == EAGAIN)
        ;
    if (conn_sock_fd == -1) {
        syslog_perror("accept(listen)");
        return FALSE;
    }
    syslog(LOG_INFO, "connect from %s", inet_ntoa(conn_sock.sin_addr));
    return TRUE;
}

// receive an incoming command socket connection
// XXX: Only a framework at the moment. All it does is accept
// XXX: a connection, and then read some data. Once it's read
// XXX: some data, it closes the server down.
// XXX: Also, having 'accept'ed, it shouldn't assume that there
// XXX: will be data there immediately - instead it should
// XXX: add it to the fd's it select()s on
int
accept_command()
{
    char cmd_buf[201];
    int bytes;
    unsigned int addrlen = sizeof(command_sock);

    bzero(&conn_sock, addrlen);
    conn_sock_fd = accept(command_sock_fd, (struct sockaddr *) &conn_sock, &addrlen);
    if (conn_sock_fd == -1) {
        syslog_perror("accept(command)");
        return FALSE;
    }

    // XXX: Really ought to check that 'bytes' != -1
    bytes = read(conn_sock_fd, cmd_buf, 200);
    write(conn_sock_fd, "Bye!\n", 5);
    cmd_buf[bytes] = 0;
    while (bytes-- > 0 && (cmd_buf[bytes] == '\r' || cmd_buf[bytes] == '\n'))
        cmd_buf[bytes] = 0;
    if (debug)
        syslog(LOG_INFO, "read command: [%s]", cmd_buf);
    close(conn_sock_fd);
    conn_sock_fd = -1;
    sleep(1);
    return TRUE;
}

// Functions for class ipcMsg
// It starts to get interesting here :-)
//
// ipcMsg is the generic message class, i.e. it deals with all the
// general interprocess communications.

// The initialiasor; set up an ipcMsg
ipcMsg::ipcMsg(char Type, long Data, short Pri)
{
    type = Type;
    pri = Pri;
    data = Data;
    from = slot;
    len = 0;
    to = 0;
    ptr = NULL;
}

// Destructor
ipcMsg::~ipcMsg()
{
    if (ptr)  // Undo any mallocs
        free(ptr);
    ptr = NULL;
}

// Manager's copy of 'send', which writes the data to a specific
// file descriptor. Non-manager processes should use alternative
// flavours of send, since clients send all their data to the
// manager, which then relays the data to it's destination. In
// this way, I avoid having to communicate file descriptors between
// processes
void
ipcMsg::send(int fd, void *extra)
{
    if (!manager)  // Client's must lock/unlock server-pipe
        sem_lock(sem_PIPE);
    write(fd, this, sizeof(class ipcMsg));
    if (len)
        write(fd, extra, len);
    if (!manager)  // Client's must lock/unlock server-pipe
        sem_unlock(sem_PIPE);
}

// Send an IPC message. All messages are initially sent to the
// server, and it's left to the server to forward to other users
// This version does some of the populating for you
void
ipcMsg::send(flag_t To, short Len, void *extra)
{
    if (manager && !To)
        return;  // Manager can't self-message
    // Write to the server pipe
    to = To;
    if (!extra)
        Len = 0;
    len = Len;
    if (!manager)  // Client's only send to server
        send(servfd[WRITEfd], extra);
    else  // Manager -- send to each client
    {
        for (int i = 0; i < MAXU; i++) {
            if ((to & (1 << i)) && ::data->pid[i] && clifd[i][WRITEfd])
                send(clifd[i][WRITEfd], extra);
        }
    }
}

// Send an IPC message -- specifying extra default values. Basically a wrapper
void
ipcMsg::send(flag_t To, char Type, long Data, char Pri, short Len, void *Extra)
{
    type = Type;
    data = Data;
    pri = Pri;
    send(To, Len, Extra);
}

// Receive an IPC message. [ NOTE: BLOCKING ]
// Returns 'true' if there was a message or 'false' if not.
// Return should always be 'true' for a client, whereas the
// server receives some messages which it simply forwards.
// NOTE: BLOCKING; don't call this to test for data. Test first,
// and if you think there is something to receive, call receive.
char
ipcMsg::receive()
{
    int bytes_read;

    if (ptr)
        free(ptr);
    bzero(this, sizeof(class ipcMsg));  // Nuke any current values, avoid confusion
    while ((bytes_read = read(ipc_fd, this, sizeof(class ipcMsg))) == -1 &&
           (errno == EAGAIN || errno == EINTR))
        ;
    if (bytes_read == 0) {  // We read EOF - not healthy
        error(LOG_ERR, "#%d: Someone closed my ipcfd; this is bad", slot);
        exit(1);
    }
    if (len) {  // Extra data to be dealt with
        ptr = malloc(len + 1);
        *((char *) ptr + len) = 0;
        while (read(ipc_fd, ptr, len) == -1 && (errno == EAGAIN || errno == EINTR))
            ;
    } else
        ptr = NULL;
    if (manager)  // We're the server
    {
        if (!to)  // It's intended for us
            return TRUE;
        send(to, len, ptr);       // It's to be forwarded
        return FALSE;             // But don't do anything else
    } else if (to & (1 << slot))  // I'm in the recipient list
        return TRUE;              // so it's worth looking at
    else                          // I'm NOT in the recipient list
    {
        if (debug)
            error(LOG_INFO, "slot#%d msg type %d from #%d", slot, type, from);
        return FALSE;
    }
}

// Check for any incoming ipc
void
check_for_ipc()
{
    for (;;) {
        pollfd fds{ ipc_fd, POLLIN | POLLRDHUP | POLLPRI, 0 };
        switch (poll(&fds, 1, 0)) {
            case 0:  // no events
                return;
            case 1:  // events on ipc_fd
                ipc_proc();
                continue;
            case -1:  // error
                if (errno == EINTR || errno == EAGAIN)
                    continue;
                tx(INTERNAL_ERROR);
                error(LOG_ERR, "check_for_ipc::select failed - exiting\n");
                exit(1);
        }
    }
}

// Handle an incoming ipc interruption
// NOTE: This assumes that you already know there is an ipc to be
// dealt with; this routine assumes there is something interesting
// on the ipc fd. If there isn't, it'll block.
void
ipc_proc()
{
    ipcMsg inc;  // Incoming message

    if (ipc_nest)  // Prevent nesting of ipc calls
        return;
    ipc_nest++;

    if (!inc.receive())  // Message wasn't worth hearing
    {
        ipc_nest--;
        return;
    }
    switch (inc.type) {
        case MCLOSEING:  // Game is shutting down
            tx(message(RESETTING));
            exit(0);
            break;
        case MMESSAGE:  // text message
            tx((const char *) inc.getPtr(), '\n');
            break;
        default:  // What the fuck was that
            tx("[HARDWARE Hiccup] I dunno what that was.\n");
            break;
    }

    ipc_nest--;  // We're leaving
}

void announce(basic_obj to, const char *msg)  // Accepts message pointer as 2nd argument
{
    // XXX: What about if 'to' is 0 - shouldn't we 'not bother'?
    ioproc(msg);
    ipcMsg out(MMESSAGE);
    for (int i = 0; i < MAXU; i++) {
        if (data->user[i].state < PLAYING)
            to &= ALLBUT(i);
    }
    out.send(to, out_buf_len, out_buf);
}

// Send a message to one or more players
void announce(basic_obj to, msgno_t msg)  // Accepts umsg number as 2nd argument
{
    announce(to, message(msg));
}

// Announce a message to all the players in a given room
void
announce_into(basic_obj to, const char *msg)
{
    if (bobs[to]->type != WROOM)
        return;
    long send_to = PlayerIdx::mask_in_room(to);
    if (send_to)  // Anyone to send to?
    {
        ioproc(msg);
        ipcMsg out(MMESSAGE);
        out.send(send_to, out_buf_len, out_buf);
    }
}
