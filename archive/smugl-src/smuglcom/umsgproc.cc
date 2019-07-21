/*
 * User defined message table processor
 */

static const char rcsid[] = "$Id: umsgproc.cc,v 1.5 1997/05/22 02:21:43 oliver Exp $";

#include "smuglcom.hpp"

void
umsg_proc(void)
    {                           /* Process the umsg file */
    char *p, *s;

    if (nextc(0) == -1)
        {
	tx("<No Entries>");
	errabort();
	return;
        }				/* None to process */
    p = data = blkget();

    do
        {
	p = skipline(s = p);
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

static __inline msgno_t
msgline(char *s)
    {                           /* Record a message and assign it an id */
    long msgno;
    msgno = add_msg(NULL);
    fwrite(s, strlen(s), 1, msgfp);
    fputc(0, msgfp);
    return msgno;
    }

msgno_t
ttumsgchk(char *s)
    {                           /* Process a quote or 'id'd message */
    s = skiplead("msgid=", s);
    s = skipspc(s);
    if (*s == '\"' || *s == '\'')
	return msgline(s + 1);
    return ismsgid(s);
    }

