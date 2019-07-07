// converted from extras.i
#include "amulcom.includes.h"

const char *
extractLine(const char *from, char *to) noexcept
{
    *to = 0;
    // Skip whole-line comment lines
    while (isCommentChar(*from)) {
        while (*from && !isEol(*(from++))) {
            ;
        }
    }
    while (*from && !isEol(*from)) {
        *(to++) = *(from++);
    }
    if (isEol(*from))
        from++;
    *to = 0;
    return from;
}

const char *
getword(const char *from) noexcept
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
        case ' ':
        case '\t': goto broke;
        default: ++from;
        }
    }

broke:
    return from;
}

bool
stripNewline(char *text) noexcept
{
    auto len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[--len] = 0;
    }
    return len;
}