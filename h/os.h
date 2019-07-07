#pragma once

#include <cstdlib>

#if defined(WIN32)

#    include <io.h>
#endif

// OS-specific helpers
namespace OS
{
void SetProcessName(const char *title);
int  Run(const char *cmd, ...);

static inline void *
Allocate(size_t bytes)
{
    return malloc(bytes);
}
static inline void *
AllocateClear(size_t bytes)
{
    return calloc(1, bytes);
}

template <typename T>
void
Free(T *&ptr, size_t size)
{
    if (ptr) {
        free(reinterpret_cast<void *>(ptr));
        ptr = nullptr;
    }
}

void CreateFile(const char *path, const char *filename);

}  // namespace OS
