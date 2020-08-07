#ifndef AMUL_VMOP_H
#define AMUL_VMOP_H

// Virtual machine op codes (instructions).
// "Actions" are the op codes that actually manipulate the game
// world and can modify state, whereas "Conditions" are purely
// reflective: they inspect the state of the game world and control
// whether one or more actions will be executed.

#include "amul.defs.h"
#include "amul.typedefs.h"

enum { MAX_VMOP_PARAMS = 3 };

struct VMOP {
    const char *name;
    uint8_t parameterCount;
    oparg_t parameters[MAX_VMOP_PARAMS];
};

extern const VMOP conditions[NCONDS];
extern const VMOP actions[NACTS];

#endif  // AMUL_VMOP_H
