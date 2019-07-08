#ifndef H_AMUL_XTRA_H
#define H_AMUL_XTRA_H 1

const char *extractLine(const char *from, char *to);
const char *getword(const char *from);
bool 	   striplead(const char *lead, char *from);

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
    while (*s && isspace(*s)) {
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
    const char *beginning = from;
    while (*lead && *from) {
        if (tolower(*from) != tolower(*lead))
            return beginning;
        ++lead, ++from;
    }
    return (*lead == 0) ? from : beginning;
}

static inline bool
canSkipLead(const char *lead, const char **from)
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
