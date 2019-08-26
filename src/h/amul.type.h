#ifndef AMUL_TYPE_H
#define AMUL_TYPE_H 1
// AMUL type definitions and includes.

#include <cerrno>     // for error numbers
#include <cinttypes>  // for PRIu64 etc
#include <cstdint>    // for sized types
#include <cstdlib>    // for size_t

#include "amulconfig.h"

using amulid_t = int32_t;

using stringid_t = amulid_t;
using roomid_t = amulid_t;
using adjid_t = amulid_t;
using verbid_t = amulid_t;
using objid_t = amulid_t;
using vmopid_t = amulid_t;
using oparg_t = amulid_t;

using demonid_t = int64_t;
using slot_t = int32_t;  // identifies one of the login 'slot's

#ifndef HAVE_ERROR_T
using error_t = int;
#endif

#ifndef HAVE_SSIZE_T
using ssize_t = int64_t;
#endif

#endif
