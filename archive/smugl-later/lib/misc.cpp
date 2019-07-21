/*
 * misc.cpp -- miscellaneous routines
 */

#include "include/includes.hpp"
#include "include/libprotos.hpp"
#include "include/variables.hpp"

/* datafile(filename) - returns the full file and pathname for a
 * given CMP file
 */
static char filename[202];

char *
datafile(const char *s)
    {
    sprintf(filename, "%sData/%s", g_dir, s);
    return filename;
    }

char *
textfile(const char *s)
    {
    sprintf(filename, "%s%s", g_dir, s);
    return filename;
    }
