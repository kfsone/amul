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
using stringid_t = uint32_t;

using roomid_t = int32_t;

using adjid_t = int32_t;

using verbid_t = int32_t;

using opparam_t = int32_t;

#endif
