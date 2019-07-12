#ifndef AMUL_H_AMUL_TEST_H
#define AMUL_H_AMUL_TEST_H 1

/*
 * Macros for rudimentary unit testing and conformance testing.
 */

#include <errno.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function contracts -- errno style return values.

// Tests a predicate to determine if input parameters are valid, else returns EINVAL
#define REQUIRE(predicate)                                                                         \
    if (!(predicate))                                                                              \
		return EINVAL

// Tests a predicate to determine if parameter constraints are met, else returns EDOM
#define CONSTRAIN(predicate)                                                                       \
    if (!(predicate))                                                                              \
        return EDOM

// Tests a predicate to confirm that an allocation was successful, else returns ENOMEM
#define CHECK_ALLOCATION(predicate)                                                                \
    if (!(predicate))                                                                              \
    return ENOMEM

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unit testing macros

#if defined(_DEBUG)
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

struct TestContext {
    int argc;
    const char **argv;
    const char *step;
    bool verbose;
    size_t tests;
    size_t passes;
    size_t lineItems;
};

typedef void (test_harness_fn) (struct TestContext *t);

#define RUN_TEST(fn) do { t->tests++; t->step = #fn; fn(t); t->passes++; } while (false)

#define EXPECT_ERROR(expected, actual)                                                         \
    if ((expected) == (actual)) {                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #actual " expecting error#%d but got %d\n", __FILE__, __LINE__, t->step, expected, actual); \
        fflush(stdout); \
        assert(expected == actual);                                                                \
    }

#define EXPECT_SUCCESS(actual)                                                         \
    if ((0) == (actual)) {                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #actual " expecting success but got error#%d\n", __FILE__, __LINE__, t->step, actual); \
        fflush(stdout); \
        assert(0 == actual);                                                                \
    }

#define EXPECT_EQUAL_VAL(expected, actual)                                                         \
    if ((expected) == (actual)) {                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #actual " expecting %llu but got %llu\n", __FILE__, __LINE__, t->step, (uint64_t)expected, (uint64_t)actual); \
        fflush(stdout); \
        assert(expected == actual);                                                                \
    }

#define EXPECT_EQUAL_PTR(expected, actual)                                                         \
    if ((void *)(expected) == (void *)(actual)) {                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #actual " expecting %p but got %p\n", __FILE__, __LINE__, t->step, (void*)expected, (void*)actual); \
        fflush(stdout); \
        assert(expected == actual);                                                                \
    }

#define EXPECT_NOT_NULL(value)                                                                         \
    if (value) {                                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #value " expected not-null.\n", __FILE__, __LINE__, t->step); \
        fflush(stdout); \
        assert(value);                                                                             \
    }

#define EXPECT_NULL(value)                                                                         \
    if (!(value)) {                                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #value " expected null, got %p\n", __FILE__, __LINE__, t->step, (void*)value); \
        fflush(stdout); \
        assert(value);                                                                             \
    }

#define EXPECT_TRUE(value)                                                                         \
    if (value) {                                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #value " expected true, got false\n", __FILE__, __LINE__, t->step); \
        fflush(stdout); \
        assert(value);                                                                             \
    }

#define EXPECT_FALSE(value)                                                                         \
    if (!(value)) {                                                                                  \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #value " expected false, got %llu\n", __FILE__, __LINE__, t->step, (uint64_t)value); \
        fflush(stdout); \
        assert(value);                                                                             \
    }

#define EXPECT_EQUAL_STR(expected, actual) \
    if (strcmp(expected, actual) == 0) { \
        t->lineItems++;                                                                                \
    } else {                                                                                       \
        printf("\n%s:%d: error:%s: " #actual " expected '%s', got '%s'\n", __FILE__, __LINE__, t->step, expected, actual); \
        fflush(stdout); \
        assert(strcmp(expected, actual) == 0);                                                                             \
    }

#else

#define EXPECT_ERROR(...)
#define EXPECT_EQUAL_VAL(...)
#define EXPECT_EQUAL_PTR(...)
#define EXPECT_TRUE(...)
#define EXPECT_FALSE(...)
#endif

#endif
