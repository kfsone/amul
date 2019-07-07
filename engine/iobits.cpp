#include "amulinc.h"

void
iocheck()
{
	REALiocheck();
	addcr = NO;
}

void
REALiocheck()
{
	int		i;
	int32_t t, d, f;
	char *  pt;
	int		p[4];

loopit:
	t = 0;
	if ((ap = (Aport *)Amiga::GetMsg(repbk)) != NULL) {
		t = ap->type;
		OS::Free(ap, sizeof(*amul));
		goto loopit;
	}
	if (t == -'R')
		goto here;
	if ((ap = (Aport *)Amiga::GetMsg(replyPort)) == NULL)
		return;
	ip = 1;
	addcr = YES;
	if (ap->type == MSG_CLOSEING || ap->type == -'R') {
	here:
		me2->helping = me2->helped = me2->following = me2->followed = -1;
		sys(RESETSTART);
		if (MyFlag == ROLE_PLAYER) {
			fopenr("reset.txt");
			do //  Print the Reset Text
			{
				i = fread(block, 1, 800, ifp);
				block[i] = 0;
				tx(block);
			} while (i == 800);
			fclose(ifp);
			ifp = NULL;
			sprintf(spc, "\n%s is resetting ... Saving at %ld.\n\nPlease call back later.\n\n",
					adname, me->score);
			tx(spc);
			pressret();
		}
		ap->from = -'O';
		Amiga::ReplyMsg(ap);
		link = 0;
		quit();
	}
	t = ap->type;
	d = ap->data;
	f = ap->from;
	pt = ap->ptr;
	p[0] = ap->p1;
	p[1] = ap->p2;
	p[2] = ap->p3;
	p[3] = ap->p4;
	if (t == MSG_DAEMON) {
		int32_t p1, p2, p3, p4, v;
		Amiga::ReplyMsg(ap);
		if (MyFlag == ROLE_DAEMON)
			tx("Processing daemon!\n");
		p1 = inoun1;
		p2 = inoun2;
		p3 = wtype[2];
		p4 = wtype[5];
		v = iverb;
		inoun1 = p[0];
		inoun2 = p[1];
		wtype[2] = p[2];
		wtype[5] = p[3];
		ip = 0;
		lang_proc(d, 0);
		inoun1 = p1;
		inoun2 = p2;
		wtype[2] = p3;
		wtype[4] = p4;
		iverb = v;
		ip = 1;
		goto voila;
	}
	if (t == MSG_FORCE)
		strcpy(input, ap->ptr);
	SendIt(MSG_BUSY, NULL, NULL);
	if (f != -1 && (lstat + f)->state == US_CONNECTED)
		Amiga::ReplyMsg(ap);
	else
		OS::Free((char *)ap, (int32_t)sizeof(*amul));
	lockusr(Af);
	// Any messages we receive should wake us up.

	if (me2->flags & PFASLEEP) {
		cure(Af, SPELL_SLEEP);
		sys(IWOKEN);
		i = 1;
	} else
		i = 0;

	if (t == MSG_SUMMONED) {
		if (d != me2->room) {
			sys(BEENSUMND);
			if (lit(me2->room) == YES && !(me2->flags & PFINVIS) &&
				!(me2->flags & PFSPELL_INVISIBLE))
				action(acp(SUMVANISH), AOTHERS);
			moveto(d);
			if (lit(me2->room) == YES && !(me2->flags & PFINVIS) &&
				!(me2->flags & PFSPELL_INVISIBLE))
				action(acp(SUMARRIVE), AOTHERS);
		}
		i = 0; // wake in transit.
		goto endlok;
	}
	if (t == MSG_DIE) {
		akillme();
		goto endlok;
	}
	if (t == MSG_EXECUTE) {
		tt.condition = 0;
		act(d, (int32_t *)&p[0]);
	}
	if (t == MSG_FORCE) {
		if (d == 0) // 0=forced, 1=follow
			txs("--+ You are forced to \"%s\" +--\n", input);
		else {
			sprintf(block, "You follow %s %s...\n", (usr + f)->name, input);
			tx(block);
			fol = 1;
		}
		forced = d + 1;
	}
	if (t == MSG_RESET_WARNING) {
		addcr = YES;
		tx(pt);
		goto endlok;
	}
	if (t != MSG_MESSAGE)
		goto endlok;
wait: // Lock my IO so I can read & clear my output buffer
loked:
	addcr = YES;
	tx(ob);
	*ob = 0;
endlok:
	me2->IOlock = -1;
	if (i == 1 && !IamINVIS && !(me2->flags & PFSPELL_INVISIBLE))
		action(acp(WOKEN), AOTHERS);
voila:
	ip = 0;
	SendIt(MSG_FREE, NULL, NULL);
	goto loopit; // Check for further messages
}

