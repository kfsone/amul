#include "h/amul.file.h"
#include "h/amul.test.h"

static inline bool
is_separator(const char* ptr) { return (*ptr == '/' || *ptr == '\\'); }

error_t
path_copy(char *into, size_t limit, size_t *offsetp, const char *path)
{
    REQUIRE(!offsetp || *offsetp < limit - 1);
    size_t offset = offsetp ? *offsetp : 0;

    const char *end = into + limit - 1;
    char *dst = into + offset;
    REQUIRE(dst < end);

    while (offset > 1 && is_separator(into + offset - 1))
        --offset;
    bool slashed = offset > 0;

    // only write single slashes, and only after seeing a
    // non-slash, so "////" => "" but "////a" => "/a"
    const char *src = path;
    for (; *src && dst < end;) {
        if (is_separator(src)) {
            slashed = true;
            ++src;
            continue;
        }
        if (slashed) {
            *(dst++) = '/';
            slashed = false;
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
path_join(char *into, size_t limit, const char *lhs, const char *rhs) {
    REQUIRE(into && limit > 0 && lhs && rhs && *rhs);

    REQUIRE(*rhs);
    REQUIRE(*lhs);

    size_t length = 0;
    error_t err = path_copy(into, limit, &length, lhs);
    if (length == 0) {
        into[length++] = '.';
    }

    // Copy the left side of the path into the pathname so we have a mutable
    // storage
    REQUIRE(length > 0 && length < limit - 1);
    into[length++] = '/';
    err = path_copy(into, limit, &length, rhs);
    REQUIRE(err == 0);

    return 0;
}
