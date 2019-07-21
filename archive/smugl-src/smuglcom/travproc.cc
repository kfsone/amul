/*
 * travproc.cpp -- TRAVEL.txt processing
 */

static const char rcsid[] = "$Id: travproc.cc,v 1.9 1999/06/11 14:27:02 oliver Exp $";

#include "smuglcom.hpp"

extern long arg_alloc, *argtab, *argptr;

void
trav_proc(void)
    {                           /* Process travel table */
    int strip, lines;
    int nvbs, i, ntt;
    char *p, *s;
    long *l;
    long tabcnt;
    ROOM *rmp;

    ntt = 0;
    tabcnt = 0;
    nextc(1);
    fopenw(ttfn);
    fopenw(ttpfn);
    data = cleanget();
    p = skipspc(data);

    do
        {
	p = skipline(s = p);
	s = skipspc(s);
	if (!*s)
            continue;
	getword(skiplead("room=", s));
	if ((cur_room = is_bob(Word, WROOM)) == -1)
            {
	    error("Undefined room: %s\n", Word);
	    p = skipdata(p);
            continue;
            }
        rmp = (ROOM *)bobs[cur_room];
	if (rmp->tabptr != -1)
            {
	    error("%s: Travel already defined.\n", Word);
	    p = skipdata(p);
            continue;
            }

        do
            {
            p = skipline(s = p);
            if (!*s)
                {
                /* Only complain if room is not a death room */
                if ((rmp->std_flags & bob_DEATH) != bob_DEATH)
                    warne("%s: No T.T entries!\n", word(rmp->id));
                rmp->tabptr = -2;
                ntt++;
                break;
                }
            s = skipspc(s);
            if (!*s)
                continue;
            if (skiplead("verbs=", s) == s)
                {
                error("%s: Missing verbs= line.\n", word(rmp->id));
                continue;
                }
            }
        while (!*s);
        if (!*s)
            continue;   /* Didn't like the verb list */

        /* Prep some values */
        s = skiplead("verbs=", s);
        lines = 0;
        verb.id = -1;
        rmp->tabptr = tabcnt;
        rmp->ttlines = 0;

vbproc:			/* Process verb list */
        nvbs = 0;
        tt.pptr = (long *) -1;
        l = (long *) temp;
        /* Break verb list down to verb no.s */
        do
            {
            s = getword(s);
            if (!*Word)
                break;
            *l = is_word(Word);
            if (*l == -1 || is_verb(Word) == -1)
                error("%s: Invalid \"%s\"\n", word(rmp->id), Word);
            l++;
            nvbs++;
            }
        while (*Word);
        if (!nvbs)
            {
            error("%s: No verbs specified after verbs=!\n", word(rmp->id));
            }
        /* Now process each instruction line */
        do
            {
            tt.not_condition = 0;
            tt.condition = -1;
            tt.action_type = ACT_GO;
            tt.action = -1;
            strip = 0;

            p = skipline(s = p);
            if (!*s)
                {
                strip = -1;
                continue;
                }
            s = skipspc(s);
            if (!*s)
                continue;
            if (!strncmp("verbs=", s, 6))
                {
                strip = 1;
                s += 6;
                break;
                }
            s = precon(s);	/* Strip pre-condition opts */

            /* Negations */
            if (*s == '!')
                {
                tt.not_condition = 1;
                s++;
                }
            if (strncmp(s, "not ", 4) == 0)
                {
                tt.not_condition = 1;
                s += 4;         /* Skip the phrase 'not ' */
                }
            s = getword(s);
            if ((tt.condition = iscond(Word)) == -1)
                {
                tt.condition = CALWAYS;
                if ((tt.action = is_bob(Word, WROOM)) != -1)
                    goto write;
                if ((tt.action = isact(Word)) == -1)
                    {
                    error("%s: Invalid condition \"%s\"\n",
                          word(rmp->id), Word);
                    continue;
                    }
                tt.action_type = ACT_DO;
                }
            else
                {
                s = skipspc(s);
                if (!(s = chkcparms(s, tt.condition)))
                    {
                    strip = 0;
                    continue;
                    }
                if (!*s)
                    {
                    error("%s: C&A line has missing action.\n", word(rmp->id));
                    continue;
                    }
                s = preact(s);
                s = getword(s);
                if ((tt.action = is_bob(Word, WROOM)) == -1)
                    {
                    if ((tt.action = isact(Word)) == -1)
                        {
                        error("%s: Invalid %s \"%s\"\n",
                              word(rmp->id), "action", Word);
                        continue;
                        }
                    tt.action_type = ACT_DO;
                    }
                }
            s = skipspc(s);
            if (tt.action_type == ACT_DO)
                {
                if (tt.action == ATRAVEL)
                    {
                    error("%s: Action 'TRAVEL' not allowed!\n");
                    continue;
                    }
                if (!(s = chkaparms(s, tt.action)))
                    {
                    strip = 0;
                    continue;
                    }
                }
    write:
            l = (long *) temp;
            for (i = 0; i < nvbs; i++)
                {
                if (i < nvbs - 1)
                    tt.pptr = (long *) -2;
                else
                    tt.pptr = (long *) -1;
                tt.verb = *(l++);
                fwrite(&tt, sizeof(tt), 1, ofp1);
                rmp->ttlines++;
                tabcnt++;
                ttents++;
                }
            lines++;
            strip = 0;
            }
        while (!strip && *p);
        if (strip == 1 && *p)
            goto vbproc;
        ntt++;
        }
    while (*p);
    if (!err && ntt != rooms && warn == 1)
        {
	rmp = roomtab;
	for (rmp = roomtab; rmp && rmp->type == WROOM; rmp = (ROOM *)rmp->next)
	    if (rmp->tabptr == -1
                && (rmp->std_flags & bob_DEATH) != bob_DEATH)
		warne("No exits for %s\n", word(rmp->id));
        }
    fwrite(argtab, sizeof(*argtab), (size_t)arg_alloc, ofp2);
    free(argtab);
    arg_alloc = 0;
    argtab = argptr = NULL;

    errabort();
    }




