#ifndef AMUL_H_AMUL_VMOP_H
#define AMUL_H_AMUL_VMOP_H 1

// Virtual machine op codes (instructions).
// "Actions" are the op codes that actually manipulate the game
// world and can modify state, whereas "Conditions" are purely
// reflective: they inspect the state of the game world and control
// whether one or more actions will be executed.

#include <h/amul.defs.h>
#include <h/amul.type.h>

enum { MAX_VMOP_PARAMS = 3 };

struct VMOP {
    const char *name;
    uint8_t     parameterCount;
    int8_t      parameters[MAX_VMOP_PARAMS];
};

extern const struct VMOP conditions[NCONDS];
extern const struct VMOP actions[NACTS];

// Virtual machine instruction
struct VMIns {
    enum Class : vmins_t {
        VIC_INVALID,
        VIC_DESTINATION,
        VIC_CONDITION,
        VIC_NOT_CONDITION,
        VIC_ACTION,
    };

    vmins_t   insClass : 3;
    vmins_t   instruction : 29;
};

#endif  // AMUL_H_AMUL_VMOP_H
