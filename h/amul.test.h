#ifndef AMUL_H_AMUL_TEST_H
#define AMUL_H_AMUL_TEST_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// "kfstest1": macros for contract checking and unit testing.
// Copyright (C) Oliver 'kfsone' Smith 2019
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <h/amul.errs.h>

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


#endif  // AMUL_H_AMUL_TEST_H
