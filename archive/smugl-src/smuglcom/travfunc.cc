/*
 *  travfunc.cpp -- Functions common to Language/Travel processing
 *  [I wrote the travel table code first, and then upgraded it
 *  to handling both the language and travel code]
 */

static const char rcsid[] = "$Id: travfunc.cc,v 1.8 1999/09/10 15:57:32 oliver Exp $";

#include "smuglcom.hpp"
#include "actuals.hpp"

#include <cctype>
#include <cstring>

#define	GROW_SIZE	1024

arg_t *argptr;			/* Where to place next c/a argument */
arg_t *argtab;			/* Table of arguments */
counter_t arg_alloc;            /* How many entries allocated */

static long chknum(const char *);
static int isgen(char);
static int antype(char *);
static int isnounh(char *);
static int rdmode(char);
static int spell(char *);
static int isstat(char *);
static long bvmode(char);
static int onoff(char *);
static int randgo(char *);
static char *optis(char *);

/* chkp(p:arguments, type, c:function, z:condition|action)
 *  p    = pointer to the current value we're looking at
 *  type = the argument type we're expecting
 *  c    = the number of the current condition or action
 *  z    = (z==1) ? argument to a condition : argument to an action
 */
static char *
chkp(char *p, arg_t type, int c, int z)
    {
    char p2char;
    char qc, *p2;
    long value = -2;		/* To satisfy -Wall ;) */

    /*=* Strip crap out *=*/
    p = optis(p);
    p2 = p = skipspc(p);
    if (!*p)
        {
	error("%s: Missing paramters (%s '%s')\n",
	      word((proc == 1) ? verb.id : bobs[cur_room]->id),
              (z == 1) ? "condition" : "action",
              (z == 1) ? cond[c].name : action[c].name);
	return NULL;
        }

    /* Extract this value and null-terminate it for easy manipulation */
    if (*p != '\"' && *p != '\'')
	while (*p && *p != 32)	/* Non-quoted expression */
	    p++;
    else
        {                       /* Quoted expression */
	qc = *(p++);		/* Search for same CLOSE quote */
	while (*p && *p != qc)
	    p++;
        }
    if (*p)
	*(p++) = 0;

    /* In some instances we allow variables (e.g. "myroom"); if this is
     * such an instance, determine the real ("actual") value */
    if ((type >= 0 && type <= 10) || type == -70)
        {				/* Processing lang tab? */
	if (*p2 == '>' || *p2 == '<')
	    value = actualval(p2 + 1, type);
	else
	    value = actualval(p2, type);
	if (value == -1)
            {			/* If it was an actual, but wrong type */
	    error("%s: Invalid variable, '%s', after %s '%s'\n",
		  verb.id, p2, (z == 1) ? "condition" : "action",
                  (z == 1) ? cond[c].name : action[c].name);
	    return NULL;
            }
        }

    if (value != -2)
        { /* must have been an 'actual' value */
        if (*p2 == '>')
            value = value | MORE;
        else if (*p2 == '<')
            value = value | LESS;
        }
    else
        { /* This is already an "actual" value */
        /* Now match the value and argument type, and validate */
        switch (type)
            {
          case -7:  /* A random-go value */
                value = randgo(p2);
                break;
          case -6:  /* On/Off/yes/no toggle */
                value = onoff(p2);
                break;
          case -5:  /* Brief/Verbose toggle */
		p2char = toupper(*p2);
                value = bvmode(p2char);
                break;
          case -4:  /* Player-statistic type */
                value = isstat(p2);
                break;
          case -3:  /* Spell type */
                value = spell(p2);
                break;
          case -2:  /* Room Description mode */
		p2char = toupper(*p2);
                value = rdmode(p2char);
                break;
          case -1:  /* Announce type */
                value = antype(p2);
                break;
          case PROOM:
                value = is_bob(p2, WROOM);
                break;
          case PVERB:
                value = is_verb(p2);
                break;
          case PADJ:
                break;
          case -70:
          case PNOUN:
                value = isnounh(p2);
                break;
          case PUMSG:
                value = ttumsgchk(p2);
                break;
          case PNUM:
                value = chknum(p2);
                break;
          case PRFLAG:
                value = is_room_flag(p2);
                break;
          case POFLAG:
                value = isoflag1(p2);
                break;
          case PSFLAG:
                value = isoflag2(p2);
                break;
          case PSEX:
		p2char = toupper(*p2);
                value = isgen(p2char);
                break;
          case PDAEMON:
                /* Daemon's all have names that start with '.' */
                if ((value = is_verb(p2)) == -1 || *p2 != '.')
                    value = -1;
                break;
          case PMOBILE:
                /* Make sure it's a noun AND it has the mobile flag */
                if ((value = is_bob(p2, WNOUN)) == -1)
                    break;
                if (static_cast<OBJ*>(bobs[value])->mobile == -1)
                    value = -1;
                break;
          default:
                /* Should never reach here. Wanna bet we do? */
                if (!(proc == 1 && type >= 0 && type <= 10))
                    {
                    warne("%s = %s.\n", (z == 1) ? "condition" : "action",
                          (z == 1) ? cond[c].name : action[c].name);
                    error("> Internal error, bad PTYPE (val: %d, item: %s)!\n",
                          word((proc == 1) ? verb.id : bobs[cur_room]->id),
                          type, p2);
                    return NULL;
                    }
            }

        /* We use -2 as a place hoder to say "NONE", but it should be -1 */
        if (value == -2 && (type == PREAL || type == PUMSG))
            {
            if (type == PREAL)
                value = -1;
            }
        else if (((value == -1 || value == -2) && type != PNUM)
                 || value == -1000001)
            {
            error("%s: Invalid parameter '%s' after %s '%s'.\n",
                  word((proc == 1) ? (verb.id) : bobs[cur_room]->id), p2,
                  (z == 1) ? "condition" : "action",
                  (z == 1) ? cond[c].name : action[c].name);
            return NULL;
            }
        }

    /* Couple of last checks */
    if (!z && c == ATREATAS && value == (IWORD + IVERB))
        {
	error("%s: Action 'treatas verb' is illegal.\n",
              word((proc == 1) ? (verb.id) : bobs[cur_room]->id));
	return NULL;
        }

    /* Grow the argument-list memory area as neccesary */
    if (arg_alloc % GROW_SIZE == 0)
        {
	long new_alloc = arg_alloc + GROW_SIZE;
	argtab = static_cast<arg_t *>(grow(argtab, new_alloc * sizeof(long), "Sizing Argument Table"));
	argptr = argtab + arg_alloc;
        }
    /* Now store the argument into memory */
    *(argptr++) = value;
    arg_alloc++;
    return p;
    }

