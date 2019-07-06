// travel parsing helpers, used by both the travel and the language parser

#include "amulcom.includes.h"

#include "constants.h"

using namespace AMUL::Logging;
using namespace Compiler;

///TODO: relocate to header
extern const char nacp[NACTS];

const char *
precon(const char *s)
{
	const char *s2 = s;

	if ((s = skiplead("if ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	if ((s = skiplead("the ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	if ((s = skiplead("i ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	s = skiplead("am ", s);

	return s;
}

const char *
preact(const char *s)
{
	const char *s2 = s;
	if ((s = skiplead("then ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	if ((s = skiplead("goto ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	if ((s = skiplead("go to ", s)) != s2) {
		s = skipspc(s);
		s2 = s;
	}
	s = skiplead("set ", s);
	return s;
}

int32_t
chknum(const char *p)
{
	int32_t n;

	if (!isdigit(*p) && !isdigit(*(p + 1)))
		return -1000001;
	if (*p == '>' || *p == '<' || *p == '-' || *p == '=')
		n = atoi(p + 1);
	else
		n = atoi(p);
	if (n >= 1000000) {
		printf("\x07\n*** Number %d exceeds limits!", n);
		return -1000001;
	}
	if (*p == '-')
		return (int32_t)-n;
	if (*p == '>')
		return (int32_t)(n + LESS);
	if (*p == '<')
		return (int32_t)(n + MORE);
	return n;
}

const char *
optis(const char *p)
{
	const char *p2 = p;
	p = skiplead("the ", p);
	p = skiplead("of ", p);
	p = skiplead("are ", p);
	p = skiplead("is ", p);
	p = skiplead("has ", p);
	p = skiplead("next ", p);
	p = skiplead("with ", p);
	p = skiplead("to ", p);
	p = skiplead("set ", p);
	p = skiplead("from ", p);
	p = skiplead("for ", p);
	p = skiplead("by ", p);
	p = skiplead("and ", p);
	p = skiplead("was ", p);
	p = skiplead("i ", p);
	p = skiplead("am ", p);
	p = skiplead("as ", p);
	p = skipspc(p);
	return p;
}

const char *
chkp(char *p, char t, int c, int z, FILE *fp)
{
	char	qc;
	int32_t x;

	p = const_cast<char*>(optis(p));
	const char* p2 = (p = const_cast<char*>(skipspc(p)));  // clean out whitespace
	if (*p == 0) {
		GetLogger().fatalf("%s: %s: incomplete condition/action line: %s='%s'",
			   (proc == 1) ? "Verb" : "Room", (proc == 1) ? verb.id : roomtab->id,
			   (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c]);
	}
	if (*p != '\"' && *p != '\'')
		while (*p != 32 && *p != 0)
			p++;
	else {
		qc = *(p++);  // look for matching close quote
		while (*p != 0 && *p != qc)
			p++;
	}
	if (*p != 0)
		*p = 0;
	else
		*(p + 1) = 0;
	if ((t >= 0 && t <= 10) || t == -70)  // processing language table?
	{
		x = actualval(p2, t);
		if (x == -1) /* If it was an actual, but wrong type */
		{
			printf("\x07\nInvalid slot label, '%s', after %s '%s' in verb '%s'.\n", p2,
				   (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c], verb.id);
			return NULL;
		}
		if (x != -2)
			goto write;
	}
	switch (t) {
	case -6: x = onoff(p2); break;
	case -5: x = bvmode(toupper(*p2)); break;
	case -4: x = isstat(p2); break;
	case -3: x = isspell(p2); break;
	case -2: x = rdmode(toupper(*p2)); break;
	case -1: x = antype(p2); break;
	case CAP_ROOM: x = isroom(p2); break;
	case CAP_VERB: x = is_verb(p2); break;
	case CAP_ADJ: break;
	case -70:
	case CAP_NOUN: x = isnounh(p2); break;
	case CAP_UMSG: x = ttumsgchk(p2); break;
	case CAP_NUM: x = chknum(p2); break;
	case CAP_ROOM_FLAG: x = isrflag(p2); break;
	case CAP_OBJ_FLAG: x = isoflag1(p2); break;
	case CAP_STAT_FLAG: x = isoflag2(p2); break;
	case CAP_GENDER: x = isgender(toupper(*p2)); break;
	case CAP_DAEMON_ID:
		if ((x = is_verb(p2)) == -1 || *p2 != '.')
			x = -1;
		break;
	default: {
		if (!(proc == 1 && t >= 0 && t <= 10)) {
			printf("\n\n\x07!! Internal error, invalid PTYPE (val: %d) in %s %s!\n\n", t,
				   (proc == 1) ? "verb" : "room", (proc == 1) ? verb.id : (rmtab + rmn)->id);
			printf("%s = %s.\n", (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c]);
			quit();
		}
	}
	}
	if (t == -70 && x == -2)
		x = -1;
	else if (((x == -1 || x == -2) && t != CAP_NUM) || x == -1000001) {
		printf("\x07\nInvalid parameter, '%s', after %s '%s' in %s '%s'.\n", p2,
			   (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c],
			   (proc == 1) ? "verb" : "room", (proc == 1) ? (verb.id) : (rmtab + rmn)->id);
		return NULL;
	}
write:
	fwrite((char *)&x, 4, 1, fp);
	FPos += 4; /* Writes a LONG */
	*p = 32;
	return skipspc(p);
}

int
isgender(char c)
{
	if (c == 'M')
		return 0;
	if (c == 'F')
		return 1;
	return -1;
}

int
antype(const char *s)
{
	if (strcmp(s, "global") == NULL)
		return AGLOBAL;
	if (strcmp(s, "everyone") == NULL)
		return AEVERY1;
	if (strcmp(s, "outside") == NULL)
		return AOUTSIDE;
	if (strcmp(s, "here") == NULL)
		return AHERE;
	if (strcmp(s, "others") == NULL)
		return AOTHERS;
	if (strcmp(s, "all") == NULL)
		return AALL;
	printf("\x07\nInvalid anouncement-group, '%s'...\n", s);
	return -1;
}

/* Test noun state, checking rooms */
int
isnounh(const char *s)
{
	int		i, l, j;
	int32_t orm;

	if (stricmp(s, "none") == NULL)
		return -2;
	FILE *fp = rfopen(Resources::Compiled::objLoc());
	l = -1;
	objtab2 = obtab2;

	for (i = 0; i < nouns; i++, objtab2++) {
		if (stricmp(s, objtab2->id) != NULL)
			continue;
		fseek(fp, (int32_t)objtab2->rmlist, 0L);
		for (j = 0; j < objtab2->nrooms; j++) {
			fread((char *)&orm, 4, 1, fp);
			if (orm == rmn) {
				l = i;
				i = nouns + 1;
				j = objtab2->nrooms;
				break;
			}
		}
		if (i < nouns)
			l = i;
	}
	fclose(fp);
	return l;
}

int
rdmode(char c)
{
	if (c == 'R')
		return RD_VERBOSE_ONCE;
	if (c == 'V')
		return RD_VERBOSE;
	if (c == 'B')
		return RD_TERSE;
	return -1;
}

int
isspell(const char *s)
{
	if (strcmp(s, "glow") == NULL)
		return SPELL_GLOW;
	if (strcmp(s, "invis") == NULL)
		return SPELL_INVISIBLE;
	if (strcmp(s, "deaf") == NULL)
		return SPELL_DEAFEN;
	if (strcmp(s, "dumb") == NULL)
		return SPELL_MUTE;
	if (strcmp(s, "blind") == NULL)
		return SPELL_BLIND;
	if (strcmp(s, "cripple") == NULL)
		return SPELL_CRIPPLE;
	if (strcmp(s, "sleep") == NULL)
		return SPELL_SLEEP;
	if (strcmp(s, "sinvis") == NULL)
		return SPELL_SUPER_INVIS;
	return -1;
}

int
isstat(const char *s)
{
	if (strcmp(s, "sctg") == NULL)
		return STSCTG;
	if (strncmp(s, "sc", 2) == NULL)
		return STSCORE;
	if (strncmp(s, "poi", 3) == NULL)
		return STSCORE;
	if (strncmp(s, "str", 3) == NULL)
		return STSTR;
	if (strncmp(s, "stam", 4) == NULL)
		return STSTAM;
	if (strncmp(s, "dext", 4) == NULL)
		return STDEX;
	if (strncmp(s, "wis", 3) == NULL)
		return STWIS;
	if (strncmp(s, "exp", 3) == NULL)
		return STEXP;
	if (strcmp(s, "magic") == NULL)
		return STMAGIC;
	return -1;
}

int
bvmode(char c)
{
	if (c == 'V')
		return VERBOSE;
	if (c == 'B')
		return TERSE;
	return -1;
}

char *
chkaparms(char *p, int c, FILE *fp)
{
	int i;

	if (nacp[c] == 0)
		return p;
	for (i = 0; i < nacp[c]; i++)
		if ((p = chkp(p, tacp[c][i], c, 0, fp)) == NULL)
			return NULL;
	return p;
}

char *
chkcparms(char *p, int c, FILE *fp)
{
	int i;

	if (ncop[c] == 0)
		return p;
	for (i = 0; i < ncop[c]; i++)
		if ((p = chkp(p, tcop[c][i], c, 1, fp)) == NULL)
			return NULL;
	return p;
}

int
onoff(char *p)
{
	if (stricmp(p, "on") == NULL || stricmp(p, "yes") == NULL)
		return 1;
	return 0;
}
