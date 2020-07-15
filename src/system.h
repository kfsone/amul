#ifndef AMUL_SYSTEM_H
#define AMUL_SYSTEM_H
// OS portability functions/wrappers

#include <cstdlib>

#include "typedefs.h"
#include "platforms.h"

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m) &S_IFMT) == S_IFREG)
#endif

extern void YieldCpu() noexcept;

static inline void *
AllocateMem(size_t bytes)
{
    return calloc(bytes, 1);
}

template<typename T>
T* AllocateInstances(size_t numInstances)
{
	return static_cast<T*>(calloc(sizeof(T), numInstances));
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
    ReleaseMem(reinterpret_cast<void**>(ptr));
}

#endif
