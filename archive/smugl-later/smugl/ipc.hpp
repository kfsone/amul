#ifndef SMUGL_SMUGL_IPC_H
#define SMUGL_SMUGL_IPC_H 1

#include "defines.hpp"
#include "typedefs.hpp"

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
    flag_t to;           // Destination mask
    uint16_t pri : 3;    // priority
    uint16_t type : 13;  // type of message
    short len;           // Length of following data
    long data;           // Data value (fairly typical)
    char from;           // which slot it's from
    char pad;            // Padding
    void *ptr;           // Pointer to extra data

  private:  // Don't allow access to these
    void send(int fd, void *extra);

  public:
    ipcMsg(char type = 0, long data = 0, short pri = 0);
    ~ipcMsg();
    void send(flag_t to, short len = 0, void *extra = nullptr);
    void send(flag_t to, char type, long data, char pri, short len = 0, void *extra = nullptr);
    bool receive();
    inline const void *getPtr() { return ptr; };
};

#define to_SERVER 0            // 'To' address for server
#define to_EVERYONE (~0)       // 'To' address for all users
#define to_MASK(x) (1 << (x))  // 'To' a bitmask

extern struct DATA *data;  // public data area

extern void tidy_ipc(int, void *);
extern void init_ipc(long memory);
extern void sem_lock(int n), sem_unlock(int n);
extern void init_sockets();
extern bool accept_connection();
extern bool accept_command();
extern void client_file_handles();
extern void check_for_ipc();
extern void ipc_proc();
extern void announce(basic_obj to, const char *msg);
extern void announce(basic_obj to, msgno_t msg);
extern void announce_into(basic_obj to, const char *msg);

extern int listen_sock_fd;
extern int conn_sock_fd;
extern int command_sock_fd;
extern int servfd[2];
extern int clifd[MAXU][2];
extern int ipc_fd;
extern bool forced;

#endif  // SMUGL_SMUGL_IPC_H
