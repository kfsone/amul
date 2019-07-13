#ifndef AMUL_H_AMUL_STRS_H
#define AMUL_H_AMUL_STRS_H 1

// Strings are sequences of characters intended to be sent to players,
// such as room descriptions, etc.
//
// Some are anonymous, e.g string literals and descriptions from the
// rooms.txt file, whereas others are named such as object
// descriptions from the "obdescs.txt" file.

#include <h/amul.type.h>

// Types of string, bit-mask
enum StringType {
    STRING_LITERAL = 0x01,      // Anonymous
    STRING_MESSAGE = 0x02,      // system/user message,
    STRING_OBJECT_DESC = 0x04,  // description used by objects,
    STRING_ROOM_DESC = 0x08,    // room descriptions,
    STRING_FORMATTED = 0x10,    // if true: text has @ tokens
};

// Fixed string IDs
enum StringIDs {
    STRINGID_EMPTY,
    STRINGID_NEWLINE,
};

// String identifier is a 64-bit value that identifies the types
// of the string has and it's offset from the beginning of the
// string table.
typedef uint32_t stringid_t;

// Copy strings as-is
char *StrCopy(char *into, size_t intoSize, const char *start, const char *end);
#define StrCopier(into, start, end) StrCopy(into, sizeof(into), start, end)

// Copy strings, lowercase
char *WordCopy(char *into, size_t intoSize, const char *start, const char *end);
#define WordCopier(into, start, end) WordCopy(into, sizeof(into), start, end)

void ZeroPad(char *string, size_t stringSize);

#endif