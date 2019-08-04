#ifndef SMUGL_SMUGL_IO_H
#define SMUGL_SMUGL_IO_H

#include <cstdint>

extern void prompt(long n);
extern void fetch_input(char *s, int length);
extern void telnet_opt(uint8_t, uint8_t);
extern void ans(const char *s);
extern void error(int pri, const char *msg, ...);
extern void syslog_perror(const char *s);

extern bool addcr;
extern bool txed;
extern char temp[];

#if defined(NEED_SEMUN) || (defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED))
// union semun is defined by including <sys/sem.h>
#else
// according to X/OPEN we have to define it ourselves
union semun {
    int val;                // value for SETVAL
    struct semid_ds *buf;   // buffer for IPC_STAT, IPC_SET
    unsigned short *array;  // array for GETALL, SETALL
    // Linux specific part:
    struct seminfo *__buf;  // buffer for IPC_INFO
};
#endif

#endif  // SMUGL_SMUGL_IO_H
