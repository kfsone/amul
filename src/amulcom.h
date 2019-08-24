#ifndef SRC_AMULCOM_H
#define SRC_AMULCOM_H 1

#include "filesystem.h"

#include <h/amul.type.h>
#include <stdbool.h>
#include <stdio.h>

extern int amulcom_main();

extern FILE *ifp;
extern FILE *ofp1;
extern FILE *ofp2;
extern FILE *ofp3;
extern FILE *ofp4;
extern FILE *ofp5;
extern FILE *afp;
extern char  gameDir[MAX_PATH_LENGTH];
extern bool  exiting;

// Close any open output files.
void CloseOutFiles();

// General purpose shutdown with an err, try to favor LogFatal()
[[noreturn]]
void Terminate(error_t err);

extern FILE *OpenGameFile(const char *filename, const char *mode);
extern void  CloseFile(FILE **fpp);

#endif
