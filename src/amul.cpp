/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL.C.......Adventure System      ****
              ****               Main Program!              ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.

*/

#define AMUL 1
#define FRAME 1
#define PORTS 1

#include <random>

#include "h/amulinc.h" /* Main Include file */
#include "h/amul.cons.h"   /* Predefined Constants etc */
#include "h/amul.gcfg.h"
#include "h/amul.version.h"  /* Version info etc. */
#include "h/amul.vars.h" /* all INTERNAL variables	*/
#include "h/msgports.h"

thread_local bool debug;
thread_local char      llen;

/* Frame specific variables */
thread_local char                serop, MyFlag;                         /* SerPort open? What am I? */
thread_local char *              input;                                 /* 400 bytes, 5 lines */
thread_local char                str[800], spc[200], mxx[40], mxy[60];  /* Output string */
thread_local char                iosup;                                 /* What kind of IO support */
thread_local bool failed, addcr, needcr;
thread_local char                inc, forced, died, fol; /* For parsers use */
thread_local char                actor, last_him, last_her;             /* People we talked about */
thread_local char                autoexits;                     /* General flags */
thread_local long                iverb, overb, iadj1, inoun1, iprep, iadj2, inoun2, lverb, ldir, lroom;
thread_local long                wtype[6], word, mins;             /* Type of word... */
thread_local uint32_t *rescnt;                           /* Reset counter from AMan */
thread_local short int           donev, skip;                      /* No. of vb's/TT's done */
thread_local char                exeunt, more, link;               /* If we've linked yet */
thread_local long                ml, donet, it;                    /* Maximum lines */
thread_local Aport *      ap, *amanp, *intam;               /* The message pointers */
thread_local MsgPort *    amanrep;                          /* AMAN reply port */
thread_local char *              ob, *gp, *ow, *lastres, *lastcrt; /* Pointer to output buffers etc */
thread_local short int           rset, rclr, ip, csyn;             /* Masks for Room Counter */

void
PutARankInto(char *s, int x)
{
    you = (usr + x);
    you2 = (linestat + x);

    if (you2->pre[0] != 0) {
        char *p = you2->pre;
        while (*p != 0)
            *(s++) = *(p++);
        *(s++) = ' ';
    }
    char *p = (you->gender == 0) ? g_game.m_ranks[you->rank].male : g_game.m_ranks[you->rank].female;
    while (*p != 0)
        *(s++) = *(p++);
    if (you2->post[0] != 0) {
        p = you2->pre;
        p = you2->post;
        *(s++) = ' ';
        while (*p != 0)
            *(s++) = *(p++);
    }
    *s = 0;
}

void
PutRankInto(char *s)
{
	PutARankInto(s, Af);
}

/****** AMUL3.C/scaled ******************************************
 *
 *   NAME
 *	scaled -- rehash object value using time-scaling!
 *
 *   SYNOPSIS
 *	Actual = scaled( Value )
 *
 *	int scaled( int );
 *
 *   FUNCTION
 *	Recalculates an object value based on AMUL Time and Rank scaling.
 *	Rather than a straight forward scaling basd on RScale and TScale,
 *	you have to calculate what percentage of each of these to use! A
 *	player of rank 1 in the last minute of game play will find objects
 *	worth their full value. The formula is quite simple, but doing %ages
 *	in 'C' always looks messy.
 *
 *   INPUTS
 *	Value  - the value to be scaled
 *
 *   RESULT
 *	Actual - the actual CURRENT value
 *
 *   NOTES
 *	Note: rscale is based on %age of total ranks achieved and tscale is
 *	      based on %age of game-time remaining.
 *
 ******************************************************************************
 *
 */

// Scale an object value
long
scaled(long value, short int flags)
{
    if (!(flags & SF_SCALED))
        return value;

    // The longer the game runs, the more valuable objects should become.
    auto gameDuration = std::max(double(*rescnt), double(mins) * 60.);
    auto timeFactor   = double(*rescnt) / gameDuration;
    auto timeAdjust   = timeFactor * double(g_game.timeScale) / 100.;

    // The higher a player's rank, the less points they can get purely from items.
    auto maxRank      = double(g_game.numRanks - 1);
    auto rankFactor   = double(me->rank) / maxRank;
    auto rankAdjust   = rankFactor * double(g_game.rankScale) / 100.;

    auto adjusted     = double(value) * timeAdjust;
    adjusted -= adjusted * rankAdjust;

    return long(adjusted);
}

