#ifndef AMULCOM_H
#define AMULCOM_H

#include "h/amul.type.h"

// Close any open output files.
void CloseOutFiles();

// General purpose shutdown with an err, try to favor LogFatal()
[[noreturn]]
void Terminate(error_t err);

extern FILE *OpenGameFile(const char *filename, const char *mode);
extern void  CloseFile(FILE **fpp);

#endif
