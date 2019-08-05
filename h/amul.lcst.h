#pragma once

// Functions for lower-casing strings

#include <algorithm>
#include <cctype>
#include <string>

extern char Word[64];

template<typename ItType>
void
AssignLower(ItType begin, ItType end, std::string::iterator into)
{
    std::transform(begin, end, into, [](unsigned int c) {
        return std::tolower(c);
    });
}

static inline void
AssignWord(std::string& into)
{
    AssignLower(std::cbegin(Word), std::cend(Word), into.begin());
}
