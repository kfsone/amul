#include <h/amul.alog.h>
#include <h/amul.file.h>
#include <h/amul.test.h>

#include "buffer.h"
#include "filesystem.h"
#include "filesystem.inl.h"
#include "filemapping.h"
#include "sourcefile.h"

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
        *fp = nullptr;
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
GetFilesSize(std::string_view filepath, size_t *sizep, bool required)
{
    REQUIRE(!filepath.empty() && sizep);

    struct stat sb = {};
    error_t     err = stat(filepath.data(), &sb);
    if (err != 0) {
        if (required)
            afatal("Could not access file (%d): %s", errno, filepath);
        return ENOENT;
    }
    *sizep = sb.st_size;
    return 0;
}

SourceFile::SourceFile(std::string_view filepath)
	: filepath{filepath}
{}

error_t
SourceFile::Open()
{
    error_t err = GetFilesSize(filepath, &size);
    if (err != 0)
        return err;
    if (size == 0)
        return ENODATA;

    err = NewFileMapping(filepath, &mapping, &size);
    if (err != 0) {
        Close();
        return err;
    }

    buffer.Assign(static_cast<const char *>(mapping), size);

    return 0;
}

void
SourceFile::Close()
{
	buffer.Close();
    CloseFileMapping(&mapping, size);
}
