#pragma once

#include <algorithm>
#include <cctype>
#include <cstdio>

static constexpr int
bitset(int bitNo)
{
	return 1 << bitNo;
}

static inline void
tx(const char *s)
{
	printf("%s", s);
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

static const char *
skipspc(const char *s)
{
	while (isspace(*s)) {
		++s;
	}
	return s;
}

static const char *
skipline(const char *s)
{
	while (*s && *s != '\n')
		++s;
	return s;
}

static const char *
skiplead(const char *lead, const char *from)
{
	const char *beginning = from;
	while (*from) {
		if (tolower(*from) != tolower(*lead))
			return beginning;
		++lead, ++from;
	}
	return from;
}

static bool
striplead(const char *lead, char *from)
{
	const char *following = skiplead(lead, from);
	if (following == from) {
		return false;
	}
	while (*following) {
		*(from++) = *(following++);
	}
	return true;
}

const char *
getword(const char *from)
{
	char *to = Word;
	*to = 0;
	from = skipspc(from);
	for (auto end = Word + sizeof(Word) - 1; to < end; ++to, ++from) {
		char c = *to = tolower(*from);
		if (c == ' ' || c == '\t') {
			c = *to = 0;
		}
		if (c == 0) {
			goto broke;
		}
	}

	// overflowed 'Word', add a trailing '\0' and drain remaining characters.
	*to = 0;
	for (;;) {
		switch (*from) {
		case 0:
		case ';':
		case '*':
		case ' ':
		case '\t': goto broke;
		default: ++from;
		}
	}

broke:
	return from;
}
