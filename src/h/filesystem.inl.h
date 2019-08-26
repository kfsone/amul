#ifndef AMUL_FILESYSTEM_INL_H
#define AMUL_FILESYSTEM_INL_H

#include <cstdio>
#include <string>

#include "h/amul.test.h"

// Inline / template functions that would otherwise clutter up filesystem.h

template<size_t Size>
error_t
MakeTextFileName(const std::string &name, char (&into)[Size])
{
    REQUIRE(!name.empty());
    return path_joiner(into, gameDir, (name + ".txt").c_str());
}

#endif  // AMUL_FILESYSTEM_INL_H
