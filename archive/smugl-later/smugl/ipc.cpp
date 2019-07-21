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

#include "smugl/smugl.hpp"
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include "smugl/manager.hpp"
#include "smugl/ipc.hpp"
#include "smugl/io.hpp"
#include "smugl/rooms.hpp"
#include "include/syslog.hpp"

#include "include/fderror.hpp"

// Private stuff
static int shmid = -1;			// Shared memory handle
static char will_tidy_ipc = 0;	// Have we set atexit(&tidy_ipc) yet?
static int ipc_nest = 0;		// To detect io_proc nesting

static struct sembuf sop = { 0, 0, 0 };

// Socket stuff
#define SOCK struct sockaddr_in	// I'm lazy, OK?
int     listen_sock_fd = -1;	// FD of socket to listen on
int     conn_sock_fd = -1;		// FD of other half of socket
int     command_sock_fd = -1;	// FD of command-port socket
SOCK    listen_sock;			// sockaddr_in of listener
SOCK    conn_sock;				// sockaddr_in of incoming connection
SOCK    command_sock;			// sockaddr_in of command sock
int     servfd[2];				// Client->Server pipe
int     clifd[MAXU][2];			// Server->Client pipes
int     ipc_fd;					// Which fd should I listen to for IPC?

// Other public stuff
bool    forced = false;			// True when we get 'forced'

char    sems[num_SEMS];			// Which semaphores we're holding

// Protos
static void tidy_ipc(void);
void    ipc_init(void);
void    sem_lock(int n), sem_unlock(int n);
static void *shmalloc(long memory);

