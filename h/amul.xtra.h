#ifndef AMUL_H_AMUL_XTRA_H
#define AMUL_H_AMUL_XTRA_H 1

#include <ctype.h>

const char *extractLine(const char *from, char *to);
const char *getword(const char *from);
bool        striplead(const char *lead, char *from);

// Test for comment marker
static inline bool
isCommentChar(char c)
{
    return c == ';' || c == '*';
}

// Test for end-of-line character
static inline bool
isEol(char c)
{
    return c == '\n';
}

// Test for end-of-string (nul terminator)
static inline bool
isEos(char c)
{
    return c == '\0';
}

// Test for any character that ends tokens on a line
static inline bool
isLineEnding(char c)
{
    return isEos(c) || isEol(c) || isCommentChar(c);
}

// End of string or start of comment
static inline bool
isLineBreak(char c)
{
    return isEos(c) || isCommentChar(c);
}

static inline int
bitset(int bitNo)
{
    return 1 << bitNo;
}

static inline void
repspc(char *s)
{
    while (*s) {
        if (*s == '\t')
            *s = ' ';
        ++s;
    }
}

static inline const char *
skipspc(const char *s)
{
    while (*s == ' ' || *s == '\t') {
        ++s;
    }
    return s;
}

static inline const char *
skipline(const char *s)
{
    while (*s) {
        if (isEol(*(s++)))
            break;
    }
    return s;
}

static inline const char *
skiplead(const char *lead, const char *from)
{
    const char *cur = skipspc(from);
    while (*lead && *cur) {
        if (tolower(*lead) != tolower(*cur))
            return from;
        ++lead, ++cur;
    }
    // if lead == 0, we reached the end of the prefix, which
    // means 'from' matched as much as it needed to
    return (*lead == 0) ? cur : from;
}

static inline bool
canSkipLead(const char *lead, const char **from)
{
    const char *end = skiplead(lead, *from);
    if (end == *from) {
        return false;
    }
    *from = end;
    return true;
}

static inline const char *
strstop(const char *in, char stop)
{
    while (*in && *in != stop) {
        ++in;
    }
    return in;
}

static inline const char *
getWordAfter(const char *lead, const char *from)
{
    return getword(skiplead(lead, from));
}

#endif
