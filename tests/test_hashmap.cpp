#include <h/amul.type.h>
#include "gtest_aliases.h"
#include <gtest/gtest.h>
#include <stdint.h>

#include "hashmap.h"

// Test the get_string_hash function
TEST(HashmapTest, GetStringHash)
{
    // The hash for a should be the seed (5381) << 5 plus itself and the ascii for a
    hashval_t a_hash = (5381 << 5) + 5381 + 'a';
    EXPECT_EQ(a_hash, get_string_hash("a"));
    EXPECT_EQ(a_hash, get_string_hash("A"));
    hashval_t ax_hash = (a_hash << 5) + a_hash + 'x';
    EXPECT_EQ(ax_hash, get_string_hash("ax"));
    EXPECT_EQ(ax_hash, get_string_hash("AX"));
    EXPECT_EQ(ax_hash, get_string_hash("Ax"));

    // test a longer string, and test that it's idempotent
    EXPECT_EQ(3379623651, get_string_hash("$string_hash"));
    EXPECT_EQ(3379623651, get_string_hash("$STRING_HASH"));
    EXPECT_EQ(5381, get_string_hash(""));
    EXPECT_EQ(3379623651, get_string_hash("$string_hash"));
    EXPECT_EQ(a_hash, get_string_hash("a"));
    EXPECT_EQ(3379623651, get_string_hash("$string_hash"));
}

TEST(HashmapTest, GetStringHashAndLen)
{
    size_t length = 0;
    get_string_hash_and_len("a", NULL, &length);
    EXPECT_EQ(1, length);

    get_string_hash_and_len("1234567890123456789012345678901234567890", NULL, &length);
    EXPECT_EQ(40, length);
}

TEST(HashmapTest, GetStringHashView)
{
    hashval_t   hash = get_string_hash("hello");
    const char *key = "helloworld";
    EXPECT_FALSE(get_string_hash(key) == hash);
    size_t length = 0;
    EXPECT_TRUE(get_string_hash_and_len(key, key + 5, &length) == hash);
    EXPECT_EQ(5, length);
}

