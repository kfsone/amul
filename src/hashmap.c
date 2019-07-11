// Simple C implementation of a chained hash map.

#include "hashmap.h"
#include "h/amul.test.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// Platform portable case-insensitive string comparison.
#if !defined(_MSC_VER)
#    define stricmp strcasecmp
#endif

// A type for storing the result of the hashing function
typedef uint32_t hashval_t;

// Calculates the "hash" of a string, based on djb2
hashval_t
get_string_hash(const char *string)
{
    hashval_t hashval = 5381;
    while (*string) {
        hashval = (hashval << 5) + hashval + (hashval_t)tolower(*(string++));
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
// GetMapSize returns the number of keys currently in the map in O(1).
//
size_t
GetMapSize(const struct HashMap *map)
{
    if (!map)
        return 0;
    return map->size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// AddToHash attempts to add a new key:value into the map. Does not attempt to overwrite existing
// values.
//
// Returns:
//	EINVAL if map or key is NULL or if key is an empty string
//  ENOMEM if an allocation fails
//  EEXISTS if the entry already exists
//  0 if the entry was added
//
error_t
AddToHash(struct HashMap *map, const char *key, const hash_value_t value)
{
    REQUIRE(map && key && *key);

    hashval_t          hashval = get_string_hash(key);
    size_t             bucketNo = hashval % map->capacity;
    struct HashBucket *bucket = map->buckets[bucketNo];

    struct HashNode *cursor = NULL;
    size_t           capacity = 0;
    if (bucket) {
        capacity = bucket->capacity;
        for (size_t i = 0; i < capacity; ++i) {
            if (stricmp(bucket->nodes[i].key, key) == 0)
                return EEXIST;
            // remember the first empty slot we see
            if (!cursor && bucket->nodes[i].key[0] == 0)
                cursor = &(bucket->nodes[i]);
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
//  EINVAL if map or key are NULL
//	ENOENT if the key is not in the map
//  0 on success and stores the value in *into if into is not NULL.
//
error_t
LookupHashValue(const struct HashMap *map, const char *key, hash_value_t *into)
{
    REQUIRE(map && key);

    hashval_t          hashval = get_string_hash(key);
    size_t             bucketNo = hashval % map->capacity;
    struct HashBucket *bucket = map->buckets[bucketNo];
    if (!bucket)
        return ENOENT;

    for (struct HashNode *cur = bucket->nodes; cur != bucket->nodes + bucket->capacity; ++cur) {
        if (stricmp(key, cur->key) == 0) {
            if (into) {
                *into = cur->value;
            }
            return 0;
        }
    }
    return ENOENT;
}

// Test the get_string_hash function
void
test_get_string_hash()
{
    // The hash for a should be the seed (5381) << 5 plus itself and the ascii for a
    hashval_t a_hash = (5381 << 5) + 5381 + 'a';
    EXPECT_EQUAL_VAL(a_hash, get_string_hash("a"));
    EXPECT_EQUAL_VAL(a_hash, get_string_hash("A"));
    hashval_t ax_hash = (a_hash << 5) + a_hash + 'x';
    EXPECT_EQUAL_VAL(ax_hash, get_string_hash("ax"));
    EXPECT_EQUAL_VAL(ax_hash, get_string_hash("AX"));
    EXPECT_EQUAL_VAL(ax_hash, get_string_hash("Ax"));

    // test a longer string, and test that it's idempotent
    EXPECT_EQUAL_VAL(3379623651, get_string_hash("$string_hash"));
    EXPECT_EQUAL_VAL(3379623651, get_string_hash("$STRING_HASH"));
    EXPECT_EQUAL_VAL(5381, get_string_hash(""));
    EXPECT_EQUAL_VAL(3379623651, get_string_hash("$string_hash"));
    EXPECT_EQUAL_VAL(a_hash, get_string_hash("a"));
    EXPECT_EQUAL_VAL(3379623651, get_string_hash("$string_hash"));
}

void
test_new_hash_map()
{
    struct HashMap *ptr = NULL;
    // check for einval until we request 4 buckets
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(0, &ptr));
    EXPECT_FALSE(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(1, &ptr));
    EXPECT_FALSE(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(2, &ptr));
    EXPECT_FALSE(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(3, &ptr));
    EXPECT_FALSE(ptr);

    // should still get einval at 4 if ptr is NULL
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(4, NULL));
    EXPECT_FALSE(ptr);

    // and requesting a non^2 size should give EDOM
    EXPECT_EQUAL_VAL(EDOM, NewHashMap(15, &ptr));

    // asking for 4 should get us a map of 4 buckets
    EXPECT_EQUAL_VAL(0, NewHashMap(4, &ptr));
    EXPECT_TRUE(ptr);
    EXPECT_EQUAL_VAL(4, ptr->capacity);
    EXPECT_EQUAL_VAL(0, ptr->size);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[0]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[1]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[2]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[3]);

    EXPECT_EQUAL_VAL(0, CloseHashMap(&ptr));
    EXPECT_FALSE(ptr);
}

void
test_add_to_hash()
{
    EXPECT_EQUAL_VAL(EINVAL, AddToHash(NULL, NULL, 0));

    struct HashMap *map = NULL;
    EXPECT_EQUAL_VAL(0, NewHashMap(4, &map));
    EXPECT_TRUE(map);

    EXPECT_EQUAL_VAL(EINVAL, AddToHash(map, NULL, 0));

    EXPECT_EQUAL_VAL(0, AddToHash(map, "hello", 101));
    EXPECT_EQUAL_VAL(1, map->size);

    hashval_t helloHash = get_string_hash("hello");
    size_t    hashBucket = helloHash % map->capacity;
    // Based on the expected hash for hello it should go in
    // bucket #1
    EXPECT_EQUAL_VAL(1, hashBucket);
    EXPECT_FALSE(map->buckets[0]);
    EXPECT_TRUE(map->buckets[1]);
    EXPECT_FALSE(map->buckets[2]);
    EXPECT_FALSE(map->buckets[3]);
    EXPECT_EQUAL_VAL(1, map->buckets[1]->capacity);
    EXPECT_TRUE(map->buckets[1]->nodes);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 101));
    EXPECT_EQUAL_VAL(1, map->size);
    EXPECT_FALSE(map->buckets[0]);
    EXPECT_TRUE(map->buckets[1]);
    EXPECT_FALSE(map->buckets[2]);
    EXPECT_FALSE(map->buckets[3]);
    EXPECT_EQUAL_VAL(1, map->buckets[1]->capacity);
    EXPECT_TRUE(map->buckets[1]->nodes);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);

    // Another string that should go into bucket 1 to
    // confirm growth, and it happens to be foo.
    hashval_t fooHash = get_string_hash("foo");
    size_t    fooBucket = fooHash % 4;
    EXPECT_EQUAL_VAL(1, fooBucket);
    EXPECT_EQUAL_VAL(0, AddToHash(map, "foo", 102));
    EXPECT_EQUAL_VAL(2, map->size);
    EXPECT_FALSE(map->buckets[0]);
    EXPECT_TRUE(map->buckets[1]);
    EXPECT_FALSE(map->buckets[2]);
    EXPECT_FALSE(map->buckets[3]);

    EXPECT_EQUAL_VAL(4, map->buckets[1]->capacity);
    EXPECT_TRUE(map->buckets[1]->nodes);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[1].key, "foo"));
    EXPECT_EQUAL_VAL(102, map->buckets[1]->nodes[1].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "foo", 0));
    EXPECT_EQUAL_VAL(2, map->size);

    // Add a string that goes in another bucket
    hashval_t aHash = get_string_hash("a");
    size_t    aBucket = aHash % map->capacity;
    EXPECT_EQUAL_VAL(2, aBucket);

    EXPECT_EQUAL_VAL(0, AddToHash(map, "a", 999));
    EXPECT_EQUAL_VAL(3, map->size);
    EXPECT_FALSE(map->buckets[0]);
    EXPECT_TRUE(map->buckets[1]);
    EXPECT_TRUE(map->buckets[2]);
    EXPECT_FALSE(map->buckets[3]);

    EXPECT_TRUE(map->buckets[2]->nodes);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[2]->nodes[0].key, "a"));
    EXPECT_EQUAL_VAL(999, map->buckets[2]->nodes[0].value);

    EXPECT_EQUAL_VAL(4, map->buckets[1]->capacity);
    EXPECT_TRUE(map->buckets[1]->nodes);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);
    EXPECT_EQUAL_VAL(0, strcmp(map->buckets[1]->nodes[1].key, "foo"));
    EXPECT_EQUAL_VAL(102, map->buckets[1]->nodes[1].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "foo", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "a", 0));

    CloseHashMap(&map);
}

