// Version number information

static const char rcsid[] = "$Id: version.cc,v 1.2 1997/04/25 01:13:37 oliver Exp $";

#if defined(_WIN32) || defined(_MSC_VER)
#include "..\h\SMUGL.h"  // Win32 doesn't see the difference
#else
#include "SMUGL.h"
#endif
#include "smugl.hpp"
#include "stdio.h"

char vername[81];  // Version string

void
set_version_string(void)
{
    sprintf(vername, "SMUGL %s - Simple Multi-User Games Language", SMUGL_BUILD);
}
