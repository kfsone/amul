#ifndef AMUL_STRS_H
#define AMUL_STRS_H 1

// Strings are sequences of characters intended to be sent to players,
// such as room descriptions, etc.
//
// Some are anonymous, e.g string literals and descriptions from the
// rooms.txt file, whereas others are named such as object
// descriptions from the "obdescs.txt" file.

#include "h/amul.type.h"

// Copy strings as-is
char *StrCopy(char *into, size_t intoSize, const char *start, const char *end);
#define StrCopier(into, start, end) StrCopy(into, sizeof(into), start, end)

// Copy strings, lowercase
char *WordCopy(char *into, size_t intoSize, const char *start, const char *end);
#define WordCopier(into, start, end) WordCopy(into, sizeof(into), start, end)

void ZeroPad(char *string, size_t stringSize);

#endif
