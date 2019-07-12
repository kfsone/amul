#include "h/amul.file.h"

#include "h/amul.test.h"

error_t
path_join(char *into, size_t limit, const char *lhs, const char *rhs) {
    REQUIRE(into && limit > 0 && lhs && rhs && *rhs);

    // Trim leading/trailing /s from each
    while (*rhs == '/' || *rhs == '\\')
        ++rhs;
    REQUIRE(*rhs);

    // Copy the left side of the path into the pathname so we have a mutable
    // storage
    int written = snprintf(into, limit, "%s", lhs);
    REQUIRE(written < limit);
    REQUIRE(into[written]);

    while (written > 0 && (into[written] == '/' || into[written] == '\\')) {
        --written;
    }
    if (!written) {
        into[written++] = '.';
        into[written++] = '/';
    }
    int rhsWritten = snprintf(into + written, limit - written, "%s", rhs);
    REQUIRE(written + rhsWritten < limit);

    return 0;
}
