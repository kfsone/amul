#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>

const char *extractLine(const char *from, char *to) noexcept;
bool        stripNewline(char *text) noexcept;
const char *getword(const char *from) noexcept;

static constexpr int
bitset(int bitNo) noexcept
{
    return 1 << bitNo;
}

static inline void
repspc(char *s) noexcept
{
    while (*s) {
        if (*s == '\t')
            *s = ' ';
        ++s;
    }
}

static inline const char *
skipspc(const char *s) noexcept
{
    while (*s && isspace(*s)) {
        ++s;
    }
    return s;
}

static inline const char *
skipline(const char *s) noexcept
{
    while (*s) {
        if (isEol(*(s++)))
            break;
    }
    return s;
}

static inline const char *
skiplead(const char *lead, const char *from) noexcept
{
    const char *beginning = from;
    while (*lead && *from) {
        if (tolower(*from) != tolower(*lead))
            return beginning;
        ++lead, ++from;
    }
    return (*lead == 0) ? from : beginning;
}

static inline bool
skiplead(const char *lead, const char **from) noexcept
{
    const char *origin = *from;
    const char *start = skipspc(origin);
    const char *cur = start;
    cur = skiplead(lead, cur);
    if (cur == start) {
        return false;
    }
    *from = cur;
    return true;
}

bool striplead(const char *lead, char *from) noexcept;

template <size_t Size>
static void
nulTerminate(char (&text)[Size]) noexcept
{
    text[Size - 1] = 0;
}

template <typename CharT>
static CharT *
strstop(CharT *in, char stop) noexcept
{
    while (*in && *in != stop) {
        ++in;
    }
    return in;
}

static inline const char *
getWordAfter(const char *lead, const char *from) noexcept
{
    return getword(skiplead(lead, from));
}
