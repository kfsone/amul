#ifndef SMUGL_H_PORTABLE_H
#define SMUGL_H_PORTABLE_H

// Portability definitions

#if defined(_WIN32) || defined(_MSC_VER)
/******************************************/
// WINDOWS Defines / Includes, etc

#define WIN32_LEAN_AND_MEAN
#include <io.h>
#include <windows.h>

#define bzero(ptr, size) memset(ptr, 0, size)
#define PATH_SEP "\\"
#define PATH_SEP_CHAR '\\'

#else
/******************************************/
// UNIX Defines / Includes, etc

#define PATH_SEP "/"
#define PATH_SEP_CHAR '/'

#endif

#endif  // SMUGL_H_PORTABLE_H