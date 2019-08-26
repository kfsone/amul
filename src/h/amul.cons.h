#ifndef AMUL_CONS_H
#define AMUL_CONS_H 1
// Common constants.

#include "h/amul.defs.h"
#include "h/amul.type.h"

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

extern const char *syntax[NSYNTS];

constexpr const char *managerPortName = "AMULMgr";
extern const char *gameFile;
extern const char *stringTextFile;
extern const char *stringIndexFile;
extern const char *travelTableFile;
extern const char *travelParamFile;
extern const char *verbDataFile;
extern const char *verbSlotFile;
extern const char *verbTableFile;
extern const char *verbParamFile;
extern const char *objectDataFile;
extern const char *objectStateFile;
extern const char *mobileDataFile;
extern const char *mobileCmdFile;

#endif
