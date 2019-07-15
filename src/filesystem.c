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
#    include <sys/mman.h>
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
        alog(AL_FATAL, "Unable to map file %s", filepath);
        return ENOENT;
    }
    void *data = MapViewOfFile(maph, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(maph);
#else
    const int flags = MAP_PRIVATE | MAP_DENYWRITE | MAP_FILE | MAP_POPULATE;
    void *data = mmap(NULL, size, PROT_READ, flags, fd, 0);
    if (data == MAP_FAILED) {
        alog(AL_FATAL, "Failed to load file %s: %d: %s", filepath, errno, strerror(errno));
        return errno;
    }
    close(fd);
#endif

    if (data == NULL) {
        alog(AL_FATAL, "Unable to load file %s", filepath);
        return EINVAL;
    }

    *datap = data;

    return 0;
}

void
CloseFileMapping(void **datap, size_t length)
{
    if (datap && *datap) {
#if defined(_MSC_VER)
        UnmapViewOfFile(*datap);
#else
        munmap(*datap, length);
#endif
    }
    *datap = NULL;
}

struct SourceFile s_sourceFile;
bool s_sourceFileInUse;

error_t
makeTextFileName(struct SourceFile *sourcefile, const char *filename)
{
    char txtFilename[MAX_PATH_LENGTH];
    snprintf(txtFilename, sizeof(txtFilename), "%s.txt", filename);
    return path_joiner(s_sourceFile.filepath, gameDir, txtFilename);
}

error_t
GetFilesSize(const char *filepath, size_t *sizep)
{
    REQUIRE(filepath && sizep);

    struct stat sb = {0};
    error_t err = stat(filepath, &sb);
    if (err != 0)
        return ENOENT;
    *sizep = sb.st_size;
    return 0;
}

error_t
NewSourceFile(const char *filename, struct SourceFile **sourcefilep)
{
    REQUIRE(filename && *filename && sourcefilep);
    if (s_sourceFileInUse)
        return ENFILE;

    struct SourceFile *sourcefile = &s_sourceFile;
    memset(sourcefile, 0, sizeof(*sourcefile));

    error_t err = makeTextFileName(sourcefile, filename);
    if (err != 0) {
        alog(AL_FATAL, "Full filename too long for %s/%s", gameDir, filename);
        return err;
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

    err = NewBuffer(sourcefile->mapping, sourcefile->size, &sourcefile->buffer);
    if (err != 0) {
        CloseSourceFile(&sourcefile);
        return err;
    }

    s_sourceFileInUse = true;
    *sourcefilep = sourcefile;

    return 0;
}

void
CloseSourceFile(struct SourceFile **sourcefilep)
{
    if (sourcefilep && *sourcefilep) {
        CloseBuffer(&(*sourcefilep)->buffer);
        CloseFileMapping(&(*sourcefilep)->mapping, (*sourcefilep)->size);
        free(*sourcefilep);
        *sourcefilep = NULL;
    }
}
