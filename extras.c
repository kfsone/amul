#include "h/amul.incs.h"
#include "h/amul.xtra.h"

// converted from extras.i
const char *
extractLine(const char *from, char *to)
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

bool
stripNewline(char *text)
{
    int len = strlen(text);
    if (len > 0 && text[len - 1] == '\n') {
        text[--len] = 0;
		return true;
    }
    return false;
}

bool
striplead(const char *lead, char *from)
{
    const char *following = skiplead(lead, from);
    if (following == from) {
        return false;
    }
    do {
        *(from) = *(following++);
    } while (*(from++));
    return true;
}