void
lockusr(int u)
{
	int32_t t, d, p;
	do {
		t = At;
		d = Ad;
		p = (int32_t)Ap;
		SendIt(MSG_LOCK, u, NULL);
		if (Ad != u && ip == 0) {
			iocheck();
			Ad = -1;
		}
	} while (Ad != u);
	At = t;
	Ad = d;
	Ap = (char *)p;
}

int
ioproc(const char *s)
{
	char *p = ow;

lp:
	if (*s == 0) {
		*p = 0;
		return p - ow;
	}
	if ((*p = *(s++)) == '@' && esc(s, p) != 0) {
		p += strlen(p);
		s += 2;
	}
	p++;
	goto lp;
}

int esc(const char *p, char *s) // Find @ escape sequences
{
	char c;

	c = tolower(*(p + 1));
	switch (tolower(*p)) {
	case 'm':
		switch (c) {
		case 'e': strcpy(s, me->name); return 1;
		case '!': sprintf(s, "%-21s", me->name); return 1;
		case 'r': PutRankInto(s); return 1;
		case 'f':
			if (me2->following == -1)
				strcpy(s, "no-one");
			else
				strcpy(s, (usr + me2->following)->name);
			return 1;
		case 'g': sprintf(s, "%ld", me2->magicpts); return 1;
		default: return 0;
		}
	case 'g':
		switch (c) {
		case 'n': strcpy(s, (me->sex == 0) ? "male" : "female"); return 1;
		case 'e': strcpy(s, (me->sex == 0) ? "he" : "she"); return 1;
		case 'o': strcpy(s, (me->sex == 0) ? "his" : "her"); return 1;
		case 'h': strcpy(s, (me->sex == 0) ? "him" : "her"); return 1;
		case 'p': sprintf(s, "%ld", me->plays); return 1;
		}
	case 's':
		if (c == 'c') {
			sprintf(s, "%ld", me->score);
			return 1;
		}
		if (c == 'g') {
			sprintf(s, "%ld", me2->sctg);
			return 1;
		}
		if (c == 'r') {
			sprintf(s, "%ld", me2->strength);
			return 1;
		}
		if (c == 't') {
			sprintf(s, "%ld", me2->stamina);
			return 1;
		}
		return 0;
	case 'v':
		if (c == 'b') {
			strcpy(s, (vbtab + overb)->id);
			return 1;
		}
		if (c == 'e') {
			strcpy(s, (vbtab + iverb)->id);
			return 1;
		}
		if (c == '1' && inoun1 >= 0 && wtype[2] == TC_NOUN) {
			sprintf(s, "%ld", scaled(State(inoun1)->value, State(inoun1)->flags));
			return 1;
		}
		if (c == '2' && inoun2 >= 0 && wtype[5] == TC_NOUN) {
			sprintf(s, "%ld", scaled(State(inoun2)->value, State(inoun2)->flags));
			return 1;
		}
	case 'w':
		if (c == '1' && inoun1 >= 0 && wtype[2] == TC_NOUN) {
			sprintf(s, "%ldg",
					((obtab + inoun1)->states + (int32_t)(obtab + inoun1)->state)->weight);
			return 1;
		}
		if (c == '2' && inoun2 >= 0 && wtype[5] == TC_NOUN) {
			sprintf(s, "%ldg",
					((obtab + inoun2)->states + (int32_t)(obtab + inoun2)->state)->weight);
			return 1;
		}
		if (c == 'i') {
			sprintf(s, "%ld", me2->wisdom);
			return 1;
		}
	case 'n':
		if (c == '1' && inoun1 >= 0 && wtype[2] == TC_NOUN) {
			strcpy(s, (obtab + inoun1)->id);
			return 1;
		}
		if (c == '1' && wtype[2] == TC_TEXT) {
			strcpy(s, (char *)inoun1);
			return 1;
		}
		if (c == '1' && inoun1 >= 0 && wtype[2] == TC_PLAYER) {
			strcpy(s, (usr + inoun1)->name);
			return 1;
		}
		if (c == '2' && inoun2 >= 0 && wtype[5] == TC_NOUN) {
			strcpy(s, (obtab + inoun2)->id);
			return 1;
		}
		if (c == '2' && wtype[5] == TC_TEXT) {
			strcpy(s, (char *)inoun2);
			return 1;
		}
		if (c == '2' && inoun2 >= 0 && wtype[5] == TC_PLAYER) {
			strcpy(s, (usr + inoun2)->name);
			return 1;
		}
		strcpy(s, "something");
		return 1;
	case 'e':
		if (c == 'x') {
			sprintf(s, "%ld", me->experience);
			return 1;
		}
	case 'l':
		if (c == 'r') {
			strcpy(s, lastres);
			return 1;
		}
		if (c == 'c') {
			strcpy(s, lastcrt);
			return 1;
		}
	case 'p':
		if (c == 'l') {
			strcpy(s, (usr + me2->fighting)->name);
			return 1;
		}
		if (c == 'w') {
			strcpy(s, me->passwd);
			return 1;
		}
		if (isdigit(c)) {
			fwait(c - '0');
			return 1;
		}
	case 'r':
		if (c == 'e') {
			timeto(s, *rescnt);
			return 1;
		}
	case 'h': // The person helping you
		if (c == 'e' && me2->helped != -1) {
			strcpy(s, (usr + me2->helped)->name);
			return 1;
		}
	case 'f': // <friend> - person you are helping
		if (c == 'r' && me2->helping != -1) {
			strcpy(s, (usr + me2->helping)->name);
			return 1;
		}
		if (c == 'm' && me2->followed != -1) {
			strcpy(s, (usr + me2->followed)->name);
			return 1;
		}
		strcpy(s, "no-one");
		return 1;
	case 'o':
		if (c == '1' && me2->wield != -1) {
			strcpy(s, (obtab + (me2->wield))->id);
			return 1;
		}
		if (c == '2' && (lstat + (me2->fighting))->wield != -1) {
			strcpy(s, (obtab + ((lstat + (me2->fighting))->wield))->id);
			return 1;
		}
		strcpy(s, "bare hands");
		return 1;
	case 'x':
		if (c == 'x')
			strcpy(s, mxx);
		if (c == 'y')
			strcpy(s, mxy);
		return 1;
	default: return 0;
	}
}