void
REALiocheck()
{
    int   i;
    long  d, f;
    char *pt;
    int   p[4];

loopit:
    long t = 0;
    if ((ap = (struct Aport *)GetMsg((struct MsgPort *)reply)) == nullptr)
        return;
    ip = 1;
    addcr = true;
    if (ap->type == MCLOSEING || ap->type == -'R') {
    here:
        me2->helping = me2->helped = me2->following = me2->followed = -1;
        sys(RESETSTART);
        if (MyFlag == am_USER) {
            fopenr("reset.txt");
			//  Print the Reset Text
            do
            {
                i = fread(block, 1, 800, ifp);
                block[i] = 0;
                tx(block);
            } while (i == 800);
            fclose(ifp);
            ifp = nullptr;
            sprintf(spc, "\n%s is resetting ... Saving at %ld.\n\nPlease call back later.\n\n",
                    adname, me->score);
            tx(spc);
            pressret();
        }
        ap->from = -'O';
        ReplyMsg((struct Message *)ap);
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
    if (t == MDAEMON) {
        long p1, p2, p3, p4, v;
        ReplyMsg((struct Message *)ap);
        if (MyFlag == am_DAEM)
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
    if (t == MFORCE)
        strcpy(input, ap->ptr);
    SendIt(MBUSY, nullptr, nullptr);
    if (f != -1 && (linestat + f)->state == PLAYING)
        ReplyMsg((struct Message *)ap);
    else
        ReleaseMem(&ap);
    lockusr(Af);
    /* Any messages we receive should wake us up. */

    if (me2->flags & PFASLEEP) {
        cure(Af, SSLEEP);
        sys(IWOKEN);
        i = 1;
    } else
        i = 0;

    if (t == MSUMMONED) {
        if (d != me2->room) {
            sys(BEENSUMND);
            if (lit(me2->room) && !(me2->flags & PFINVIS) && !(me2->flags & PFSINVIS))
                action(acp(SUMVANISH), AOTHERS);
            moveto(d);
            if (lit(me2->room) && !(me2->flags & PFINVIS) && !(me2->flags & PFSINVIS))
                action(acp(SUMARRIVE), AOTHERS);
        }
        i = 0; /* wake in transit. */
        goto endlok;
    }
    if (t == MDIE) {
        akillme();
        goto endlok;
    }
    if (t == MEXECUTE) {
        tt.condition = 0;
        act(d, (long *)&p[0]);
    }
    if (t == MFORCE) {
        if (d == 0) /* 0=forced, 1=follow */
            txs("--+ You are forced to \"%s\" +--\n", input);
        else {
            sprintf(block, "You follow %s %s...\n", (usr + f)->name, input);
            tx(block);
            fol = 1;
        }
        forced = d + 1;
    }
    if (t == MRWARN) {
        addcr = true;
        tx(pt);
        goto endlok;
    }
    if (t != MMESSAGE)
        goto endlok;
wait: /* Lock my IO so I can read & clear my output buffer */
loked:
    addcr = true;
    tx(ob);
    *ob = 0;
endlok:
    me2->IOlock = -1;
    if (i == 1 && !IamINVIS && !(me2->flags & PFSINVIS))
        action(acp(WOKEN), AOTHERS);
voila:
    ip = 0;
    SendIt(MFREE, nullptr, nullptr);
    goto loopit; /* Check for further messages */
}

void
iocheck()
{
    REALiocheck();
    addcr = false;
}

///TODO: This is not ok
void
fwait(long n)
{
    if (n < 1)
        n = 1;
    for (int i = 0; i < 7; i++) {
        Delay(n * 7);
        iocheck();
    }
}

void
timeto(char *s, long secs)
{
    if (secs >= 3600) /* More than an hour */
    {
        int x = secs / 3600;       /* Hours */
        int y = secs - (x * 3600); /* Minutes & seconds */
        if (y < 60)            /* Upto 1 minute? */
        {
            sprintf(s, "%ld %s, %ld %s", x, (x > 1) ? "hours" : "hour", y,
                    (y > 1) ? "seconds" : "second");
            return;
        }
        y = y / 60;
        sprintf(s, "%ld %s and %ld %s", x, (x > 1) ? "hours" : "hour", y,
                (y > 1) ? "minutes" : "minute");
        return;
    }
    int x = secs / 60;
    int y = secs - (x * 60);
    if (x == 0 && y == 0)
        strcpy(s, "now");
    if (x != 0 && y == 0)
        sprintf(s, "%ld %s", x, (x > 1) ? "minutes" : "minute");
    if (x == 0 && y != 0)
        sprintf(s, "%ld %s", y, (y > 1) ? "seconds" : "second");
    if (x != 0 && y != 0)
        sprintf(s, "%ld %s and %ld %s", x, (x > 1) ? "minutes" : "minute", y,
                (y > 1) ? "seconds" : "second");
}

/* Find @ escape sequences */
int
esc(char *p, char *s)
{
    char c = tolower(*(p + 1));
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
        case 'g': sprintf(s, "%d", int(me2->magicpts)); return 1;
        default: return 0;
        }
    case 'g':
        switch (c) {
        case 'n': strcpy(s, (me->gender == 0) ? "male" : "female"); return 1;
        case 'e': strcpy(s, (me->gender == 0) ? "he" : "she"); return 1;
        case 'o': strcpy(s, (me->gender == 0) ? "his" : "her"); return 1;
        case 'h': strcpy(s, (me->gender == 0) ? "him" : "her"); return 1;
        case 'p': sprintf(s, "%d", int(me->plays)); return 1;
        }
    case 's':
        if (c == 'c') {
            sprintf(s, "%d", int(me->score));
            return 1;
        }
        if (c == 'g') {
            sprintf(s, "%d", int(me2->sctg));
            return 1;
        }
        if (c == 'r') {
            sprintf(s, "%d", int(me2->strength));
            return 1;
        }
        if (c == 't') {
            sprintf(s, "%d", int(me2->stamina));
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
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            const auto &state = g_game.m_objects[inoun1].State();
            sprintf(s, "%ld", scaled(state.value, state.flags));
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            const auto &state = g_game.m_objects[inoun2].State();
            sprintf(s, "%ld", scaled(state.value, state.flags));
            return 1;
        }
    case 'w':
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            const auto &state = g_game.m_objects[inoun1].State();
            sprintf(s, "%dg", state.weight);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            const auto &state = g_game.m_objects[inoun2].State();
            sprintf(s, "%dg", state.weight);
            return 1;
        }
        if (c == 'i') {
            sprintf(s, "%ld", me2->wisdom);
            return 1;
        }
    case 'n':
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            strcpy(s, GetObject(inoun1).id);
            return 1;
        }
        if (c == '1' && wtype[2] == WTEXT) {
            strcpy(s, (char *)inoun1);
            return 1;
        }
        if (c == '1' && inoun1 >= 0 && wtype[2] == WPLAYER) {
            strcpy(s, (usr + inoun1)->name);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            strcpy(s, GetObject(inoun2).id);
            return 1;
        }
        if (c == '2' && wtype[5] == WTEXT) {
            strcpy(s, (char *)inoun2);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WPLAYER) {
            strcpy(s, (usr + inoun2)->name);
            return 1;
        }
        strcpy(s, "something");
        return 1;
    case 'e':
        if (c == 'x') {
            sprintf(s, "%d", me->experience);
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
    case 'h': /* The person helping you */
        if (c == 'e' && me2->helped != -1) {
            strcpy(s, (usr + me2->helped)->name);
            return 1;
        }
    case 'f': /* <friend> - person you are helping */
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
            strcpy(s, GetObject(me2->wield).did);
            return 1;
        }
        auto fighting = linestat + me2->fighting;
        if (c == '2' && fighting->wield != -1) {
            strcpy(s, GetObject(fighting->wield).id);
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
ioproc(char *s)
{
    char *p = ow;
    for ( ;; ) {
        if (*s == 0) {
            *p = 0;
            break;
        }
        if ((*p = *(s++)) == '@' && esc(s, p) != 0) {
            p += strlen(p);
            s += 2;
        } else if (*(p++) == '\n')
            *(p++) = '\r';
    }
}

void
txc(char c)
{
    switch (iosup) {
    case 0: putchar(c); return;
    case LOGFILE: return;
    }
    if (c == '\n') {
        txc('\r');
        needcr = false;
    } else
        needcr = true;
}

void
tx(char *s)
{
    int   i, l;
    char *p, *ls, *lp;

    if (iosup == LOGFILE)
        return;
    if (addcr && needcr)
        txc('\n');
    addcr = false;
    needcr = false;
    if (iosup == 0) {
        printf("%s", s);
        return;
    }

    ioproc(s);
    s = ow;
    l = 0;
    while (*s != 0) {
        p = spc;
        i = 0;
        ls = lp = nullptr;
        do {
            if (*s == '\n')
                break;
            if (i < 79 && (*s == 9 || *s == 32)) {
                ls = s;
                lp = p;
            }
            if (*s == '\r') {
                s++;
                continue;
            }
            *(p++) = *(s++);
            i++;
        } while (*s != 0 && *s != '\n' && (me->llen < 8 || i < (me->llen - 1)) && *s != 12);

        if (i > 0)
            needcr = true;
        if (((me->llen - 1) >= 8 && i == (me->llen - 1)) && *s != '\n') {
            if (*s == ' ' || *s == 9)
                s++;
            else if (*s != 0 && ls != nullptr) {
                s = ls + 1;
                p = lp + 1;
            }
            if (iosup == SERIO)
                *(p++) = '\r';
            *(p++) = '\n';
            needcr = false;
        }
        if (*s == '\n') {
            if (iosup == SERIO)
                *(p++) = '\r';
            *(p++) = '\n';
            s++;
            needcr = false;
        }
        *p = 0;
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

/****** AMUL3.C/ainteract ******************************************
 *
 *   NAME
 *	ainteract -- mask player out from 'action'/'announce' messages
 *
 *   SYNOPSIS
 *	ainteract( Player )
 *
 *	void ainteract( short int );
 *
 *   FUNCTION
 *	Masks a player as currently being 'interacted' with. This currently
 *	only affects messageing, ie ACTION and ANNOUNCE (and their
 *	derivatives) don't reach the other player. For example, if you
 *	give something to someone nearby, and want to tell the OTHER
 *	players in the room. In the future this may be used as a mini-locking
 *	system, for example to prevent a player leaving a room halfway through
 *	someone giving him something, or as someone attacks him.
 *
 *   INPUTS
 *	Player - number of a player online
 *
 *   NOTES
 *	PLEASE use this whenever you ARE interacting with a player, and
 *	you have a few lines of Lang.Txt to go... In future this will
 *	prevent ALL sorts of things, eg the player logging out three-quarters
 *	of the way through your action.
 *
 ******************************************************************************
 *
 */

void
ainteract(int who)
{
    actor = -1;
    if ((linestat + who)->state == PLAYING)
    	actor = who;
}

void
utx(int n, char *s)
{
    ioproc(s);
    if (n == Af)
        tx(s);
    else
        interact(MMESSAGE, n, -1);
}

void
utxn(int plyr, char *format, int n)
{
    sprintf(str, format, n);
    utx(plyr, str);
}

void
txn(char *format, int n)
{
    sprintf(str, format, n);
    tx(str);
}

void
txs(char *format, char *s)
{
    sprintf(str, format, s);
    tx(str);
}

/* Get to str, and max length l */
void
Inp(char *s, int l)
{
    char *p = s;
    int c = *p = 0;
    forced = 0;
    do {
        if (ip == 0)
            iocheck();
        if (forced != 0)
            return;
        if (iosup == LOGFILE)
            return '\n';
        if (ip == 0)
            iocheck();
        if (forced != 0)
            return;
        c = c & 255;
        if (c == nullptr)
            continue;
        if (l == nullptr)
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
        needcr = true;
    } while (c != '\n');
    if (isspace(*(s + strlen(s) - 1)))
        *(s + strlen(s) - 1) = 0;
    needcr = false;
    if (ip == 0)
        iocheck();
}

void
ans(char *s)
{
    if (me->flags & ufANSI)
        txs("[%s", s);
}

void
asave()
{
    save_me();
    txn(acp(SAVED), me->score);
}

// Update my record.
void
save_me()
{
    if (me->score < 0)
        me->score = 0;
    fopena("Players Data");
    fseek(afp, me2->rec * sizeof(*me), 0);
    fwrite(me->name, sizeof(*me), 1, afp);
    fclose(afp);
    afp = nullptr;
}

void
aquit()
{
    sys(REALLYQUIT);
    block[0] = 0;
    Inp(block, 4);
    if (toupper(block[0]) == 'Y') {
        nohelp();
        exeunt = 1;
        save_me();
        donet = ml + 1;
    }
}

/*== This must abide FULLY by INVIS & INVIS2... Should we make it so that
     visible players can't see an invisible players entry, they can just
     see that they're here?						  */
void
whohere()
{
    if (!lit(me2->room))
        return;
    if (((rmtab + me2->room)->flags & HIDE) == HIDE && me->rank != ranks - 1) {
        sys(WHO_HIDE);
        return;
    }
    for (int i = 0; i < MAXU; i++) {
        if (i != Af && canSee(Af, i) && !((linestat + i)->flags & PFMOVING)) {
            PutARankInto(str, i);
            sprintf(block, acp(ISHERE), (usr + i)->name, str);
            if (((linestat + i)->flags & PFSITTING) != 0)
                strcat(block, ", sitting down");
            if (((linestat + i)->flags & PFLYING) != 0)
                strcat(block, ", lying down");
            if (((linestat + i)->flags & PFASLEEP) != 0)
                strcat(block, ", asleep");
            if ((linestat + i)->numobj == 0) {
                strcat(block, ".\n");
                tx(block);
            } else {
                strcat(block, " ");
                invent(i);
            }
        }
    }
}

void
awho(int type)
{
    if (type == TYPEV) {
        for (int i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (linestat + i)->state > 1 &&
                (!((linestat + i)->flags & PFSINVIS))) {
                str[0] = 0;
                if (isPINVIS(i)) {
                    str[0] = '(';
                    str[1] = 0;
                }
                strcat(str, (usr + i)->name);
                strcat(str, " the ");
                PutARankInto(str + strlen(str), i);
                strcat(str, acp(ISPLAYING));
                if (i == Af)
                    strcat(str, " (you)");
                if (isPINVIS(i))
                    strcat(str, ").\n");
                else
                    strcat(str, ".\n");
                tx(str);
            }
    } else {
        int j = 0;
        str[0] = 0;
        for (int i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (linestat + i)->state > 1 &&
                (!((linestat + i)->flags & PFSINVIS))) {
                if (i != Af) {
                    if (j++ != 0)
                        strcat(str, ", ");
                    if (isPINVIS(i))
                        sprintf(spc, "(%s)", (usr + i)->name);
                    else
                        sprintf(spc, "%s", (usr + i)->name);
                    strcat(str, spc);
                }
            }
        if (j == 0)
            tx("Just you.\n");
        else
            txs("%s and you!\n", str);
    }
}

// Get the users flag bits
void
flagbits()
{
    me->llen = DLLEN;
    me->slen = DSLEN;
    me->rchar = DRCHAR;
    sprintf(spc,
            "Default settings are:\n\nScreen width = %ld, Lines = %ld, Redo = '%c', Flags = ANSI: "
            "%s, Add LF: %s\n\nChange settings (Y/n): ",
            (me->llen), (me->slen), me->rchar, (me->flags & ufANSI) ? "On" : "Off",
            (me->flags & ufCRLF) ? "On" : "Off");
    tx(spc);
    Inp(str, 2);
    if (toupper(str[0]) == 'N')
        return;

    getllen();
    getslen();
    getrchar();
    getflags();
    save_me();
}

void
getllen()
{
    sprintf(input, "%ld %s", me->llen, "characters");
    sprintf(str, "\nEnter %s%s[%s]: ", "screen width", " ", input);
    tx(str);
    Inp(str, 4);
    if (str[0] != 0)
        me->llen = atoi(str);
    sprintf(input, "%ld %s", me->llen, "characters");
    sprintf(str, "%s set to %s.\n", "screen width", input);
    tx(str);
}

void
getslen()
{
    sprintf(input, "%ld %s", me->slen, "lines");
    sprintf(str, "\nEnter %s%s[%s]: ", "screen length", " (0 to disable MORE? prompting) ", input);
    tx(str);
    Inp(str, 3);
    if (str[0] != 0)
        me->slen = atoi(str);
    sprintf(input, "%ld %s", me->slen, "lines");
    sprintf(str, "%s set to %s.\n", "screen length", input);
    tx(str);
}

void
getrchar()
{
    char rchar = me->rchar;
    me->rchar = 0;
    sprintf(input, "currently \"%c\"", rchar);
    sprintf(str, "\nEnter %s%s[%s]: ", "redo-character", " ", input);
    tx(str);
    Inp(str, 2);
    if (str[0] != 0)
        me->rchar = str[0];
    else
        me->rchar = rchar;
    if (me->rchar == '/' || isspace(str[0])) {
        tx("Invalid redo-character (how do you expect to do / commands?)\n");
        me->rchar = rchar;
        return;
    }
    sprintf(input, "\"%c\"", me->rchar);
    sprintf(str, "%s set to %s.\n", "redo-character", input);
    tx(str);
}

void
getflags()
{
    tx("Follow CR with a Line Feed? ");
    if (me->flags & ufCRLF)
        tx("[Y/n]: ");
    else
        tx("[y/N]: ");
    Inp(str, 2);
    if (toupper(str[0]) == 'Y' || toupper(str[0]) == 'N')
        me->flags = me->flags & (toupper(str[0]) == 'Y') ? ufCRLF : -(1 + ufCRLF);
    tx("Use ANSI control codes?     ");
    if (me->flags & ufANSI)
        tx("[Y/n]: ");
    else
        tx("[y/N]: ");
    Inp(str, 2);
    if (toupper(str[0]) == 'Y' || toupper(str[0]) == 'N')
        me->flags = me->flags & (toupper(str[0]) == 'Y') ? ufANSI : -(1 + ufANSI);
}

/*============= Please adhere to AutoDoc style docs herewith ================*/

/*===========================================================================*
 *
 * The following functions are low-level, and considered fairly constant.
 * However, some of them (or all of them) may be moved to the AMUL.Library
 * at a later date...
 *
 *===========================================================================*/

/****** amul.library/isValidNumber ******************************************
 *
 *   NAME
 *	isValidNumber -- mathematically compare two numbers, including <> and =.
 *
 *   SYNOPSIS
 *	ret = isValidNumber( Number, Value )
 *	d0            d0      d1
 *
 *	BOOLEAN isValidNumber( ULONG, ULONG );
 *
 *   FUNCTION
 *	Quantifies and/or equates the value of two numbers.
 *
 *   INPUTS
 *	Number - Real integer numeric value.
 *	Value  - Integer value with optional quantifier (MORE/LESS) ie
 *	         '<' or '>'.
 *
 *   RESULT
 *	ret    - TRUE if Number equates with Value (ie 5 IS <10)
 *
 *   EXAMPLE
 *	isValidNumber(10, ( LESS & 20 ));	Returns TRUE.
 *    Lang.Txt:
 *	numb ?brick >%wall
 *
 *   NOTES
 *	Remember to process the values using ACTUAL before passing them,
 *	thus: Numb(actual(n1), actual(n2));
 *
 ******************************************************************************/

bool
isValidNumber(long x, long n)
{
    if (n == x) {
        return true;
    }
    if ((n & MORE) == MORE) {
        n = n - MORE;
        if (n > x)
            return true;
    }
    if ((n & LESS) == LESS) {
        n = n - LESS;
        if (n < x)
            return true;
    }
    return false;
}

/*===========================================================================*
 *
 * The following functions are considered low-level enough to stay unchanged.
 * The will NOT be made into library calls, as they operate ONLY on local
 * variables, and are rigid enough to prevent this. Please make sure you do
 * NOT place functions likely to change regularily in here!
 *
 *===========================================================================*/

/****** AMUL3.C/atreatas ******************************************
 *
 *   NAME
 *	atreatas -- switch parseing to another verb.
 *
 *   SYNOPSIS
 *	atreatas( NewVerb )
 *
 *	void atreatas( int );
 *
 *   FUNCTION
 *	Switches current processing to a new verb, keeping the old
 *	syntax (ie iwords).
 *
 *   INPUTS
 *	NewVerb - the number of the new verb to be processed.
 *
 *   RESULT
 *	vbptr   - points to new verbs table entry.
 *	iverb   - set to NewVerb
 *	ml      - set to -(1+verb) to reset the current loop
 *
 *   SEE ALSO
 *	amul2.c/asyntax ado
 *
 ******************************************************************************
 *
 */

void
atreatas(verbid_t verbno)
{
    donet = 0;
    inc = 0;
    if (tt.verb == -1) {
        vbptr = vbtab + verbno;
        ml = -(1 + verbno);
    }
    iverb = verbno;
    iocheck();
}

/****** amul3.c/afailparse ******************************************
 *
 *   NAME
 *	afailparse -- flags the current parse as having failed
 *
 *   SYNOPSIS
 *	void failparse();
 *
 *   FUNCTION
 *	Prevents continued parseing of the current phrase. Any
 *	other phrases will be parsed, but this won't. Used for
 *	looped-parses caused by such as 'All' and noun-classes.
 *
 *   SEE ALSO
 *	aabortparse, afinishparse, aendparse
 *
 ******************************************************************************
 *
 */

void
afailparse()
{
    donet = ml + 1;
    ml = -1;
    failed = true;
}

/****** amul3.c/afinishparse ******************************************
 *
 *   NAME
 *	afinishparse -- unused
 *
 *   SYNOPSIS
 *
 *   FUNCTION
 *
 *   SEE ALSO
 *	aabortparse, afailparse, aendparse
 *
 ******************************************************************************
 *
 */

void
afinishparse()
{
}

/****** amul3.c/aabortparse ******************************************
 *
 *   NAME
 *	aabortparse -- unused
 *
 *   SYNOPSIS
 *
 *   FUNCTION
 *
 *   SEE ALSO
 *	aabortparse, afailparse, aendparse
 *
 ******************************************************************************
 *
 */

void
aabortparse()
{
    donet = ml + 1;
    more = 0;
}

/****** AMUL3.C/ado ******************************************
 *
 *   NAME
 *	ado -- AMUL's equivalent of GOSUB
 *
 *   SYNOPSIS
 *	ado( Verb )
 *
 *	void ado( int );
 *
 *   FUNCTION
 *	Causes the parser to process <verb> before continuing the current
 *	parse. This works similiarily to GOSUB in Basic, and allows the user
 *	to write sub-routine verbs.
 *
 *   INPUTS
 *	Verb - the verb to be processed.
 *
 *   EXAMPLE
 *	ado("\"travel");	/ * Visits the "travel verb. * /
 *
 *   SEE ALSO
 *	atreatas
 *
 ******************************************************************************
 *
 */

void
ado(verbid_t verb)
{
    // context snapshot
    long old_ml = ml;
    long old_donet = donet;
    long old_verb = iverb;
    iverb = verb;
    long old_ttv = tt.verb;
    long old_rm = me2->room;
    _TT_ENT *old_ttabp = ttabp;
    _VERB_STRUCT *ovbptr = vbptr;
    _SLOTTAB *ostptr = stptr;

    lang_proc(verb, 1);

    iverb = old_verb;

    if (failed == true forced != 0 || died != 0 || exeunt != 0) {
        donet = ml + 1;
        ml = -1;
        failed = true;
        return;
    }

    donet = old_donet;
    ml = old_ml;
    vbptr = vbtab + donet;
    tt.verb = old_ttv;
    roomtab = rmtab + old_rm;
    ttabp = old_ttabp;
    vbptr = ovbptr;
    stptr = ostptr;
}

/****** AMUL3.C/add_obj ******************************************
 *
 *   NAME
 *	add_obj -- Add an object to a players inventory (movement)
 *
 *   SYNOPSIS
 *	add_obj( Player )
 *
 *	void add_obj( int );
 *
 *   FUNCTION
 *	Updates the players statistics and settings to indicate addition
 *	of an object to his inventory. The objects location IS set to
 *	indicate the player (you don't need to move it!). The object must
 *	be pointed to by global variable objtab.
 *
 *   INPUTS
 *	Player - number of the player to give it to.
 *	objtab - must point to the objects structure.
 *
 *   NOTES
 *	If objtab does NOT point to the right object, things will go astray!
 *
 *   SEE ALSO
 *	rem_obj
 *
 ******************************************************************************
 *
 */

// Add an object into a players inventory
void
add_obj(int to)
{
    *objtab->rmlist = -(5 + to);  // It now belongs to the player
    (linestat + to)->numobj++;
    (linestat + to)->weight += STATE->weight;
    const auto &rank = g_ranks[usr[to].rank];
    (linestat + to)->strength -= (rank.strength * STATE->weight) / rank.maxweight;
    if (STATE->flags & SF_LIT)
        (linestat + to)->light++;
}

/****** AMUL3.C/rem_obj ******************************************
 *
 *   NAME
 *	rem_obj -- Remove an object from a players inventory (no move).
 *
 *   SYNOPSIS
 *	rem_obj( Player, Noun )
 *
 *	void rem_obj( int, int );
 *
 *   FUNCTION
 *	Removes an object from the players inventory without changing the
 *	location of the object. Simply all flags pertaining to the posession
 *	of the object or requiring it (eg decreased strength, or if its
 *	wielded) are cleared.
 *
 *   INPUTS
 *	Player - who's inventory its in.
 *	Noun   - the object number.
 *
 *   NOTES
 *	The calling function MUST change the objects location, otherwise
 *	the player will effectively still own the object.
 *
 *   SEE ALSO
 *	add_obj()
 *
 ******************************************************************************
 *
 */

// Remove object from inventory
void
rem_obj(int to, int ob)
{
    (linestat + to)->numobj--;
    (linestat + to)->weight -= STATE->weight;
    const auto &rank = g_ranks[usr[to].rank];
    (linestat + to)->strength += (rank.strength * STATE->weight) / rank.maxweight;
    if (STATE->flags & SF_LIT)
        (linestat + to)->light--;
    if (me2->wield == ob)
        me2->wield = -1; /*== Don't let me wield it */
}

/****** AMUL3.C/asyntax ******************************************
 *
 *   NAME
 *	asyntax -- set new noun1 & noun2, using slot labels too!
 *
 *   SYNOPSIS
 *	asyntax( Noun1, Noun2 )
 *
 *	void asyntax( ulong, ulong );
 *
 *   FUNCTION
 *	Alters the content of inoun1 and inoun2 along with the word-type
 *	slots, thus altering the current input syntax. The actual value
 *	of Noun1 and Noun2 is calculated by asyntax, so effectives and
 *	slot labels should be passed RAW. This allows a call something
 *	like: asyntax( IWORD + INOUN2, IWORD + INOUN1); which is the
 *	equivalent of "- syntax noun2 noun1".
 *
 *   INPUTS
 *	Noun1 - unprocessed item for noun1.
 *	Noun2 - unprocessed item for noun2.
 *
 *   EXAMPLE
 *	asyntax(*(tt.pptr+ncop[tt.condition]), *(tt.pptr+ncop[tt.condition]+1));
 *
 *   NOTES
 *	If noun1 or noun2 are not REAL values, they will be processed here.
 *	Passing a REAL value will assume the passed item was a noun. If you
 *	use asyntax(TP1, TP2) you could EASILY be passing a player number!
 *
 *   SEE ALSO
 *	ado, atreatas
 ******************************************************************************
 *
 */

void
asyntax(int n1, int n2)
{
    unsigned long t1, t2;

    inc = 0;
    /* === N1 Handling === */
    if (n1 == WNONE)
        t1 = n1;
    else if ((n1 & IWORD)) /* Is it an IWORD? */
    {
        switch (n1 & -(1 + IWORD)) {
        case IVERB:
            n1 = iverb;
            t1 = wtype[0];
            break;
        case IADJ1:
            n1 = iadj1;
            t1 = wtype[1];
            break;
        case INOUN1:
            n1 = inoun1;
            t1 = wtype[2];
            break;
        case IPREP:
            n1 = iprep;
            t1 = wtype[3];
            break;
        case IADJ2:
            n1 = iadj2;
            t1 = wtype[4];
            break;
        case INOUN2:
            n1 = inoun2;
            t1 = wtype[5];
            break;
        default:
            n1 = inoun1;
            t1 = wtype[2];
            break;
        }
    } else {
        n1 = isnoun((obtab + n1)->id, (obtab + n1)->adj, (vbtab + iverb)->sort);
        t1 = WNOUN;
    }

    /* === N2 Handling === */
    if (n2 == WNONE)
        t2 = n2;
    else if ((n2 & IWORD)) /* Is it an IWORD? */
    {
        switch (n2 & -(1 + IWORD)) {
        case IVERB:
            n2 = iverb;
            t2 = wtype[0];
            break;
        case IADJ1:
            n2 = iadj1;
            t2 = wtype[1];
            break;
        case INOUN1:
            n2 = inoun1;
            t2 = wtype[2];
            break;
        case IPREP:
            n2 = iprep;
            t2 = wtype[3];
            break;
        case IADJ2:
            n2 = iadj2;
            t2 = wtype[4];
            break;
        case INOUN2:
            n2 = inoun2;
            t2 = wtype[5];
            break;
        default:
            n2 = inoun1;
            t2 = wtype[2];
            break;
        }
    } else {
        n2 = isnoun((obtab + n2)->id, (obtab + n2)->adj, (vbtab + iverb)->sort2);
        t2 = WNOUN;
    }

    inoun1 = n1;
    wtype[2] = t1;
    inoun2 = n2;
    wtype[5] = t2;
    ml = -(iverb + 1);
}

/****** AMUL3.C/iocopy ******************************************
 *
 *   NAME
 *	iocopy -- process and copy a string (restricted length).
 *
 *   SYNOPSIS
 *	iocopy( Dest, Source, MaxLen )
 *
 *	void iocopy( BYTE, BYTE, ULONG );
 *
 *   FUNCTION
 *	Puts the source string through ioproc, causing any escape characters
 *	to be processed, and then copies the output to another string.
 *
 *   INPUTS
 *	Dest   - The target string
 *	Source - The input string (unprocessed)
 *	MaxLen - Maximum number of characters to be copied.
 *
 *   RESULT
 *	Dest   - Remains unchanged.
 *	Source - Contains the processed string, upto MaxLen bytes long.
 *
 *   NOTES
 *	MaxLen does NOT include the NULL byte, always allow for this.
 *
 *   SEE ALSO
 *	frame/IOBits.C:ioproc()
 *
 ******************************************************************************
 *
 */

/* -- Quick copy - used by iocopy and others -- */

void
qcopy(char *p2, char *p, int max)
{
    Forbid();
    for (int i = 0; i < max && *p != 0 && *p != '\n'; i++)
        *(p2++) = *(p++);
    *p2 = 0;
    Permit();
}

void
iocopy(char *Dest, char *Source, unsigned long Max)
{
    ioproc(Source);
    qcopy(Dest, Source, Max);
}

/****** AMUL3.C/DoThis ******************************************
 *
 *   NAME
 *	DoThis -- Tell another player to follow me or perform an action.
 *
 *   SYNOPSIS
 *	DoThis( Player, Command, Type )
 *
 *	void DoThis( SHORT, BYTE, SHORT );
 *
 *   FUNCTION
 *	Forces another player to perfom an action. Type tells the other end
 *	how to cope with this. Used for FORCE and FOLLOW.
 *
 *   INPUTS
 *	Player  - Number of the player to tell.
 *	Command - Pointer to the text to be processed.
 *	Type    - 0 to FORCE player, 1 to make player FOLLOW.
 *
 *   EXAMPLE
 *	DoThis( TP1, "quit", 0 );	/-* Force player to quit *-/
 *	DoThis( TP1, "east", 1 );	/-* Force them to follow you east *-/
 *
 ******************************************************************************
 *
 */

void
DoThis(int x, char *cmd, short int type)
{
    lockusr(x);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = MFORCE;
    IAd = type;
    IAp = cmd;
    PutMsg((linestat + x)->rep, (struct Message *)intam);
    (linestat + x)->IOlock = -1;
}

/****** AMUL3.C/StopFollow ******************************************
 *
 *   NAME
 *	StopFollow -- Stop being a follower.
 *
 *   SYNOPSIS
 *	void StopFollow( void );
 *
 *   FUNCTION
 *	If the current verb (overb) is a Travel verb and we were following
 *	the we can no-longer follow them.
 *
 *   SEE ALSO
 *	LoseFollower(), Follow(), DoThis()
 *
 ******************************************************************************
 *
 */

void
StopFollow()
{
    Forbid();
    if (fol != 0 || me2->following == -1 || (vbtab + overb)->flags & VB_TRAVEL) {
        Permit();
        return;
    }
    if ((linestat + me2->following)->state != PLAYING ||
        (linestat + me2->following)->followed != Af) {
        me2->following = -1;
        Permit();
        return;
    }
    (linestat + me2->following)->followed = -1;
    Permit();
    tx("You are no-longer following @mf.\n");
    me2->following = -1;
}

/****** AMUL3.C/internal ******************************************
 *
 *   NAME
 *	internal -- process internal control command.
 *
 *   SYNOPSIS
 *	internal( Command )
 *
 *	void internal( BYTE );
 *
 *   FUNCTION
 *	Processes an internal-command string pointed to by Command. Options
 *	available are listed below.
 *
 *   INPUTS
 *	Command - points to an ASCIZ string containing a command sequence.
 *		available commands are:
 *			?		Displays available commands.
 *			p [password]	Change users password
 *			lf [on|off]	Toggles linefeeds on/off
 *			ar [on|off]	Toggles auto-redo on/off
 *			r <redo char>	Changes users redo-char
 *			mN <macro>	Changes macro #N (n=1-4)
 *			ansi [on|off]	Toggles ANSI on/off for the user
 *			llen <line len>	Changes users line-length
 *			plen <page len>	Changes users page-length
 *
 ******************************************************************************
 *
 */

void
internal(char *s)
{
    if (*s == '?') {
        tx("AMULEd v0.5 - All commands prefixed with a \"/\"\n\n");
        tx(" /?\tDisplays this list\n");
        tx(" /p\tChange password\n");
        tx(" /lf\tToggle linefeeds on/off\n");
        tx(" /ar\tToggle auto-redo on/off\n");
        tx(" /r\tSet redo-character\n");
        tx(" /mN\tSet macro #N (n=1-4)\n");
        tx(" /an\tToggle ANSI on/off\n");
        tx(" /x\tSet line-length\n");
        tx(" /y\tSet page-length\n\n");
        return;
    }

    char *p = s;
    while (*p != 0) {
        *p = tolower(*p);
        p++;
    }

    if (*s == 'r') {
        getrchar();
        return;
    }
    if (*s == 'x') {
        getllen();
        return;
    }
    if (*s == 'y') {
        getslen();
        return;
    }

    if (*s == 'l') {
        me->flags = me->flags ^ ufCRLF;
        txs("LineFeed follows carriage return %sABLED.\n", (me->flags & ufCRLF) ? "EN" : "DIS");
        return;
    }

    if (*s == 'a') {
        switch (*(s + 1)) {
        case 'n':
            me->flags = me->flags ^ ufANSI;
            ans("1m");
            txs("ANSI control codes now %sABLED.\n", (me->flags & ufANSI) ? "EN" : "DIS");
            ans("0;37m");
            save_me();
            return;
        }
    }

    if (*s == 'p') {
        tx("Enter old password  : ");
        Inp(input, 250);
        if (input[0] == 0) {
            tx("Cancelled.\n");
            return;
        }
        if (stricmp(input, me->passwd) != NULL) {
            tx("Invalid password.\n");
            return;
        }
        tx("Enter new password  : ");
        Inp(input, 250);
        if (input[0] == 0) {
            tx("Cancelled.\n");
            return;
        }
        if (stricmp(input, me->passwd) == NULL) {
            tx("Passwords are the same.\n");
            return;
        }
        tx("Confirm new password: ");
        Inp(block, 250);
        if (stricmp(input, block) != NULL) {
            tx("Passwords did not match.\n");
            return;
        }
        strcpy(me->passwd, input);
        tx("Password changed.\n");
        save_me();
        return;
    }

    tx("Invalid internal command. Type /? for list of commands.\n");
}

/****** AMUL3.C/LoseFollower ******************************************
 *
 *   NAME
 *	LoseFollower -- Get rid of the person following us.
 *
 *   SYNOPSIS
 *	void LoseFollower( void );
 *
 *   FUNCTION
 *	Unhooks the player who WAS following us (if there was one) and
 *	tells them they can no-longer follow us.
 *
 *   SEE ALSO
 *	StopFollow(), Follow(), DoThis()
 *
 ******************************************************************************
 *
 */

void
LoseFollower()
{
    if (me2->followed == -1)
        return;
    (linestat + (me2->followed))->following = -1; /* Unhook them */
    utx(me2->followed, "You are no-longer able to follow @me.\n");
    me2->followed = -1; /* Unhook me */
}

/****** AMUL3.C/ShowFile ******************************************
 *
 *   NAME
 *	ShowFile -- Send file to user (add extension)
 *
 *   SYNOPSIS
 *	ShowFile( FileName )
 *
 *	void ShowFile( BYTE );
 *
 *   FUNCTION
 *	Locates the file (experiments with extensions and paths) and
 *	displays it to the user. If there is insufficient memory or
 *	it is unable to find the file, it informs the user and takes
 *	apropriate action.
 *
 *   INPUTS
 *	FileName - ASCIZ string containing the file name. First try
 *		   to open file assumes the file is in the adventure
 *		   directory with the extension .TXT
 *
 *   EXAMPLE
 *	ShowFile("Scenario");
 *	ShowFile("Ram:Scenario.Txt");
 *
 *   NOTES
 *	Tries: <AdvPath>/<File Name>.TXT
 *	       <AdvPath>/<File Name>
 *	       <File Name>.TXT
 *	       <File Name>
 *
 ******************************************************************************
 *
 */

void
ShowFile(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s.txt", dir, s);
    if ((ifp = fopen(block, "rb")) != NULL)
        goto show;
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) != NULL)
        goto show;
    sprintf(block, "%s.txt", s);
    if ((ifp = fopen(block, "rb")) != NULL)
        goto show;
    sprintf(block, "%s", s);
    if ((ifp = fopen(block, "rb")) != NULL)
        goto show;
    txs("\n--+ Please inform the dungeon master that file %s is missing.\n\n", s);
    return;
show:
    fseek(ifp, 0, 2L);
    long fsize = ftell(ifp);
    fseek(ifp, 0, 0L);
	char *p = (char *)AllocMem(fsize + 2);
    if (p == NULL) {
        txs("\n--+ \x07System memory too low, exiting! +--\n");
        forced = 1;
        exeunt = 1;
        kquit("out of memory!\n");
    }
    fread(p, fsize, 1, ifp);
    tx(p);
    ReleaseMem(&p);
    pressret();
}

/****** AMUL3.C/showin ******************************************
 *
 *   NAME
 *	showin -- Display the contents of an object.
 *
 *   SYNOPSIS
 *	showin( Object, verbose )
 *
 *	void showin( int, bool );
 *
 *   FUNCTION
 *	Displays the contents of an object, modified depending on the
 *	objects 'putto' flag. Mode determines whether output is given when
 *	the contents of the object cannot be seen or there it is empty.
 *
 *   INPUTS
 *	Object -- the object's id number.
 *	verbose   -- YES to force display of contents, or to inform the player
 *		  if the object is empty.
 *		  NO not to list the contents of the object if it is opaque,
 *		  and not to display anything when it is empty.
 *
 ******************************************************************************
 *
 */

std::string
showin(objid_t objId, bool verbose)
{
    const auto &object = GetObject(objId);
    if ((object.flags & SF_OPAQUE) && !verbose) {
        return "\n";
    }
    std::string result {};
    if (verbose) {
        if (object.putto == 0)
            result = "The {adj} {noun} contains ";
        else
            result = "{prep} the {adj} {noun} you find: ";
        ReplaceAll(result, "{adj}", obj.adj != -1 ? GetAdj(obj.adj).word, "");
        ReplaceAll(result, "{noun}", obj.id);
    }
    if (object.inside <= 0) {
        if (!result.empty())
            result += "nothing.\n";
    } else {
        const roomid_t containerId = -(INS + objId);
        bool first { true };
        int itemsLeft = object.inside;
        for (int child = 0; child < g_game.numObjects && itemsLeft > 0; child++) {
            if (!isin(child, containerId))
                continue;
            if (!first)
                result += ", ";
            result += GetObject(child).id;
            first = false;
            itemsLeft--;
        }
        result += "\n";
    }

    return result;
}

/****** AMUL3.C/isStatFull ******************************************
 *
 *   NAME
 *	isStatFull -- Check if players property is at full
 *
 *   SYNOPSIS
 *	isStatFull( Stat, Player )
 *
 *	BOOLEAN isStatFull( USHORT, USHORT );
 *
 *   FUNCTION
 *	Tests to see if a players 'stat' is at full power and returns a
 *	TRUE or FALSE result (YES or NO).
 *
 *   INPUTS
 *	stat   -- a players stat number (see h/AMUL.Defs.H)
 *	player -- number of the player to check
 *
 *   RESULT
 *	YES if it is
 *	NO  if it isn't
 *
 ******************************************************************************
 *
 */

bool
isStatFull(int st, int p)
{
    you = (usr + p);
    you2 = (linestat + p);
    switch (st) {
    case STSCORE:
        return false;
    case STSCTG:
        return false;
    case STSTR:
        if (you2->strength < you->strength)
            return false;
        break;
    case STDEX:
        if (you2->dext < you->dext)
            return false;
        break;
    case STSTAM:
        if (you2->stamina < you->stamina)
            return false;
        break;
    case STWIS:
        if (you2->wisdom < you->wisdom)
            return false;
        break;
    case STMAGIC:
        if (you2->magicpts < you->magicpts)
            return false;
        break;
    case STEXP:
        if (you->experience < g_ranks[you->rank].experience)
            return false;
        break;
    }
    return true;
}

/****** blank.form/empty ******************************************
 *
 *   NAME
 *
 *   SYNOPSIS
 *
 *   FUNCTION
 *
 *   INPUTS
 *
 *   RESULT
 *
 *   EXAMPLE
 *
 *   NOTES
 *
 *   BUGS
 *
 *   SEE ALSO
 *
 ******************************************************************************
 *
 */

void
asetstat(int obj, int stat)
{
    objtab = obtab + obj;
    int x = owner(obj);
    bool wasLit = lit(loc(obj));
    /* Remove from owners inventory */
	int w = -1;
    if (x != -1) {
        w = (linestat + x)->wield;
        rem_obj(x, obj);
    }
    int f = STATE->flags & SF_LIT;
    objtab->state = stat;
    if (objtab->flags & OF_SHOWFIRE) {
        if (f == 0)
            STATE->flags = STATE->flags & -(1 + SF_LIT);
        else
            STATE->flags = STATE->flags | SF_LIT;
        if (x != -1)
            add_obj(x);
        return; /* Don't need to check for change of lights! */
    }

    if (x != -1) {
        add_obj(x); /* And put it back again */
        /*== Should check to see if its too heavy now */
        lighting(x, AHERE);
        (linestat + x)->wield = w;
    }

    bool nowLit = lit(loc(obj));
    if (nowLit == wasLit)
        return;
    if (!nowLit) {
        actionfrom(obj, acp(NOWTOODARK));
        sys(NOWTOODARK);
    } else {
        actionfrom(obj, acp(NOWLIGHT));
        sys(NOWLIGHT);
    }
}

void
awhere(int obj)
{
    bool found { false };
    for (int i = 0; i < g_game.numObjects; i++) {
        if (stricmp((obtab + obj)->id, (obtab + i)->id) == NULL) {
            if (!canSeeObject(i, Af))
                continue;
            if (int j = owner(i); j != -1) {
                if (lit((linestat + j)->room)) {
                    if (j != Af) {
                        tx("You see ");
                        ans("1m");
                        tx((usr + j)->name);
                        ans("0;37m");
                        tx(".\n");
                    } else
                        tx("There is one in your possesion.\n");
                    found = true;
                }
                continue;
            }
            long *rp = (obtab + i)->rmlist;
            for (int j = 0; j < (obtab + i)->nrooms; j++) {
                if (*(rp + j) == -1)
                    continue;
                if (*(rp + j) >= 0) {
                    if (*(rp + j) == -1 || !lit(*(rp + j)))
                        continue;
                    roomtab = rmtab + *(rp + j);
                    desc_here(2);
                } else {
                    int k = -(INS + *(rp + j));
                    sprintf(block, "There is one %s something known as %s!\n",
                            obputs[(obtab + k)->putto], (obtab + k)->id);
                    tx(block);
                }
                found = true;
            }
        }
    }
    if (found == -1)
        sys(SPELLFAIL);
}

void
osflag(int o, int flag)
{
    objtab = obtab + o;

    bool wasLit = false;
    int own = owner(o);
    if (own == -1)
        wasLit = lit(loc(o));
    else
        rem_obj(own, o);
    else
        wasLit = lit(loc(o));
    STATE->flags = flag;
    if (own != -1) {
        add_obj(o);
        lighting(own, AHERE);
        return;
    }

    if (lit(loc(o)) != wasLit) {
        if (wasLit) {
            actionfrom(o, acp(NOWTOODARK));
            sys(NOWTOODARK);
        } else {
            actionfrom(o, acp(NOWLIGHT));
            sys(NOWLIGHT);
        }
    }
}

/* Set @xx and @xy corresponding to a specific player */

void
setmxy(int Flags, int Them)
{
    if (Them == Af || canSee(Them, Af)) /* If he can see me */
    {
        ioproc("@me");
        strcpy(mxx, ow);
        ioproc("@me the @mr");
        strcpy(mxy, ow);
        return;
    }
    if (pROOM(Them) == me2->room) {
        switch (Flags) {
        case ACTION:
        case EVENT:
        case TEXTS:
            strcpy(mxx, "Someone nearby");
            strcpy(mxy, "Someone nearby");
            return;
        case NOISE:
            strcpy(mxx, "Someone nearby");
            ioproc("A @gn voice nearby");
            strcpy(mxy, ow);
            return;
        }
    }
    /* They aren't in the same room */
    switch (Flags) {
    case ACTION:
    case EVENT:
        strcpy(mxx, "Someone");
        if (me->rank == ranks - 1)
            strcpy(mxy, "Someone very powerful");
        else
            strcpy(mxy, "Someone");
        return;
    case TEXTS:
        ioproc("@me");
        strcpy(mxx, ow);
        if (me->rank == ranks - 1)
            ioproc("@me the @mr");
        strcpy(mxy, ow);
        return;
    case NOISE:
        strcpy(mxx, "Someone");
        if (me->rank == ranks - 1)
            ioproc("A powerful @gn voice somewhere in the distance");
        else
            ioproc("A @gn voice in the distance");
        strcpy(mxy, ow);
        return;
    default:
        strcpy(mxx, "Someone");
        strcpy(mxy, "Someone");
    }
}

int
owner(int obj)
{
    return GetObject(obj).Owner();
}

void
show_rank(int p, int rankn, int gender)
{
    str[0] = 0;
    make_rank(p, rankn, gender);
    tx(str);
}

void
make_rank(int p, int rankn, int gender)
{
    strcat(str, " the ");
    p += 5;
    if (*(linestat + p)->pre != 0) {
        strcat(str, (linestat + p)->pre);
        strcat(str, " ");
    }
    strcat(str, (gender == 0) ? g_ranks[rankn].male : g_ranks[rankn].female);
    if (*(linestat + p)->post != 0) {
        strcat(str, " ");
        strcat(str, (linestat + p)->post);
    }
}

// Move player to room, testing ligthing!
void
moveto(int r)
{
    // Set the players current lighting to NONE and then test lighting for
    // the room. Then move the player and restore his lighting; test again.

    StopFollow();
    me2->flags = me2->flags | PFMOVING;
    lroom = me2->room;
    int i = me2->light;
    me2->light = 0;
    lighting(Af, AOTHERS);
    me2->room = r;
    me2->light = i;
    me2->hadlight = 0;
    lighting(Af, AOTHERS);
    look((rmtab + me2->room)->id, me->rdmode);
    me2->flags = me2->flags & -(1 + PFMOVING);
}

{
}

// Do I own a 'obj' in state 'stat'?
bool
gotin(objid_t objId, int st)
{
    const auto &obj = GetObject(objId);
    for (const auto &container : g_game.m_objects) {
        if (st != -1 && container.state != st)
            continue;
        if (container.Owner() != Af)
            continue;
        if (stricmp(obj.id, container.id) == 0)
            return true;
    }
    return false;
}

/* The next two commands are ACTIONS but work like conditions */

// Check player is near object, else msg + endparse
int
achecknear(int obj)
{
    if (!nearto(obj)) {
        txs("I can't see the %s!\n", GetObject(obj).id);
        donet = ml + 1;
        return -1;
    }
    inc = 0;
    return 0;
}

// Check the player can 'get' the object
int
acheckget(objid_t objId)
{
    if (carrying(objId) != -1) {
        tx("You've already got it!\n");
        donet = ml + 1;
        return -1;
    }
    if (achecknear(objId) == -1)
        return -1;
    else
        inc = 1;
    auto &obj = GetObject(objId);
    if ((obj.flags & OF_SCENERY) || (obj.State().flags & SF_ALIVE) || obj.nrooms != 1) {
        tx("You can't move that!\n");
        donet = ml + 1;
        return -1;
    }
    const auto &rank = g_ranks[me->rank];
    if (rank.numobj <= 0) {
        tx("You can't manage it!\n");
        donet = ml + 1;
        return -1;
    }
    if (obj.State().weight > rank.maxweight) {
        tx("You aren't strong enough to lift that!\n");
        donet = ml + 1;
        return -1;
    }
    if (me2->numobj + 1 > rank.numobj) {
        tx("You can't carry any more! You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    if (obj.State().weight + me2->weight > rank.maxweight) {
        tx("It's too heavy. You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    inc = 0;
    return 0;
}

void
look_here(int f, int rm)
{
    // Can I see?
    if (me2->flags & PFBLIND) {
        list_what(rm, false);
        return;
    }
    // Can I see in here?
    if (!lit(me2->room)) {
        sys(TOODARK);
        *(rctab + rm) = *(rctab + rm) & rclr;
    } else {
        desc_here(f);
        list_what(rm, false);
    }

    if ((roomtab->flags & DEATH) && me->rank != ranks - 1) {
        if (dmove[0] == 0)
            strcpy(dmove, (rmtab + lroom)->id);
        akillme();
        return;
    } else
        whohere();
}

void
desc_here(int f)
{
    fseek(ifp, roomtab->desptr, 0L);
    if (roomtab->flags & DMOVE) /* A dmove room? */
        fgets(dmove, IDL, ifp);
    else
        dmove[0] = 0;

    /* Print short description */
    char *p = block;
    if (!(roomtab->flags & DEATH))
        ans("1m");
    char c;
    while ((c = fgetc(ifp)) != 0 && c != EOF && c != '\n') {
        *(p++) = c;
        *p = 0;
        if (p > block + 1020) {
            tx(block);
            p = block;
            *p = 0;
        }
    }
    if (p != block)
        tx(block); /* Make sure we dump it! */
    /* If I am the toprank show me the room id too! */
    ans("0;37m");
    if (me->rank == ranks - 1) {
        sprintf(block, "   (%s)", roomtab->id);
        tx(block);
        block[0] = 0;
    }
    txc('\n');
    if (c == '\n' && f == RDVB) {
        /* Display LONG description! */
        p = block;
        while ((c = fgetc(ifp)) != 0 && c != EOF) {
            *(p++) = c;
            *p = 0;
            if (p > block + 1020) {
                tx(block);
                p = block;
                *p = 0;
            }
        }
        if (p != block)
            tx(block);
    }
}

void
list_what(roomid_t rmId, bool visible)
{
    if (!lit(me2->room))
        return sys(TOOMAKE);
    if (me2->flags & PFBLIND)
        sys(YOURBLIND);
    const bool isHideaway = (GetRoom(rmId).flags & HIDEWY) == HIDEWY;
    const bool isTopRank = me->rank == ranks - 1;
    if (isHideaway && visible && !isTopRank) {
        sys(NOWTSPECIAL); /* Wizards can see in hideaways! */
        return;
    }
    bool found { false };
    for (int o = 0; o < g_game.numObjects; o++) /* All objects */
    {
        /* Only let the right people see the object */
        if (!canSeeObject(o, Af))
            continue;
        const auto &obj = GetObject(o);
        if (isHideaway && (!visible || !isTopRank) && !(obj.flags & OF_SCENERY))
            continue;
        if (!lit(me2->room) !(obj.flags & OF_SMELL))
            continue;
        for (size_t or = 0; or < obj.nrooms; or ++) {
            if (obj.GetRoom(or) == r && obj.State().descrip >= 0) {
                found = true;
                if (obj.IsInvis())
                    ans("3m");
                descobj(o);
                if (obj.IsInvis())
                    ans("0m");
                break;
            }
        }
    }
    if (!found && visible)
        sys(NOWTSPECIAL);
}

void
descobj(objid_t objId)
{
    const auto &obj = GetObject(objId);
    const auto &state = obj.State();
    if (state.descrip < 0)
        return;
    ///TODO: Replace %s use with {}
    std::string str = GetString(state.descrip);
    if ((obj.flags & OF_SHOWFIRE) && (state.flags & SF_LIT)) {
        if (str.back() == '\n' || str.back() == '{')
            str.pop_back();
        str += " The {adj} {noun} is on fire.\n";
    }
    ReplaceAll(str, "{adj}", obj.adj >= -1 ? GetAdj(obj.adj) : "");
    ReplaceAll(str, "{noun}", obj.id);
    if (obj.contains <= 0) {
        tx(str.c_str());    ///SV: Direct
        return;
    }
    if (str.back() == '\n' || str.back() == '{')
        str.pop_back();

    str += " ";
    str += showin(Ob, false);

    tx(str.c_str());  ///SV: Direct
}

void
inflict(int x, int s)
{
    you2 = linestat + x;
    if (you2->state != PLAYING)
        return;
    switch (s) {
    case SGLOW:
        if (!(you2->flags & PFGLOW)) {
            you2->flags = (you2->flags | PFGLOW);
            you2->light++;
        }
        break;
    case SINVIS:
        you2->flags = you2->flags | PFINVIS;
        break;
    case SDEAF:
        you2->flags = you2->flags | PFDEAF;
        break;
    case SBLIND:
        you2->flags = you2->flags | PFBLIND;
        break;
    case SCRIPPLE:
        you2->flags = you2->flags | PFCRIP;
        break;
    case SDUMB:
        you2->flags = you2->flags | PFDUMB;
        break;
    case SSLEEP:
        you2->flags = you2->flags | PFASLEEP;
        break;
    case SSINVIS:
        you2->flags = you2->flags | PFSINVIS;
        break;
    }
    calcdext();
    lighting(x, AHERE);
}

void
cure(int x, int s)
{
    you2 = linestat + x;
    if (you2->state != PLAYING)
        return;
    switch (s) {
    case SGLOW:
        if (you2->flags & PFGLOW) {
            you2->flags = (you2->flags & (-1 - PFGLOW));
            you2->light--;
        }
        break;
    case SINVIS:
        you2->flags = you2->flags & -(1 + PFINVIS);
        break;
    case SDEAF:
        you2->flags = you2->flags & -(1 + PFDEAF);
        break;
    case SBLIND:
        you2->flags = you2->flags & -(1 + PFBLIND);
        break;
    case SCRIPPLE:
        you2->flags = you2->flags & -(1 + PFCRIP);
        break;
    case SDUMB:
        you2->flags = you2->flags & -(1 + PFDUMB);
        break;
    case SSLEEP:
        you2->flags = you2->flags & -(1 + PFASLEEP);
        break;
    case SSINVIS:
        you2->flags = you2->flags & -(1 + PFSINVIS);
        break;
    }
    calcdext();
    lighting(x, AHERE);
}

void
summon(int plyr)
{
    if ((linestat + plyr)->room == me2->room) {
        txs(acp(CANTSUMN), (usr + plyr)->name);
        return;
    }
    interact(MSUMMONED, plyr, me2->room);
}

void
adestroy(int obj)
{
    Forbid();
    loseobj(obj);
    for (int i = 0; i < (obtab + obj)->nrooms; i++)
        *((obtab + obj)->rmlist + i) = -1;
    (obtab + obj)->flags = (obtab + obj)->flags | OF_ZONKED;
    Permit();
}

void
arecover(int obj)
{
    if (State(obj)->flags & SF_LIT)
        me2->light++;
    for (int i = 0; i < (obtab + obj)->nrooms; i++)
        *((obtab + obj)->rmlist + i) = me2->room;
    (obtab + obj)->flags = (obtab + obj)->flags & -(1 + OF_ZONKED);
    lighting(Af, AHERE);
}

// Refresh the player's stats
void
refresh()
{
    const auto &rank = g_ranks[me->rank];
    if (me->strength <= 0)
        me->strength = rank.strength;
    me2->strength = me->strength;
    if (me->stamina <= 0)
        me->stamina = rank.stamina;
    me2->stamina = me->stamina;
    if (me->dext <= 0)
        me->dext = rank.dext;
    me2->dext = me->dext;
    me2->dextadj = 0;
    if (me->wisdom <= 0)
        me->wisdom = rank.wisdom;
    me2->wisdom = me->wisdom;
    if (me->experience <= 0)
        me->experience = rank.experience;
    if (me->magicpts <= 0)
        me->magicpts = rank.magicpts;
    me2->magicpts = me->magicpts;
    calcdext();
}

void
zapme()
{
    Forbid();
    char *p = (char *)me->name;
    exeunt = 1;
    for (int i = 0; i < sizeof(usr); i++)
        *(p++) = 0;
    Permit();
    save_me();
    nohelp();
}

void
send(int o, int to)
{
    bool wasLit = lit(to);
    loseobj(o);
    for (int i = 0; i < objtab->nrooms; i++)
        *(objtab->rmlist + i) = to;
    if (lit(to) != wasLit)
        actionin(to, acp(NOWLIGHT));
}

// Change player's gender
void
achange(int u)
{
    if (u == Af) {
        me->gender = 1 - me->gender;
        sys(CHANGESEX);
    } else
        sendex(u, ACHANGESEX, u, NONE); /* Tell them to clear up! */
}

// Fixed to allow increase/decrease
void
newrank(int plyr, int newRankNo)
{
    const auto &oldRank = g_ranks[me->rank];
    const auto &newRank = g_ranks[newRankNo];
    if (newRank.tasks != 0) {
        if ((me->tasks & (1 << (newRank.tasks) - 1)) == NULL) {
            sys(NOTASK);
            return;
        }
    }

    me->rank = newRankNo;   ///TODO: Sync
    sys(MADERANK);

    /* Update Current Line Stats */
    me2->strength += newRank.strength - oldRank.strength;
    if (me2->strength > newRank.strength)
        me2->strength = newRank.strength;
    me2->stamina += newRank.stamina - oldRank.stamina;
    if (me2->stamina > newRank.stamina)
        me2->stamina = newRank.stamina;
    me2->wisdom += newRank.wisdom - oldRank.wisdom;
    if (me2->wisdom > newRank.wisdom)
        me2->wisdom = newRank.wisdom;
    me->experience += newRank.experience - oldRank.experience;
    if (me->experience > newRank.experience)
        me->experience = newRank.experience;
    me2->magicpts += newRank.magicpts - oldRank.magicpts;
    if (me2->magicpts > newRank.magicpts)
        me2->magicpts = newRank.magicpts;
    calcdext();

    /* Update File Stats */
    me->strength = newRank.strength;
    me->stamina = newRank.stamina;
    me->wisdom = newRank.wisdom;
    me->dext = newRank.dext;
    me->experience += newRank.experience - oldRank.experience;
    me->magicpts = newRank.magicpts;

    if (newRankNo == ranks - 1) {
        sys(TOPRANK);
        SendIt(MMADEWIZ, 0, me->name);
    }
}

void
aadd(int howmuch, int stat, int plyr)
{
    if (howmuch < 0)
        return asub(-howmuch, stat, plyr);
    if (howmuch == 0)
        return;
    if (plyr == Af) {
        switch (stat) {
        case STSCORE:
            me->score += howmuch;
            me2->sctg += howmuch;
            ans("1m");
            utxn(plyr, "(%ld)\n", me->score);
            ans("0;37m");
            for (int r = ranks - 1; r >= 0; r--) {
                if (me->score >= g_ranks[r].score) {
                    if (me->rank == r)
                        break;
                    newrank(plyr, r);
                    break;
                }
            }
            break;
        case STSTR:
            me2->strength += howmuch;
            break;
        case STSTAM:
            me2->stamina += howmuch;
            sprintf(block, "<STAM: %ld/%ld>\n", me2->stamina, me->stamina);
            ans("1m");
            tx(block);
            ans("0;37m");
            break;
        case STDEX:
            me2->dextadj += howmuch;
            break;
        case STEXP:
            me->experience += howmuch;
            break;
        case STWIS:
            me2->wisdom += howmuch;
            break;
        case STMAGIC:
            me2->magicpts += howmuch;
            break;
        }
    } else
        sendex(plyr, AADD, howmuch, stat, plyr); /* Tell them to clear up! */
}

void
asub(int howmuch, int stat, int plyr)
{
    if (howmuch < 0)
        return asub(-howmuch, stat, plyr);
    if (howmuch == 0)
        return;
    if (plyr == Af) {
        switch (stat) {
        case STSCORE:
            me->score -= howmuch;
            me2->sctg -= howmuch;
            if (me->score < 0)
                me->score = 0;
            ans("1m");
            utxn(plyr, "(%ld)\n", me->score);
            ans("0;37m");
            for (int r = 0; r < ranks - 1; r++) {
                if (me->score >= g_ranks[r+1].score)
                    continue;
                if (me->rank == r)
                    break;
                newrank(plyr, r);
            }
            break;
        case STSTR:
            me2->strength -= howmuch;
            if (me2->strength < 0)
                me2->strength = 0;
            break;
        case STSTAM:
            me2->stamina -= howmuch;
            if (me2->stamina < 0)
                me2->stamina = 0;
            sprintf(block, "\n<STAM: %ld/%ld>\n", me2->stamina, me->stamina);
            ans("1m");
            tx(block);
            ans("0;37m");
            if (me2->stamina <= 0) {
                akillme();
                died = 1;
            }
            break;
        case STDEX:
            me2->dextadj -= howmuch;
            break;
        case STWIS:
            me2->wisdom -= howmuch;
            if (me2->wisdom < 0)
                me2->wisdom = 0;
            break;
        case STEXP:
            me->experience -= howmuch;
            if (me->experience < 0)
                me->experience = 0;
            break;
        case STMAGIC:
            me2->magicpts -= howmuch;
            if (me2->magicpts < 0)
                me2->magicpts = 0;
            break;
        }
    } else
        sendex(plyr, ASUB, howmuch, stat, plyr); /* Tell them to clear up! */
}

void
afix(int stat, int plyr)
{
    if (plyr == Af) {
        const auto &rank = g_ranks[me->rank];
        switch (stat) {
        case STSTR:
            me2->strength =
                    (rank.strength * rank.maxweight - me2->weight) /
                    rank.maxweight;
            break;
        case STSTAM:
            me2->stamina = rank.stamina;
            break;
        case STDEX:
            me2->dextadj = 0;
            calcdext();
            break;
        case STWIS:
            me2->wisdom = rank.wisdom;
            break;
        case STEXP:
            me->experience = rank.experience;
            break;
        case STMAGIC:
            me2->magicpts = rank.magicpts;
            break;
        }
    } else
        sendex(plyr, AFIX, stat, plyr, 0); /* Tell them to clear up! */
}

// Loud noises/events
void
announce(char *s, int towho)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is deaf, ignore him */
        if (actor == i || ((linestat + i)->state < 2) || ((linestat + i)->flags & PFDEAF))
            continue;
        /*
           The next line says:
            if this is another player, and he's in another room,
            and the room is a silent room, ignore him.
        */
        if (i != Af && (linestat + i)->room != me2->room && /* --v */
            ((rmtab + (linestat + i)->room)->flags & SILENT))
            continue;
        int x = 0;
        switch (towho) {
        case AALL:
        case AEVERY1:
            x = 1;
            break;
        case AGLOBAL:
            if (i != Af)
                x = 1;
            break;
        case AOTHERS:
            if (i == Af)
                break;
        case AHERE:
            if ((linestat + i)->room == me2->room)
                x = 1;
            break;
        case AOUTSIDE:
            if ((linestat + i)->room != me2->room)
                x = 1;
            break;
        }
        if (x == 1) {
            setmxy(NOISE, i);
            utx(i, s);
        }
    }
}

// Loud noises/events
void
announcein(int toroom, char *s)
{
    int i;
    for (int i = 0; i < MAXU; i++) {
        /* If the player is deaf, ignore him */
        if (actor == i || ((linestat + i)->state < 2) || ((linestat + i)->flags & PFDEAF) ||
            (linestat + i)->room != toroom)
            continue;
        setmxy(NOISE, i);
        utx(i, s);
    }
}

// Loud noises/events
void
announcefrom(int obj, char *s)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is deaf or can see me, ignore him */
        if (actor == i || ((linestat + i)->state < 2) || ((linestat + i)->flags & PFDEAF) ||
            (linestat + i)->room == me2->room)
            continue;
        /* Check if the player is NEAR to someone carrying the object */
		int o = owner(obj);
        if (o != -1 && (linestat + o)->room != (linestat + i)->room)
            continue;
        if (o == -1 && !isin(obj, (linestat + o)->room))
            continue;
        setmxy(NOISE, i);
        utx(i, s);
    }
}

