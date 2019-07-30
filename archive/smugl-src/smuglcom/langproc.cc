/*
 * Lang.TXT processor
 */

static const char rcsid[] = "$Id: langproc.cc,v 1.10 1999/06/11 14:27:02 oliver Exp $";

#include "smuglcom.hpp"
#include "actuals.hpp"

#define	GROW_SIZE	512
#define	INVALID	"Invalid syntax= line!"

static int chae_proc(char *from, char *to);
static void chae_err(void);
static void setslots(int);
static int iswtype(char *s);
static void vbprob(const char *s, char *p);

extern counter_t arg_alloc;
extern long *argtab, *argptr;

static VBTAB *vttab;	/* Table of 'vt's */
counter_t vt_alloc;             /* Number of 'vt's in use */
static VBTAB *vtp;	/* Current 'vt' */
static SLOTTAB *sttab;	/* Table of slottab's */
counter_t st_alloc;             /* Allocation of slottabs */
static SLOTTAB *stp;	/* Current slottab */

/* The default 'CHAE' pattern is -1/C/H/E for both sets */
static const char default_chae[] = { -1, 'C', 'H', 'E', -1, 'C', 'H', 'E' };

void
lang_proc(void)                 /* Process language table */
    {
    char *p, *s;
    char *ls;
    /* n=general, cs=Current Slot, x=slot */
    int n, cs;
    signed long x;
    char lastc;

    verbs = 0;
    nextc(1);
    proc = 1;
    fopenw(langfn);

    p = cleanget(256 * sizeof(*vbtab));

    vbtab = (VERB *)p;
    vbptr = vbtab;

    p = skipspc(p + (256 * sizeof(*vbtab)));

    do
        {
        p = skipline(ls = s = p);
        if (!*s)		/* Blank line? */
            continue;
        s = skipspc(s);
        if (!*s)		/* Empty line */
            continue;
        verb.id = -1;
        if (!strncmp(s, "travel=", 7))
            {
            s += 7;
            verb.ents = 0;
            verb.flags = VB_TRAVEL;
            do
                {
                s = getword(s);
                if (Word[0] == 0)
                    break;
                verbs++;
                if (is_verb(Word) != -1)
                    {
                    error("%s: Verb already defined\n", Word);
                    }
                else
                    {
                    verb.id = new_word(Word, FALSE);
                    *(vbtab + (verbs - 1)) = verb;
                    }
                }
            while (*s);
            continue;
            }
        s = skiplead("verb=", s);
        s = getword(s);
        if (!*Word)
            {
            warne("found a verb= without an ID!\n");
            p = skipdata(p);
            continue;
            }
        if (is_word(Word))
            {
            if (is_verb(Word) != -1)
                {
                error("%s: Verb already defined\n", Word);
                p = skipdata(p);
                verbs--;
                continue;
                }
            else if (is_bob(Word, WROOM) != -1)
                warne("verb %s conflicts with a room id (bad idea)\n", Word);
            }
        verb.id = new_word(Word, FALSE);
        memcpy(verb.precedences, default_chae, sizeof(verb.precedence));
        verbs++;

        /* Check the verb flags and/or 'chae' options */
        verb.flags = 0L;
        if (*s)
            {
            s = getword(s);
            if (!strcmp("travel", Word))
                {
                verb.flags |= VB_TRAVEL;
                s = getword(s);
                }
            if (!strcmp("dream", Word))
                {
                verb.flags |= VB_DREAM;
                s = getword(s);
                }
            if (*Word && chae_proc(Word, verb.precedence[0]) == -1)
                {
                s = getword(s);
                if (*Word)
                    chae_proc(Word, verb.precedence[1]);
                }
            }

        verb.ents = 0;
        verb.ptr = (SLOTTAB *) (st_alloc * sizeof(vbslot));

        /* Skip Empty lines */
        do
            {
            p = skipline(ls = s = p);
            if (!*s)
                {
                if (!verb.ents && !verb.flags & VB_TRAVEL)
                    warne("%s: No entries!\n", word(verb.id));
                break;
                }
            s = skipspc(s);
            }
        while (!*s);
        if (!*s)                /* Incase we hit end-of-paragraph */
            continue;

        setslots(WANY);
        if (strncmp(s, "syntax=", 7))
            {
            verb.ents++;
            *(p - 1) = 10;
            p = s;
            goto endsynt;
            }

        /* Syntax line loop */
synloop:
        s = skipspc(skiplead("syntax=", s));
        setslots(WNONE);
        verb.ents++;
        s = skipspc(skiplead("verb", s));
        /* ? line is '[syntax=][verb] any' or '[syntax=][verb] none' */
        if (!strncmp("any", s, 3))
            {
            setslots(WANY);
            goto endsynt;
            }
        if (!strncmp("none", s, 4))
            {
            setslots(WNONE);
            goto endsynt;
            }

sp2:			/* Syntax line processing */
        s = getword(s);
        if (!*Word)
            goto endsynt;
        if ((n = iswtype(Word)) == -3)
            {
            sprintf(block, "Invalid phrase, '%s', on syntax line!", Word);
            vbprob(block, ls);
            goto endsynt;
            }
        if (!*Word)
            x = WANY;
        else
            {

            /* Eliminate illegal combinations */
            if (n == WNONE || n == WANY)
                {
                sprintf(block, "Tried to use %s= on syntax line", syntax[n]);
                vbprob(block, ls);
                goto endsynt;
                }
            if (n == WPLAYER && strcmp(Word, "me"))
                {
                vbprob("Tried to specify player other than self", ls);
                goto endsynt;
                }

            /* Check "tag" is the correct type */
            x = -1;
            switch (n)
                {
              case WADJ:		/* Need ISADJ() - do TT entry too */
              case WNOUN:
                    x = is_bob(Word, WNOUN);
                    break;
              case WPLAYER:
                    if (!strcmp(Word, "me"))
                        x = -3;
                    break;
              case WROOM:
                    x = is_bob(Word, WROOM);
                    break;
              case WSYN:
                    warne("%s/Internal: Syns not supported!\n", word(verb.id));
                    x = WANY;
                    break;
              case WTEXT:
                    x = ismsgid(Word);
                    break;
              case WVERB:
                    x = is_verb(Word);
                    break;
              case WCLASS:
                    x = WANY;
                    break;
              case WNUMBER:
                    if (Word[0] == '-')
                        x = -atoi(Word + 1);
                    else
                        x = atoi(Word);
                    break;
              default:
                    printf("** Internal: Invalid W-type!\n");
                }

            if (n == WNUMBER && (x > 100000 || -x > 100000))
                {
                sprintf(temp, "Invalid number, %ld", x);
                vbprob(temp, ls);
                }
            if (x == -1 && n != WNUMBER)
                {
                sprintf(temp, "Invalid setting, '%s' after %s=", Word, syntax[n + 1]);
                vbprob(temp, ls);
                }
            if (x == -3 && n == WNOUN)
                x = -1;
            }

        /* Now fit into correct slot */
	if (n == WADJ)
	    {
	    // Adjectives are a special case because they have their
	    // own slots to be stored in, so we must process them
	    // seperately
	    if (vbslot.wtype[0] != WNONE || vbslot.adj[0] != -1)
		{
		if (vbslot.adj[1] != -1 || vbslot.wtype[1] != WNONE)
		    {
		    vbprob(INVALID, ls);
		    n = -5;
		    break;
		    }
		vbslot.adj[1] = x;
		}
	    else {
		vbslot.adj[0] = x;
		}

	    // Don't do the rest of the processing
	    goto sp2;
	    }
        cs = 0;			/* Noun1 */
        switch (n)
            {
          case WNOUN:
                if (vbslot.wtype[0] != WNONE && vbslot.wtype[1] != WNONE)
                    {
                    vbprob(INVALID, ls);
                    n = -5;
                    break;
                    }
                if (vbslot.wtype[0] != WNONE)
                    cs = 1;
                break;
          case WPLAYER:
          case WROOM:
          case WSYN:
          case WTEXT:
          case WVERB:
          case WCLASS:
          case WNUMBER:
                if (vbslot.wtype[0] != WNONE && vbslot.wtype[1] != WNONE)
                    {
                    vbprob(INVALID, ls);
                    n = -5;
                    break;
                    }
                if (vbslot.wtype[0] != WNONE)
                    cs = 1;
                break;
            }
        if (n == -5)
            goto sp2;
        vbslot.wtype[cs] = n;
        vbslot.slot[cs] = x;
        goto sp2;

endsynt:
        vbslot.ents = 0;
        vbslot.ptr = (VBTAB *) (vt_alloc * sizeof(vt));

commands:
        lastc = 'x';

cmdloop:
        /* Reset the basic details */
        vt.not_condition = 0;
        vt.condition = -1;
        vt.action_type = ACT_DO;
        vt.action = -1;

        p = skipline(ls = s = p);
        if (!*s)
            {
            lastc = 1;
            goto writeslot;
            }
        s = skipspc(s);
        if (!*s)
            goto cmdloop;
        if (!strncmp("syntax=", s, 7))
            {
            lastc = 0;
            goto writeslot;
            }

        vbslot.ents++;
        vt.pptr = (long *) (arg_alloc * sizeof(long));

        /* Process the condition */
        s = precon(s);
        /* Negations */
        if (*s == '!')
            {
            vt.not_condition = 1;
            s++;
            }
        else if (strncmp(s, "not ", 4) == 0)
            {
            vt.not_condition = 1;
            s += 4;             /* skip the phrase  'not ' */
            }

        s = getword(s);
        if ((vt.condition = iscond(Word)) == -1)
            {
            if ((vt.action = isact(Word)) == -1)
                {
                if ((vt.action = is_bob(Word, WROOM)) != -1)
                    {
                    vt.action_type = ACT_GO;
                    vt.condition = CALWAYS;
                    goto writecna;
                    }
                sprintf(block, "Invalid condition, '%s'", Word);
                vbprob(block, ls);
                goto commands;
                }
            vt.condition = CALWAYS;
            }
        else
            {
            if (!(s = chkcparms(s, vt.condition)))
                goto commands;
            if (!*s)
                {
                if ((vt.action = isact(cond[vt.condition].name)) == -1)
                    {
                    sprintf(block, "Missing action after condition '%s'", cond[vt.condition].name);
                    vbprob(block, ls);
                    goto commands;
                    }
                vt.condition = CALWAYS;
                goto writecna;
                }
            s = preact(s);
            s = getword(s);
            if ((vt.action = isact(Word)) == -1)
                {
                if ((vt.action = is_bob(Word, WROOM)) != -1)
                    {
                    vt.action_type = ACT_GO;
                    goto writecna;
                    }
                sprintf(block, "Invalid action, '%s'", Word);
                vbprob(block, ls);
                goto commands;
                }
            }
        if (!(s = chkaparms(s, vt.action)))
            goto commands;

writecna:
        if (vt_alloc % GROW_SIZE == 0)
            {
            counter_t new_alloc = vt_alloc + GROW_SIZE;
            vttab = (VBTAB *)grow(vttab, new_alloc * sizeof(vt), "Resizing 'vt' table");
            vtp = vttab + vt_alloc;
            }
        *(vtp++) = vt;
        vt_alloc++;
        goto commands;

writeslot:
        if (st_alloc % GROW_SIZE == 0)
            {
            counter_t new_alloc = st_alloc + GROW_SIZE;
            sttab = (SLOTTAB *)grow(sttab, new_alloc * sizeof(vbslot),
                                           "Resizing 'vbslot' table");
            stp = sttab + st_alloc;
            }
        *(stp++) = vbslot;
        st_alloc++;
        if (lastc > 1)
            goto commands;
        if (*s && !lastc)
            goto synloop;

        lastc = '\n';

        /* Write the record out */
        *(vbtab + (verbs - 1)) = verb;
        if ((long) (vbtab + (verbs - 1)) > (long) p)
            printf("@! table overtaking p\n");
        }
    while (*p);
    /*** Write out the verb data ***/
    /* First: number of verbs, syntax slots and command entries */
    fwrite(&verbs, sizeof(counter_t), 1, ofp1);
    fwrite(&st_alloc, sizeof(counter_t), 1, ofp1);
    fwrite(&vt_alloc, sizeof(counter_t), 1, ofp1);
    /* Second: The verbs themselves */
    fwrite(vbtab,  sizeof(verb), (size_t)verbs, ofp1);
    /* Third: Write out the syntax table */
    fwrite(sttab, sizeof(vbslot), (size_t)st_alloc, ofp1);
    /* Write out command tables */
    fwrite(vttab, sizeof(vt), (size_t)vt_alloc, ofp1);
    /* Write the argument lists out */
    fwrite(argtab, sizeof(long), (size_t)arg_alloc, ofp1);
    /* Give back the released memory */
    free(sttab);
    free(vttab);
    free(argtab);
    arg_alloc = 0;
    argtab = argptr = NULL;
    errabort();
    }

