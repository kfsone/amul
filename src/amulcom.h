#ifndef SRC_AMULCOM_H
#define SRC_AMULCOM_H 1

#include <stdio.h>

extern int amulcom_main();

extern FILE *OpenGameFile(const char *filename, const char *mode);
extern void  CloseFile(FILE **fpp);

#endif
