// miscellaneous routines
// This file is for 'stand alone' functions that don't otherwise fit into
// the object hierachy and don't fit into one of the other catagories

#include <cerrno>
#include <cstring>

#include "fderror.hpp"
#include "fileio.hpp"
#include "io.hpp"
#include "libprotos.hpp"
#include "misc.hpp"
#include "smugl.hpp"
#include "syslog.hpp"

size_t
filesize(const char *const filename)
// Return size of an (unopened) file
{
    struct stat sbuf;

    // stat returns -1 on failure, or returns 0 and fills out sbuf
    if (stat(filename, &sbuf) == -1)
        return -1L;

    // Otherwise st_size contains the byte size of the file
    return sbuf.st_size;
}

fileInfo *
locate_file(const char *file, bool it_matters)
{
    static fileInfo fi;

    if (strcmp(rightstr(file, 4), ".CMP") == 0)
        fi.name = datafile(file);
    else
        fi.name = textfile(file);
    fi.size = filesize(fi.name);
    if (fi.size == -1 && (it_matters || ENOENT)) {
        sysLog.Write(_FLT, "can't access file '%s': %s", file, strerror(errno));
        /*ABORT*/
    }

    return &fi;
}

// Load a file into a given area of memory
//  if base is nullptr, then we will malloc you some memory
//  it_matters determines whether failure is fatal or not
long
read_file(const char *file, void *&base, bool it_matters)
{
    fileInfo *fi;
    int fd;

    fi = locate_file(file, it_matters);
    fd = open(fi->name, O_RDONLY);
    if (fd == -1) {
        if (it_matters) {
            sysLog(_FLT, "open(%s, RDONLY): %s\n", file, strerror(errno));
            exit(1);
        }
        return -1;
    }
    if (base == nullptr) {
        base = malloc(fi->size + 2);
        memset(base, 0, fi->size + 2);
    }
    if ((long) read(fd, base, fi->size) < (long) fi->size)
        throw Smugl::FDReadError(file, errno, fd);
    close(fd);
    // We return the file size
    return fi->size;
}

void
pressret()
// Prompt the user to press return
{
    char c;
    tx(message(RETURN));
    fetch_input(&c, 0);
    fflush(stdout);
}

void
ShowFile(const char *file)
// Display a file
{
    void *text = nullptr;
    size_t size = read_file(file, text, false);
    if (size == -1) {
        txprintf(">> Unable to open file '%s'\n", file);
        sysLog.Write(_FLW, "ShowFile: can't open file %s", file);
        return;
    }
    if (size == 0)
        return;
    tx((char *) text, '\n');
    free(text);
    return;
}
