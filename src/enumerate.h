#pragma once
#ifndef AMUL_ENUMERATE_H
#define AMUL_ENUMERATE_H

#include "typedefs.h"  // for size_t

#include <algorithm>

// Find the index of an element in a sequence container that matches a predicate.
static constexpr auto Enumerate = [](const auto &sequence, auto predicate) {
    auto cbegin = std::cbegin(sequence), cend = std::cend(sequence);
    if (auto it = std::find_if(cbegin, cend, predicate); it != cend)
        return optional<size_t>(std::distance(cbegin, it));
    return optional<size_t>{};
};

#endif  // AMUL_ENUMERATE_H
