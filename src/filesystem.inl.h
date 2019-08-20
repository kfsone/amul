#ifndef AMUL_FILESYSTEM_INL_H
#define AMUL_FILESYSTEM_INL_H

#include <cstdio>
#include <string>

#include "amul.test.h"

// Inline / template functions that would otherwise clutter up filesystem.h

static inline error_t
MakeTextFileName(const std::string &name, std::string &into)
{
    REQUIRE(!name.empty());
    return PathJoin(into, gameDir, name + ".txt");
}

#endif  // AMUL_FILESYSTEM_INL_H
