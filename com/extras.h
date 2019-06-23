#pragma once

#include <cctype>
#include <cstdio>

static inline int
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