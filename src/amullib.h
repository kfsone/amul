#pragma once
#ifndef AMUL_AMULLIB_H
#define AMUL_AMULLIB_H

///////////////////////////////////////////////////////////////////////////////////////////////////
// System includes.
//
#include <new>
#include <string>
#include <string_view>
#include <utility>

///////////////////////////////////////////////////////////////////////////////////////////////////
// AMUL includes
//
#include "typedefs.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// "amullib" library prototypes
//
int32_t RandomInt(int32_t lower, int32_t upper) noexcept;

// Case-insensitive match of two strings, allowing ' ' == '_' and allows
// lhs to end if rhs has a space (' ') at the same position.
// Returns true on success.
bool Match(string_view lhs, string_view rhs) noexcept;

// Test if something is a synonym and get what it's an alias for
bool IsSynonym(const char *s) noexcept;

pair<size_t /*length*/, verbid_t /*aliasTo*/> IsVerbSynonym(const char *s) noexcept;

pair<size_t /*length*/, objid_t /*aliasTo*/> IsNounSynonym(const char *s) noexcept;

verbid_t IsVerb(string_view) noexcept;
prepid_t IsPreposition(const char *s) noexcept;

char *FindEndQuote(char *src) noexcept;

#endif  // AMUL_AMULLIB_H