// Clean up the IPC and shared memory (presumably for reset or exit)
// We need the arguments because we're also a signal handler, and
// in many compiler environments, signal handlers need these arguments
static void
tidy_ipc(void)
{
	if (!g_manager)				// Client - close the handles down
		tx("\n\nNO CARRIER\n\n");
	if (!g_manager)
		sysLog.Write(_FLD, "disconnected #%d", g_slot);
	else if (!g_fork_on_load)
		sysLog.Write(_FLI, "game shut down");

	// Close the sockets down
	if (conn_sock_fd != -1)
		close(conn_sock_fd);
	if (g_manager && g_fork_on_load != -1)
	{
		if (command_sock_fd != -1)
			_close(command_sock_fd);
		if (listen_sock_fd != -1)
			_close(listen_sock_fd);
	}

	if (data)
	{
		if (g_manager && g_fork_on_load != -1)
		{
			semun   rmid;		// To satisfy -wall under GNU

			rmid.val = 0;
			semctl(data->semid, IPC_RMID, 0, rmid);
			data->semid = -1;
		}

		shmdt((char *) data);	// Detatch from the data segment
		data = NULL;
	}

	// Delete the shared memory segment if it exists and we're the parent
	if (g_manager && g_fork_on_load != -1 && shmid != -1)
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
	data->shmbase = (void *) NORMALISE(data + 1);

	// Now setup the sempahores used for locking control
	data->semid = semget(IPC_PRIVATE, num_SEMS, (IPC_CREAT | IPC_EXCL | 0666));
	if (data->semid == -1)
	{
		sysLog.Perror(_FLT, "can't initialise semaphores");
		/*ABORT*/
	}
	// Clear the locks
	for (int i = 0; i < num_SEMS; i++)
	{
		sems[i] = 1;			// So sem_unlock can
		sem_unlock(i);
	}

	// Now create the child->server pipes
	if (pipe(servfd) == -1)
	{
		sysLog.Perror(_FLT, "pipe");
		/*ABORT*/
	}
	ipc_fd = servfd[READfd];	// By default, listen here for IPC

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
void*
shmalloc(long memory)
{
	void* new_shm = (char *) -1;

	shmid = shmget(IPC_PRIVATE, memory, (IPC_CREAT | IPC_EXCL | 0666));
	if (shmid != -1)
		new_shm = shmat(shmid, NULL, 0);
	if (shmid == -1 || new_shm == (char *) -1)
	{
		sysLog.Perror(_FLT, "can't initialise shared memory");
	}
	return new_shm;
}

/* sem_lock - lock a sempahore
 * 'take's a sempahore, waiting until it can be aquired or is deleted
 */
void
sem_lock(int semnum)
{
	if (data && semnum >= 0)	// No point if data isn't assigned
	{
		if (!sems[semnum])		// Don't actually perform multiple locks
		{
			sop.sem_num = semnum;
			sop.sem_op = -1;	// Decrement waits until semaphore is free
			while (semop(data->semid, &sop, 1) == -1 && (errno == EINTR || errno == EAGAIN)) ;
		}
		sems[semnum]++;			// Count the locks
	}
}

/* sem_unlock - release a semaphore
 * 'give's a semaphore back to the queue
 */
void
sem_unlock(int semnum)
{
	if (data && semnum >= 0 && sems[semnum] > 0)
	{
		if (--sems[semnum])		// Do we have multiple locks here?
			return;
		sop.sem_num = semnum;
		sop.sem_op = 1;			// Return the semaphore
		while (semop(data->semid, &sop, 1) == -1 && (errno == EINTR || errno == EAGAIN)) ;
	}
}

// Setup and initialise a listening socket
int
setup_socket(short port, long addr, SOCK * sock, int listen_to)
{
	int     sock_fd;
	int     ret;
	int     tmp = 1;

	bzero(sock, sizeof(SOCK));
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock_fd == -1)
	{
		sysLog.Perror(_FLT, "socket");
		/*ABORT*/
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
	if (ret == -1)
	{
		if (errno == EADDRINUSE)
			sysLog.Write(_FLT, "Port %s:%d already in use, unable to bind", inet_ntoa(sock->sin_addr), port);
		else
			sysLog.Write(_FLT, "bind(%s:%d): %s", inet_ntoa(sock->sin_addr), port, strerror(errno));
		/*ABORT*/
	}

	// Make the socket a listener
	ret = listen(sock_fd, listen_to);
	if (ret == -1)
	{
		sysLog.Write(_FLT, "listen(%s:%d): %s", inet_ntoa(sock->sin_addr), port, strerror(errno));
		/*ABORT*/
	}

	// Make the socket non-blocking
#if defined(sun) || defined(M_UNIX) || defined(NeXT)
	if (ioctl(sock_fd, FIONBIO, &l) == -1)
	{
		sysLog.Perror(_FLT, "ioctl socket FIONBIO");
		/*ABORT*/
	}
#else // sun
	if (fcntl(sock_fd, F_SETFL, FNDELAY) == -1)
	{
		sysLog.Perror(_FLT, "ioctl socket FNDELAY");
		/*ABORT*/
	}
#endif // sun

	return sock_fd;
}

// Setup the listen sockets - wrapper for setup_socket
void
init_sockets(void)
{
	bzero(&conn_sock, sizeof(SOCK));

	// Create the inbound half of our socket
	listen_sock_fd = setup_socket(data->port, INADDR_ANY, &listen_sock, 4);
	command_sock_fd = setup_socket(data->port + 1, COMMAND_ADDR, &command_sock, 1);
}

// receive an incoming socket connection
bool
accept_connection(void)
{
	static unsigned int addrlen = sizeof(listen_sock);

	bzero(&conn_sock, addrlen);
	while ((conn_sock_fd = accept(listen_sock_fd, (struct sockaddr *) &conn_sock, &addrlen)) == -1 && errno == EAGAIN)
		;
	if (conn_sock_fd == -1)
	{
		sysLog.Perror(_FLE, "accept(listen)");
		return false;
	}
	sysLog.Write(_FLI, "connect from %s", inet_ntoa(conn_sock.sin_addr));
	return true;
}

// receive an incoming command socket connection
// XXX: Only a framework at the moment. All it does is accept
// XXX: a connection, and then read some data. Once it's read
// XXX: some data, it closes the server down.
// XXX: Also, having 'accept'ed, it shouldn't assume that there
// XXX: will be data there immediately - instead it should
// XXX: add it to the fd's it select()s on
bool
accept_command(void)
{
	char    cmd_buf[201];
	int     bytes;
	unsigned int addrlen = sizeof(command_sock);

	bzero(&conn_sock, addrlen);
	conn_sock_fd = accept(command_sock_fd, (struct sockaddr *) &conn_sock, &addrlen);
	if (conn_sock_fd == -1)
	{
		sysLog.Perror(_FLE, "accept(command)");
		return false;
	}

	// XXX: Really ought to check that 'bytes' != -1
	bytes = _read(conn_sock_fd, cmd_buf, 200);
	if ( bytes >= 0 )
	{
		if ( _write(conn_sock_fd, "Bye!\n", 5) == 5 )
		{
			cmd_buf[bytes] = 0;
			while (bytes-- > 0 && (cmd_buf[bytes] == '\r' || cmd_buf[bytes] == '\n'))
				cmd_buf[bytes] = 0;
			sysLog.Write(_FLD, "read command: [%s]", cmd_buf);
		}
		else
		{
			cmd_buf[0] = 0 ;
		}
	}
	else
	{
		cmd_buf[0] = 0 ;
	}
	_close(conn_sock_fd);
	conn_sock_fd = -1;
	sleep(1);
	return true;
}

// Functions for class ipcMsg
// It starts to get interesting here :-)
//
// ipcMsg is the generic message class, i.e. it deals with all the
// general interprocess communications.

// The initialiasor; set up an ipcMsg

ipcMsg::ipcMsg(char Type /*=0*/ , long Data /*=0*/ , short Pri /*=0*/ )
:	to(0), pri(Pri), type(Type), len(0), data(Data), from(g_slot), pad(0), ptr(NULL)
{
}

// Destructor
ipcMsg::~ipcMsg(void)
{
	if (ptr)					// Undo any mallocs
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
	if (!g_manager)				// Client's must lock/unlock server-pipe
		sem_lock(sem_PIPE);
	int wv = _write(fd, this, sizeof(*this)) ;
	if ( wv < ssize_t(sizeof(*this)) )
		wv = -1 ;
	else if (len)
	{
		if ( _write(fd, extra, len) < len )
			wv = -1 ;
	}
	if (!g_manager)				// Client's must lock/unlock server-pipe
		sem_unlock(sem_PIPE);

	if ( wv < 0 )
		throw Smugl::FDWriteError("ipcsock", errno, fd) ;
}

// Send an IPC message. All messages are initially sent to the
// server, and it's left to the server to forward to other users
// This version does some of the populating for you
void

ipcMsg::send(unsigned long To, short Len /*=0*/ , void *extra /*=NULL*/ )
{
	if (g_manager && !To)
		return;					// Manager can't self-message
	// Write to the server pipe
	to = To;
	if (!extra)
		Len = 0;
	len = Len;
	if (!g_manager)				// Client's only send to server
		send(servfd[WRITEfd], extra);
	else						// Manager -- send to each client
	{
		for (int i = 0; i < MAXU; i++)
		{
			if ((to & (1 << i)) &&::data->pid[i] && clifd[i][WRITEfd])
				send(clifd[i][WRITEfd], extra);
		}
	}
}

// Send an IPC message -- specifying extra default values. Basically a wrapper
void

ipcMsg::send(u_long To, char Type, long Data, char Pri, short Len /*=0*/ , void *Extra /*=NULL*/ )
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
bool
ipcMsg::receive(void)
{
	int     bytes_read;

	if (ptr)
		free(ptr);
	bzero(this, sizeof(class ipcMsg));	// Nuke any current values, avoid confusion
	while ((bytes_read = _read(ipc_fd, this, sizeof(class ipcMsg))) == -1 && (errno == EAGAIN || errno == EINTR)) ;
	if (bytes_read == 0)
	{							// We read EOF - not healthy
		sysLog.Write(_FLF, "#%d: Someone closed my ipcfd; this is bad", g_slot);
		/*ABORT*/
	}
	if (len)
	{							// Extra data to be dealt with
		ptr = malloc(len + 1);
		*((char *) ptr + len) = 0;
		while (_read(ipc_fd, ptr, len) == -1 && (errno == EAGAIN || errno == EINTR)) ;
	}
	else
		ptr = NULL;
	if (g_manager)				// We're the server
	{
		if (!to)				// It's intended for us
			return true;
		send(to, len, ptr);		// It's to be forwarded
		return false;			// But don't do anything else
	}
	else if (to & (1 << g_slot)) // I'm in the recipient list
		return true;			// so it's worth looking at
	else						// I'm NOT in the recipient list
	{
		sysLog.Write(_FLD, "slot#%d msg type %d from #%d", g_slot, type, from);
		return false;
	}
}

// Check for any incoming ipc
void
check_for_ipc(void)
{
	timeval zero;
	fd_set  test;

	FD_ZERO(&test);
	FD_SET(ipc_fd, &test);
	zero.tv_sec = 0;
	zero.tv_usec = 0;
	int     sel;

	do
	{
		sel = select(ipc_fd + 1, &test, NULL, &test, &zero);
		if (sel == -1 && (errno == EINTR || errno == EAGAIN))
			continue;
		else if (sel == -1)
		{
			tx(INTERNAL_ERROR);
			sysLog.Write(_FLT, "check_for_ipc::select failed - exiting\n");
			/*ABORT*/
		}
		else if (sel != 0)
			ipc_proc();
	}
	while (sel != 0);
}

// Handle an incoming ipc interruption
// NOTE: This assumes that you already know there is an ipc to be
// dealt with; this routine assumes there is something interesting
// on the ipc fd. If there isn't, it'll block.
void
ipc_proc(void)
{
	ipcMsg  inc;				// Incoming message

	if (ipc_nest)				// Prevent nesting of ipc calls
		return;
	ipc_nest++;

	if (!inc.receive())			// Message wasn't worth hearing
	{
		ipc_nest--;
		return;
	}
	switch (inc.type)
	{
	case MCLOSEING:			// Game is shutting down
		tx(message(RESETTING));
		exit(0);
		break;
	case MMESSAGE:				// text message
		tx((const char *) inc.getPtr(), '\n');
		break;
	default:					// What the fuck was that
		tx("[HARDWARE Hiccup] I dunno what that was.\n");
		break;
	}

	ipc_nest--;					// We're leaving
}

void
announce(long to, const char *msg)	// Accepts message pointer as 2nd argument
{
	// XXX: What about if 'to' is 0 - shouldn't we 'not bother'?
	ioproc(msg);
	ipcMsg  out(MMESSAGE);

	for (int i = 0; i < MAXU; i++)
	{
		if (data->user[i].state < PLAYING)
			to &= ALLBUT(i);
	}
	out.send(to, out_buf_len, out_buf);
}

// Send a message to one or more players
void
announce(long to, long msg)		// Accepts umsg number as 2nd argument
{
	announce(to, message(msg));
}

// Announce a message to all the players in a given room
void
announce_into(basic_obj to, const char *msg)
{
	if (bobs[to]->type != WROOM)
		return;
	long    send_to = PlayerIdx::mask_in_room(to);

	if (send_to)				// Anyone to send to?
	{
		ioproc(msg);
		ipcMsg  out(MMESSAGE);

		out.send(send_to, out_buf_len, out_buf);
	}
}
