#pragma once
/* Portability definitions */

#if defined(_WIN32) || defined(_MSC_VER)
/******************************************/
/*  WINDOWS Defines / Includes, etc       */

# include <windows.h>
# include <io.h>
# include <fcntl.h>
# include <direct.h>

# define  bzero(ptr, size)  memset(ptr, 0, size)
# define PATH_SEP "\\"
# define PATH_SEP_CHAR '\\'

typedef SSIZE_T ssize_t;

#else
/******************************************/
/* UNIX Defines / Includes, etc           */

# define PATH_SEP "/"
# define PATH_SEP_CHAR '/'

# define _open(...)  open(__VA_ARGS__)
# define _close(...) close(__VA_ARGS__)
# define _write(...) write(__VA_ARGS__)
# define _read(...)  read(__VA_ARGS__)
# define _unlink(...) unlink(__VA_ARGS__)
# define _getcwd(...) getcwd(__VA_ARGS__)
# define _mkdir(...) mkdir(__VA_ARGS__)
# define _strdup(...) strdup(__VA_ARGS__)
#endif

#ifndef S_ISDIR
# define S_ISDIR(_mode_)  (((_mode_) & S_IFMT) == S_IFDIR)
#endif

#ifndef MAXPATHLEN
# define MAXPATHLEN 256
#endif
