#ifndef SMUGL_H_PORTABLE_H
#define SMUGL_H_PORTABLE_H
#pragma once
// Portability definitions

#if defined(_WIN32) || defined(_MSC_VER)
/******************************************/
/*  WINDOWS Defines / Includes, etc       */

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <direct.h>
#include <io.h>
#include <windows.h>
#if defined(GetObject)
#undef GetObject
#endif

#define PATH_SEP "\\"
#define PATH_SEP_CHAR '\\'

#else
/******************************************/
/* UNIX Defines / Includes, etc           */

#define PATH_SEP "/"
#define PATH_SEP_CHAR '/'

#include <strings.h>
#define stricmp strcasecmp
#define strnicmp strncasecmp

#endif

#endif  // SMUGL_H_PORTABLE_H
