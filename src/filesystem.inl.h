#ifndef AMUL_SRC_FILESYSTEM_INL_H
#define AMUL_SRC_FILESYSTEM_INL_H

#include <cstdio>
#include <h/amul.test.h>

// Inline / template functions that would otherwise clutter up filesystem.h

template <size_t Size>
error_t
MakeTextFileName(const char *sourceName, char (&into)[Size])
{
    REQUIRE(sourceName);
    char txtFilename[MAX_PATH_LENGTH];
    snprintf(txtFilename, sizeof(txtFilename), "%s.txt", sourceName);
    return path_joiner(into, gameDir, txtFilename);
}

#endif  // AMUL_SRC_FILESYSTEM_INL_H
