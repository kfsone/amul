#include "h/amul.test.h"
#include "src/hashmap.h"

// Test the get_string_hash function
void
test_get_string_hash(struct TestContext *t)
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

    size_t length = 0;
    get_string_hash_and_len("a", &length);
    EXPECT_EQUAL_VAL(1, length);

    get_string_hash_and_len("1234567890123456789012345678901234567890", &length);
    EXPECT_EQUAL_VAL(40, length);
}

void
test_new_hash_map(struct TestContext *t)
{
    struct HashMap *ptr = NULL;
    // check for einval until we request 4 buckets
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(0, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(1, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(2, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(3, &ptr));
    EXPECT_NULL(ptr);

    // should still get einval at 4 if ptr is NULL
    EXPECT_EQUAL_VAL(EINVAL, NewHashMap(4, NULL));
    EXPECT_NULL(ptr);

    // and requesting a non^2 size should give EDOM
    EXPECT_EQUAL_VAL(EDOM, NewHashMap(15, &ptr));

    // asking for 4 should get us a map of 4 buckets
    EXPECT_SUCCESS(NewHashMap(4, &ptr));
    EXPECT_NOT_NULL(ptr);
    EXPECT_EQUAL_VAL(4, ptr->capacity);
    EXPECT_EQUAL_VAL(0, ptr->size);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[0]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[1]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[2]);
    EXPECT_EQUAL_PTR(NULL, ptr->buckets[3]);

    EXPECT_SUCCESS(CloseHashMap(&ptr));
    EXPECT_NULL(ptr);
}

void
test_add_to_hash(struct TestContext *t)
{
    EXPECT_EQUAL_VAL(EINVAL, AddToHash(NULL, NULL, 0));

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    EXPECT_EQUAL_VAL(EINVAL, AddToHash(map, NULL, 0));
    EXPECT_EQUAL_VAL(EINVAL, AddToHash(map, "1234567890123456789012345678901234567890", 0));

    EXPECT_SUCCESS(AddToHash(map, "hello", 101));
    EXPECT_EQUAL_VAL(1, map->size);

    hashval_t helloHash = get_string_hash("hello");
    size_t    hashBucket = helloHash % map->capacity;
    // Based on the expected hash for hello it should go in
    // bucket #1
    EXPECT_EQUAL_VAL(1, hashBucket);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);
    EXPECT_EQUAL_VAL(1, map->buckets[1]->capacity);
    EXPECT_NOT_NULL(map->buckets[1]->nodes);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 101));
    EXPECT_EQUAL_VAL(1, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);
    EXPECT_EQUAL_VAL(1, map->buckets[1]->capacity);
    EXPECT_NOT_NULL(map->buckets[1]->nodes);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);

    // Another string that should go into bucket 1 to
    // confirm growth, and it happens to be foo.
    hashval_t fooHash = get_string_hash("foo");
    size_t    fooBucket = fooHash % 4;
    EXPECT_EQUAL_VAL(1, fooBucket);
    EXPECT_SUCCESS(AddToHash(map, "foo", 102));
    EXPECT_EQUAL_VAL(2, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);

    EXPECT_EQUAL_VAL(4, map->buckets[1]->capacity);
    EXPECT_NOT_NULL(map->buckets[1]->nodes);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[1].key, "foo"));
    EXPECT_EQUAL_VAL(102, map->buckets[1]->nodes[1].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "foo", 0));
    EXPECT_EQUAL_VAL(2, map->size);

    // Add a string that goes in another bucket
    hashval_t aHash = get_string_hash("a");
    size_t    aBucket = aHash % map->capacity;
    EXPECT_EQUAL_VAL(2, aBucket);

    EXPECT_SUCCESS(AddToHash(map, "a", 999));
    EXPECT_EQUAL_VAL(3, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NOT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);

    EXPECT_NOT_NULL(map->buckets[2]->nodes);
    EXPECT_SUCCESS(strcmp(map->buckets[2]->nodes[0].key, "a"));
    EXPECT_EQUAL_VAL(999, map->buckets[2]->nodes[0].value);

    EXPECT_EQUAL_VAL(4, map->buckets[1]->capacity);
    EXPECT_NOT_NULL(map->buckets[1]->nodes);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[0].key, "hello"));
    EXPECT_EQUAL_VAL(101, map->buckets[1]->nodes[0].value);
    EXPECT_SUCCESS(strcmp(map->buckets[1]->nodes[1].key, "foo"));
    EXPECT_EQUAL_VAL(102, map->buckets[1]->nodes[1].value);

    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "hello", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "foo", 0));
    EXPECT_EQUAL_VAL(EEXIST, AddToHash(map, "a", 0));

    CloseHashMap(&map);
}

