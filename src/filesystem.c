#include <h/amul.alog.h>
#include <h/amul.test.h>

#include "buffer.h"
#include "filesystem.h"
#include "sourcefile.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(_MSC_VER)
#    include <unistd.h>  // for unlink
#else
#    define WIN32_LEAN_AND_MEAN
#    define NOMINMAX
#    include <io.h>
#    include <windows.h>
#endif

char gameDir[MAX_PATH_LENGTH];

// File names
const char *gameDataFile = "Prof.amulo";
const char *stringTextFile = "strings.amulo";
const char *roomDataFile = "room.amulo";
const char *rankDataFile = "rank.amulo";
const char *travelTableFile = "traveld.amulo";
const char *travelParamFile = "travelp.amulo";
const char *verbDataFile = "verbd.amulo";
const char *verbSlotFile = "verbst.amulo";
const char *verbTableFile = "verbtd.amulo";
const char *verbParamFile = "verbtp.amulo";
const char *synonymDataFile = "synonymd.amulo";
const char *synonymIndexFile = "synonymi.amulo";
const char *objectDataFile = "objectd.amulo";
const char *objectRoomFile = "objectr.amulo";
const char *objectStateFile = "objects.amulo";
const char *adjectiveDataFile = "adjd.amulo";
const char *mobileDataFile = "mobiled.amulo";
const char *mobileCmdFile = "mobilec.amulo";

static inline bool
isSeparator(const char *ptr)
{
    return (*ptr == '/' || *ptr == '\\');
}

error_t
PathCopy(char *into, size_t limit, size_t *offsetp, const char *path)
{
    REQUIRE(!offsetp || *offsetp < limit - 1);
    size_t offset = offsetp ? *offsetp : 0;

    const char *end = into + limit - 1;
    char *      dst = into + offset;
    REQUIRE(dst < end);

    while (offset > 1 && isSeparator(end - 1)) {
        --dst;
    }
    bool slashed = offset > 0 && dst == into + offset;

    // only write single slashes, and only after seeing a
    // non-slash, so "////" => "" but "////a" => "/a"
    const char *src = path;
    for (; *src && dst < end;) {
        if (isSeparator(src)) {
            slashed = true;
            ++src;
            continue;
        }
        if (slashed) {
            *(dst++) = '/';
            slashed = false;
            continue;
        }
        *(dst++) = *(src++);
    }
    // special case: /
    if (slashed && dst == into) {
        *(dst++) = '/';
    }
    if (*src && dst >= end)
        return EINVAL;
    *dst = 0;
    if (offsetp) {
        *offsetp = dst - into;
    }
    return 0;
}

error_t
PathJoin(char *into, size_t limit, const char *lhs, const char *rhs)
{
    REQUIRE(into && limit > 0 && lhs && rhs && *rhs);

    REQUIRE(*rhs);
    REQUIRE(*lhs);

    size_t  length = 0;
    error_t err = PathCopy(into, limit, &length, lhs);
    if (length == 0) {
        into[length++] = '.';
    }

    // Copy the left side of the path into the pathname so we have a mutable
    // storage
    REQUIRE(length > 0 && length < limit - 1);
    err = PathCopy(into, limit, &length, rhs);
    REQUIRE(err == 0 && length < limit);

    return 0;
}

void
CloseFile(FILE **fp)
{
    if (fp && *fp) {
        fclose(*fp);
        *fp = NULL;
    }
}

void
UnlinkGameFile(const char *gamefile)
{
    char filepath[MAX_PATH_LENGTH];
    if (gamedir_joiner(gamefile) == 0) {
        unlink(filepath);
    }
}

error_t
NewFileMapping(const char *filepath, void **datap, size_t *sizep)
{
    REQUIRE(filepath && datap && sizep);

    struct stat sb = {0};
    error_t     err = stat(filepath, &sb);
    if (err != 0)
        return ENOENT;

    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
        return ENOENT;

#if defined(_MSC_VER)
    HANDLE osfh = (HANDLE)_get_osfhandle(fd);
    HANDLE maph = CreateFileMapping(osfh, NULL, PAGE_READONLY, 0, 0, NULL);
    close(fd);
    if (!maph) {
        alog(AL_FATAL, "Unable to map file %s", filepath);
        return ENOENT;
    }
    const void *data = MapViewOfFile(maph, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(maph);
    if (data == NULL)
        alog(AL_FATAL, "Unable to load file %s", filepath);
#else
#    error("Implement mmap")
#endif

    *datap = data;
    *sizep = sb.st_size;

    return 0;
}

void
CloseFileMapping(void **datap)
{
    if (datap && *datap) {
#if defined(_MSC_VER)
        UnmapViewOfFile(*datap);
#else
#    error("Implement unmap");
#endif
    }
    *datap = NULL;
}

error_t
NewSourceFile(const char *filename, struct SourceFile **sourcefilep)
{
    REQUIRE(filename && sourcefilep);

    char txtFilename[MAX_PATH_LENGTH];
    snprintf(txtFilename, sizeof(txtFilename), "%s.txt", filename);

    struct SourceFile *sourcefile = calloc(1, sizeof(struct SourceFile));
    CHECK_ALLOCATION(sourcefile);
    error_t err = path_joiner(sourcefile->filepath, gameDir, txtFilename);
    if (err != 0) {
        return err;
    }
    size_t size = 0;
    err = NewFileMapping(sourcefile->filepath, &sourcefile->mapping, &size);
    if (err != 0) {
        CloseSourceFile(&sourcefile);
        return err;
    }

    if (size > 0) {
        err = NewBuffer(sourcefile->mapping, size, &sourcefile->buffer);
        if (err != 0) {
            CloseSourceFile(&sourcefile);
            return err;
        }
    }
    return 0;
}

void
CloseSourceFile(struct SourceFile **sourcefilep)
{
    if (sourcefilep && *sourcefilep) {
        CloseBuffer(&(*sourcefilep)->buffer);
        CloseFileMapping(&(*sourcefilep)->mapping);
        free(*sourcefilep);
        *sourcefilep = NULL;
    }
}
