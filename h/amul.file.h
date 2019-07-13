#ifndef AMUL_SRC_FILESYSTEM_H
#define AMUL_SRC_FILESYSTEM_H

#include <stdlib.h>  // for size_t

typedef int error_t;

#if defined(__cplusplus)
extern "C" {
#endif

error_t path_copy(char *into, size_t limit, size_t *offset, const char *path);

// Normalize and concat two filenames into one '/' separated path name.
// Returns EINVAL if into is null, limit is 0, lhs is null, rhs is null
// or if the combined paths exceed limit.
error_t path_join(char *into, size_t limit, const char *lhs, const char *rhs);

#if defined(__cplusplus)
};
#endif

#define path_joiner(into, lhs, rhs) path_join(into, sizeof(int), lhs, rhs)

#endif