void
test_lookup_hash_value(struct TestContext *t)
{
    // Sanity check sanity checking.
    EXPECT_EQUAL_VAL(EINVAL, LookupHashValue(NULL, NULL, NULL));

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    EXPECT_EQUAL_VAL(EINVAL, LookupHashValue(map, NULL, NULL));

    // For storing the returned value
    hash_value_t value = 0;

    // Confirm we can't look up strings pre-population
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "abc", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "hello", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    // Add something
    EXPECT_SUCCESS(AddToHash(map, "hello", 101));

    // Check it should be the only thing that returns a value, and
    // we expect to see the correct value
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "abc", NULL));
    EXPECT_SUCCESS(LookupHashValue(map, "hello", NULL));
    EXPECT_SUCCESS(LookupHashValue(map, "hello", &value));
    EXPECT_EQUAL_VAL(101, value);
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    // Add some more keys
    EXPECT_SUCCESS(AddToHash(map, "fish", 103));
    EXPECT_SUCCESS(AddToHash(map, "abc", 102));

    // And things should still be good
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "", NULL));
    EXPECT_SUCCESS(LookupHashValue(map, "abc", &value));
    EXPECT_EQUAL_VAL(102, value);
    EXPECT_SUCCESS(LookupHashValue(map, "hello", &value));
    EXPECT_EQUAL_VAL(101, value);
    EXPECT_EQUAL_VAL(ENOENT, LookupHashValue(map, "$rubber:duck!", NULL));

    EXPECT_EQUAL_VAL(EINVAL, LookupHashValue(map, "1234567890123456789012345678901234567890", 0));
    EXPECT_FALSE(HashContains(map, "1234567890123456789012345678901234567890"));

    CloseHashMap(&map);
}

void
test_hash_large_population(struct TestContext *t)
{
    // By "large", we expect at least one key to have 4+ entries.

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(16, &map));
    EXPECT_NOT_NULL(map);

    char key[MAX_HASH_KEY_SIZE];
    for (int i = 0; i < 256; ++i) {
        sprintf(key, "key%04d", i);
        EXPECT_SUCCESS(AddToHash(map, key, i + 1));
    }
    EXPECT_EQUAL_VAL(256, GetMapSize(map));

    for (int i = 0; i < 256; ++i) {
        sprintf(key, "key%04d", i);
        EXPECT_TRUE(HashContains(map, key));
    }
    for (int i = 0; i < 256; ++i) {
        sprintf(key, "%04d", i);
        EXPECT_FALSE(HashContains(map, key));
        sprintf(key, "notkey%04d", i);
        EXPECT_FALSE(HashContains(map, key));
        sprintf(key, "key%04d", i + 256);
        EXPECT_FALSE(HashContains(map, key));
    }

    CloseHashMap(&map);
}

void
hashmap_tests(struct TestContext *t)
{
    RUN_TEST(test_get_string_hash);
    RUN_TEST(test_new_hash_map);
    RUN_TEST(test_add_to_hash);
    RUN_TEST(test_lookup_hash_value);
}
