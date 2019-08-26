#ifndef AMUL_SYSTEM_H
#define AMUL_SYSTEM_H
// OS portability functions/wrappers

#include <cstdlib>

#include "h/amul.type.h"

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) &S_IFMT) == S_IFREG)
#endif

extern void Yield() noexcept;

static inline void *
AllocateMem(size_t bytes)
{
    return calloc(bytes, 1);
}

static inline void
ReleaseMem(void **ptr)
{
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = nullptr;
    }
}

template<typename T>
void
ReleaseMem(T **ptr)
{
    ReleaseMem((void **) ptr);
}

#endif