/* Check the paramters to an action */
char *
chkaparms(char *p, int c)
    {
    int i;
    for (i = 0; i < action[c].argc; i++)
        {
	if (!(p = chkp(p, action[c].argv[i], c, 0)))
	    return NULL;
        }
    return p;
    }

/* Check the paramters to a condition */
char *
chkcparms(char *p, int c)
    {
    int i;
    for (i = 0; i < cond[c].argc; i++)
	if (!(p = chkp(p, cond[c].argv[i], c, 1)))
	    return NULL;
    return p;
    }

static long
chknum(const char *s)                 /* Check a numeric arguments */
    {
    long n;

    /* Is this a variable? (less than, greater than, etc) */
    if (*s == '>' || *s == '<' || *s == '-' || *s == '=')
	n = atoi(s + 1);
    else if (!isdigit(*s) && !isdigit(*(s + 1)))
	return -1000001;
    else
	n = atoi(s);
    if (n >= 1000000)
        {
	error("%s: Number \"%s\" exceeds limit!\n",
	      word((proc == 1) ? verb.id : bobs[cur_room]->id), s);
	return -1000001;
        }
    if (*s == '-')
	return -n;
    if (*s == '>')
	return (n | LESS);
    if (*s == '<')
	return (n | MORE);
    return n;
    }

char *
optis(char *s)
    {                           /* Remove optional strings before condition */
    char *old_s = s;
    s = precon(s);
    s = skiplead("of ", s);
    s = skiplead("are ", s);
    s = skiplead("has ", s);
    s = skiplead("with ", s);
    s = skiplead("to ", s);
    s = skiplead("set ", s);
    s = skiplead("from ", s);
    s = skiplead("by ", s);
    s = skiplead("and ", s);
    s = skiplead("was ", s);
    s = skiplead("for ", s);
    if (s != old_s)
        s = optis(s);
    return skipspc(s);
    }

