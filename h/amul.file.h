#ifndef AMUL_SRC_FILESYSTEM_H
#define AMUL_SRC_FILESYSTEM_H

#include <stdlib.h>  // for size_t

typedef int error_t;

error_t path_join(char *into, size_t limit, const char *lhs, const char *rhs);

#endif
