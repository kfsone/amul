/*
 * misc.cpp -- miscellaneous routines
 */

static const char rcsid[] = "$Id: misc.cc,v 1.4 1997/05/22 02:21:16 oliver Exp $";

#include "includes.hpp"
#include "libprotos.hpp"
#include "variables.hpp"

/* datafile(filename) - returns the full file and pathname for a
 * given CMP file
 */
static char filename[202];

char *
datafile(const char *s)
    {
    sprintf(filename, "%sData/%s", dir, s);
    return filename;
    }

char *
textfile(const char *s)
    {
    sprintf(filename, "%s%s", dir, s);
    return filename;
    }