void
interact(int msg, int n, int d)
{
	if ((lstat + n)->state < US_CONNECTED)
		return;
	lockusr(n);
	if (msg == MSG_MESSAGE)
		strcat((lstat + n)->buf, ow);
	if ((intam = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
		memfail("comms port");
	IAm->mn_Length = (UWORD)sizeof(*amul);
	IAf = Af;
	IAm->mn_Node.ln_Type = NT_MESSAGE;
	IAm->mn_ReplyPort = repbk;
	IAt = msg;
	IAd = d;
	(lstat + n)->IOlock = -1;
	Amiga::PutMsg((lstat + n)->rep, intam);
}

void
sendex(int n, int d, int p1, int p2, int p3, int p4)
{
	if ((lstat + n)->state < US_CONNECTED)
		return;
	lockusr(n);
	if ((intam = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
		memfail("comms port");
	IAm->mn_Length = (UWORD)sizeof(*amul);
	IAf = Af;
	IAm->mn_Node.ln_Type = NT_MESSAGE;
	IAm->mn_ReplyPort = repbk;
	IAt = MSG_EXECUTE;
	IAd = -(1 + d);
	intam->p1 = p1;
	intam->p2 = p2;
	intam->p3 = p3;
	intam->p4 = p4;
	(lstat + n)->IOlock = -1;
	Amiga::PutMsg((lstat + n)->rep, intam);
}

void
ans(const char *s)
{
	if (me->flags & UF_ANSI)
		txs("[%s", s);
}

void
putc(char c)
{
	putchar(c);
}

void
tx(const char *s)
{
	int   i, l;
	char *p, *ls, *lp;

	if (addcr == YES && needcr == YES)
		txc('\n');
	addcr = NO;
	needcr = NO;

	ioproc(s);
	s = ow;
	l = 0;
	while (*s != 0) {
		p = spc;
		i = 0;
		ls = lp = NULL;
		do {
			if (*s == '\n')
				break;
			if (i < 79 && (*s == 9 || *s == 32)) {
				ls = s;
				lp = p;
			}
			*(p++) = *(s++);
			i++;
		} while (*s != 0 && *s != '\n' && (me->llen < 8 || i < (me->llen - 1)) && *s != 12);

		if (i > 0)
			needcr = YES;
		if (((me->llen - 1) >= 8 && i == (me->llen - 1)) && *s != '\n') {
			if (*s == ' ' || *s == 9)
				s++;
			else if (*s != 0 && ls != NULL) {
				s = ls + 1;
				p = lp + 1;
			}
			*(p++) = '\n';
			needcr = NO;
		}
		if (*s == '\n') {
			*(p++) = '\n';
			s++;
			needcr = NO;
		}
		*p = 0;
		putc(spc);
		l++;
		if (me->slen > 0 && l >= (me->slen) && *s != 12) {
			pressret();
			l = 0;
		}
		if (*s == 12) {
			s++;
			l = 0;
		}
	}
}

void
utx(int n, const char *s)
{
	ioproc(s);
	if (n == Af)
		tx(s);
	else
		interact(MSG_MESSAGE, n, -1);
}

void
utxn(int plyr, const char *format, int n)
{
	sprintf(str, format, n);
	utx(plyr, str);
}

void
txc(char c)
{
	putchar(c);
	if (c == '\n') {
		needcr = NO;
	} else
		needcr = YES;
}

void
txn(const char *format, int n)
{
	sprintf(str, format, n);
	tx(str);
}

void
txs(const char *format, const char *s)
{
	sprintf(str, format, s);
	tx(str);
}

// Get to str, and max length l
void
Inp(char *s, int l)
{
	char *p;
	int   c;

	p = s;
	*p = c = 0;
	forced = 0;
	do {
		if (ip == 0)
			iocheck();
		if (forced != 0)
			return;
		c = c & 255;
		if (c == NULL)
			continue;
		if (l == NULL)
			return;
		if (c == 8) {
			if (p > s) {
				txc(8);
				txc(32);
				txc(8);
				*(--p) = 0;
			}
			continue;
		}
		if (c == 10 || c == 13) {
			c = '\n';
			*(p++) = 0;
			txc((char)c);
			continue;
		}
		if (c == 27 || c > 0 && c < 23 || c == me->rchar) {
			txc('\n');
			tx((rktab + me->rank)->prompt);
			tx(s);
			continue;
		}
		if (c == 24 || c == 23) {
			while (p != s) {
				txc(8);
				txc(32);
				txc(8);
				p--;
			}
			*p = 0;
			continue;
		}
		if (c < 32 || c > 127)
			continue;
		if (p >= s + l - 1)
			continue;
		*(p++) = (char)c;
		*p = 0;
		txc((char)c);
		needcr = YES;
	} while (c != '\n');
	if (isspace(*(s + strlen(s) - 1)))
		*(s + strlen(s) - 1) = 0;
	needcr = NO;
	if (ip == 0)
		iocheck();
}
