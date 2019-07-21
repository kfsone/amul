#ifndef AMUL_H_AMUL_TYPE_H
#define AMUL_H_AMUL_TYPE_H 1
// AMUL type definitions and includes.

#include <errno.h>     // for error numbers
#include <inttypes.h>  // for PRIu64 etc
#include <stdbool.h>
#include <stdint.h>  // for sized types
#include <stdlib.h>  // for size_t

#include <h/amul.errs.h>

// The string id is actually its offset in the strings file
typedef uint32_t stringid_t;

typedef int32_t roomid_t;

typedef int16_t adjid_t;

typedef int16_t verbid_t;

typedef int32_t opparam_t;  // parameter to an operation

#endif
