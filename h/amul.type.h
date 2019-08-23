#ifndef AMUL_H_AMUL_TYPE_H
#define AMUL_H_AMUL_TYPE_H 1
// AMUL type definitions and includes.

#include "amulconfig.h"

#include <cerrno>     // for error numbers
#include <cinttypes>  // for PRIu64 etc
#include <cstdbool>
#include <cstdint>  // for sized types
#include <cstdlib>  // for size_t

using amulid_t = int32_t;

using stringid_t = amulid_t;
using roomid_t = amulid_t;
using adjid_t = amulid_t;
using verbid_t = amulid_t;
using objid_t = amulid_t;
using vmopid_t = amulid_t;
using oparg_t = amulid_t;

using error_t = int;

#endif
