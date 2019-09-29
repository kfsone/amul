#pragma once
#ifndef AMUL_TYPE_H
#define AMUL_TYPE_H 1
// AMUL type definitions and includes.

#include <cerrno>     // for error numbers
#include <cinttypes>  // for PRIu64 etc
#include <cstdint>    // for sized types
#include <cstdlib>    // for size_t
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#if defined(HAVE_CONFIG_H)
#include "amulconfig.h"
#endif

using std::forward;
using std::make_pair;
using std::make_shared;
using std::make_unique;
using std::move;
using std::optional;
using std::pair;
using std::shared_ptr;
using std::string_view;
using std::unique_ptr;

using namespace std::literals::string_view_literals;

using amulid_t = intptr_t;  // so it can double as a pointer

using adjid_t = amulid_t;
using flag_t = uint32_t;
using lflag_t = uint64_t;
using objid_t = amulid_t;
using oparg_t = amulid_t;
using prepid_t = int16_t;
using rankid_t = int32_t;
using roomid_t = amulid_t;
using stringid_t = amulid_t;
using verbid_t = amulid_t;
using vmopid_t = int32_t;

using demonid_t = int64_t;
using slotid_t = int32_t;     // identifies one of the login 'slot's
using slotmask_t = uint64_t;  // Identifies players by their slot, by bit

using stat_t = int32_t;  // Character "stat" attributes

#ifndef HAVE_ERROR_T
using error_t = int;
#endif

#ifndef HAVE_SSIZE_T
using ssize_t = int64_t;
#endif

#endif
