#include <h/amul.alog.h>
#include <h/amul.test.h>

#include "buffer.h"
#include "filesystem.h"
#include "filesystem.inl.h"
#include "sourcefile.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#if !defined(_MSC_VER)
#    include <sys/mman.h>
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

    const char *end = into + limit;
    char *      dst = into + offset;
    REQUIRE(dst < end);

    bool wantSlash = offset > 0 && !isSeparator(dst - 1);

    // only write single slashes, and only after seeing a
    // non-slash, so "////" => "" but "////a" => "/a"
    const char *src = path;
    for (; *src && dst < end;) {
        if (isSeparator(src)) {
            wantSlash = true;
            ++src;
            continue;
        }
        if (wantSlash) {
            *(dst++) = '/';
            wantSlash = false;
            continue;
        }
        *(dst++) = *(src++);
    }
    // special case: /
    if (wantSlash && dst == into) {
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
    if (err != 0)
        return err;
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

#ifndef _MSC_VER
static const int MMAP_FLAGS = MAP_PRIVATE | MAP_FILE |
#    ifdef MAP_POPULATE
                              MAP_POPULATE |
#    endif
#    ifdef MAP_DENYWRITE
                              MAP_DENYWRITE |
#    endif
#    ifdef MAP_NOCACHE
                              MAP_NOCACHE |
#    endif
                              0;
#endif

error_t
NewFileMapping(const char *filepath, void **datap, size_t size)
{
    REQUIRE(filepath && datap && size);
    REQUIRE(*datap == NULL);

    int fd = open(filepath, O_RDONLY);
    if (fd < 0)
        return ENOENT;

#if defined(_MSC_VER)
    HANDLE osfh = (HANDLE)_get_osfhandle(fd);
    HANDLE maph = CreateFileMapping(osfh, NULL, PAGE_READONLY, 0, 0, NULL);
    close(fd);
    if (!maph) {
        afatal("Unable to map file %s", filepath);
    }
    void *data = MapViewOfFile(maph, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(maph);
#else
    void *data = mmap(NULL, size, PROT_READ, MMAP_FLAGS, fd, 0);
    if (data == MAP_FAILED) {
        afatal("Failed to load file %s: %d: %s", filepath, errno, strerror(errno));
    }
    close(fd);
#endif

    if (data == nullptr)
        afatal("Unable to load file %s", filepath);

    *datap = data;

    return 0;
}

void
CloseFileMapping(void **datap, size_t length)
{
    if (datap && *datap) {
#if defined(_MSC_VER)
        BOOL result = UnmapViewOfFile(*datap);
        if (!result)
            afatal("Error closing file mapping");
#else
        munmap(*datap, length);
#endif
    }
    if (datap)
        *datap = NULL;
}

SourceFile s_sourceFile{};
bool       s_sourceFileInUse;

error_t
GetFilesSize(const char *filepath, size_t *sizep)
{
    REQUIRE(filepath && sizep);

    struct stat sb = {};
    error_t     err = stat(filepath, &sb);
    if (err != 0)
        return ENOENT;
    *sizep = sb.st_size;
    return 0;
}

error_t
NewSourceFile(const char *filename, SourceFile **sourcefilep)
{
    REQUIRE(filename && *filename && sourcefilep);
    if (s_sourceFileInUse)
        return ENFILE;
    // gamedir must be populated
    if (!gameDir[0])
        return EDOM;

    SourceFile *sourcefile = &s_sourceFile;
    memset(sourcefile, 0, sizeof(*sourcefile));

    error_t err = MakeTextFileName(filename, sourcefile->filepath);
    if (err != 0) {
        alog(AL_ERROR, "Full filename too long for %s/%s", gameDir, filename);
        return EDOM;
    }

    err = GetFilesSize(sourcefile->filepath, &sourcefile->size);
    if (err != 0)
        return err;
    if (sourcefile->size == 0)
        return ENODATA;

    err = NewFileMapping(sourcefile->filepath, &sourcefile->mapping, sourcefile->size);
    if (err != 0) {
        CloseSourceFile(&sourcefile);
        return err;
    }

    sourcefile->buffer.Assign(static_cast<const char *>(sourcefile->mapping), sourcefile->size);
    s_sourceFileInUse = true;
    *sourcefilep = sourcefile;

    return 0;
}

void
CloseSourceFile(SourceFile **sourcefilep)
{
    if (sourcefilep && *sourcefilep) {
        (*sourcefilep)->buffer.Close();
        CloseFileMapping(&(*sourcefilep)->mapping, (*sourcefilep)->size);
        *sourcefilep = NULL;
        s_sourceFileInUse = false;
    }
}