// Loud noises/events (via an object)
void
objannounce(int obj, char *s)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is deaf or can see me, ignore him */
        if (actor == i || ((linestat + i)->state < 2) || ((linestat + i)->flags & PFDEAF))
            continue;
        /* Check if the player is NEAR to someone carrying the object */
		int o = owner(obj);
        if (o != -1 && (linestat + o)->room != (linestat + i)->room)
            continue;
        if (o == -1 && !isin(obj, (linestat + o)->room))
            continue;
        setmxy(NOISE, i);
        utx(i, s);
    }
}

// Quiet actions/notices
void
action(char *s, int towho)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is asleep, or blind, skip him */
        if (actor == i || ((linestat + i)->state < 2) ||
            ((linestat + i)->flags & (PFBLIND + PFASLEEP)) != 0)
            continue;
        int x = 0;
        switch (towho) {
        case AALL:
        case AEVERY1:
            x = 1;
            break;
        case AGLOBAL:
            if (i != Af)
                x = 1;
            break;
        case AOTHERS:
            if (i == Af)
                break;
        case AHERE:
            if ((linestat + i)->room == me2->room && canSee(i, Af))
                x = 1;
            break;
        case AOUTSIDE:
            if ((linestat + i)->room != me2->room)
                x = 1;
            break;
        }
        if (x == 1) {
            setmxy(ACTION, i);
            utx(i, s);
        }
    }
}

