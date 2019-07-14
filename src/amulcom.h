#ifndef SRC_AMULCOM_H
#define SRC_AMULCOM_H 1

#include <h/amul.file.h>
#include <h/amul.type.h>
#include <stdio.h>
#include <stdbool.h>

extern int amulcom_main();

extern FILE *ifp;
extern FILE *ofp1;
extern FILE *ofp2;
extern FILE *ofp3;
extern FILE *ofp4;
extern FILE *ofp5;
extern FILE *afp;
extern char  gameDir[MAX_PATH_LENGTH];
extern bool  exiting, reuseRoomData, checkDmoves;

// Close any open output files.
void CloseOutFiles();

// General purpose shutdown with an err (try to favor alog(AL_FATAL))
void         Terminate(error_t err);

extern FILE *OpenGameFile(const char *filename, const char *mode);
extern void  CloseFile(FILE **fpp);

#endif
