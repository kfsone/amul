// This may look like C, but it's really -*- C++ -*-
// $Id: misc.hpp,v 1.4 1999/06/08 15:36:50 oliver Exp $

struct fileInfo {
 const char *name;
 long size;
};

extern long filesize(const char *filename);
extern fileInfo *locate_file(const char *name, int it_matters);
extern long read_file(const char *name, void *&base, int it_matters);
extern void pressret(void);
extern void ShowFile(const char *s);
