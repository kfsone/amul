#ifndef AMUL_SRC_FILESYSTEM_H
#define AMUL_SRC_FILESYSTEM_H

#include <h/amul.enum.h>
#include <h/amul.type.h>

#include <cstring>
#include <string_view>

extern char gameDir[MAX_PATH_LENGTH];

void UnlinkGameFile(const char *gamefile);

error_t GetFilesSize(std::string_view filepath, size_t *size, bool required = false);

// Copy and normalize a path into another location
error_t PathCopy(char *into, size_t limit, size_t *offset, const char *path);
#define path_copier(into, from) PathCopy(into, sizeof(into), 0, from)

// Normalize and append one path onto another, adding a separator between them
// if need be.
static inline error_t
_path_concater(char *into, size_t limit, const char *path)
{
    size_t offset = strlen(into);
    return PathCopy(into, limit, &offset, path);
}
#define path_concater(into, from) _path_concater(into, sizeof(into), from)

// Normalize and concat two filenames into one '/' separated path name.
// Returns EINVAL if into is null, limit is 0, lhs is null, rhs is null
// or if the combined paths exceed limit.
error_t PathJoin(char *into, size_t limit, const char *lhs, const char *rhs);

#define path_joiner(into, lhs, rhs) PathJoin(into, sizeof(into), lhs, rhs)
#define gamedir_joiner(filename) path_joiner(filepath, gameDir, filename)
#define safe_gamedir_joiner(filename)                                                              \
    if (gamedir_joiner(filename) != 0)                                                             \
        alog(AL_FATAL, "Unable to form filename for %s / %s", gameDir, filename);

#endif
