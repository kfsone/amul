// Lang.TXT processor
#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

static void
chae_err()
{
	GetLogger().errorf("Invalid '#CHAE' flags, \"%s\" in verb \"%s\".\n", Word, verb.id);
}

// Process LANG.TXT
void
lang_proc()
{
	char lastc, *p, *p2, *s1, *s2;
	// n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2)
	int		 n, cs, s, r;
	uint32_t of2p, of3p;

	verbs = 0;
	nextc(1);
	OS::CreateFile(Resources::Compiled::lang1());
	fopena(Resources::Compiled::lang1());
	ofp1 = afp;
	afp = NULL;
	fopenw(Resources::Compiled::lang2());
	fopenw(Resources::Compiled::lang3());
	fopenw(Resources::Compiled::lang4());

	blkget(&vbmem, (char **)&vbtab, 64 * (sizeof(verb)));
	vbptr = vbtab + 64;
	s1 = (char *)vbptr;
	vbptr = vbtab;
	of2p = ftell(ofp2);
	of3p = ftell(ofp3);
	FPos = ftell(ofp4);

	do {
		GetContext().checkErrorCount();

		verbs++;
		p = block;
	loop:
		do {
			s1 = sgetl((s2 = s1), block);
			*(s1 - 1) = 0;
		} while (com(block) == -1 && *s1 != 0);
		if (*s1 == 0) {
			verbs--;
			break;
		}
		tidy(block);
		if (block[0] == 0)
			goto loop;
		p = skiplead("verb=", block);
		p = getword(p);
		if (Word[0] == 0) {
			printf("!! \x07 verb= line without a verb!\n");
			goto loop;
		}
		if (strlen(Word) > IDL) {
			GetLogger().errorf("Invalid Verb ID: \"%s\"", Word);

			do {
				s1 = sgetl((s2 = s1), block);
				*(s1 - 1) = 0;
			} while (*s1 != 0 && block[0] != 0);
			if (*s1 == 0)
				break;
			goto loop;
		}

		strcpy(verb.id, Word);

		p2 = verb.sort;
		*(p2++) = -1;
		*(p2++) = 'C';
		*(p2++) = 'H';
		*(p2++) = 'A';
		*(p2++) = 'E';
		*(p2++) = -1;
		*(p2++) = 'C';
		*(p2++) = 'H';
		*(p2++) = 'A';
		*(p2++) = 'E';

		verb.flags = VB_TRAVEL;
		if (*p == 0 || *p == ';' || *p == '*')
			goto noflags;
		p = getword(p);
		if (strcmp("travel", Word) == NULL) {
			verb.flags = 0;
			p = getword(p);
		}
		if (strcmp("dream", Word) == NULL) {
			verb.flags += VB_DREAM;
			p = getword(p);
		}
		if (Word[0] == 0 || Word[0] == ';' || Word[0] == '*')
			goto noflags;

		if (chae_proc(Word, verb.sort) == -1)
			goto noflags;
		p = getword(p);
		if (Word[0] != 0 && Word[0] != ';' && Word[0] != '*')
			chae_proc(Word, verb.sort2);

	noflags:
		verb.ents = 0;
		verb.ptr = (struct _SLOTTAB *)of2p;

	stuffloop:
		do {
			s2 = s1;
			s1 = sgetl(s1, block);
			*(s1 - 1) = 0;
		} while (*s1 != 0 && block[0] != 0 && com(block) == -1);
		if (*s1 == 0 || block[0] == 0) {
			if (verb.ents == 0 && (verb.flags & VB_TRAVEL))
				printf("!! \x07Verb \"%s\" has no entries!\n", verb.id);
			goto write;
		}

		tidy(block);
		if (block[0] == 0)
			goto stuffloop;

		if ((p = skiplead("syntax=", block)) == block) {
			vbprob("no syntax= line", s2);
			goto stuffloop;
		}

		// Syntax line loop
	synloop:
		setslots(TC_NONE);
		verb.ents++;
		p = skiplead("verb", p);
		p2 = getword(p);
		p2 = skipspc(p2);

		// If syntax line is 'syntax=verb any' or 'syntax=none'
		if (*p2 == 0 && strcmp("any", Word) == NULL) {
			setslots(TC_ANY);
			goto endsynt;
		}
		if (*p2 == 0 && strcmp("none", Word) == NULL) {
			setslots(TC_NONE);
			goto endsynt;
		}

	sp2:  // Syntax line processing
		p = skipspc(p);
		if (*p == ';' || *p == '|' || *p == '*')
			goto endsynt;
		Word[0] = 0;
		p = getword(p);
		if (Word[0] == 0)
			goto endsynt;
		if ((n = iswtype(Word)) == -3) {
			sprintf(block, "Invalid phrase, '%s', on syntax line!", Word);
			vbprob(block, s2);
			goto commands;
		}
		if (Word[0] == 0) {
			s = TC_ANY;
			goto skipeq;
		}

		// First of all, eliminate illegal combinations
		if (n == TC_NONE || n == TC_ANY) {  // you cannot say none=fred any=fred etc
			sprintf(block, "Tried to defined %s= on syntax line", syntax[n]);
			vbprob(block, s2);
			goto endsynt;
		}
		if (n == TC_PLAYER && strcmp(Word, "me") != NULL && strcmp(Word, "myself") != NULL) {
			vbprob("Tried to specify player other than self", s2);
			goto endsynt;
		}

		// Now check that the 'tag' is the correct type of word

		s = -1;
		switch (n) {
		case TC_ADJ:
		// Need ISADJ() - do TT entry too
		case TC_NOUN: s = isnoun(Word); break;
		case TC_PREP: s = isprep(Word); break;
		case TC_PLAYER:
			if (strcmp(Word, "me") == NULL || strcmp(Word, "myself") == NULL)
				s = -3;
			break;
		case TC_ROOM: s = isroom(Word); break;
		case TC_SYN: printf("!! Syn's not supported at this time!\n"); s = TC_ANY;
		case TC_TEXT: s = chkumsg(Word); break;
		case TC_VERB: s = is_verb(Word); break;
		case TC_CLASS: s = TC_ANY;
		case WC_NUMBER:
			if (Word[0] == '-')
				s = atoi(Word + 1);
			else
				s = atoi(Word);
		default: printf("** Internal error! Invalid W-type!\n");
		}

		if (n == WC_NUMBER && s > 100000 || -s > 100000) {
			sprintf(fnm, "Invalid number, %ld", s);
			vbprob(fnm, s2);
		}
		if (s == -1 && n != WC_NUMBER) {
			sprintf(fnm, "Invalid setting, '%s' after %s=", Word, syntax[n + 1]);
			vbprob(fnm, s2);
		}
		if (s == -3 && n == TC_NOUN)
			s = -1;

	skipeq:  // (Skipped the equals signs)
		// Now fit this into the correct slot
		cs = 1;  // Noun1
		switch (n) {
		case TC_ADJ:
			if (vbslot.wtype[1] != TC_NONE && vbslot.wtype[4] != TC_NONE) {
				vbprob("Invalid NOUN NOUN ADJ combination", s2);
				n = -5;
				break;
			}
			if (vbslot.wtype[1] != TC_NONE && vbslot.wtype[3] != TC_NONE) {
				vbprob("Invalid NOUN ADJ NOUN ADJ combination", s2);
				n = -5;
				break;
			}
			if (vbslot.wtype[0] != TC_NONE && vbslot.wtype[3] != TC_NONE) {
				vbprob("More than two adjectives on a line", s2);
				n = -5;
				break;
			}
			if (vbslot.wtype[0] != TC_NONE)
				cs = 3;
			else
				cs = 0;
			break;
		case TC_NOUN:
			if (vbslot.wtype[1] != TC_NONE && vbslot.wtype[4] != TC_NONE) {
				vbprob("Invalid noun arrangement", s2);
				n = -5;
				break;
			}
			if (vbslot.wtype[1] != TC_NONE)
				cs = 4;
			break;
		case TC_PREP:
			if (vbslot.wtype[2] != TC_NONE) {
				vbprob("Invalid PREP arrangement", s2);
				n = -5;
				break;
			}
			cs = 2;
			break;
		case TC_PLAYER:
		case TC_ROOM:
		case TC_SYN:
		case TC_TEXT:
		case TC_VERB:
		case TC_CLASS:
		case WC_NUMBER:
			if (vbslot.wtype[1] != TC_NONE && vbslot.wtype[4] != TC_NONE) {
				sprintf(block, "No free noun slot for '%s' entry", syntax[n + 1]);
				vbprob(block, s2);
				n = -5;
				break;
			}
			if (vbslot.wtype[1] != TC_NONE)
				cs = 4;
			break;
		}
		if (n == -5)
			goto sp2;
		// Put the bits into the slots!
		vbslot.wtype[cs] = n;
		vbslot.slot[cs] = s;
		goto sp2;

	endsynt:
		vbslot.ents = 0;
		vbslot.ptr = (struct _VBTAB *)of3p;

	commands:
		lastc = 'x';
		proc = 0;

		do {
			s2 = s1;
			s1 = sgetl(s1, block);
			*(s1 - 1) = 0;
		} while (*s1 != 0 && block[0] != 0 && com(block) == -1);
		if (block[0] == 0 || *s1 == 0) {
			lastc = 1;
			goto writeslot;
		}
		tidy(block);
		if ((p = skiplead("syntax=", block)) != block) {
			lastc = 0;
			goto writeslot;
		}
		if (*p == 0)
			goto commands;

		vbslot.ents++;
		r = -1;
		vt.pptr = (int32_t *)FPos;

		// Process the condition
	notloop:
		p = precon(p);
		p = getword(p);

		// always endparse
		if (strcmp(Word, ALWAYSEP) == NULL) {
			vt.condition = CALWAYS;
			vt.action = -(1 + AENDPARSE);
			goto writecna;
		}
		if (strcmp(Word, "not") == NULL || (Word[0] == '!' && Word[1] == 0)) {
			r = -1 * r;
			goto notloop;
		}

		// If they forgot space between !<condition>, eg !toprank
	notlp2:
		if (Word[0] == '!') {
			strcpy(Word, Word + 1);
			r = -1 * r;
			goto notlp2;
		}

		if ((vt.condition = iscond(Word)) == -1) {
			proc = 1;
			if ((vt.action = isact(Word)) == -1) {
				if ((vt.action = isroom(Word)) != -1) {
					vt.condition = CALWAYS;
					goto writecna;
				}
				sprintf(block, "Invalid condition, '%s'", Word);
				vbprob(block, s2);
				proc = 0;
				goto commands;
			}
			vt.condition = CALWAYS;
			goto doaction;
		}
		p = skipspc(p);
		proc = 1;
		if ((p = chkcparms(p, vt.condition, ofp4)) == NULL) {
			GetContext().addError();
			goto commands;
		}
		if (*p == 0) {
			if ((vt.action = isact(conds[vt.condition])) == -1) {
				vbprob("Missing action", s2);
				goto commands;
			}
			vt.action = 0 - (vt.action + 1);
			vt.condition = CALWAYS;
			goto writecna;
		}
		if (r == 1)
			vt.condition = -1 - vt.condition;
		p = preact(p);
		p = getword(p);
		if ((vt.action = isact(Word)) == -1) {
			if ((vt.action = isroom(Word)) != -1)
				goto writecna;
			sprintf(block, "Invalid action, '%s'", Word);
			vbprob(block, s2);
			goto commands;
		}
	doaction:
		p = skipspc(p);
		if ((p = chkaparms(p, vt.action, ofp4)) == NULL) {
			GetContext().addError();
			goto commands;
		}
		vt.action = 0 - (vt.action + 1);

	writecna:  // Write the C & A lines
		fwrite((char *)&vt.condition, sizeof(vt), 1, ofp3);
		proc = 0;
		of3p += sizeof(vt);
		goto commands;

	writeslot:
		fwrite(vbslot.wtype, sizeof(vbslot), 1, ofp2);
		proc = 0;
		of2p += sizeof(vbslot);
		if (lastc > 1)
			goto commands;
		if (lastc == 0)
			goto synloop;

		lastc = '\n';
	write:
		fwrite(verb.id, sizeof(verb), 1, ofp1);
		proc = 0;
		*(vbtab + (verbs - 1)) = verb;
		if ((uintptr_t)(vbtab + (verbs - 1)) > (uintptr_t)s1)
			printf("@! table overtaking s1\n");
	} while (*s1 != 0);

	GetContext().terminateOnErrors();

	close_ofps();
}

