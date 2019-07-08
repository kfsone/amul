#pragma once

#include <string>

// Essentially: virtual machine instructions
struct Instruction {
    std::string name;
    uint8_t     parameterCount;
    int8_t      parameters[3];
};

extern const Instruction conditions[NCONDS];
extern const Instruction actions[NACTS];
