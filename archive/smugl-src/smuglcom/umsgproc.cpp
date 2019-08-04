/*
 * User defined message table processor
 */

#include <cstring>

#include "errors.hpp"
#include "fileio.hpp"
#include "smuglcom.hpp"

void
umsg_proc()
{  // Process the umsg file
    if (nextc(0) == -1) {
        tx("<No Entries>");
        errabort();
        return;
    }

    char *p = data = blkget();
    do {
        char *s = p;
        char *p = skipline(s);
        if (!*s)
            continue;

        clean_up(s);
        s = skipspc(s);
        if (!*s || *s == ';')
            continue;
        s = skiplead("msgid=", s);
        getword(s);
        if (Word[0] == 0)
            continue;

        if (Word[0] == '$') {
            error("Invalid ID, '%s'.\n", Word);
            p = skipdata(p);
            continue;
        }
        add_msg(Word);
        p = text_proc(p, msgfp);
    } while (*p);
    errabort();
}

static __inline msgno_t
msgline(char *s)
{  // Record a message and assign it an id
    long msgno;
    msgno = add_msg(nullptr);
    fwrite(s, strlen(s), 1, msgfp);
    fputc(0, msgfp);
    return msgno;
}

msgno_t
ttumsgchk(char *s)
{  // Process a quote or 'id'd message
    s = skiplead("msgid=", s);
    s = skipspc(s);
    if (*s == '\"' || *s == '\'')
        return msgline(s + 1);
    return ismsgid(s);
}