/* Remove 'whitewords' that can be ignored within an condition */
char *
precon(char *s)
    {
    char *old_s = s;
    s = skiplead("the ", skiplead("if ", s));
    s = skiplead("i ", s);
    s = skiplead("as ", s);
    s = skiplead("am ", s);
    if (s != old_s)
        s = precon(s);
    return s;
    }

/* Remove 'whitewords' that can be ignored within an action */
char *
preact(char *s)
    {
    char *old_s = s;
    s = skiplead("as", skiplead("to ", skiplead("go ", skiplead("goto ", skiplead("then ", s)))));
    if (s != old_s)
        s = preact(s);
    return s;
    }

/* Return gender (0==male 1==female) or -1 */
static int
isgen(char c)
    {
    if (c == 'M')
	return 0;
    if (c == 'F')
	return 1;
    return -1;
    }

/* Return announce type (see A... enums) or -1 */
static int
antype(char *s)
    {
    if (!strcmp(s, "global"))
	return AGLOBAL;
    if (!strcmp(s, "everyone"))
	return AEVERY1;
    if (!strcmp(s, "outside"))
	return AOUTSIDE;
    if (!strcmp(s, "here"))
	return AHERE;
    if (!strcmp(s, "others"))
	return AOTHERS;
    if (!strcmp(s, "all"))
	return AALL;
    if (!strcmp(s, "notsee"))
	return ANOTSEE;
    if (!strcmp(s, "cansee"))
	return ACANSEE;
    if (!strcmp(s, "cantsee"))
	return ANOTSEE;
    return -1;
    }

/* Look for a noun, prefering one that should be in this room */
/* (applies to travel table only) */
static int
isnounh(char *s)
    {
    vocid_t id;

    if (!strcmp(s, "none"))
	return -2;

    if ( (id = is_word(s)) == -1)
        return -1;

    int last = -1;              // Closest match
    int i = 0;
    for (OBJ *ptr = obtab ; i < nouns && ptr; i++, ptr = static_cast<OBJ*>(ptr->next))
        {
	if (ptr->id != id)
	    continue;
        if (is_inside(ptr->bob, cur_room))
            return i;
	last = i;               // This is a close match (id matches)
        }
    return last;                // Return closest match
    }

/* Room Description modes */
static int
rdmode(char c)
    {
    if (c == 'R')
	return RDRC;
    if (c == 'V')
	return RDVB;
    if (c == 'B')
	return RDBF;
    return -1;
    }

/* Spell types */
static int
spell(char *s)
    {
    if (!strncmp(s, "gl", 2))
	return SGLOW;
    if (!strncmp(s, "in", 2))
	return SINVIS;
    if (!strncmp(s, "de", 2))
	return SDEAF;
    if (!strncmp(s, "du", 2))
	return SDUMB;
    if (!strncmp(s, "bl", 2))
	return SBLIND;
    if (!strncmp(s, "cr", 2))
	return SCRIPPLE;
    if (!strncmp(s, "sl", 2))
	return SSLEEP;
    if (!strncmp(s, "si", 2))
	return SSINVIS;
    return -1;
    }

/* Player statistics */
static int
isstat(char *s)
    {
    if (strcmp(s, "sctg") == 0L)
	return STSCTG;
    if (strncmp(s, "sc", 2) == 0L)
	return STSCORE;
    if (strncmp(s, "poi", 3) == 0L)
	return STSCORE;
    if (strncmp(s, "str", 3) == 0L)
	return STSTR;
    if (strncmp(s, "sta", 3) == 0L)
	return STSTAM;
    if (strncmp(s, "de", 2) == 0L)
	return STDEX;
    if (strncmp(s, "wi", 2) == 0L)
	return STWIS;
    if (strncmp(s, "ex", 2) == 0L)
	return STEXP;
    if (strcmp(s, "magic") == 0L)
	return STMAGIC;
    return -1;
    }

/* Brief/Verbose modes */
static long
bvmode(char c)
    {
    if (c == 'V')
	return TYPEV;
    if (c == 'B')
	return TYPEB;
    return -1;
    }

/* On or off values */
static int
onoff(char *p)
    {
    if (!strcmp(p, "on") || !strcmp(p, "yes"))
	return 1;
    return 0;
    }

/* Randomgo options */
static int
randgo(char *p)
    {
    if (tolower(*p) == 's')
	return 0;
    if (tolower(*p) == 'a')
	return 1;
    return -1;
    }