/* Quiet actions/notices */
void
actionin(int toroom, char *s)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is asleep, or blind, skip him */
        if (actor == i || ((linestat + i)->state < PLAYING) ||
            ((linestat + i)->flags & (PFBLIND + PFASLEEP)) || (linestat + i)->room != toroom)
            continue;
        setmxy(ACTION, i);
        utx(i, s);
    }
}

/* Quiet actions/notices */
void
actionfrom(int obj, char *s)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is asleep, or blind, skip him */
        if (actor == i || ((linestat + i)->state < 2) ||
            ((linestat + i)->flags & (PFBLIND + PFASLEEP)) || (linestat + i)->room == me2->room)
            continue;
        /* Check if the player is NEAR to someone carrying the object */
		int o = owner(obj);
        if (o != -1)
            if ((linestat + o)->room != (linestat + i)->room)
                continue;
        if (o == -1 && !isin(obj, (linestat + i)->room))
            continue;
        setmxy(ACTION, i);
        utx(i, s);
    }
}

/* Quiet actions/notices */
void
objaction(int obj, char *s)
{
    for (int i = 0; i < MAXU; i++) {
        /* If the player is asleep, or blind, skip him */
        if (actor == i || ((linestat + i)->state < 2) ||
            ((linestat + i)->flags & (PFBLIND + PFASLEEP)))
            continue;
        /* Check if the player is NEAR to someone carrying the object */
        int o = owner(obj);
        if (o != -1)
            if ((linestat + o)->room != (linestat + i)->room)
                continue;
        if (o == -1 && !isin(obj, (linestat + i)->room))
            continue;
        setmxy(ACTION, i);
        utx(i, s);
    }
}

void
ableep(int n)
{
    for (int i = 0; i < n; i++) {
        tx(". ");
        fwait(1);
    }
    txc('\n');
}

// twho - who to notify
void
lighting(int x, int twho)
{
    if ((linestat + x)->light == (linestat + x)->hadlight ||
        !((rmtab + (linestat + x)->room)->flags & DARK))
        return;
    if ((linestat + x)->light <= 0) {
        if ((linestat + x)->hadlight <= 0)
            return;
        (linestat + x)->hadlight = (linestat + x)->light = 0;
        if (!lit((linestat + x)->room))
            action(acp(NOWTOODARK), twho);
    } else {
        if ((linestat + x)->hadlight != 0)
            return;
        if (!lit((linestat + x)->room))
            action(acp(NOWLIGHT), twho);
        (linestat + x)->hadlight = (linestat + x)->light;
    }
}

/* Remove object from its owners inventory */
void
loseobj(int obj)
{
    objtab = obtab + obj;

	int o = owner(obj);
    if (o != -1) {
        for (int i = 0; i < objtab->nrooms; i++)
            *(objtab->rmlist + i) = -1;
        rem_obj(o);
        lighting(o, AHERE);
    }
}

void
nohelp()
{
    if (me2->helping != -1)
        utx(me2->helping, "@me is no-longer able to help you...\n");
    (linestat + me2->helping)->helped--;
    me2->helping = -1;
    you2 = linestat;
    for (int i = 0; i < MAXU; i++, you2++)
        if (you2->helping == Af) {
            utx(i, "You are no longer able to help @me.\n");
            you2->helping = -1;
        }
    me2->helping = me2->helped = -1;
}

void
aforce(int x, char *cmd)
{
	DoThis(x, cmd, 0);
}

void
afight(int plyr)
{
    if (plyr == Af)
        return;
    if ((rmtab + me2->room)->flags & PEACEFUL) {
        sys(NOFIGHT);
        return;
    }
    if ((linestat + plyr)->fighting == Af) {
        txs("You are already fighting %s!\n", (usr + plyr)->name);
        donet = ml + 1;
        return;
    }
    if ((linestat + plyr)->fighting != -1) {
        txs("%s is already in a fight!\n", (usr + plyr)->name);
        donet = ml + 1;
        return;
    }
    you2 = linestat + plyr;
    you2->flags = you2->flags | PFFIGHT;
    me2->flags = me2->flags | PFFIGHT | PFATTACKER;
    you2->fighting = Af;
    me2->fighting = plyr;
    Permit();
}

void
clearfight()
{
    Forbid();
    if (me2->fighting != -1 && me2->fighting != Af) {
        finishfight(me2->fighting);
    }
    finishfight(Af);
    Permit();
}

void
finishfight(int plyr)
{
    you2 = linestat + plyr;
    you2->flags = you2->flags & (-1 - (PFFIGHT | PFATTACKER));
    you2->fighting = -1;
}