/* returns -1 if 's' is not a known word, otherwise returns the vocab id */
int
is_verb(const char *s)
    {
    int i, w;
    if (!verbs)
	return -1;
    if ( (w = is_word(s)) == -1)
        return -1;
    if (w == verb.id)
        return verbs - 1;
    vbptr = vbtab;
    for (i = 0; i < verbs; i++, vbptr++)
	{
	if (vbptr->id == w)
	    return i;
	}
    return -1;
    }

static int
chae_proc(char *f, char *t)	/* From and To */
    {                           /* Process a 'CHAE' string */
    int n;

    if ((*f < '0' || *f > '9') && *f != '?')
        {
	chae_err();
	return -1;
        }

    if (*f == '?')
        {
	*(t++) = -1;
	f++;
        }
    else
        {
	n = atoi(f);
	while (isdigit(*f) && *f)
	    f++;
	if (!*f)
            {
	    chae_err();
	    return -1;
            }
	*(t++) = (char) n;
        }

    for (n = 1; n < 4; n++)
        {
	if (*f == 'c' || *f == 'h' || *f == 'a' || *f == 'e')
            {
	    *(t++) = toupper(*f);
	    f++;
            }
	else
            {
	    chae_err();
	    return -1;
            }
        }

    return 0;
    }

static void
chae_err(void)
    {                           /* Report a 'CHAE' error */
    error("%s: Invalid sort-order flags \"%s\"\n", word(verb.id), Word);
    }

