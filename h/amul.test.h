#ifndef AMUL_H_AMUL_TEST_H
#define AMUL_H_AMUL_TEST_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// "kfstest1": macros for contract checking and unit testing.
// Copyright (C) Oliver 'kfsone' Smith 2019
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>

#include <h/amul.errs.h>

#ifndef NDEBUG
#    include <assert.h>
#    include <stdbool.h>
#    include <stdint.h>
#    include <stdio.h>
#    include <string.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Function contracts -- errno style return values.

// Tests a predicate to determine if input parameters are valid, else returns EINVAL
#define REQUIRE(predicate)                                                                         \
    do {                                                                                           \
        if (!(predicate)) {                                                                        \
            return EINVAL;                                                                         \
        }                                                                                          \
    } while (0)

// If X returns an error, return that.
#define ERROR_CHECK(predicate)                                                                     \
    do {                                                                                           \
        const error_t _err##__LINENO__ = (predicate);                                              \
        if (_err##__LINENO__ != 0)                                                                 \
            return _err##__LINENO__;                                                               \
    } while (0)

// Tests a predicate to determine if parameter constraints are met, else returns EDOM
#define CONSTRAIN(predicate)                                                                       \
    do {                                                                                           \
        if (!(predicate))                                                                          \
            return EDOM;                                                                           \
    } while (0)

// Tests a predicate to confirm that an allocation was successful, else returns ENOMEM
#define CHECK_ALLOCATION(predicate)                                                                \
    do {                                                                                           \
        if (!(predicate))                                                                          \
            return ENOMEM;                                                                         \
    } while (0)

///////////////////////////////////////////////////////////////////////////////////////////////////
// Unit testing macros
//
// These only have any effect with NDEBUG is not defined, the same convention that assert.h uses.

// TestContext is passed to every test suite to provide them with the testing environment, such
// as command line, number of tests, etc, as well as a means to share data between tests
// (userData) and to provide tearUp/tearDown functions for tests.

struct TestContext {
    int          argc;
    const char **argv;
    const char * step;
    bool         verbose;
    size_t       tests;
    size_t       passes;
    size_t       lineItems;
    void *       userData;
    void (*tearUp)(struct TestContext *);
    void (*tearDown)(struct TestContext *);
};

typedef void(testsuite_fn)(struct TestContext *t);

#ifndef NDEBUG

#    define LPRINTF(fmt, ...)                                                                      \
        fprintf(stderr, "\n%s:%d: error:%s: " fmt "\n", __FILE__, __LINE__, t->step,               \
                ##__VA_ARGS__);                                                                    \
        fflush(stderr)

#    define RUN_TEST(fn)                                                                           \
        do {                                                                                       \
            t->tests++;                                                                            \
            t->step = #fn;                                                                         \
            if (t->tearUp)                                                                         \
                t->tearUp(t);                                                                      \
            fn(t);                                                                                 \
            if (t->tearDown)                                                                       \
                t->tearDown(t);                                                                    \
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

#    define EXPECT_ZERO(actual)                                                                    \
        if ((uintptr_t)(0) == (uintptr_t)(actual)) {                                               \
            t->lineItems++;                                                                        \
        } else {                                                                                   \
            LPRINTF(#actual " expecting ZERO; got #%" PRid64 "", (int64_t)(actual));               \
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
            assert((void *)(expected) == (void *)(actual));                                        \
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
            assert(!(actual));                                                                     \
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
#    define EXPECT_ZERO(...)
#    define EXPECT_VAL_EQUAL(...)
#    define EXPECT_PTR_EQUAL(...)
#    define EXPECT_NOT_NULL(...)
#    define EXPECT_NULL(...)
#    define EXPECT_TRUE(...)
#    define EXPECT_FALSE(...)
#    define EXPECT_STR_EQUAL(...)

#endif

#endif  // AMUL_H_AMUL_TEST_H
