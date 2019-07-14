#ifndef H_AMUL_CONS_H
#define H_AMUL_CONS_H 1
// Common constants.

#include <h/amul.defs.h>
#include <h/amul.type.h>

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

// Language conditionals
extern const char *conds[NCONDS];
// Parameter count for each condition
extern const char ncop[NCONDS];
// Actual parameter list
extern const char tcop[NCONDS][MAX_COND_PARMS];

// Language actions
extern const char *acts[NACTS];
// Parameter count for each condition
extern const char nacp[NACTS];
// Actual parameter list
extern const char tacp[NACTS][MAX_ACT_PARMS];

extern const char *   syntax[NSYNTS];
extern const uint16_t syntl[NSYNTS];

extern const char *gameDataFile;
extern const char *stringTextFile;
extern const char *roomDataFile;
extern const char *rankDataFile;
extern const char *travelTableFile;
extern const char *travelParamFile;
extern const char *verbDataFile;
extern const char *verbSlotFile;
extern const char *verbTableFile;
extern const char *verbParamFile;
extern const char *synonymDataFile;
extern const char *synonymIndexFile;
extern const char *objectDataFile;
extern const char *objectRoomFile;
extern const char *objectStateFile;
extern const char *adjectiveDataFile;
extern const char *mobileDataFile;
extern const char *mobileCmdFile;

#endif
