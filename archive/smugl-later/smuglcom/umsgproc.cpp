/*
 * User defined message table processor
 */

#include "smuglcom/smuglcom.hpp"

void
umsg_proc(void)
{								// Process the umsg file
	if (nextc(0) == -1)			// None to process.
	{
		tx("<No Entries>");
		errabort();
		return;
	}

	char* p = data = blkget();
	do
	{
		char* s = p ;
		char* p = skipline(s);
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

static msgno_t
msgline(char* s)
{								/* Record a message and assign it an id */
	const msgno_t msgno = add_msg(NULL);
	fwrite(s, strlen(s), 1, msgfp);
	fputc(0, msgfp);
	return msgno;
}

msgno_t
ttumsgchk(char* s)
{								/* Process a quote or 'id'd message */
	s = skiplead("msgid=", s);
	s = skipspc(s);
	if (*s == '\"' || *s == '\'')
		return msgline(s + 1);
	return ismsgid(s);
}
