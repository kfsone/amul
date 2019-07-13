#ifndef AMUL_SRC_AMULCOM_RUNTIME_H
#define AMUL_SRC_AMULCOM_RUNTIME_H

#include <h/amul.file.h>
#include <h/amul.type.h>

#include <stdio.h>

extern FILE *ifp;
extern FILE *ofp1;
extern FILE *ofp2;
extern FILE *ofp3;
extern FILE *ofp4;
extern FILE *ofp5;
extern FILE *afp;

extern char gameDir[MAX_PATH_LENGTH];
extern bool exiting, reuseRoomData, checkDmoves;

// Close any open output files.
void CloseOutFiles();

// General purpose shutdown with an err (try to favor alog(AL_FATAL))
void terminate(error_t err);

extern error_t InitRuntimeModule();

#endif