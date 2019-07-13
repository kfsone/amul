#ifndef AMUL_SRC_HASHMAP_H
#define AMUL_SRC_HASHMAP_H

#include "h/amul.test.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "h/amul.hash.h"

// Platform portable case-insensitive string comparison.
#if !defined(_MSC_VER)
#    include <strings.h>
#    define stricmp strcasecmp
#    define strnicmp strcasecmp
#endif

// A type for storing the result of the hashing function
typedef uint32_t hashval_t;

// Internal functions
hashval_t get_string_hash_and_len(const char *string, const char *stringEnd, size_t *length);

static inline hashval_t
get_string_hash(const char *string)
{
    return get_string_hash_and_len(string, NULL, NULL);
}

#endif
