#ifndef AMUL_H_AMUL_HASH_H
#define AMUL_H_AMUL_HASH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint64_t hash_value_t;  // type of value stored in the hash table
typedef int      error_t;

enum { MAX_HASH_KEY_STRLEN = 23, MAX_HASH_KEY_SIZE = MAX_HASH_KEY_STRLEN + sizeof('\0') };
struct HashNode {
    char     key[MAX_HASH_KEY_SIZE];
    uint64_t value;  // opaque 64-bit value
};

struct HashBucket {
    size_t          capacity;  // how big is the bucket
    struct HashNode nodes[];   // any nodes I point to
};

struct HashMap {
    size_t capacity;  // how many buckets I have
    size_t size;      // how many total nodes I have

    struct HashBucket *buckets[];
};

#if defined(__cplusplus)
extern "C++" {
#endif

error_t NewHashMap(size_t buckets, struct HashMap **into);
error_t CloseHashMap(struct HashMap **mapptr);
error_t AddToHash(struct HashMap *map, const char *key, const char *end, const hash_value_t value);
error_t
LookupHashValue(const struct HashMap *map, const char *key, const char *end, hash_value_t *into);

#if defined(__cplusplus)
}
#endif

// Quick helpers
#define AddStrToHash(map, key, value) AddToHash(map, key, NULL, value)
#define LookupStrHashValue(map, key, intop) LookupHashValue(map, key, NULL, intop)

static inline bool
HashContainsStr(const struct HashMap *map, const char *key)
{
    return (0 == LookupStrHashValue(map, key, NULL));
}
static inline bool
HashContains(const struct HashMap *map, const char *key, const char *keyEnd)
{
    return (0 == LookupHashValue(map, key, keyEnd, NULL));
}
static inline size_t
GetMapSize(const struct HashMap *map)
{
    return map ? map->size : 0;
}

#endif
