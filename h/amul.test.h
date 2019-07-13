#ifndef AMUL_H_AMUL_TEST_H
#define AMUL_H_AMUL_TEST_H 1

/*
 * Macros for rudimentary unit testing and conformance testing.
 */

#include <errno.h>
#include <inttypes.h>
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

#ifndef NDEBUG
#    include <assert.h>
#    include <stdbool.h>
#    include <stdint.h>
#    include <stdio.h>
#    include <string.h>

struct TestContext {
    int          argc;
    const char **argv;
    const char * step;
    bool         verbose;
    size_t       tests;
    size_t       passes;
    size_t       lineItems;
};

typedef void(test_harness_fn)(struct TestContext *t);

#    define LPRINTF(fmt, ...)                                                                      \
        fprintf(stderr, "\n%s:%d: error:%s: " fmt "\n", __FILE__, __LINE__, t->step,               \
                ##__VA_ARGS__); \
        fflush(stderr)

#    define RUN_TEST(fn)                                                                           \
        do {                                                                                       \
            t->tests++;                                                                            \
            t->step = #fn;                                                                         \
            fn(t);                                                                                 \
            t->passes++;                                                                           \
        } while (false)

#    define EXPECT_ERROR(expected, actual)                                                         \
        if ((expected) == (actual)) {                                                              \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting error#%d; got %d", (expected), (actual));                  \
            assert((expected) == (actual));                                                        \
        }

#    define EXPECT_SUCCESS(actual)                                                                 \
        if ((0) == (actual)) {                                                                     \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting SUCCESS; got error#%d", (actual));                         \
            assert(0 == (actual));                                                                 \
        }

#    define EXPECT_VAL_EQUAL(expected, actual)                                                     \
        if ((expected) == (actual)) {                                                              \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting %" PRId64 "; got %" PRId64, (int64_t)(expected),           \
                    (int64_t)(actual));                                                            \
            assert((expected) == (actual));                                                        \
        }

#    define EXPECT_PTR_EQUAL(expected, actual)                                                     \
        if ((void *)(expected) == (void *)(actual)) {                                              \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting %p; got %p", (void *)(expected), (void *)(actual));        \
            assert((expected) == (actual));                                                        \
        }

#    define EXPECT_NOT_NULL(actual)                                                                \
        if ((actual)) {                                                                            \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting !NULL; got NULL");                                         \
            assert((actual));                                                                      \
        }

#    define EXPECT_NULL(actual)                                                                    \
        if (!(actual)) {                                                                           \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting NULL; got %p", (void *)(actual));                          \
            assert((actual));                                                                      \
        }

#    define EXPECT_TRUE(actual)                                                                    \
        if ((actual)) {                                                                            \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting true; got false");                                         \
            assert((actual));                                                                      \
        }

#    define EXPECT_FALSE(actual)                                                                   \
        if (!(actual)) {                                                                           \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting false; got %" PRId64, (int64_t)(actual));                  \
            assert(!(actual));                                                                      \
        }

#    define EXPECT_STR_EQUAL(expected, actual)                                                     \
        if (strcmp(expected, actual) == 0) {                                                       \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting '%s'; got '%s'", (expected), (actual));                    \
            assert(strcmp(expected, actual) == 0);                                                 \
        }

#else

#    define EXPECT_ERROR(...)
#    define EXPECT_SUCCESS(...)
#    define EXPECT_VAL_EQUAL(...)
#    define EXPECT_PTR_EQUAL(...)
#    define EXPECT_TRUE(...)
#    define EXPECT_FALSE(...)
#    define EXPECT_NOT_NULL(...)
#    define EXPECT_NULL(...)
#    define EXPECT_STR_EQUAL(...)
#endif

#endif
