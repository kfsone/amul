// Simple C implementation of a chained hash map.

#include "hashmap.h"

// Calculates the "hash" of a string, based on djb2
hashval_t
get_string_hash_and_len(const char *string, const char *stringEnd, size_t *length)
{
    hashval_t   hashval = 5381;
    const char *p = string;
    while (*p && (stringEnd == NULL || p < stringEnd)) {
        hashval = (hashval << 5) + hashval + (hashval_t)tolower(*(p++));
    }
    if (length) {
        *length = p - string;
    }
    return hashval;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// NewHashMap constructs a new hash map instance with a given number of buckets.
//
// Use CloseHashMap() to release resources associated with the map when done.
//
// Returns:
//	EINVAL if buckets is < 4 or into is NULL
//  EDOM if buckets is not a power-of-2
//  ENOMEM if out of memory
//	0 on success and stores the address of the map in *into.
//
error_t
NewHashMap(size_t buckets, struct HashMap **into)
{
    // 'buckets' must be a power of 2. Since a power of 2 is always
    // a single bit, and the first highest value with that bit set,
    // anding against N-1 will yield 0; whereas for any non-power of
    // 2, anding against N-1 will yield, at least, the nearest power
    // of 2. e.g 6&5 == 4, 8&7 == 0
    REQUIRE(buckets >= 4);
    REQUIRE(into);
    CONSTRAIN((buckets & (buckets - 1)) == 0);

    struct HashMap *instance =
            calloc(sizeof(struct HashMap) + sizeof(struct HashBucket) * buckets, 1);
    CHECK_ALLOCATION(instance);

    instance->capacity = buckets;

    *into = instance;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// CloseHashMap frees all resources used by the map and invalidates the pointer to it.
//
// Returns EINVAL if `map` is NULL
//
error_t
CloseHashMap(struct HashMap **map)
{
    REQUIRE(map);
    if (!*map)
        return 0;

    for (size_t i = 0; i < (*map)->capacity; ++i) {
        struct HashBucket *bucket = (*map)->buckets[i];
        if (bucket) {
            bucket->capacity = 0;
            free(bucket);
        }
    }

    free(*map);
    *map = NULL;

    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AddToHash attempts to add a new key:value into the map. Does not attempt to overwrite existing
// values.
//
// Returns:
//	EINVAL if map or key is NULL or if key is an empty string or is too long
//  ENOMEM if an allocation fails
//  0 if the entry was added
//
error_t
AddToHash(struct HashMap *map, const char *key, const char *keyEnd, const hash_value_t value)
{
    REQUIRE(map && key && *key);
    REQUIRE(keyEnd == NULL || keyEnd > key);

    size_t             length = 0;
    hashval_t          hashval = get_string_hash_and_len(key, keyEnd, &length);
    size_t             bucketNo = hashval % map->capacity;
    struct HashBucket *bucket = map->buckets[bucketNo];
    if (length > MAX_HASH_KEY_STRLEN) {
        return EINVAL;
    }

    struct HashNode *cursor = NULL;
    size_t           capacity = 0;
    if (bucket) {
        capacity = bucket->capacity;
        for (size_t i = 0; i < capacity; ++i) {
            // remember the first empty slot we see
            if (!cursor && bucket->nodes[i].key[0] == 0) {
                cursor = &(bucket->nodes[i]);
                continue;
            }
            if (strnicmp(bucket->nodes[i].key, key, length) == 0) {
                if (bucket->nodes[i].key[length] == 0) {
                    bucket->nodes[i].value = value;
                    return 0;
                }
            }
        }
    }

    if (!cursor) {
        // the first time we add a node, capacity becomes 1, but when we get a collision,
        // grow in 4s.
        size_t newCapacity;
        switch (capacity) {
        case 0: newCapacity = 1; break;
        case 1: newCapacity = 4; break;
        default: newCapacity = capacity + 4; break;
        }

        size_t newSize = sizeof(struct HashBucket) + sizeof(struct HashNode) * newCapacity;
        struct HashBucket *newBucket = realloc(bucket, newSize);
        CHECK_ALLOCATION(newBucket);
        bucket = map->buckets[bucketNo] = newBucket;
        bucket->capacity = newCapacity;

        cursor = bucket->nodes + capacity;
        // clear all the new data we just gained
        const size_t newBytes = (newCapacity - capacity) * sizeof(struct HashNode);
        memset(cursor, 0, newBytes);
    }

    strncpy(cursor->key, key, sizeof(cursor->key));
    cursor->value = value;

    map->size++;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// LookupHashValue looks for a string in the map and optionally provides the value associated with
// it, if found.
//
/// Returns
//  EINVAL if map or key are NULL, or the key is too long
//	ENOENT if the key is not in the map
//  0 on success and stores the value in *into if into is not NULL.
//
error_t
LookupHashValue(const struct HashMap *map, const char *key, const char *keyEnd, hash_value_t *into)
{
    REQUIRE(map && key);

    size_t    length = 0;
    hashval_t hashval = get_string_hash_and_len(key, keyEnd, &length);
    if (length > MAX_HASH_KEY_STRLEN) {
        return EINVAL;
    }

    size_t             bucketNo = hashval % map->capacity;
    struct HashBucket *bucket = map->buckets[bucketNo];
    if (!bucket)
        return ENOENT;

    for (struct HashNode *cur = bucket->nodes; cur != bucket->nodes + bucket->capacity; ++cur) {
        if (strnicmp(key, cur->key, length) == 0) {
            if (cur->key[length] != 0)
                continue;
            if (into) {
                *into = cur->value;
            }
            return 0;
        }
    }
    return ENOENT;
}
