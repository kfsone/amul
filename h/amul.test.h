#ifndef AMUL_H_AMUL_TEST_H
#define AMUL_H_AMUL_TEST_H 1

/*
 * Macros for rudimentary unit testing and conformance testing.
 */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#if defined(DEBUG)
extern size_t numTests;
#endif

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

#if defined(DEBUG)
#define EXPECT_EQUAL_VAL(expected, actual)                                                         \
    if ((expected) == (actual)) {                                                                  \
        ++numTests;                                                                                \
    } else {                                                                                       \
        printf("%s:%d: error: " #actual " expecting %lu but got %lu\n", __FILE__, __LINE__,        \
               (uint64_t)expected, (uint64_t)actual);                                              \
        fflush(stdout);                                                                            \
        assert(expected == actual);                                                                \
    }

#define EXPECT_EQUAL_PTR(expected, actual)                                                         \
    if ((void *)(expected) == (void *)(actual)) {                                                  \
        ++numTests;                                                                                \
    } else {                                                                                       \
        printf("%s:%d: error: expecting %p but got %p\n", __FILE__, __LINE__, (void *)expected,    \
               (void *)actual);                                                                    \
        fflush(stdout);                                                                            \
        assert(expected == actual);                                                                \
    }

#define EXPECT_TRUE(value)                                                                         \
    if (value) {                                                                                  \
        ++numTests;                                                                                \
    } else {                                                                                       \
        printf("%s:%d: error: expected true/non-null, got false/null", __FILE__, __LINE__);        \
        fflush(stdout);                                                                            \
        assert(value);                                                                             \
    }

#define EXPECT_FALSE(value)                                                                        \
    if (!value) {                                                                                   \
        ++numTests;                                                                                \
    } else {                                                                                       \
        printf("%s:%d: error: expected false/null, got true/%16lx", __FILE__, __LINE__,            \
               (uint64_t)(value));                                                                 \
        fflush(stdout);                                                                            \
        assert(!value);                                                                            \
    }
#else
#define EXPECT_EQUAL_VAL(...)
#define EXPECT_EQUAL_PTR(...)
#define EXPECT_TRUE(...)
#define EXPECT_FALSE(...)
#endif

#endif