TEST(HashmapTest, NewHashMapChecks)
{
    struct HashMap *ptr = nullptr;
    // check for einval until we request 4 buckets
    EXPECT_EQ(EINVAL, NewHashMap(0, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQ(EINVAL, NewHashMap(1, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQ(EINVAL, NewHashMap(2, &ptr));
    EXPECT_NULL(ptr);
    EXPECT_EQ(EINVAL, NewHashMap(3, &ptr));
    EXPECT_NULL(ptr);

    // should still get einval at 4 if ptr is NULL
    EXPECT_EQ(EINVAL, NewHashMap(4, NULL));
    EXPECT_NULL(ptr);

    // and requesting a non^2 size should give EDOM
    EXPECT_EQ(EDOM, NewHashMap(15, &ptr));
}

TEST(HashmapTest, NewHashMap)
{
    struct HashMap *ptr = nullptr;
    // asking for 4 should get us a map of 4 buckets
    EXPECT_SUCCESS(NewHashMap(4, &ptr));
    EXPECT_NOT_NULL(ptr);
    EXPECT_EQ(4, ptr->capacity);
    EXPECT_EQ(0, ptr->size);
    EXPECT_EQ(NULL, ptr->buckets[0]);
    EXPECT_EQ(NULL, ptr->buckets[1]);
    EXPECT_EQ(NULL, ptr->buckets[2]);
    EXPECT_EQ(NULL, ptr->buckets[3]);

    EXPECT_SUCCESS(CloseHashMap(&ptr));
    EXPECT_NULL(ptr);
}

TEST(HashmapTest, AddStrToHashChecks)
{
    EXPECT_ERROR(EINVAL, AddStrToHash(NULL, NULL, 0));

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    EXPECT_ERROR(EINVAL, AddStrToHash(map, NULL, 0));
    EXPECT_ERROR(EINVAL, AddStrToHash(map, "1234567890123456789012345678901234567890", 0));

}

TEST(HashMapTest, AddStrToHashBasic)
{
    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    EXPECT_SUCCESS(AddStrToHash(map, "hello", 3));
    EXPECT_EQ(1, map->size);
}

TEST(HashmapTest, AddStrToHash)
{
    struct HashMap *map = NULL;
    NewHashMap(4, &map);

    AddStrToHash(map, "hello", 3);

    hashval_t helloHash = get_string_hash("hello");
    size_t    hashBucket = helloHash % map->capacity;
    // Based on the expected hash for hello it should go in
    // bucket #1
    EXPECT_EQ(1, hashBucket);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);
    EXPECT_EQ(1, map->buckets[1]->capacity);
    EXPECT_STREQ(map->buckets[1]->nodes[0].key, "hello");
    EXPECT_EQ(3, map->buckets[1]->nodes[0].value);

    EXPECT_EQ(0, AddStrToHash(map, "hello", 101));
    EXPECT_EQ(1, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);
    EXPECT_EQ(1, map->buckets[1]->capacity);
    EXPECT_STREQ(map->buckets[1]->nodes[0].key, "hello");
    EXPECT_EQ(101, map->buckets[1]->nodes[0].value);

    // Another string that should go into bucket 1 to
    // confirm growth, and it happens to be foo.
    hashval_t fooHash = get_string_hash("foo");
    size_t    fooBucket = fooHash % 4;
    EXPECT_EQ(1, fooBucket);
    EXPECT_SUCCESS(AddStrToHash(map, "foo", 102));
    EXPECT_EQ(2, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);

    EXPECT_EQ(4, map->buckets[1]->capacity);
    EXPECT_STREQ(map->buckets[1]->nodes[0].key, "hello");
    EXPECT_EQ(101, map->buckets[1]->nodes[0].value);
    EXPECT_STREQ(map->buckets[1]->nodes[1].key, "foo");
    EXPECT_EQ(102, map->buckets[1]->nodes[1].value);

    EXPECT_SUCCESS(AddStrToHash(map, "hello", 111));
    EXPECT_SUCCESS(AddStrToHash(map, "foo", 112));
    EXPECT_EQ(2, map->size);

    // Add a string that goes in another bucket
    hashval_t aHash = get_string_hash("a");
    size_t    aBucket = aHash % map->capacity;
    EXPECT_EQ(2, aBucket);

    EXPECT_SUCCESS(AddStrToHash(map, "a", 999));
    EXPECT_EQ(3, map->size);
    EXPECT_NULL(map->buckets[0]);
    EXPECT_NOT_NULL(map->buckets[1]);
    EXPECT_NOT_NULL(map->buckets[2]);
    EXPECT_NULL(map->buckets[3]);

    EXPECT_STREQ(map->buckets[2]->nodes[0].key, "a");
    EXPECT_EQ(999, map->buckets[2]->nodes[0].value);

    EXPECT_EQ(4, map->buckets[1]->capacity);
	EXPECT_STREQ(map->buckets[1]->nodes[0].key, "hello");
    EXPECT_EQ(111, map->buckets[1]->nodes[0].value);
	EXPECT_STREQ(map->buckets[1]->nodes[1].key, "foo");
    EXPECT_EQ(112, map->buckets[1]->nodes[1].value);

    EXPECT_SUCCESS(AddStrToHash(map, "hello", 0));
    EXPECT_SUCCESS(AddStrToHash(map, "foo", 0));
    EXPECT_SUCCESS(AddStrToHash(map, "a", 0));

    CloseHashMap(&map);
}

TEST(HashmapTest, AddToHash)
{
    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    const char *key = "1234567890123456789012345678901234567890";

    EXPECT_ERROR(EINVAL, AddToHash(map, key, NULL, 0));
    EXPECT_SUCCESS(AddToHash(map, key, key + 8, 0));

    CloseHashMap(&map);
}

TEST(HashmapTest, LookupStrHashValue)
{
    // Sanity check sanity checking.
    EXPECT_ERROR(EINVAL, LookupStrHashValue(NULL, NULL, NULL));

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(4, &map));
    EXPECT_NOT_NULL(map);

    EXPECT_ERROR(EINVAL, LookupStrHashValue(map, NULL, NULL));

    // For storing the returned value
    hash_value_t value = 0;

    // Confirm we can't look up strings pre-population
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "", NULL));
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "abc", NULL));
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "hello", NULL));
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "$rubber:duck!", NULL));

    // Add something
    EXPECT_SUCCESS(AddStrToHash(map, "hello", 101));

    // Check it should be the only thing that returns a value, and
    // we expect to see the correct value
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "", NULL));
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "abc", NULL));
    EXPECT_SUCCESS(LookupStrHashValue(map, "hello", NULL));
    EXPECT_SUCCESS(LookupStrHashValue(map, "hello", &value));
    EXPECT_EQ(101, value);
    EXPECT_ERROR(ENOENT, LookupStrHashValue(map, "$rubber:duck!", NULL));

    // Add some more keys
    EXPECT_SUCCESS(AddStrToHash(map, "fish", 103));
    EXPECT_SUCCESS(AddStrToHash(map, "abc", 102));

    // And things should still be good
    EXPECT_EQ(ENOENT, LookupStrHashValue(map, "", NULL));
    EXPECT_SUCCESS(LookupStrHashValue(map, "abc", &value));
    EXPECT_EQ(102, value);
    EXPECT_SUCCESS(LookupStrHashValue(map, "hello", &value));
    EXPECT_EQ(101, value);
    EXPECT_EQ(ENOENT, LookupStrHashValue(map, "$rubber:duck!", NULL));

    EXPECT_EQ(EINVAL, LookupStrHashValue(map, "1234567890123456789012345678901234567890", 0));
    EXPECT_FALSE(HashContainsStr(map, "1234567890123456789012345678901234567890"));

    CloseHashMap(&map);
}

TEST(HashmapTest, LargePopulation)
{
    // By "large", we expect at least one key to have 4+ entries.

    struct HashMap *map = NULL;
    EXPECT_SUCCESS(NewHashMap(16, &map));
    EXPECT_NOT_NULL(map);

    char key[MAX_HASH_KEY_SIZE];
    for (uint64_t i = 0; i < 256; ++i) {
        sprintf(key, "key%04" PRIu64, i);
        EXPECT_SUCCESS(AddStrToHash(map, key, i + 1LL));
    }
    EXPECT_EQ(256, GetMapSize(map));

    for (uint64_t i = 0; i < 256; ++i) {
        sprintf(key, "key%04" PRIu64, i);
        EXPECT_TRUE(HashContainsStr(map, key));
        sprintf(key, "notkey%04" PRIu64, i);
        EXPECT_FALSE(HashContainsStr(map, key));
        sprintf(key, "key%04" PRIu64, i + 256UL);
        EXPECT_FALSE(HashContainsStr(map, key));
    }

    CloseHashMap(&map);
}