static void
setslots(int i)
    {				/* Initialise vb-slot entries */
    vbslot.wtype[0] = vbslot.wtype[1] = (char)i;
    vbslot.slot[0] = vbslot.slot[1] = WANY;
    vbslot.adj[0] = vbslot.adj[1] = -1;
    }

static int
iswtype(char *s)
{                           /* Determine if 's' is a 'word type', e.g. 'noun', etc */
    for (int i = 0; i < NSYNTS; i++) {
		char *p = skiplead(syntax[i], s);
		if (p == s)
			continue;
		if (*p == 0) {
			// Exact match
			*s = 0;
			return i - 1;
		}
		if (*p != '=' || *(p+1) == 0)
			continue;
		memmove(s, p + 1, strlen(p));
		return i - 1;
   	}
  	return -3;
}

static void
vbprob(const char *s, char *p)
    {				/* Declare a PROBLEM, and which verb its in */
    error("%s: line '%s'\n   > %s!\n", word(verb.id), p, s);
    }

/* Before agreeing a match, remember to check that the relevant slot isn't set
** to  NONE.   Variable  N  is  a  wtype...  If the phrases 'noun', 'noun1' or
** 'noun2'  are  used, instead of matching the phrases WTYPE with n, match the
** relevant SLOT with n...
**
** So,  if the syntax line is 'verb text player' the command 'tell noun2 text'
** will  call  isactual with *s=noun2, n=WPLAYER....  is you read the 'actual'
** structure  definition,  'noun2' is type 'WNOUN'.  WNOUN != WPLAYER, HOWEVER
** the  slot  for  noun2 (vbslot.wtype[3]) is WPLAYER, and this is REALLY what
** the user is refering to. */
int
actualval(const char *s, arg_t n)
    {                           /* Determine the actual value of a variable */
    int i;

    if (n != PREAL && (strchr("?%^~`*#", *s)))
        {
	if (n != WNUMBER && !(n == WROOM && *s == '*'))
	    return -1;
	if (*s == '~')
	    return RAND0 + atoi(s + 1);
	if (*s == '`')
	    return RAND1 + atoi(s + 1);
	i = actualval(s + 1, PREAL);
	if (i == -1)
	    return -1;
	if (*s == '#' && (i & IWORD || (i & MEPRM && i & (SELF | FRIEND | HELPER | ENEMY))))
	    return PRANK + i;
	if (i & IWORD)
	    switch (*s)
                {
              case '?':
                    return OBVAL + i;
              case '%':
                    return OBDAM + i;
              case '^':
                    return OBWHT + i;
              case '*':
                    return OBLOC + i;
                }
	return -1;
        }
    if (!isalpha(*s))
	return -2;
    for (i = 0; i < NACTUALS; i++)
        {
	if (strcmp(s, actual[i].name))
	    continue;
	/* If not a slot label, and wtypes match, is okay! */
	if (!(actual[i].value & IWORD))
	    return (actual[i].wtype == n || n == PREAL) ? actual[i].value : -1;

	/* we know its a slot label - check which: */
	switch (actual[i].value - IWORD)
            {
          case IVERB:		/* Verb */
                if (n == PVERB || n == PREAL)
                    return actual[i].value;
                return -1;
          case IADJ1:		/* Adj #1 */
                if (vbslot.adj[0] == n)
                    return actual[i].value;
                if (*(s + strlen(s) - 1) != '1' && vbslot.adj[1] == n)
                    return IWORD + IADJ2;
                if (n == PREAL)
                    return actual[i].value;
                return -1;
          case INOUN1:		/* noun 1 */
                if (vbslot.wtype[0] == n)
                    return actual[i].value;
                if (*(s + strlen(s) - 1) != '1')
                    {
                    if (actual[i].wtype == vbslot.wtype[0])
                        return actual[i].value;
                    if (actual[i].wtype == vbslot.wtype[1])
                        return IWORD + INOUN2;
                    if (vbslot.wtype[1] == n || n == PREAL)
                        return actual[i].value;
                    return -1;
                    }
                if (n == PREAL)
                    return actual[i].value;
                return -1;
          case IADJ2:
                return (vbslot.adj[1] == n || n == PREAL) ?
                    actual[i].value : -1;
          case INOUN2:
                return (vbslot.wtype[1] == n || n == PREAL) ?
                    actual[i].value : -1;
          default:
                return -1;
            }
        }
    return -2;			/* It was no actual! */
    }

#undef	INVALID
