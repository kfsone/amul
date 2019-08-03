/*
 * smsgproc.cpp -- process SYSMSG.txt
 *
 * SYSMSGs MUST be listed in order, and MUST all exist! These should
 * be supplied with the package, so the user has a set of defaults.
 */

static const char rcsid[] = "$Id: smsgproc.cc,v 1.5 1997/05/22 02:21:41 oliver Exp $";

#include "smuglcom.hpp"

void
smsg_proc(void)
{
    char *p, *s;
    long smsgs;

    smsgs = 0;
    if (nextc(0) == -1) {
        error("No System Messages Present - Can't Continue\n");
        errabort(); /* Nothing to process! */
    }

    data = blkget();
    p = skipspc(data);

    do {
        p = skipline(s = p);
        s = skipspc(s);
        if (!*s || *s == ';')
            continue;
        s = skiplead("msgid=", s);
        getword(s);
        if (!*Word)
            break;

        if (Word[0] != '$') {
            error("Invalid SysMsg ID '%s'!\n", Word);
            p = skipdata(p);
            continue;
        }
        if (atoi(Word + 1) != smsgs + 1) {
            error("Message %s out of sequence!\n", Word);
            p = skipdata(p);
            continue;
        }
        add_msg(NULL);
        p = text_proc(p, msgfp);
        smsgs++;
    } while (*p && smsgs < NSMSGS);
    if (smsgs != NSMSGS)
        error("%ld message(s) missing!\n\n", NSMSGS - smsgs);
    errabort();
}