void
acombat()
{
/* Check this out for Stats:
To hit:  Str=40 Exp=50 Dext=10
Defence: Dext=70 Exp=20 Str=10
No hits: Players level different by 2 double attacks etc.
Damage:  Str / 10 + weapon.		<--- made this random!

== Should ALSO go on how crippled a player is... A cripple can't
strike out at another player! Also, blindness should affect your
attack/defence. */

    int aatt, adef, adam, datt, ddef, str;
    int          oldstr, minpksl;

    calcdext();

    if (me2->fighting == Af || me2->fighting == -1 || me2->state < PLAYING || me2->stamina <= 0) {
        donet = ml + 1; /* End parse */
        finishfight(Af);
        return; /* Macro : Permit(); return */
    }

    you = usr + me2->fighting;
    you2 = linestat + me2->fighting;
    minpksl = g_ranks[you->rank].minpksl;

    if (you2->state < PLAYING || you2->room != me2->room || you2->stamina <= 0) {
        donet = ml + 1;
        finishfight(Af);
        return;
    }

    if (me2->wield != -1) {
        objtab = obtab + me2->wield;
        str = (20 * me2->strength) + STATE->damage;
    } else
        str = (20 * me2->strength);

    const auto &maxRank = g_ranks.back();
    if (me->dext == 0)
        aatt = 5;
    else
        aatt = (50 * me->experience) / maxRank.experience +
               (40 * me2->strength) / maxRank.strength +
               (10 * me2->dext) / maxRank.dext;

    if (me->dext == 0)
        adef = 5;
    else
        adef = (5 * me->experience) / maxRank.experience +
               (15 * me2->strength) / maxRank.strength +
               (80 * me2->dext) / maxRank.dext;

    /*	if(me2->flags & PFCRIP)  { aatt = aatt / 5; adef = adef / 10; }
        if(me2->flags & PFBLIND) { aatt = aatt / 2; adef = adef / 4;  } */

    if (you2->wield != -1) {
        objtab = obtab + you2->wield;
        str = (20 * you2->strength) + STATE->damage;
    } else
        str = (20 * you2->strength);

    if (you->dext == 0)
        datt = 5;
    else
        datt = (50 * you->experience) / maxRank.experience +
               (40 * you2->strength) / maxRank.strength +
               (10 * you2->dext) / maxRank.dext;

    if (you->dext == 0)
        ddef = 5;
    else
        ddef = (5 * you->experience) / maxRank.experience +
               (15 * you2->strength) / maxRank.strength +
               (80 * you2->dext) / maxRank.dext;

    /*	if(you2->flags & PFCRIP)  { datt = datt / 5; ddef = ddef / 10; }
        if(you2->flags & PFBLIND) { datt = datt / 2; ddef = ddef / 4;  } */

    oldstr = you2->stamina;
    adam = -1;
    if (randint(0, 100) < aatt || (ddef <= 0 && randint(0, 100) < 50)) {
        if (randint(0, 100) < ddef) {
            if (you2->wield != -1) {
                sys(WBLOCK);
                utx(me2->fighting, acp(WDEFEND));
            } else {
                sys(BLOCK);
                utx(me2->fighting, acp(DEFEND));
            }
            /*			if((i=isverb("\"block"))!=-1) lang_proc(i,0);	*/
        } else {
            if (me2->wield != -1) {
                sys(WATTACK);
                objtab = obtab + me2->wield;
                adam = (me2->strength / 10) + 1 + randint(0, STATE->damage);
                utx(me2->fighting, acp(WHIT));
            } else {
                adam = (me2->strength / 10) + 1;
                sys(ATTACK);
                utx(me2->fighting, acp(AMHIT));
            }
            asub(adam, STSTAM, me2->fighting);
            /*			if((i=isverb("\"hit"))!=-1) lang_proc(i,0);	*/
        }
    } else {
        sys(MISSED);
        utx(me2->fighting, acp(HEMISSED));
        /*		if((i=isverb("\"miss"))!=-1) lang_proc(i,0);		*/
    }
    oldstr -= adam;
    if ((me2->flags & PFATTACKER) && oldstr > 0) {
        /*		if(me2->helped != -1 && (linestat+me2->helped)->room==me2->room)	Well?	*/

        sendex(me2->fighting, ACOMBAT, -1, 0, 0); /* Tell them to clear up! */
    }
    if (oldstr <= 0) {
        donet = ml + 1; /* End parse */
        tx("You have defeated @pl!\n");
        aadd(minpksl, STSCORE, Af);
        finishfight(Af);
    }
}

void
exits()
{
    int    ac;

    roomtab = rmtab + me2->room;
    if (roomtab->tabptr == -1) {
        tx("There are no visible exits.\n");
        return;
    }

    vbptr = vbtab;
    int c = tt.condition;
    int a = tt.action;
    struct _TT_ENT *otp = ttabp;
    long *pptr = tt.pptr;

    for (int v = 0; v < verbs; v++, vbptr++) {
        if (vbptr->flags & VB_TRAVEL)
            continue; /* Not a trav verb */

        roomtab = rmtab + me2->room;
        int l = -1;
        int maxl = roomtab->ttlines;
        struct _TT_ENT *tp = ttp + roomtab->tabptr;
        bool brk { false };
        for (int i = 0; i < maxl && !brk; i++) {
            ttabp = tp + i;
            tt.condition = ttabp->condition;
            tt.pptr = ttabp->pptr;
            if (ttabp->verb == v && (l = cond(ttabp->condition, l)) != -1) {
                if (ttabp->action >= 0) {
                    txs("%-10s ", vbptr->id);
                    brk = true;
                    roomtab = rmtab + (ttabp->action);
                    if (roomtab->flags & DEATH)
                        sys(CERTDEATH);
                    else
                        desc_here(RDBF);
                    break;
                }
                ac = -1 - ttabp->action;
                switch (ac) {
                case AKILLME:
                    txs("%s: It's difficult to tell...\n", vbptr->id);
                case AENDPARSE:
                case AFAILPARSE:
                case AABORTPARSE:
                case ARESPOND:
                    maxl = -1;
                    break;
                case ASKIP:
                    i += TP1;
                }
                if (tt.condition == CANTEP || tt.condition == CALTEP || tt.condition == CELTEP)
                    maxl = -1;
            }
        }
    }
    tt.condition = c;
    tt.action = a;
    ttabp->pptr = pptr;
    ttabp = otp;
}

void
follow(int x, char *cmd)
{
    lockusr(x);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = MFORCE;
    IAd = 1;
    IAp = cmd;
    PutMsg((linestat + x)->rep, (struct Message *)intam);
    (linestat + x)->IOlock = -1;
}

void
log(char *s)
{
    ioproc(s);
    s = ow;
    while (*s != 0) {
        if (*s == '\n' || *s == '\r') {
            strcpy(s, s + 1);
            continue;
        }
        s++;
    }
    SendIt(MLOG, NULL, ow);
}

void
akillme()
{
    if (me2->fighting != -1)
        clearfight();
    iocheck();
    me->plays = -1;
    exeunt = forced = 1;
    donet = ml + 1;
    ml = -1;
    me2->state = 0;
    nohelp();
    ans("1m");
    sys(DIED);
    ans("0;37m");
}

void
show_tasks(int p)
{
    sprintf(block, "Tasks completed by ");
    if (p != Af)
        strcat(block, (usr + p)->name);
    else
        strcat(block, "you");
    strcat(block, ": ");
    if (me->tasks == 0)
        strcat(block, "None.\n");
    else {
        int i;
        char         tsk[6];
        tsk[0] = 0;
        for (i = 0; i < 32; i++) {
            if ((usr + p)->tasks & (1 << i)) {
                if (tsk[0] == 0)
                    sprintf(tsk, "%d", i + 1);
                else
                    sprintf(tsk, ", %d", i + 1);
                strcat(block, tsk);
            }
        }
        strcat(block, ".\n");
    }
    tx(block);
}

// Drop everything to a room
void
dropall(int torm)
{
    for (int i = 0; i < g_game.numObjects && me2->numobj > 0; i++)
        if (*(obtab + i)->rmlist == -(5 + Af))
            adrop(i, torm);
    me2->numobj = 0;
}

void
invent(int plyr)
{
    char *p = block + strlen(block);
    strcpy(p, "carrying ");
    *(p += 9) = 0;
    if ((linestat + plyr)->numobj == 0) {
        strcat(p, "nothing.\n");
        tx(block);
        return;
    }
    objtab = obtab;
    int j = 0;
    int pr = -(5 + plyr);
    for (int i = 0; i < g_game.numObjects; i++, objtab++)
        if (*objtab->rmlist == pr && canSeeObject(i, Af)) {
            if (j++ != 0)
                strcat(p, ", ");
            strcat(p, objtab->id);
            if (objtab->flags & OF_INVIS)
                strcat(p, " (hidden)");
        }
    if (j == 0)
        strcat(p, "nothing.\n");
    else
        strcat(p, ".\n");
    tx(block);
}

void
ascore(int type)
{
    calcdext();

    if (type == TYPEV) {
        sprintf(block, "Recorded details:		%s\n\n", vername);
        tx(block);
        tx("Name: @m! Sex  : @gn		Played   : @gp times\n");
        ioproc("@mr");
        txs("Rank: %-20s  Score: @sc points	This game: @sg points\n", ow);
        const auto &rank = g_ranks[me->rank];
        sprintf(block, "Strength: @sr/%ld. Stamina: @st/%ld. Dexterity %ld/%ld.\n",
                rank.strength, rank.stamina, me2->dext, rank.dext);
        tx(block);
        sprintf(block, "Magic Points: @mg/%ld. Wisdom:  @wi.\n", rank.magicpts);
        tx(block);

        sprintf(block, "\nCurrent Info:\nObjects Carried: %ld/%ld,	Weight Carried: %ld/%ldg\n",
                me2->numobj, rank.numobj, me2->weight, rank.maxweight);
        tx(block);
        tx("Following: @mf.	");
        if (me2->helping != -1)
            tx("Helping: @fr.  ");
        if (me2->helped != -1)
            tx("Helped by: @he.");
        if (me2->helping != -1 || me2->helped != -1)
            txc('\n');
        /*== Current weapon */
        if (me2->wield != -1)
            txs("Currently wielding: %s.\n", (obtab + me2->wield)->id);
        show_tasks(Af);
    } else {
        txs("Score: @sc. ", ow);
        sprintf(block, "Strength: @sr/%ld. Stamina: @st/%ld. Dexterity: %ld/%ld. Magic: @mg/%ld\n",
                rank.strength, rank.stamina, me2->dext, rank.dext, rank.magicpts);
        tx(block);
    }
}

void
calcdext()
{
    const auto &rank = g_ranks[me->rank];
    me2->dext = rank.dext;

    if (me2->flags & PFSITTING)
        me2->dext = me2->dext / 2;
    else if (me2->flags & PFLYING)
        me2->dext = me2->dext / 3;
    if (!lit(me2->room) || (me2->flags & PFBLIND))
        me2->dext = me2->dext / 5;

    me2->dext -=
            ((me2->dext / 10) -
             (((me2->dext / 10) * (rank.maxweight - (me2->weight))) / rank.maxweight));

    if (me2->flags & PFINVIS)
        me2->dext += (me2->dext / 3);
    if (me2->flags & PFSINVIS)
        me2->dext += (me2->dext / 2);
    if (me->flags & PFCRIP)
        me2->dext = 0;
    me2->dext += me2->dextadj;
}

void
toprank()
{
    for (const auto &rank : g_ranks) {
        // turn the task into a bit
        me->tasks |= 1 << (rank.tasks - 1);
    }
    aadd(g_ranks.back().score - me->score + 1, STSCORE, Af);
}

void
damage(int obj, int howmuch)
{
    objtab = obtab + obj;
    if (objtab->flags & OF_SCENERY)
        return;
    STATE->strength -= howmuch;
    if (STATE->strength < 0 && (objtab->flags & OF_SHOWFIRE)) {
        txs("The %s has burnt away.", objtab->id);
        *objtab->rmlist = -1;
    }
    if (STATE->strength < 0 && (!(objtab->flags & OF_SHOWFIRE))) {
        txs("The %s has just broken.", objtab->id);
        *objtab->rmlist = -1;
    }
}

void
repair(int obj, int howmuch)
{
    objtab = obtab + obj;
    if (objtab->flags & OF_SCENERY)
        return;
    STATE->strength += howmuch;
}


// There are 3 roles the amul client plays:
// 1- player,
// 2- background event processing (demon),
// 3- npc (mobile)
// 
// 2 and 3 are "special"s

/* Daemon processing host */
static void
demonService()
{
	printf("-- Demon processor loaded\n");
	///AUDIT: while !g_resetInProgress?
    for ( ;; ) {
        Wait(-1);
        iocheck();
    }
}

static void
mobileService()
{
    printf("-- Mobile processor loaded\n");
	///NOTE: This snapshot is from right before I implemented the mobile
	/// system in 0.9.00, so there's no actual mobile implementation :(

	// Most mobiles will probably want an easily accessible list of travel.
	std::vector<verbid_t> travelVerbs {};
	for (verbid_t i = 0; i < verbs; ++i) {
		if (vbtab[i].flags & VB_TRAVEL) {
			travelVerbs.push_back(i);
		}
	}

	for ( ;; ) {
		Wait(-1);
		iocheck();
	}
}

///TODO: Rename 'serviceHandler'
void
Special_Proc()
{
	// Invoke a demon or mobile worker
    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    wtype[0] = wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = WNONE;
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = last_him = last_her = it = -1;
    switch (MyFlag) /* What type of processor ? */
    {
    case am_DAEM: /* Execute the boot-up daemon */
        if (verbid_t i = isverb("\"boot"); i != -1)
            lang_proc(i, 0);
        demonService(); /* Daemon Processor */
		break;
    case am_MOBS:
		mobileService();
		break;
    default:
        printf("XX Unsupported special processor requested\n");
    }
    quit();
}

void
look(char *s, int f)
{
    int roomno, mod;

    /* Some complex stuff here!
      if f==0 (rdmode=RoomCount) and we have been here before,
        look(brief)
      if f==0 (rdmode=RoomCount) and this is our first visit,
        look(verbose)
      if f==0 visit the room
                                   */

    if ((roomno = isroom(s)) == -1)
        return;
    roomtab = rmtab + roomno;
    if (f == RDRC && ((*(rctab + roomno) & rset) != rset))
        mod = RDVB;
    else
        mod = f;
    if (f != 2)
        *(rctab + roomno) = *(rctab + roomno) | rset;
    look_here(mod, roomno);
}


/* Add object to players inventory */
void
agive(int obj, int to)
{
    int own, orm;

    objtab = obtab + obj;

    if ((objtab->flags & OF_SCENERY) || (STATE->flags & SF_ALIVE) || objtab->nrooms != 1)
        return;
    if ((own = owner(obj)) != -1)
        rem_obj(own, obj);
    orm = *objtab->rmlist;
    add_obj(to);

    /*== The lighting conditions for transfering an object between a
         variable source and destination are complex! See below!	*/
    if (STATE->flags & SF_LIT) {
        if (own == -1) /*== Did I just pick and was it from here? */
        {
            if (orm == (linestat + own)->room)
                return;
        } else { /*== If I got it from someone, and he is near me... */
            if ((linestat + own)->room == (linestat + to)->room)
                return;
            lighting(own, AHERE); /*== Else check his lights! */
        }
        lighting(to, AHERE);
    }
}

/* Drop the object (to a room) */
void
adrop(int ob, int r, int f)
{
    objtab = obtab + ob;
    *objtab->rmlist = r;
    rem_obj(Af, ob);
    lighting(Af, AHERE);

    /* If the room IS a 'swamp', give em points */
    if ((rmtab + me2->room)->flags & SANCTRY) {
        /*== Only give points if player hasn't quit. */
        if (exeunt == 0)
            aadd(scaled(STATE->value, STATE->flags), STSCORE, Af);
        *objtab->rmlist = -1;
    }
}

// Is my room lit?
bool
lit(int r)
{
    if (!((rmtab + r)->flags & DARK))
        return true;
    Forbid();
    you2 = linestat;
    for (int i = 0; i < MAXU; i++, you2++)
        if (you2->room == r && you2->hadlight != 0) {
            Permit();
            return true;
        }
    _OBJ_STRUCT *objp = &GetRoom(0);
    for (int i = 0; i < g_game.numObjects; i++, objp++) {
        if ((objp->State().flags & SF_LIT) != SF_LIT)
            continue;
        const auto rooms = objp->Rooms();
        for (size_t j = 0; j < objp->nrooms; ++j) {
            if (objp->Room(j) == r) {
                Permit();
                return true;
            }
    }
    Permit();
    return false;
}

auto
loc(int o)
{
    const auto &obj = GetObject(o);
    roomid_t room = obj.Room(0);
    if (room >= -1)
        return room;
    if (room > -INS)
    if (*(obtab + o)->rmlist >= -5 && *(obtab + o)->rmlist <= -(5 + MAXU))
        return (int)(linestat + owner(o))->room;
    /* Else its in a container */
    return -1;
}

auto
carrying(int obj)
{
    if (me2->numobj == 0)
        return -1;
    if (owner(obj) == Af)
        return obj;
    return -1;
}

bool
nearto(int ob)
{
    if (!canSeeObject(ob, Af))
        return false;
    if (isin(ob, me2->room))
        return true;
    if (carrying(ob) != -1)
        return true;
    return false;
}

// Can others in this room see me?
bool
isVisible()
{
    if (!lit(me2->room))
        return false;
    if (IamINVIS || IamSINVIS)
        return false;
    return true;
}

// If player could manage to carry object
bool
canGive(int obj, int plyr)
{
    objtab = obtab + obj;

    if ((linestat + plyr)->weight + STATE->weight > (rktab + (usr + plyr)->rank)->maxweight)
        return false;
    if ((linestat + plyr)->numobj + 1 > (rktab + (usr + plyr)->rank)->numobj)
        return false;
    if ((objtab->flags & OF_SCENERY) || (objtab->flags & OF_COUNTER) || objtab->nrooms != 1)
        return false;
    return true;
}

verbid_t
isverb(char *s)
{
    vbptr = vbtab;
    for (verbid_t i = 0; i < verbs; i++, vbptr++)
        if (match(vbptr->id, s) == NULL)
            return i;
    return -1;
}

verbid_t
isaverb(char **s)
{
    verbid_t ret = isverb(*s);
    if (ret != -1) {
        (*s) += strlen((vbtab + ret)->id);
        return ret;
    }
    if ((ret = isvsyn(*s)) != -1) {
        (*s) += ret;
        return -2 - csyn;
    }
    return -1;
}

// Is VERB syn
int
isvsyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) < -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    return (int)(csyn = -1);
}

// Is noun syn
int
isnsyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) > -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

