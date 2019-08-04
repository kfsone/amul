#ifndef SMUGL_H_FILEIO_H
#define SMUGL_H_FILEIO_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstdlib>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_DIRECT_H
#include <direct.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <cstdio>

FILE *rfopen(const char *s);
char *text_proc(char *p, FILE *destfp);
void get_line(FILE *fp, char *into, int limit);

#endif  // SMUGL_H_FILEIO_H
