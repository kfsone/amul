#ifndef AMUL_H_AMUL_TYPE_H
#define AMUL_H_AMUL_TYPE_H 1
// AMUL type definitions and includes.

#include <cerrno>     // for error numbers
#include <cinttypes>  // for PRIu64 etc
#include <cstdbool>
#include <cstdint>  // for sized types
#include <cstdlib>  // for size_t

// The string id is actually its offset in the strings file
using stringid_t = uint32_t;
using roomid_t = int32_t;
using adjid_t = int32_t;
using verbid_t = int32_t;
using error_t = int;
using flag_t = uint32_t;
using exflag_t = uint64_t;

// Virtual machine instructions
using opparam_t = int32_t;

// Instructions can only be positive.
using vmins_t = uint32_t;

#endif
