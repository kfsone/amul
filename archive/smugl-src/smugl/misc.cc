// miscellaneous routines
// This file is for 'stand alone' functions that don't otherwise fit into
// the object hierachy and don't fit into one of the other catagories

static const char rcsid[] = "$Id: misc.cc,v 1.7 1999/06/08 15:36:50 oliver Exp $";

#include <cerrno>
#include <cstring>

#include "fileio.hpp"
#include "io.hpp"
#include "libprotos.hpp"
#include "misc.hpp"
#include "smugl.hpp"

long filesize(const char *filename)  // Return size of an (unopened) file
{
    struct stat sbuf;

    // stat returns -1 on failure, or returns 0 and fills out sbuf
    if (stat(filename, &sbuf) == -1)
        return -1L;

    // Otherwise st_size contains the byte size of the file
    return sbuf.st_size;
}

fileInfo *
locate_file(const char *file, int it_matters)
{
    static fileInfo fi;
    if (strcmp(rightstr(file, 4), ".CMP") == 0)
        fi.name = datafile(file);
    else
        fi.name = textfile(file);
    fi.size = filesize(fi.name);
    if (fi.size == -1 && (it_matters | ENOENT)) {
        error(LOG_ERR, "can't access file '%s': %s", file, strerror(errno));
        exit(1);
    }

    return &fi;
}

// Load a file into a given area of memory
//  if base is NULL, then we will malloc you some memory
//  it_matters determines whether failure is fatal or not
long
read_file(const char *file, void *&base, int it_matters)
{
    fileInfo *fi;
    int fd;

    fi = locate_file(file, it_matters);
    fd = open(fi->name, O_RDONLY);
    if (fd == -1) {
        if (it_matters) {
            error(LOG_ERR, "open(%s, RDONLY): %s\n", file, strerror(errno));
            exit(1);
        }
        return -1;
    }
    if (base == nullptr) {
        base = malloc(fi->size + 2);
        bzero(base, fi->size + 2);
    }
    read(fd, base, fi->size);
    close(fd);
    // We return the file size
    return fi->size;
}

void pressret()  // Prompt the user to press return
{
    char c;
    tx(message(RETURN));
    fetch_input(&c, 0);
    fflush(stdout);
}

void ShowFile(const char *file)  // Display a file
{
    void *text = nullptr;
    long size;

    size = read_file(file, text, FALSE);
    if (size == -1) {
        txprintf(">> Unable to open file '%s'\n", file);
        syslog(LOG_INFO, "ShowFile: can't open file %s", file);
        return;
    }
    if (size == 0)
        return;
    tx((char *) text, '\n');
    free(text);
    return;
}
