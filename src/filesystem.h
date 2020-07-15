#ifndef AMUL_FILESYSTEM_H
#define AMUL_FILESYSTEM_H

#include <cstring>
#include <string_view>

#include "amul.enum.h"
#include "typedefs.h"
#include "platforms.h"

extern std::string gameDir;

extern void CloseFile(FILE **fpp);

void UnlinkGameFile(const char *gamefile);

error_t GetFilesSize(string_view filepath, size_t *size, bool required = false);

// Copy and normalize a path into another location
void PathAdd(std::string &into, const string_view rhs);

// Normalize and concat two filenames into one '/' separated path name.
// Returns EINVAL if into is null, limit is 0, lhs is null, rhs is null
// or if the combined paths exceed limit.
error_t PathJoin(std::string &into, const string_view lhs, const string_view rhs);

#define gamedir_joiner(filename) PathJoin(filepath, gameDir, filename)
#define safe_gamedir_joiner(filename)                                                              \
    if (gamedir_joiner(filename) != 0)                                                             \
        LogFatal("Unable to form filename for '", gameDir, "' / '", filename, "'")

#endif

