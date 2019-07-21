// This may look like C, but it's really -*- C++ -*-
// $Id: io.hpp,v 1.4 1999/06/08 15:36:50 oliver Exp $
//extern void tx(char *s, char c=0); // In smugl.h
//extern void txprintf(char *s, ...); // In smugl.h
//extern void txc(char c); // In smugl.h
extern void prompt(long n);
extern void fetch_input(char *s, int length);
extern void telnet_opt(u_char, u_char);
extern void ans(const char *s);
extern void error(int pri, const char *msg, ...);
extern void syslog_perror(const char *s);

extern char addcr;
extern char txed;
extern char temp[];

/* From the latest manpages ATOW */
       #if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
       /* union semun is defined by including <sys/sem.h> */
       #else
       /* according to X/OPEN we have to define it ourselves */
       union semun {
               int val;                    /* value for SETVAL */
               struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
               unsigned short int *array;  /* array for GETALL, SETALL */
               struct seminfo *__buf;      /* buffer for IPC_INFO */
       };
       #endif
/* Bah -- end insert */
