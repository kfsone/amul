#pragma once

#include <cstdlib>

// OS-specific helpers
namespace OS {

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
void
Free(void *ptr, size_t size)
{
	free(ptr);
}

}  // namespace OS