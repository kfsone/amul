#pragma once
#ifndef AMUL_ENUMERATE_H
#define AMUL_ENUMERATE_H

#include "amul.typedefs.h"

#include <algorithm>

// Find the index of an element in a sequence container that matches a predicate.
static constexpr auto Enumerate = [](const auto &sequence, auto predicate) {
	using return_t = decltype(std::distance(std::cbegin(sequence), std::cend(sequence)));
    auto cbegin = std::cbegin(sequence), cend = std::cend(sequence);
    if (auto it = std::find_if(cbegin, cend, predicate); it != cend)
        return optional<return_t>(std::distance(cbegin, it));
    return optional<return_t>{};
};

#endif  // AMUL_ENUMERATE_H
