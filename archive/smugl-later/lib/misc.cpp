/*
 * misc.cpp -- miscellaneous routines
 */

#include "includes.hpp"
#include "libprotos.hpp"
#include "variables.hpp"

#include <cstdio>

/* datafile(filename) - returns the full file and pathname for a
 * given CMP file
 */
static char filename[202];

char *
datafile(const char *s)
{
    snprintf(filename, sizeof(filename), "%sData/%s", g_dir, s);
    return filename;
}

char *
textfile(const char *s)
{
    snprintf(filename, sizeof(filename), "%s%s", g_dir, s);
    return filename;
}
