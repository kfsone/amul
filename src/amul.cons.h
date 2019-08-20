#ifndef AMUL_CONS_H
#define AMUL_CONS_H 1
// Common constants.

#include "amul.defs.h"
#include "typedefs.h"

// Room flag names
extern const char *rflag[NRFLAGS];
// Object flags
extern const char *obflags1[NOFLAGS];
// Object parameters
extern const char *obparms[NOPARMS];
// Object state flags
extern const char *obflags2[NSFLAGS];
// Number of "put" (destination type)s
extern const char *obputs[NPUTS];
// Prepositions
extern const char *prep[NPREP];

extern const char *syntaxes[NSYNTS];

constexpr const char *managerPortName = "AMUL Server";
extern const char *gameFile;
extern const char *npcDataFile;
extern const char *npcCmdFile;

#endif