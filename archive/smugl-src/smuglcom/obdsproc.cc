/*
 * object description table process
 */

static const char rcsid[] = "$Id: obdsproc.cc,v 1.5 1997/05/22 02:21:38 oliver Exp $";

#include "smuglcom.hpp"

void
obds_proc(void)
    {
    char *p, *s;

    if (nextc(0) == -1)
        {
	tx("<No Entries>");
	errabort();
	return;
        }				/* None to process */
    data = blkget();
    p = data;

    do
        {
	p = skipline(s = p);
	if (!*s)
	    continue;
	clean_up(s);
	s = skipspc(s);
	if (!*s || *s == ';')
	    continue;
	s = skiplead("desc=", s);
	getword(s);
	if (Word[0] == 0)
	    continue;

	if (Word[0] == '$')
            {
	    error("Invalid ID, '%s'.\n", Word);
	    p = skipdata(p);
	    continue;
            }
	add_msg(Word);
	p = text_proc(p, msgfp);
        }
    while (*p);
    errabort();
    }