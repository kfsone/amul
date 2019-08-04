// Version number information

#if defined(_WIN32) || defined(_MSC_VER)
#include "..\h\SMUGL.h"  // Win32 doesn't see the difference
#else
#include "SMUGL.h"
#endif
#include "smugl.hpp"
#include <cstdio>

char vername[81];  // Version string

void
set_version_string()
{
    sprintf(vername, "SMUGL %s - Simple Multi-User Games Language", SMUGL_BUILD);
}