void
test_lookup_hash_value()
{
    // Sanity check sanity checking.
    EXPECT_EQUAL_VAL(EINVAL, LookupHashValue(NULL, NULL, NULL));

    struct HashMap *map = NULL;
    EXPECT_EQUAL_VAL(0, NewHashMap(4, &map));
    EXPECT_TRUE(map);

    EXPECT_EQUAL_VAL(EINVAL, LookupHashValue(map, NULL, NULL));

    // For storing the returned value
    hash_value_t value = 0;

    // Confirm we can't look up strings pre-population
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "abc", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "hello", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    // Add something
    EXPECT_EQUAL_VAL(0, AddToHash(map, "hello", 101));

    // Check it should be the only thing that returns a value, and
    // we expect to see the correct value
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "abc", NULL));
    EXPECT_EQUAL_VAL(0, LookupHashValue(map, "hello", NULL));
    EXPECT_EQUAL_VAL(0, LookupHashValue(map, "hello", &value));
    EXPECT_EQUAL_VAL(101, value);
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    // Add some more keys
    EXPECT_EQUAL_VAL(0, AddToHash(map, "fish", 103));
    EXPECT_EQUAL_VAL(0, AddToHash(map, "abc", 102));

    // And things should still be good
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_EQUAL_VAL(0, LookupHashValue(map, "abc", &value));
    EXPECT_EQUAL_VAL(102, value);
    EXPECT_EQUAL_VAL(0, LookupHashValue(map, "hello", &value));
    EXPECT_EQUAL_VAL(101, value);
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    CloseHashMap(&map);
}

int
main()
{
    test_get_string_hash();

    test_new_hash_map();

    test_add_to_hash();

    test_lookup_hash_value();
}
