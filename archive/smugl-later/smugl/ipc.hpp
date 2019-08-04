#pragma once
// This may look like C, but it's really -*- C++ -*-
// Global stuff

enum              // Semaphore numbers
{ sem_DATA,       // Lock *data
  sem_PIPE,       // Pipe to the server
  sem_VOCAB,      // Write-lock on vocab table
  sem_MOTION,     // Only one player can move at a time
  sem_PLYR_FILE,  // Lock for the players file
  num_SEMS        // Total number of semaphores
};

class ipcMsg
{
  public:
    u_long to;          // Destination mask
    u_short pri : 3;    // priority
    u_short type : 13;  // type of message
    short len;          // Length of following data
    long data;          // Data value (fairly typical)
    char from;          // which slot it's from
    char pad;           // Padding
    void *ptr;          // Pointer to extra data

  private:  // Don't allow access to these
    void send(int fd, void *extra);

  public:
    ipcMsg(char type = 0, long data = 0, short pri = 0);
    ~ipcMsg(void);
    void send(unsigned long to, short len = 0, void *extra = NULL);
    void send(unsigned long to, char type, long data, char pri, short len = 0, void *extra = NULL);
    bool receive(void);
    inline const void *getPtr(void) { return ptr; };
};

#define to_SERVER 0          // 'To' address for server
#define to_EVERYONE ~0       // 'To' address for all users
#define to_MASK(x) (1 << x)  // 'To' a bitmask

extern struct DATA *data;  // public data area

extern void tidy_ipc(int, void *);
extern void init_ipc(long memory);
extern void sem_lock(int n), sem_unlock(int n);
extern void init_sockets(void);
extern bool accept_connection(void);
extern bool accept_command(void);
extern void client_file_handles(void);
extern void check_for_ipc(void);
extern void ipc_proc(void);
extern void announce(long to, const char *msg);
extern void announce(long to, long msg);
extern void announce_into(long into, const char *msg);

extern int listen_sock_fd;
extern int conn_sock_fd;
extern int command_sock_fd;
extern int servfd[2];
extern int clifd[MAXU][2];
extern int ipc_fd;
extern bool forced;