// From and To
int
chae_proc(char *f, char *t)
{
	int n;

	if (*f < '0' || *f > '9' && *f != '?') {
		chae_err();
		return -1;
	}

	if (*f == '?') {
		*(t++) = -1;
		f++;
	} else {
		n = atoi(f);
		while (isdigit(*f) && *f != 0)
			f++;
		if (*f == 0) {
			chae_err();
			return -1;
		}
		*(t++) = (char)n;
	}

	for (n = 1; n < 5; n++) {
		if (*f == 'c' || *f == 'h' || *f == 'a' || *f == 'e') {
			*(t++) = toupper(*f);
			f++;
		} else {
			chae_err();
			return -1;
		}
	}

	return 0;
}

// Set the VT slots
void
setslots(unsigned char i)
{
	vbslot.wtype[0] = TC_ANY;
	vbslot.wtype[1] = i;
	vbslot.wtype[2] = i;
	vbslot.wtype[3] = TC_ANY;
	vbslot.wtype[4] = i;
	vbslot.slot[0] = vbslot.slot[1] = vbslot.slot[2] = vbslot.slot[3] = vbslot.slot[4] = TC_ANY;
}

// Is 'text' a ptype
int
iswtype(const char *s)
{
	int i;

	for (i = 0; i < NSYNTS; i++) {
		if (strcmp(s, syntax[i]) == NULL) {
			*s = 0;
			return i - 1;
		}
		if (strncmp(s, syntax[i], syntl[i]) != NULL)
			continue;
		if (*(s + syntl[i]) != '=')
			continue;
		strcpy(s, s + syntl[i] + 1);
		return i - 1;
	}
	return -3;
}

