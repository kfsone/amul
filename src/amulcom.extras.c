#include <h/amul.xtra.h>

///////////////////////////////////////////////////////////////////////////////
// converted from extras.i
//
const char *
extractLine(const char *from, char *to)
{
    *to = 0;
    // Skip whole-line comment lines
    for (;;) {
        if (!isCommentChar(*from))
            break;
        from = strstop((char *)from, '\n');
    }
    // copy the text into to
    while (*from && !isEol(*from)) {
        *(to++) = *(from++);
    }
    if (isEol(*from))
        from++;
    *to = 0;
    return from;
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

///////////////////////////////////////////////////////////////////////////////
// AmigaStubs

#include <h/amigastubs.h>

static struct Task myTask;

struct Task *
FindTask(const char *name)
{
    if (name != NULL)
        return NULL;
    return &myTask;
}
