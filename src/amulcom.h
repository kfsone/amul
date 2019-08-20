#ifndef AMULCOM_H
#define AMULCOM_H

#include "typedefs.h"

// Close any open output files.
void CloseOutFiles();

// General purpose shutdown with an err, try to favor LogFatal()
[[noreturn]] void Terminate(error_t err);

extern FILE *OpenGameFile(const char *filename, const char *mode);

#endif