// Declare a PROBLEM, and which verb its in
void
vbprob(char *error, char *input)
{
	printf("## Verb: %s, Line: %s\n", verb.id, input);
	GetLogger().error(error);
}

/* Note about matching actuals...

Before agreeing a match, remember to check that the relevant slot isn't
set to NONE.
Variable N is a wtype... If the phrases 'noun', 'noun1' or 'noun2' are used,
instead of matching the phrases WTYPE with n, match the relevant SLOT with
n...

So, if the syntax line is 'verb text player' the command 'tell noun2 text'
will call isactual with *s=noun2, n=TC_PLAYER.... is you read the 'actual'
structure definition, 'noun2' is type 'TC_NOUN'. TC_NOUN != TC_PLAYER, HOWEVER
the slot for noun2 (vbslot.wtype[4]) is TC_PLAYER, and this is REALLY what the
user is refering too.							     */
// Get actual value
int
actualval(const char *s, int n)
{
	int i;

	if (n != -70 && (*s == '?' || *s == '%' || *s == '^' || *s == '~' || *s == '\`')) {
		if (n != WC_NUMBER)
			return -1;
		if (*s == '~')
			return RAND0 + atoi(s + 1);
		if (*s == '\`')
			return RAND1 + atoi(s + 1);
		i = actualval(s + 1, -70);
		if (i == -1)
			return -1;
		if ((i & IWORD) == 0)
			return -1;
		if (*s == '?')
			return OBVAL + i;
		if (*s == '%')
			return OBDAM + i;
		if (*s == '^')
			return OBWHT + i;
		if (*s == '*')
			return OBLOC + i;
		if (*s == '#')
			return PRANK + i;
		return -1;
	}
	if (!isalpha(*s))
		return -2;
	for (i = 0; i < NACTUALS; i++) {
		if (stricmp(s, actual[i].name) != NULL)
			continue;
		// If its not a slot label, and the wtypes match, we's okay!
		if (!(actual[i].value & IWORD))
			return (actual[i].wtype == n || n == -70) ? actual[i].value : -1;

		// Now we know its a slot label... check which:
		switch (actual[i].value - IWORD) {
		case IVERB:  // Verb
			if (n == CAP_VERB || n == CAP_REAL)
				return actual[i].value;
			return -1;
		case IADJ1:  // Adjective #1
			if (vbslot.wtype[0] == n)
				return actual[i].value;
			if (*(s + strlen(s) - 1) != '1' && vbslot.wtype[3] == n)
				return IWORD + IADJ2;
			if (n == CAP_REAL)
				return actual[i].value;
			return -1;
		case INOUN1:  // noun 1
			if (vbslot.wtype[1] == n)
				return actual[i].value;
			if (*(s + strlen(s) - 1) != '1' && vbslot.wtype[4] == n)
				return IWORD + INOUN2;
			if (n == CAP_REAL)
				return actual[i].value;
			return -1;
		case IADJ2: return (vbslot.wtype[3] == n || n == -70) ? actual[i].value : -1;
		case INOUN2: return (vbslot.wtype[4] == n || n == -70) ? actual[i].value : -1;
		default: return -1;  // Nah... Guru instead 8-)
		}
	}
	return -2;  // It was no actual
}
