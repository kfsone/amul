#ifndef AMUL_SRC_FILESYSTEM_INL_H
#define AMUL_SRC_FILESYSTEM_INL_H

#include <h/amul.test.h>

#include <cstdio>
#include <string>

// Inline / template functions that would otherwise clutter up filesystem.h

template <size_t Size>
error_t
MakeTextFileName(const std::string &name, char (&into)[Size])
{
    REQUIRE(!name.empty());
    return path_joiner(into, gameDir, (name + ".txt").c_str());
}

#endif  // AMUL_SRC_FILESYSTEM_INL_H
