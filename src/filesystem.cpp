#include "h/amul.file.h"
#include "h/amul.test.h"
#include "h/filesystem.h"
#include "h/filesystem.inl.h"
#include "h/logging.h"

char gameDir[MAX_PATH_LENGTH];

// File names
const char *gameDataFile = "Data/prof.amulo";
const char *stringTextFile = "Data/strings.amulo";
const char *stringIndexFile = "Data/stringi.amulo";
const char *travelTableFile = "Data/traveld.amulo";
const char *travelParamFile = "Data/travelp.amulo";
const char *verbDataFile = "Data/verbd.amulo";
const char *verbSlotFile = "Data/verbst.amulo";
const char *verbTableFile = "Data/verbtd.amulo";
const char *verbParamFile = "Data/verbtp.amulo";
const char *objectDataFile = "Data/objectd.amulo";
const char *objectStateFile = "Data/objects.amulo";
const char *adjectiveDataFile = "Data/adjd.amulo";
const char *mobileDataFile = "Data/mobiled.amulo";
const char *mobileCmdFile = "Data/mobilec.amulo";

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
            LogFatal("Could not access file (", errno, "): ", filepath);
        return ENOENT;
    }
    *sizep = sb.st_size;
    return 0;
}
