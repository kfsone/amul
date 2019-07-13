#ifndef AMUL_H_AMUL_TYPE_H
#define AMUL_H_AMUL_TYPE_H 1

// AMUL type definitions.

#include <errno.h>   // for error numbers
#include <stdint.h>  // for sized types
#include <stdlib.h>  // for size_t

// For returning error values.
typedef int error_t;

// The string id is actually its offset in the strings file
typedef uint32_t stringid_t;

#endif