int
issyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (toupper(*p) == toupper(*s) && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

// Due to the complexity of a multi-ocurance/state environment, I gave up
// trying to do a 'sort', storing the last, nearest object, and went for
// an eight pass seek-and-return parse. This may be damned slow with a
// few thousand objects, but if you only have a thousand, it'll be OK!

int
isnoun(char *s, int adj, char *pat)
{
    int pass, x;

    if (iverb != -1) {
        verb.sort[0] = *pat;
        strncpy(verb.sort + 1, pat + 1, 4);
    } else {
        strcpy(verb.sort + 1, "CHAE");
        verb.sort[0] = -1;
    }
    if ((obtab + start)->adj == adj && CHAEtype(start) == verb.sort[1] &&
        (obtab + start)->state == verb.sort[0])
        return start;

    bool done_e = 0;
    int lsuc = -1, lobj = -1;
    int start = isanoun(s);
    for (int pass = 1; pass < 5; pass++) {
        /* At this point, we out to try BOTH phases, however, in the
           second phase, try for the word. Next 'pass', if there is
           no suitable item, drop back to that from the previous... */
        if (verb.sort[pass] == 'X')
            return lobj;
        if (verb.sort[0] == -1) {
            if ((x = scan(start, verb.sort[pass], -1, s, adj)) != -1) {
                if (adj != -1 || (obtab + x)->adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lsuc = 0;
                lobj = x;
            } else if (lsuc == 0)
                return lobj;
        } else {
            /* Did we get a match? */
            if ((x = scan(start, verb.sort[pass], 0, s, adj)) != -1) {
                /* If adjectives match */
                if (adj != -1 || (obtab + x)->adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lobj = x;
                lsuc = 0;
                continue;
            }
            if ((x = scan(start, verb.sort[pass], -1, s, adj)) != -1) {
                if (adj != -1 || (obtab + x)->adj == adj) {
                    lobj = x;
                    lsuc = 0;
                    continue;
                } /* Must have matched */
                if (lsuc == 0)
                    return lobj;
            }
            if (lsuc == 0)
                return lobj;
        };
        if (verb.sort[pass] == 'E')
            done_e = true;
    }
    if (!done_e)
        return scan(0, 'E', -1, s, adj);
    else
        return lobj;
}

int
isanoun(char *s)
{
    struct _OBJ_STRUCT *obpt = obtab;
    for (int i = 0; i < g_game.numObjects; i++, obpt++)
        if (!(obpt->flags & OF_COUNTER) && stricmp(s, obpt->id) == NULL)
            return i;
    return -1;
}

int
scan(int start, char Type, int tst, char *s, int adj)
{
    int last = -1;
    struct _OBJ_STRUCT *obpt = obtab + start;
    for (int i = start; i < g_game.numObjects; i++, obpt++) {
        if ((obpt->flags & OF_COUNTER) || (adj != -1 && obpt->adj != adj))
            continue;
        if (match(obpt->id, s) != NULL || CHAEtype(i) != Type)
            continue;
        /* If state doesn't matter or states match, we'll try it */
        if (verb.sort[0] == -1 || tst == -1 || obpt->state == verb.sort[0]) {
            if (adj == obpt->adj)
                return i;
            else
                last = i;
        } else if (last == -1)
            last = i;
    }
    return last;
}

char
CHAEtype(int obj)
{
    if (carrying(obj) != -1)
        return 'C';
    if (isin(obj, me2->room))
        return 'H';
    int i = owner(obj);
    if (i != -1 && (linestat + i)->room == me2->room)
        return 'A';
    return 'E';
}

auto
isadj(char *s)
{
    char *p = adtab;
    for (int i = 0; i < adjs; i++, p += IDL + 1) {
        if (match(p, s) != -1)
            return i;
    }
    return -1;
}

auto
isprep(char *s)
{
    for (int i = 0; i < NPREP; i++)
        if (stricmp(s, prep[i]) == NULL)
            return i;
    return -1;
}

bool
isin(objid_t objId, roomid_t rmId)
{
    const auto &obj = GetObject(objId);
    for (int i = 0; i < obj.nrooms; i++)
        if (obj.Room(i) == r)
            return true;
    return false;
}

auto
isroom(char *s)
{
    for (int r = 0; r < rooms; r++)
        if (stricmp((rmtab + r)->id, s) == 0)
            return r;
    return -1;
}

bool
isInflicted(int plyr, int spell)
{
    you2 = linestat + plyr;
    switch (spell) {
    case SGLOW:
        if (you2->flags & PFGLOW)
            return true;
        break;
    case SINVIS:
        if (you2->flags & PFINVIS)
            return true;
        break;
    case SDEAF:
        if (you2->flags & PFDEAF)
            return true;
        break;
    case SBLIND:
        if (you2->flags & PFBLIND)
            return true;
        break;
    case SCRIPPLE:
        if (you2->flags & PFCRIP)
            return true;
        break;
    case SDUMB:
        if (you2->flags & PFDUMB)
            return true;
        break;
    case SSLEEP:
        if (you2->flags & PFASLEEP)
            return true;
        break;
    case SSINVIS:
        if (you2->flags & PFSINVIS)
            return true;
        break;
    }
    return false;
}

bool
testStat(int plyr, int st, int x)
{
    switch (st) {
    case STSTR: return isValidNumber((linestat + plyr)->strength, x);
    case STSTAM: return isValidNumber((linestat + plyr)->stamina, x);
    case STEXP: return isValidNumber((usr + plyr)->experience, x);
    case STWIS: return isValidNumber((linestat + plyr)->wisdom, x);
    case STDEX: return isValidNumber((linestat + plyr)->dext, x);
    case STMAGIC: return isValidNumber((linestat + plyr)->magicpts, x);
    case STSCTG: return isValidNumber((linestat + plyr)->sctg, x);
    default: return false;
    }
}

// If <player1> can see <player2>
bool
canSee(int p1, int p2)
{
    /* You can't see YOURSELF, and check for various other things... */
    if (*(usr + p2)->name == 0 || p1 == p2)
        return false;
    if ((linestat + p2)->state != PLAYING)
        return false;
    /* If different rooms, or current room is dark */
    if (pROOM(p1) != pROOM(p2))
        return false;
    /* If p2 is Super Invis, he can't be seen! */
    if ((linestat + p2)->flags & PFSINVIS)
        return false;
    /* If player is blind, obviously can't see p2! */
    if ((linestat + p1)->flags & PFBLIND)
        return false;
    if (!lit(pROOM(p1)))
        return false;
    /* If you are in a 'hide' room and he isn't a wizard */
    if (pRANK(p1) == ranks - 1)
        return true;
    if (((rmtab + pROOM(p1))->flags & HIDE))
        return false;
    /* If he isn't invisible */
    if (!isPINVIS(p2))
        return true;
    /* Otherwise */
    if (isPINVIS(p1) && pRANK(p1) >= invis - 1)
        return true;
    if (pRANK(p1) >= invis2 - 1)
        return true;
    return false;
}

bool
canSeeObject(int obj, int who)
{
    if (((obtab + obj)->flags & OF_SMELL) && !((linestat + who)->flags & PFBLIND))
        return false;
    if ((linestat + who)->flags & PFBLIND &&
        (!((obtab + obj)->flags & OF_SMELL) || *(obtab + obj)->rmlist != -(5 + who)))
        return false;
    if (!isOINVIS(obj))
        return true;
    if (isPINVIS(who) && pRANK(who) >= invis - 1)
        return true;
    if (pRANK(who) < invis2 - 1)
        return false;
    return true;
}

bool
castWillSucceed(int rnk, int points, int chance)
{
    if (me->rank < rnk - 1) {
        sys(LOWLEVEL);
        return false;
    }

    if (me2->magicpts < points) {
        sys(NOMAGIC);
        return false;
    }

    if (me->rank < rnk)
        chance = ((me->rank) + 1 - rnk) * ((100 - chance) / (ranks - rnk)) + chance;
    if (randint(0, 100) < chance) {
        sys(SPELLFAIL);
        return false;
    }
    me->magicpts -= points;
    return true;
}

/* Daemon processing bizness! */

// Start a private (player owned) demon
void
dpstart(int d, int seconds)
{
	dstart(d, seconds, MDSTART);
}

// Start a global demon
void
dgstart(int d, int seconds)
{
	dstart(d, seconds, MGDSTART);
}

// Start a demon
void
dstart(int d, int seconds, int type)
{
	// Immediate?
    if (seconds == 0) {
        long v = iverb;
        long lv = lverb;
        long ld = ldir;
        long lr = lroom;
        long a1 = iadj1;
        long a2 = iadj2;
        long n1 = inoun1;
        long n2 = inoun2;
        long pp = iprep;
        lang_proc(d, 0);
        iverb = v;
        lverb = lv;
        ldir = ld;
        lroom = lr;
        iadj1 = a1;
        iadj2 = a2;
        inoun1 = n1;
        inoun2 = n2;
        iprep = pp;
    } else {
        Apx1 = inoun1;
        Apx2 = inoun2;
        Apx3 = wtype[2];
        Apx4 = wtype[5];
        SendIt(t, d, (char *)c); /* Inform AMAN... */
    }
}

// Force a demon to execute
void
dbegin(int d)
{
}

void
dshow(int d)
{
    SendIt(MCHECKD, d, NULL);
    if (Ad == -1) {
        tx("eventually");
        return;
    }
    timeto(block, Ap1);
    tx(block);
}

// Dispatch a demon to the manager
void
dsend(int p, int d, int c)
{
    if (p == Af)
        dpstart(d, c);
    sendex(p, ASTART, d, c, 0, 0); /* Tell THEM to start the daemon */
}


/* What to do when ^C pressed */ 
int
CXBRK()
{
	return 0;
}

void
memfail(char *s)
{
    txs("** Unable to allocate memory for %s! **\n", s);
    quit();
}

void
givebackmemory()
{
    if (repbk != NULL)
        DeletePort(repbk);
    if (reply != NULL)
        DeletePort(reply);
    ReleaseMem(&amul);
    if (amanrep != NULL)
        DeletePort(amanrep);
    ReleaseMem(&amanp);
    ReleaseMem(&ob);
    ReleaseMem(&ow);
    ReleaseMem(&input);
    if (serio != NULL)
        DeleteExtIO((struct IORequest *)serio);
    if (wserio != NULL)
        DeleteExtIO((struct IORequest *)wserio);
}

void
fopenr(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block, "rea");
        tx(spc);
        quit();
    }
}

void
fopena(char *s)
{
    if (afp != NULL)
        fclose(afp);
    sprintf(block, "%s%s", dir, s);
    if ((afp = fopen(block, "rb+")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block, "appen");
        tx(spc);
        quit();
    }
}

void
close_ofps()
{
    if (ofp1 != NULL)
        fclose(ofp1);
    if (ofp2 != NULL)
        fclose(ofp2);
    if (ofp3 != NULL)
        fclose(ofp3);
    ofp1 = ofp2 = ofp3 = NULL;
}

void
kquit(char *s)
{
    sprintf(block, "@me just dropped carrier.\n");
    action(ow, AOTHERS);
    quit();
}

void
quit()
{
    txs("\n%s exiting.\n\n", vername);
    scrend(); /* custom screen */
    if (link > 0 && amul->from > -1)
        SendIt(MDISCNCT, 0, me->name);
    if (ifp != NULL)
        fclose(ifp);
    if (afp != NULL)
        fclose(afp);
    close_ofps();
    givebackmemory();
    exit(0);
}

void
SendIt(int t, int d, char *p)
{
    if (link == 0)
        return;
    AMt = t;
    AMd = d;
    AMp = p;
    PutMsg(port, (struct Message *)amanp);
    WaitPort(amanrep);
    GetMsg(amanrep);
    Af = AMf;
    Ad = AMd;
    Ap = AMp;
    At = AMt;
    Ap1 = Apx1;
    Ap2 = Apx2;
    Ap3 = Apx3;
    Ap4 = Apx4;
}

void
pressret()
{
    sys(RETURN);
    Inp(block, 0);
    int l = strlen(umsgp + *(umsgip + RETURN));
    while (l > 0) {
        txc(8);
        txc(32);
        txc(8);
        l--;
    }
}

void
sys(int n)
{
	tx(umsgp + *(umsgip + n));
}

void
crsys(int n)
{
    txc('\n');
    tx(umsgp + *(umsgip + n));
}

/* GetID.C -- Get user name & details. Create record for new users. */

short int start_rm[512];

void
getid()
{
    int   i, nrs;
    FILE *fp;

    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = actor = -1;
    *(me2->pre) = *(me2->post) = 0;
    strcpy(me2->arr, acp(ARRIVED));
    strcpy(me2->dep, acp(LEFT));
    last_him = last_her = -1;
    me->rchar = 0;

    me2->rec = -1;
    me2->flags = 0;
    while (!ok) {
        getname();

        strcpy(him.name, block);
        sprintf(block, "%s%s", dir, plyrfn);

        /* Ensure the players file has been created */
        if ((fp = fopen(block, "ab+")) == NULL) {
            tx("Unable to create new player file...\n");
            quit();
        }
        fclose(fp);
        fp = afp = NULL;

        fopena(plyrfn); /* See if user is already in file */
        while (!feof(afp)) {
            fread(me->name, sizeof(him), 1, afp);
            if (stricmp(me->name, him.name) == NULL)
                break;
        }
        me2->rec = ftell(afp) / sizeof(him);

        if (stricmp(me->name, him.name) != NULL)
            ok = newid();
        else
            ok = getpasswd();
    }

    /* Inform AMAN that we have begun! */
    SendIt(MLOGGED, 0, me->name);
    for (i = ranks - 1; i >= 0; i--)
        if (me->score >= (rktab + i)->score) {
            me->rank = i;
            break;
        }
    refresh();
    if (me->plays != 1)
        sys(WELCOMEBAK);

    /* Work out the players start location */
    roomtab = rmtab;
    me2->room = -1;
    lroom = -1;
    nrs = 0;
loop:
    for (i = 0; i < rooms; i++, roomtab++) {
        if (roomtab->flags & STARTL)
            start_rm[nrs++] = i;
    }

    if (nrs == 0)
        me2->room = 0;
    if (nrs == 1)
        me2->room = start_rm[0];
    if (nrs > 1) {
        roomid_t dest = randint(0, nrs - 1);
        me2->room = start_rm[dest];
    }
    roomtab = rmtab + me2->room;

    me2->wield = -1;
    me2->helping = -1;
    me2->helped = -1;
    me2->following = -1;
    me2->followed = -1;
    me2->fighting = -1;
    me2->numobj = 0;
    me2->weight = 0;
    me2->hadlight = 0;
    me2->light = 0;
    me2->flags = 0;
    me2->sctg = 0;
    ans("1m");
    if (me->flags & ufANSI)
        sys(ANSION);
    ans("0;37m");
    me->strength = (rktab + me->rank)->strength;
    me->stamina = (rktab + me->rank)->stamina;
    me->magicpts = (rktab + me->rank)->magicpts;
    if ((i = isverb("\"start")) != -1)
        lang_proc(i, 0);
    action(acp(COMMENCED), AOTHERS);
    look(roomtab->id, me->rdmode);
}

void
getname()
{
    char *p;
    int   i;

    do {
        word = -3;

        crsys(WHATNAME);
        block[0] = 0;
        Inp(block, NAMEL + 1);
        txc('\n');
        if (block[0] == 0)
            quit();
        if (strlen(block) < 3 || strlen(block) > NAMEL) {
            sys(LENWRONG);
            continue;
        }

        p = block;
        if ((i = type(&p)) > -1 && i != WPLAYER) {
            sys(NAME_USED);
            continue;
        }
        if (i == WPLAYER && word != Af) {
            utx(word, acp(LOGINASME));
            strcpy(me->name, block);
            sys(ALREADYIN);
            continue;
        }
        word = -2;
    } while (word != -2);
}

bool
newid()
{
    strcpy(me->name, him.name);
    sys(CREATE);
    *me->name = 0;
    block[0] = 0;
    Inp(block, 3);
    if (toupper(block[0]) != 'Y')
        return false;

    me->score = 0;
    me->plays = 1;
    me->strength = rktab->strength;
    me->stamina = rktab->stamina;
    me->dext = rktab->dext;
    me->wisdom = rktab->wisdom;
    me->experience = rktab->experience;
    me->magicpts = rktab->magicpts;
    me->tasks = 0;
    me->tries = me->gender = me->rank = me->rdmode = 0;
    me->llen = DLLEN;
    me->slen = DSLEN;

    do {
        crsys(WHATGENDER);
        block[0] = 0;
        strcpy(me->name, him.name);
        Inp(block, 2);
        block[0] = toupper(block[0]);
        if (block[0] != 'M' && block[0] != 'F') {
            crsys(GENDINVALID);
        }
    } while (block[0] != 'M' && block[0] != 'F');
    me->gender = (block[0] == 'M') ? 0 : 1;

    do {
        crsys(ENTERPASSWD);
        block[0] = 0;
        Inp(block, 10);
        if (strlen(block) < 3 || strlen(block) > 8) {
            crsys(PASLENWRONG);
        }
    } while (strlen(block) < 3 || strlen(block) > 8);
    strcpy(me->passwd, block);

    me->name[0] = toupper(me->name[0]);
    for (int i = 1; i < strlen(me->name); i++) {
        if (me->name[i - 1] == ' ')
            me->name[i] = toupper(me->name[i]);
        else
            me->name[i] = tolower(me->name[i]);
    }

    crsys(ASK4ANSI);
    block[0] = 0;
    Inp(block, 4);
    me->flags = (toupper(block[0]) == 'Y') ? me->flags | ufANSI : me->flags & -(1 + ufANSI);

    flagbits();

    save_me();
    txc('\n');

    /* Send them the scenario */
    ShowFile("scenario");

    crsys(YOUBEGIN);
    txc('\n');

    return true;
}

bool
getpasswd()
{
    me2->rec--; /* Move 'back' a record */

    for (int i = 0; i < 4; i++) {
        if (i == 3) {
            sys(TRIESOUT); /* Update bad try count */
            me->tries++;
            save_me();
            quit();
        }
        txn("\nTry #%d -- ", i + 1);
        sys(ENTERPASSWD);
        block[0] = 0;
        Inp(block, 10);
        if (stricmp(block, me->passwd) == NULL)
            break;
    }
    me->plays++;
    if (me->tries > 0) {
        ans("1m");
        txc(0x7);
        txc('\n');
        txn(acp(FAILEDTRIES), me->tries);
        txc('\n');
        ans("0;37m");
    }
    me->tries = 0;
    return true;
}

void
lockusr(int u)
{
    long t, d, p;
    do {
        t = At;
        d = Ad;
        p = (long)Ap;
        SendIt(MLOCK, u, NULL);
        if (Ad != u && ip == 0) {
            iocheck();
            Ad = -1;
        }
    } while (Ad != u);
    At = t;
    Ad = d;
    Ap = (char *)p;
}

void
interact(int msg, int n, int d)
{
    if ((linestat + n)->state < PLAYING)
        return;
    lockusr(n);
    if (msg == MMESSAGE)
        strcat((linestat + n)->buf, ow);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = msg;
    IAd = d;
    (linestat + n)->IOlock = -1;
    PutMsg((linestat + n)->rep, (struct Message *)intam);
}

void
sendex(int n, int d, int p1, int p2, int p3, int p4)
{
    if ((linestat + n)->state < PLAYING)
        return;
    lockusr(n);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = MEXECUTE;
    IAd = -(1 + d);
    intam->p1 = p1;
    intam->p2 = p2;
    intam->p3 = p3;
    intam->p4 = p4;
    (linestat + n)->IOlock = -1;
    PutMsg((linestat + n)->rep, (struct Message *)intam);
}

/* The AMUL parser and VT processor */

/* Copy INPUT to BLOCK, taking one sentence at a time, etc */
void
grab()
{
    char *s = input;
    more = 10;
    forced = 0;
    exeunt = 0;
    failed = false;;
    died = 0;
    donet = -1;
    ml = 0;

    do {
        char *d = block;
        block[0] = 0;
    loop:
        *d = 0;
        while (isspace(*s))
            s++; /* Skip spaces */
        if (*s == 0) {
            *(s + 1) = 0;
            goto proc;
        }
    quotes:
        if (block[0] != 0)
            *(d++) = ' '; /* Force spacing */
        if (*s == '\'' || *s == '\"') {
            char quote = *(d++) = *(s++);    /* Store which */
            while (*s != quote && *s != 0)   /* Find match or \0 */
                *(d++) = *(s++);
            if (*s == 0)
                *(s + 1) = 0;
            *(d++) = *(s++);
            *d = 0;
            if (*s != 0)
                s++; /* Skip " or ' at end */
            goto loop;
        }

        char *p = d;
        while (*s != 0 && !isspace(*s) && *s != '!' && *s != ';' && *s != ',' && *s != '.' &&
               *s != '\"' && *s != '\'')
            *(d++) = *(s++);
        *d = 0;
        *(d + 1) = 0;
        if (stricmp(p, "then") == NULL || stricmp(p, "and") == NULL) {
            *p = 0;
            goto proc;
        }
        if (*s == '\'' || *s == '\"')
            goto quotes;
        if (isspace(*s))
            goto loop;
    proc:
        if (*s != 0 && *s != '\'' && *s != '\"')
            s++;
        if (block[0] == 0)
            continue;
        /* Print the prompt & the line, if not first text */
        if (more != 10) {
            ans("3m");
            tx((rktab + me->rank)->prompt);
            txs("%s\n", block);
            ans("0;37m");
        }
        if (parser() == -1)
            return;
    } while (*s != 0 && more == 0 && exeunt == 0 && forced == 0 && !failed && died == 0);
    iocheck();
}

void
parser()
{
    int om = more; /* We need to know if this is phrase one in a mo... */
    more = 0;
    if (strlen(block) == 0)
        return;

    int x;
phrase:
    wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = WNONE;
    iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = -1;
    char *p = block + strlen(block) - 1;
    while (p != block && isspace(*p))
        *(p--) = 0;
    if (me->rank >= (minsgo - 1) && (x = isroom(block)) != -1) {
        if (isVisible())
            action(acp(SGOVANISH), AOTHERS);
        StopFollow();
        LoseFollower();
        sys(SGO);
        moveto(x);
        if (isVisible())
            action(acp(SGOAPPEAR), AOTHERS);
        return;
    }
    iocheck();
    if (forced != 0 || exeunt != 0 || died != 0)
        return;
    p = block;
    if (*p == '\"' || *p == '\'') {
        char *p2;
        if (*(p + 1) == 0)
            return;
        if ((iverb = isverb("\"speech")) == -1) {
            sys(CANTDO);
            return -1;
        }
        p2 = p + 1;
    loop:
        while (*p != *p2 && *p2 != 0)
            p2++;
        *(p2 + 1) = *(p2) = 0;
        inoun1 = (long)p + 1;
        wtype[2] = WTEXT;
        goto skip;
    }
    if ((word = isaverb(&p)) == -1) {
        char *bp;
        /*== If not a verb, then check for g_game.numObjects. If they typed
            > get NounA, NounB, NounC  then allow it.
             If they typed
            > kiss PlayerA, NounA
             don't allow it.
        */

        bp = p;
        if (om == 10 || (x = type(&bp)) == -2) {
            sys(INVALIDVERB);
            more = 1;
            return -1;
        }
        word = iverb;
    }
    x = WVERB;
    vbptr = vbtab + word;
    if ((me2->flags & PFASLEEP) && !(vbptr->flags & VB_DREAM)) {
        tx("You can't do anything until you wake up!\n");
        failed = true;
        return -1;
    }
    if (!(vbptr->flags & VB_TRAVEL)) {
        if ((x = isverb("\"travel")) != -1) {
            vbptr = vbtab + word;
            if (lang_proc(x, 1) == 0)
                return;
        }
        vbptr = vbtab + word;
    }
    if (iverb >= 0)
        lverb = iverb;
    iverb = word;
    vbptr = vbtab + iverb;
    wtype[0] = WVERB;

    /* adjectives are optional, so assume next word is a noun */
l1:
    if (*p == 0)
        goto ended;
    wtype[2] = type(&p);
    inoun1 = word;
    if (wtype[2] == WNOUN)
        it = inoun1;
    if (wtype[2] == WADJ) {
        if (wtype[1] != -1) {
            sys(NONOUN);
            return -1;
        }
        wtype[1] = WADJ;
        iadj1 = inoun1;
        wtype[2] = -1;
        inoun1 = -1;
        goto l1;
    }
    if (wtype[2] == WPREP) {
        if (wtype[3] != -1) {
            sys(WORDMIX);
            return -1;
        }
        wtype[3] = WPREP;
        iprep = inoun1;
        wtype[2] = -1;
        inoun1 = -1;
    }
l2:
    if (*p == 0)
        goto ended;
    wtype[5] = type(&p);
    inoun2 = word;
    if (wtype[5] == WNOUN)
        it = inoun2;
    if (wtype[5] == WPREP) {
        if (wtype[3] != -1) {
            sys(WORDMIX);
            return -1;
        }
        wtype[3] = WPREP;
        iprep = inoun2;
        wtype[5] = -1;
        inoun2 = -1;
        goto l2;
    }
    if (wtype[5] == WADJ) {
        if (wtype[4] != -1) {
            sys(NONOUN);
            return -1;
        }
        wtype[4] = WADJ;
        iadj2 = inoun2;
        wtype[5] = -1;
        inoun2 = -1;
        goto l2;
    }
ended:
    overb = iverb;
    vbptr = vbtab + iverb;
skip:
    iocheck();
    if (forced != 0 || exeunt != 0 || died != 0 || failed == true)
        return;
    return lang_proc(iverb, 0);
}

auto
lang_proc(int v, char e)
{
    int j, l, m;

    forced = 0;
    exeunt = 0;
    failed = false;
    died = 0;
    donet = 0;
    ml = 0;
    int d = -2;
    tt.verb = -1;
    vbptr = vbtab + v;
caloop:
    for (int i = 0; i < (vbtab + v)->ents; i++) {
        int m = 0;
        stptr = vbptr->ptr + i;
        donet = 0;
        ml = stptr->ents;
        if (stptr->wtype[2] != WANY)
            for (j = 0; j < 5 && m == 0; j++) {
                if (stptr->wtype[j] == WANY && (j == 0 || j == 3 || wtype[j + 1] != WNONE))
                    continue;
                if (stptr->wtype[j] != wtype[j + 1]) {
                    m = 1;
                    continue;
                }
                /* We have a match, now see if its the same word! */
                if (stptr->slot[j] == WANY)
                    continue;
                switch (j) {
                case 0:
                    if (iadj1 != stptr->slot[j])
                        m = 1;
                    break;
                case 1:
                    if (stptr->slot[j] == WNONE && inoun1 == WNONE)
                        break;
                    if (stptr->wtype[j] == WPLAYER && inoun1 == Af && stptr->slot[j] == -3)
                        break;
                    if (stptr->wtype[j] == WTEXT &&
                        stricmp((char *)inoun1, umsgp + *(umsgip + stptr->slot[j])) == NULL)
                        break;
                    if (stptr->wtype[j] == WNOUN &&
                        stricmp((obtab + inoun1)->id, (obtab + stptr->slot[j])->id) == NULL)
                        break;
                    if (inoun1 != stptr->slot[j])
                        m = 1;
                    break;
                case 2:
                    if (iprep != stptr->slot[j])
                        m = 1;
                    break;
                case 3:
                    if (iadj2 != stptr->slot[j])
                        m = 1;
                    break;
                case 4:
                    if (stptr->slot[j] == WNONE && inoun2 == WNONE)
                        break;
                    if (stptr->wtype[j] == WPLAYER && inoun2 == Af && stptr->slot[j] == -3)
                        break;
                    if (stptr->wtype[j] == WTEXT &&
                        stricmp((char *)inoun2, umsgp + *(umsgip + stptr->slot[j])) == NULL)
                        break;
                    if (stptr->wtype[j] == WNOUN &&
                        stricmp((obtab + inoun2)->id, (obtab + stptr->slot[j])->id) == NULL)
                        break;
                    if (inoun2 != stptr->slot[j])
                        m = 1;
                    break;
                }
            }
        if (m != 0)
            goto after;
        l = -1;
        d = -1;
        for (donet = 0; donet < ml; donet++) {
            stptr = vbptr->ptr + i;
            vtabp = stptr->ptr + donet;
            tt.action = vtabp->action;
            tt.pptr = vtabp->pptr;
            if (skip != 0) {
                skip--;
                continue;
            }
            if ((l = cond(vtabp->condition, l)) == -1)
                continue;
            inc = 1;
            act(tt.action, vtabp->pptr);
            if (inc == 1)
                d = 0;
            if (ml < -1) {
                d = lang_proc(iverb, e);
                donet = ml + 1;
                break;
            }
            if (ml < 0 || failed == true || forced != 0 || died != 0 || exeunt != 0)
                break;
        }
        if (failed == true || forced != 0 || died != 0 || exeunt != 0)
            break;
    after:
        if (donet > ml)
            break;
    }
    if (d > -1)
        return 0; /* If we processed something... */

    vbptr = vbtab + v;
    if (!(vbptr->flags & VB_TRAVEL)) {
        int iv;
        iv = iverb;
        iverb = v;
        if (ttproc() == 0)
            return 0;
        else
            d = -1;
        iverb = iv;
    }
    if (d == -2 && e == 0)
        sys(ALMOST);
    if (d == -1 && e == 0) {
        if (!(vbptr->flags & VB_TRAVEL))
            sys(CANTGO);
        else
            sys(CANTDO);
    }
    return -1;
}

/* Phrase/sentence processing */

int
ttproc()
{
    exeunt = died = donet = skip = 0;
    failed = false;
    roomtab = rmtab + me2->room;
    int l = -1;
    tt.verb = iverb;
    if (roomtab->tabptr == -1) {
        sys(CANTGO);
        return 0;
    }
    int dun = -1;
    ml = roomtab->ttlines;
    struct _TT_ENT *tp = ttp + roomtab->tabptr;

    iocheck();
    if (forced != 0 || died != 0 || exeunt != 0)
        return 0;
more:
    for (int i = donet; i < ml; i++) {
        ttabp = tp + i;
        int match = -1;
        donet++;
        tt.pptr = ttabp->pptr;
        if (skip != 0) {
            skip--;
            continue;
        }
        if (ttabp->verb == iverb && (l = cond(ttabp->condition, l)) != -1) {
            match = i;
            break;
        }
    }
    skip = 0;
    if (ttabp->condition == CSPELL && match == -1)
        return 0;
    if (match == -1)
        return dun;
    tt.condition = ttabp->condition;
    inc = 1;
    act(ttabp->action, ttabp->pptr);
    if (inc == 1)
        dun = 0;
    if (ml < -1) {
        ml = roomtab->ttlines;
        donet = 0;
    }
    if (donet < ml && exeunt == 0 && died == 0)
        goto more;
    return 0;
}

void
act(long ac, long *pt)
{
    long x1, x2;
    int  i;
    tt.action = ac;
    tt.pptr = pt;
    if (tt.condition < 0)
        tt.condition = -1 - tt.condition;

    if (ac < 0) {
        ac = -1 - ac;
        switch (ac) {
        case ASAVE: asave(); break;
        case ASCORE: ascore(TP1); break;
        case ASETSTAT: asetstat(TP1, TP2); break;
        case ALOOK: look((rmtab + me2->room)->id, 1); break;
        case AWHAT: list_what(me2->room, true); break;
        case AWHERE: awhere(TP1); break;
        case AWHO: awho(TP1); break;
        case ATREATAS: atreatas(TP1); return;
        case ASKIP: skip += TP1; break;
        case ATRAVEL:
            x1 = donet;
            x2 = ml;
            if (ttproc() == 0)
                donet = ml = x2;
            else {
                ml = x2;
                donet = x1;
            };
            break;
        case AQUIT: aquit();
        case AENDPARSE: donet = ml + 1; break;
        case AKILLME: akillme(); break;
        case AFAILPARSE: afailparse(); break;
        case AFINISHPARSE: afinishparse(); break;
        case AABORTPARSE: aabortparse(); break;
        case AWAIT: fwait(TP1); break;
        case ABLEEP: ableep(TP1); break;
        case AWHEREAMI: txs("Current room is known as \"%s\".\n", (rmtab + me2->room)->id); break;
        case ASEND: send(TP1, TP2); break;
        case AERROR: afailparse();     /* Set those flags! */
        case ARESPOND: donet = ml + 1; /* Then do reply etc */
        case AREPLY:
        case AMESSAGE: tx(AP1); break;
        case AANOUN: announce(AP2, TP1); break;
        case ACHANGESEX: achange(TP1); break;
        case ASLEEP: me2->flags = me2->flags | PFASLEEP; break;
        case AWAKE: me2->flags = me2->flags & (-1 - PFASLEEP); break;
        case ASIT:
            me2->flags = me2->flags | PFSITTING;
            me2->flags = me2->flags & (-1 - PFLYING);
            break;
        case ASTAND: me2->flags = me2->flags & (-1 - PFSITTING - PFLYING); break;
        case ALIE:
            me2->flags = me2->flags | PFLYING;
            me2->flags = me2->flags & (-1 - PFSITTING);
            break;
        case ARDMODE:
            me->rdmode = TP1;
            txs("%s mode selected.\n",
                (me->rdmode == RDRC) ? "Roomcount" : (me->rdmode == RDVB) ? "Verbose" : "Brief");
            break;
        case ARESET: SendIt(MRESET, 0, NULL); break; /* Tell AMAN that we want a reset! */
        case AACTION: action(AP2, TP1); break;
        case AMOVE: moveto(TP1); break;
        case AMSGIN: announcein(TP1, AP2); break;
        case AACTIN: actionin(TP1, AP2); break;
        case AMSGFROM: announcefrom(TP1, AP2); break;
        case AACTFROM: actionfrom(TP1, AP2); break;
        case ATELL:
            if (!((linestat + TP1)->flags & PFDEAF)) {
                setmxy(NOISE, TP1);
                utx(TP1, AP2);
            }
            break;
        case AADDVAL: aadd(scaled(State(TP1)->value, State(TP1)->flags), STSCORE, Af); break;
        case AGET: agive(TP1, Af); break;
        case ADROP: adrop(TP1, me2->room); break;
        case AINVENT:
            strcpy(block, "You are ");
            invent(Af);
            break;
        case AGIVE: agive(TP1, TP2); break;
        case AINFLICT: inflict(TP1, TP2); break;
        case ACURE: cure(TP1, TP2); break;
        case ASUMMON: summon(TP1); break;
        case AADD: aadd(TP1, TP2, TP3); break;
        case ASUB: asub(TP1, TP2, TP3); break;
        case ACHECKNEAR: achecknear(TP1); break;
        case ACHECKGET: acheckget(TP1); break;
        case ADESTROY: adestroy(TP1); break;
        case ARECOVER: arecover(TP1); break;
        case ASTART: dpstart(TP1, TP2); break;
        case AGSTART: dgstart(TP1, TP2); break;
        case ACANCEL: SendIt(MDCANCEL, TP1, NULL); break;
        case ABEGIN: dbegin(TP1); break;
        case ASHOWTIMER: dshow(TP1); break;
        case AOBJAN: objannounce(TP1, AP2); break;
        case AOBJACT: objaction(TP1, AP2); break;
        case ACONTENTS:
            tx(showin(TP1, true).c_str());  ///SV: Direct
            break;
        case AFORCE: aforce(TP1, TP2); break;
        case AHELP:
            me2->helping = TP1;
            (linestat + TP1)->helped = Af;
            break;
        case ASTOPHELP:
            (linestat + me2->helping)->helped = -1;
            me2->helping = -1;
            break;
        case AFIX: afix(TP1, TP2); break;
        case AOBJINVIS: (obtab + TP1)->flags = (obtab + TP1)->flags | OF_INVIS; break;
        case AOBJSHOW: (obtab + TP1)->flags = (obtab + TP1)->flags & (-1 - OF_INVIS); break;
        case AFIGHT: afight(TP1); break;
        case AFLEE:
            dropall((linestat + me2->fighting)->room);
            clearfight();
            break;
        case ALOG: log(AP1); break;
        case ACOMBAT: acombat(); break;
        case AWIELD:
            me2->wield = TP1;
            break;
        /* - */ case AFOLLOW:
            (linestat + TP1)->followed = me2->unum;
            me2->following = TP1;
            break;
        /* - */ case ALOSE:
            LoseFollower();
            break;
        /* - */ case ASTOPFOLLOW:
            StopFollow();
            break;
        case AEXITS: exits(); break;
        case ATASK: me->tasks = me->tasks | (1 << (TP1 - 1)); break;
        case ASHOWTASK: show_tasks(Af); break;
        case ASYNTAX:
            asyntax(*(tt.pptr + ncop[tt.condition]), *(tt.pptr + ncop[tt.condition] + 1));
            break;
        case ASETPRE: iocopy((linestat + TP1)->pre, AP2, 79); break;
        case ASETPOST: iocopy((linestat + TP1)->post, AP2, 79); break;
        case ASETARR:
            qcopy((linestat + TP1)->arr, AP2, 79);
            strcat((linestat + TP1)->arr, "\n");
            break;
        case ASETDEP:
            qcopy((linestat + TP1)->dep, AP2, 79);
            strcat((linestat + TP1)->dep, "\n");
            break;
        case ASENDDAEMON: dsend(TP1, TP2, TP3); break;
        case ADO: ado(TP1); break;
        case AINTERACT: ainteract(TP1); break;
        case AAUTOEXITS: autoexits = (char)TP1; break;
        case ABURN: osflag(CP1, State(CP1)->flags | SF_LIT); break;
        case ADOUSE: osflag(CP1, State(CP1)->flags & -(1 + SF_LIT)); break;
        case AINC:
            if ((obtab + CP1)->state != ((obtab + CP1)->nstates - 1))
                asetstat(CP1, (obtab + CP1)->state + 1);
            break;
        case ADEC:
            if ((obtab + CP1)->state != 0)
                asetstat(CP1, (obtab + CP1)->state - 1);
            break;
        case ATOPRANK: toprank(); break;
        case ADEDUCT: deduct(TP1, TP2); break;
        case ADAMAGE: damage(TP1, TP2); break;
        case AREPAIR: repair(TP1, TP2); break;
        default: txn("** Internal error - illegal action %ld!\n", ac);
        }
    } else {
        bool flag { false };

        needcr = false;
        iocheck();
        if (exeunt != 0 || died != 0)
            return;
        if (fol == 0)
            StopFollow();
        Forbid();
        if ((rmtab + ac)->flags & SMALL) /* Allow for losing follower! */
        {
            for (i = 0; i < MAXU; i++)
                if ((linestat + i)->room == ac) {
                    Permit();
                    sys(NOROOM);
                    actionin(ac, acp(NOROOMIN));
                    LoseFollower();
                    return;
                };
        }
        me2->flags = me2->flags | PFMOVING; /* As of now I'm out of here. */
        if (isVisible()) {
            Permit();
            action(me2->dep, AOTHERS);
            Forbid();
        }
        ldir = iverb;
        lroom = me2->room;
        i = me2->light;
        me2->light = 0;
        Permit();
        lighting(Af, AOTHERS);
        StopFollow();
        me2->room = ac;
        me2->light = i;
        me2->hadlight = 0;
        lighting(Af, AOTHERS);
        if (isVisible())
            action(me2->arr, AOTHERS);
        me2->flags = me2->flags & -(1 + PFMOVING);
        if (me2->followed > -1 && me2->followed != Af && (!IamINVIS) && (!IamSINVIS)) {
            /* If we didn't just execute a travel verb, we've lost them.
               If the other player hasn't caught up with us, lose them! */
            if (((vbtab + overb)->flags & VB_TRAVEL) || (linestat + me2->followed)->room != lroom ||
                ((linestat + me2->followed)->flags & PFMOVING))
                LoseFollower();
            else {
                DoThis(me2->followed, (vbtab + overb)->id, 1);
                flag = true;
            }
        } else
            me2->followed = -1;
        look((rmtab + me2->room)->id, me->rdmode);
        if (exeunt != 0 || died != 0)
            return;
        if (autoexits != 0)
            exits();
    }
    if (tt.condition == CANTEP || tt.condition == CALTEP || tt.condition == CELTEP)
        donet = ml + 1;
}

// Execute a condition on me
int
cond(long n, int l)
{
    int mult = 1;
    int ret = 1;
    tt.condition = (n < 0) ? -1 - n : n;
    if (n < 0) {
        n = -1 - n;
        mult = -1;
    }

    /* Do the conditions */
    switch (n) {
    case CALTEP:
    case CSTAR:
    case CALWAYS: break;
    case CANTEP:
    case CAND: ret = l; break;
    case CELTEP:
    case CELSE: ret = -l; break;
    case CLIGHT:
        if (!lit(me2->room))
            ret = -1;
        break;
    case CISHERE:
        if (!isin(CP1, me2->room))
            ret = -1;
        break;
    case CMYRANK:
        if (!isValidNumber(me->rank + 1, CP1))
            ret = -1;
        break;
    case CSTATE:
        if ((obtab + CP1)->flags & OF_ZONKED)
            ret = -1;
        if (!isValidNumber((obtab + CP1)->state, CP2))
            ret = -1;
        break;
    case CMYSEX:
        if (me->gender != CP1)
            ret = -1;
        break;
    case CLASTVB:
        if (lverb != CP1)
            ret = -1;
        break;
    case CLASTDIR:
        if (ldir != CP1)
            ret = -1;
        break;
    case CLASTROOM:
        if (lroom != CP1)
            ret = -1;
        break;
    case CASLEEP:
        if (!(me2->flags & PFASLEEP))
            ret = -1;
        break;
    case CSITTING:
        if (!(me2->flags & PFSITTING))
            ret = -1;
        break;
    case CLYING:
        if (!(me2->flags & PFLYING))
            ret = -1;
        break;
    case CRAND:
        if (!isValidNumber(randint(0, CP1), *(tt.pptr + 1))
            ret = -1;
        break;
    case CRDMODE:
        if (me->rdmode != CP1)
            ret = -1;
        break;
    case CONLYUSER:
        for (int i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (linestat + i)->state > 1)
                ret = -1;
        break;
    case CALONE:
        for (int i = 0; i < MAXU; i++)
            if (((linestat + i)->room == me2->room) && i != Af)
                ret = -1;
        break;
    case CINROOM:
        if (me2->room != CP1)
            ret = -1;
        break;
    case COPENS:
        if (!((obtab + CP1)->flags & OF_OPENS))
            ret = -1;
        break;
    case CGOTNOWT:
        if (me2->numobj != 0)
            ret = -1;
        break;
    case CCARRYING:
        if (!gotin(CP1, -1))
            ret = -1;
        break;
    case CNEARTO:
        if (!nearto(CP1))
            ret = -1;
        break;
    case CHIDDEN:
        if (isVisible())
            ret = -1;
        break;
    case CCANGIVE:
        if (!canGivecangive(CP1, CP2))
            ret = -1;
        break;
    case CINFL:
    case CINFLICTED:
        if (!isInflicted(CP1, CP2))
            ret = -1;
        break;
    case CSAMEROOM:
        if ((linestat + CP1)->room != me2->room)
            ret = -1;
        break;
    case CTOPRANK:
        if (me->rank != ranks - 1)
            ret = -1;
        break;
    case CSOMEONEHAS:
        if (((*(obtab + CP1)->rmlist) > -5) || ((*(obtab + CP1)->rmlist) < (-5 - MAXU)))
            ret = -1;
        break;
    case CGOTA:
        if (!gotin(CP1, CP2))
            ret = -1;
        break;
    case CACTIVE:
        SendIt(MCHECKD, CP1, NULL);
        if (Ad == -1)
            ret = -1;
        break;
    case CTIMER:
        SendIt(MCHECKD, CP1, NULL);
        if (Ad == -1 || !isValidNumber(Ap1, CP2))
            ret = -1;
        break;
    case CBURNS:
        if (!((obtab + CP1)->flags & OF_FLAMABLE))
            ret = -1;
        break;
    case CCONTAINER:
        if ((obtab + CP1)->contains <= 0)
            ret = -1;
        break;
    case CEMPTY:
        if ((obtab + CP1)->inside != 0)
            ret = -1;
        break;
    case COBJSIN:
        if (!isValidNumber((obtab + CP1)->inside, CP2))
            ret = -1;
        break;
    case CHELPING:
        if (me2->helping != CP1)
            ret = -1;
        break;
    case CGOTHELP:
        if (me2->helped == -1)
            ret = -1;
        break;
    case CANYHELP:
        if (me2->helping == -1)
            ret = -1;
        break;
    case CSTAT:
        if (!testStat(CP2, CP1, CP3))
            ret = -1;
        break;
    case COBJINV:
        if (!isOINVIS(CP1))
            ret = -1;
        break;
    case CFIGHTING:
        if (!((linestat + CP1)->flags & PFFIGHT))
            ret = -1;
        break;
    case CTASKSET:
        if (!(me->tasks & (1 << (CP1 - 1))))
            ret = -1;
        break;
    case CCANSEE:
        if (!canSee(Af, CP1))
            ret = -1;
        break;
    case CVISIBLETO:
        if (!canSee(CP1, Af))
            ret = -1;
        break;
    case CNOUN1:
        if (wtype[2] != WNOUN) {
            ret = -1;
            break;
        }
        if (CP1 == inoun1)
            break;
        if (CP1 == WNONE) {
            ret = -1;
            break;
        }
        if (stricmp((obtab + CP1)->id, (obtab + inoun1)->id) != NULL)
            ret = -1;
        break;
    case CNOUN2:
        if (wtype[5] != WNOUN) {
            ret = -1;
            break;
        }
        if (CP1 == WNONE) {
            ret = -1;
            break;
        }
        if (stricmp((obtab + CP1)->id, (obtab + inoun2)->id) != NULL)
            ret = -1;
        break;
    case CAUTOEXITS:
        if (autoexits == 0)
            return -1;
        break;
    case CDEBUG:
        if (debug == 0)
            return -1;
        break;
    case CFULL:
        if (!isStatFull(CP1, CP2))
            return -1;
        break;
    case CTIME:
        if (!isValidNumber(*rescnt, CP1))
            return -1;
        break;
    case CDEC:
        if ((obtab + CP1)->state == 0)
            return -1;
        asetstat(CP1, (obtab + CP1)->state - 1);
        break;
    case CINC:
        if ((obtab + CP1)->state >= ((obtab + CP1)->nstates - 1))
            return -1;
        asetstat(CP1, (obtab + CP1)->state + 1);
        break;
    case CLIT:
        if (!(State(CP1)->flags & SF_LIT))
            return -1;
        break;
    case CFIRE:
        if (!((obtab + CP1)->flags & OF_SHOWFIRE))
            return -1;
        break;
    case CHEALTH:
        if (!isValidNumber((((linestat + CP1)->stamina * 100) / (rktab + (usr + CP1)->rank)->stamina), CP2))
            ret = -1;
        break;
    case CMAGIC:
        if (!castWillSucceed(CP1, CP2, CP3))
            return -1;
        break;
    case CSPELL:
        if (!isValidNumber((linestat + CP1)->wisdom, randint(0, CP2)))
            ret = -1;
        break;
    case CIN:
        if (!isin(CP2, CP1))
            ret = -1;
        break;
    default: ret = -1;
    }

    return mult * ret;
}

int
type(char **s)
{
strip:
    char *p = *s;
    while (isspace(*p))
        p++;
    *s = p;
    if (*p == 0) {
        word = -1;
        return -1;
    } /* none */

    /* Check for a players name BEFORE checking for white words! */

    for (word = 0; word < MAXU; word++)
        if ((linestat + word)->state == PLAYING && match((usr + word)->name, p) == NULL) {
            *s += strlen((usr + word)->name);
            return WPLAYER;
        }

    /* cut any white words out */
    if (tolower(*p) == 'a') {
        if (match("an", p) == NULL) {
            *s += 2;
            goto strip;
        }
        if (match("at", p) == NULL) {
            *s += 2;
            goto strip;
        }
        if (match("a", p) == NULL) {
            *s++;
            goto strip;
        }
    }
    if (tolower(*p) == 't') {
        if (match("the", p) == NULL) {
            *s += 3;
            goto strip;
        }
        if (match("to", p) == NULL) {
            *s += 2;
            goto strip;
        }
    }
    if (tolower(*p) == 'f') {
        if (match("for", p) == NULL) {
            *s += 3;
            goto strip;
        }
        if (match("from", p) == NULL) {
            *s += 4;
            goto strip;
        }
    }
    if (match("is", p) == NULL) {
        *s += 2;
        goto strip;
    }
    if (match("using", p) == NULL) {
        *s += 5;
        goto strip;
    }
    if (match("with", p) == NULL) {
        *s += 4;
        goto strip;
    }

    if (*p == '\"' || *p == '\'') {
        char c;
        c = *p;
        char *p2 = ++p;
        word = (int)p;
        while (*p2 != 0 && *p2 != c)
            p2++;
        if (*p2 != 0)
            *(p2++) = 0;
        else
            *(p2 + 1) = 0;
        *s = p2;
        return WTEXT;
    }

    if (match("it", p) == NULL) {
        word = it;
        *s += 2;
        return (it != -1) ? WNOUN : -2;
        ;
    }

    if (tolower(*p) == 'm') {
        if (match("me", p) == NULL) {
            word = Af;
            *s += 2;
            return WPLAYER;
        }
        if (match("myself", p) == NULL) {
            word = Af;
            *s += 2;
            return WPLAYER;
        }
    }

    /* inoun/sort crap is related to object chae patterns */

    if ((word = issyn(*s)) != -1) {
        *s += word;
        if (csyn < -1) {
            word = -2 - csyn;
            return WVERB;
        }
        if (inoun1 == inoun2 == -1) {
            word = csyn;
            return WNOUN;
        }
        if ((word =
                     isnoun((obtab + csyn)->id, (inoun1 == -1) ? iadj1 : iadj2,
                            (inoun1 == -1) ? (vbtab + iverb)->sort : (vbtab + iverb)->sort2)) != -1)
            return WNOUN;
        return -1;
    }
    if (inoun1 == inoun2 == -1) {
        if ((word = isanoun(*s)) != -1) {
            *s += strlen((obtab + word)->id);
            return WNOUN;
        }
    } else if (
            (word = isnoun(
                     *s, (inoun1 == -1) ? iadj1 : iadj2,
                     (inoun1 == -1) ? (vbtab + iverb)->sort : (vbtab + iverb)->sort2)) != -1) {
        *s += strlen((obtab + word)->id);
        return WNOUN;
    }
    if ((word = isadj(*s)) != -1) {
        *s += strlen(adtab + (word * (IDL + 1)));
        return WADJ;
    }
    if ((word = isroom(*s)) != -1) {
        *s += strlen((rmtab + word)->id);
        return WROOM;
    }
    if ((word = isprep(*s)) != -1) {
        *s += strlen(prep[word]);
        goto strip;
    }
    if ((word = isverb(*s)) != -1) {
        *s += strlen(vbptr->id);
        return WVERB;
    }
    while (!isspace(*p) && *p != 0)
        p++;
    word = -1;
    return -2; /* Unknown */
}

int
actual(unsigned long n)
{
    // Rand 0 is a simple rand(N)
    if (n & RAND0)
        return randint(0, n ^ RAND0);
    // Rand 1 is N +/- .5N
    if (n & RAND1) {
        auto nval = n ^ RAND1;
        return nval / 2 + randint(nval);
    }
    if (n & PRANK)
        return pRANK(actual(n & -(1 + PRANK)));

    if ((n & (OBVAL + OBDAM + OBWHT)) != 0) {
        int x;
        x = actual(n & (-1 - (OBVAL + OBDAM + OBWHT)));
        switch (n & (OBVAL + OBDAM + OBWHT)) {
        case OBVAL: return (int)scaled(State(x)->value, State(x)->flags);
        case OBWHT: return (int)State(x)->weight;
        case OBDAM: return (int)State(x)->strength;
        case OBLOC: return (int)loc(x);
        }
        return -1;
    }
    if ((n & IWORD) == IWORD) {
        /* Replace with no. of a users word */
        switch (n & (-1 - IWORD)) {
        case IVERB: return iverb;
        case IADJ1: return iadj1;
        case INOUN1: return inoun1;
        case IPREP: return iprep;
        case IADJ2: return iadj2;
        case INOUN2: return inoun2;
        }
        return -1;
    }
    if ((n & MEPRM) == MEPRM) {
        /* Replace with details of self */
        switch (n & (-1 - MEPRM)) {
        case LOCATE: return -1; /* Not implemented */
        case SELF: return (int)Af;
        case HERE: return (int)me2->room;
        case RANK: return (int)me->rank;
        case FRIEND: return (int)me2->helping;
        case HELPER: return (int)me2->helped;
        case ENEMY: return (int)me2->fighting;
        case WEAPON: return (int)me2->wield;
        case SCORE: return (int)me->score;
        case SCTG: return (int)me2->sctg;
        case STR: return (int)me2->strength;
        case LASTROOM: return (int)lroom;
        case LASTDIR: return (int)ldir;
        case LASTVB: return (int)lverb;
        }
        return -1;
    }
    return (int)n;
}

int
actptr(unsigned long n)
{
    if (n == -1 || n == -2)
        return (int)&"\0";
    if ((n & IWORD) == IWORD) {
        /* Replace with no. of a users word */
        switch (n & (-1 - IWORD)) {
        case INOUN2: return (int)&"@n2\n";
        default: return (int)&"@n1\n";
        }
    }
    return (int)umsgp + *(umsgip + n);
}

void
deduct(int plyr, int howmuch)
{
    if (howmuch < 0)
        return;
    if (plyr == Af) {
        int amount = me->score * howmuch / 100;
        asub(amount, STSCORE, Af);
    } else
        sendex(plyr, ADEDUCT, plyr, howmuch, 0, 0); /* Tell them to clear up! */
}

// Original AMUL entry point
int
amul_main(int argc, char *argv[])
{
    int i;

    lverb = -1;
    iverb = -1;
    ip = 1;
    needcr = false;
    addcr = false;
    MyFlag = am_USER;

    sprintf(vername, "AMUL v%d.%d (%s)", VERSION, REVISION, DATE);

    if (argc > 1 && argv[1][0] != '-') {
        printf("\n\x07!! Invalid argument, %s!\n", argv[1]);
        quit();
    }
    if (argc > 1) {
        switch (toupper(*(argv[1] + 1))) {
        case 3: /* Daemon processor */
            MyFlag = am_DAEM;
            iosup = LOGFILE;
            break;
        case 4:
            MyFlag = am_MOBS;
            iosup = LOGFILE;
            break;
        case 'C': /* Custom screen */
            switchC(argc, *(argv[1] + 2));
            break;
        case 'S': /* Serial Device */
            switchS((argc > 2) ? argv[2] : NULL, (argc > 3) ? argv[3] : NULL,
                    (argc > 4) ? argv[4] : "0", (argc > 5) ? argv[5] : "Y");
            break;
        case 'N': /* NTSC Screen */
            switchC(argc, 'N');
            break;
        default
            : /* None specified */
        {
            txs("\nInvalid parameter '%s'!\n", argv[1]);
            quit();
        }
        }
    } else
        switchC(argc, 0);
    if ((ob = (char *)AllocateMem(5000)) == NULL)
        memfail("IO buffers");
    if ((ow = (char *)AllocateMem(3000)) == NULL)
        memfail("IO buffers");
    if ((input = (char *)AllocateMem(400)) == NULL)
        memfail("IO buffers");
    if ((port = FindPort(mannam)) == NULL) {
        tx("Manager not running!\n");
        quit();
    }
    if ((reply = CreatePort(0L, 0L)) == NULL)
        memfail("user port");
    if ((repbk = CreatePort(0L, 0L)) == NULL)
        memfail("comms reply");
    if ((amanrep = CreatePort(0L, 0L)) == NULL)
        memfail("aman port");
    if ((amul = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    if ((amanp = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    Am.mn_Length = (UWORD)sizeof(*amul);
    Am.mn_Node.ln_Type = NT_MESSAGE;
    Am.mn_ReplyPort = amanrep;
    switch (MyFlag) /* What type of line? */
    {
    case am_DAEM:
        Af = MAXU;
        break;
    case am_MOBS:
        Af = MAXU + 1;
        break;
    }
    *amanp = *amul;
    link = 1;
    SendIt(MCNCT, -10, NULL); /* Go for a connection! */
    linestat = (struct LS *)Ad;
    me2 = linestat + Af;
    me2->IOlock = Af;
    ip = 0;
    usr = (struct _PLAYER *)Ap;
    me = usr + Af;
    me2->rep = reply;
    if (Ad == -'R') {
        tx("\n...Reset In Progress...\n");
        Delay(50);
        quit();
    }
    reset(); /* Read in data files */
    if (Af < 0) {
        sys(NOSLOTS);
        pressret();
        quit();
    }
    if (iosup == CUSSCREEN) {
        sprintf(wtil, "%s   Line: %2d  ", vername, Af);
        strcat(wtil, "Logging in!");
        SetWindowTitles(wG, wtil, wtil);
    }
    me2->unum = Af;
    me2->sup = iosup;
    me2->buf = ob;
    *ob = 0;
    me2->IOlock = -1;
    *ob = 0;
    SendIt(MFREE, NULL, NULL);
    iocheck();

    /* Special processors go HERE: */

    if (Af >= MAXU)
        Special_Proc();

    /* Clear room flags, and send scenario */
    rset = (1 << Af);
    rclr = -1 - rset;
    for (i = 0; i < rooms; i++)
        *(rctab + i) = (*(rctab + i) & rclr);

    do /* Print the title */
    {
        i = fread(block, 1, 900, ifp);
        block[i] = 0;
        tx(block);
    } while (i == 900);
    fclose(ifp);
    ifp = NULL;

    getid(); /*  GET USERS INFO */

    if (iosup == CUSSCREEN) {
        sprintf(wtil, "%s   Line: %2d  ", vername, Af);
        strcat(wtil, "Player: ");
        strcat(wtil, me->name);
        SetWindowTitles(wG, wtil, wtil);
    }

    last_him = last_her = it = -1;

    do {
        died = 0;
        actor = -1;
        fol = 0;
        needcr = false;
        addcr = false;
        if (last_him != -1 && (linestat + last_him)->state != PLAYING)
            last_him = -1;
        if (last_her != -1 && (linestat + last_her)->state != PLAYING)
            last_her = -1;
        iocheck();
        tx(g_ranks[me->rank].prompt);
        needcr = true;
        block[0] = 0;
        Inp(input, 390);
        if (exeunt != 0)
            break;
        if (stricmp(input, "help") == NULL) {
            sys(HELP);
            continue;
        }
        if (input[0] == '/') {
            internal(input + 1);
            continue;
        }
        if (stricmp(input, "***debug") == NULL) {
            debug = !debug;
            continue;
        }
        if (input[0] == 0)
            continue;
    gloop:
        failed = false;
        forced = 0;
        died = 0;
        exeunt = 0;
        if (grab() == -1)
            break;
        iocheck();
        if (forced != 0 && died == 0 && exeunt == 0)
            goto gloop;
    } while (exeunt == 0 && died == 0);

quitgame: /* Quite the game, tidily. */
    if (died == 0)
        action(acp(EXITED), AGLOBAL);
    else
        action(acp(HEDIED), AGLOBAL);
    forced = 0;
    exeunt = 0;
    died = 0;
    if (me->plays == 1)
        sys(BYEPASSWD);
    if (dmove[0] != 0)
        dropall(isroom(dmove));
    else
        dropall(me2->room);
    LoseFollower(); /* Lose who ever is following us. */
    if (iosup == CUSSCREEN)
        pressret();
    quit();
}

