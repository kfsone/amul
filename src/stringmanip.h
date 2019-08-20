#pragma once
#ifndef AMUL_SRC_STRINGMANIP_H
#define AMUL_SRC_STRINGMANIP_H

#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

using std::string_view;

void
ReplaceAll(std::string &text, string_view pattern, string_view replacement) noexcept;

inline std::string &
StringLower(std::string &str) noexcept
{
    std::transform(
            str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

#endif  // AMUL_SRC_STRINGMANIP_H