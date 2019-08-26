#include <random>

#include "frame/daemons.c"
#include "frame/filebits.c"
#include "frame/parser.c"

int
owner(int obj)
{
    int r = -5 - *(obtab + obj)->rmlist;
    if (r >= MAXU || r < 0)
        return -1;
    return r;
}

void
show_rank(int p, int rankn, int sex)
{
    str[0] = 0;
    make_rank(p, rankn, sex);
    tx(str);
}

void
make_rank(int p, int rankn, int sex)
{
    strcat(str, " the ");
    p += 5;
    if (*(linestat + p)->pre != 0) {
        strcat(str, (linestat + p)->pre);
        strcat(str, " ");
    }
    strcat(str, (sex == 0) ? g_ranks[rankn].male : g_ranks[rankn].female);
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
gotin(int obj, int st)
{
    for (int i = 0; i < nouns; i++) {
        if (stricmp((obtab + i)->id, (obtab + obj)->id) == NULL &&
            	((obtab + i)->state == st || st == -1) && owner(i) == Af)
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
        txs("I can't see the %s!\n", (obtab + obj)->id);
        donet = ml + 1;
        return -1;
    }
    inc = 0;
    return 0;
}

// Check the player can 'get' the object
int
acheckget(int obj)
{
    if (carrying(obj) != -1) {
        tx("You've already got it!\n");
        donet = ml + 1;
        return -1;
    }
    if (achecknear(obj) == -1)
        return -1;
    else
        inc = 1;
    objtab = obtab + obj;
    if ((objtab->flags & OF_SCENERY) || (STATE->flags & SF_ALIVE) || objtab->nrooms != 1) {
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
    if (STATE->weight > rank.maxweight) {
        tx("You aren't strong enough to lift that!\n");
        donet = ml + 1;
        return -1;
    }
    if (me2->numobj + 1 > rank.numobj) {
        tx("You can't carry any more! You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    if (STATE->weight + me2->weight > rank.maxweight) {
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
        list_what(rm, 0);
        return;
    }
    // Can I see in here?
    if (!lit(me2->room)) {
        sys(TOODARK);
        *(rctab + rm) = *(rctab + rm) & rclr;
    } else {
        desc_here(f);
        list_what(rm, 0);
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
list_what(int r, int i)
{
    if (!lit(me2->room))
        return sys(TOOMAKE);
    if (me2->flags & PFBLIND)
        sys(YOURBLIND);
    if (((rmtab + r)->flags & HIDEWY) && i != 0 && me->rank != ranks - 1) {
        sys(NOWTSPECIAL); /* Wizards can see in hideaways! */
    }
    int f = -1;
    for (int o = 0; o < nouns; o++) /* All objects */
    {
        /* Only let the right people see the object */
        if (!canSeeObject(o, Af))
            continue;
        if (((rmtab + r)->flags & HIDEWY) && (i == 0 || (i == 1 && me->rank != ranks - 1)) &&
            !((obtab + o)->flags & OF_SCENERY))
            continue;
        if (!lit(me2->room) !((obtab + o)->flags & OF_SMELL))
            continue;
        obj = *(obtab + o);
        for (int or = 0; or < obj.nrooms; or ++) {
            if (*(obj.rmlist + or) == r && State(o)->descrip >= 0) {
                if (isOINVIS(o))
                    ans("3m");
                f++;
                descobj(o);
                if (isOINVIS(o))
                    ans("0;37m");
            }
        }
    }
    if (f == -1 && i == 1)
        sys(NOWTSPECIAL);
}

void
descobj(int Ob)
{
    obj.states = (obtab + Ob)->states + (long)(obtab + Ob)->state;
    if (obj.states->descrip == 0)
        return;
    sprintf(str, desctab + obj.states->descrip, adtab + ((obtab + Ob)->adj * (IDL + 1)));
    if (((obtab + Ob)->flags & OF_SHOWFIRE) && (obj.states->flags & SF_LIT)) {
        if (*(str + strlen(str) - 1) == '\n' || *(str + strlen(str) - 1) == '{') {
            *(str + strlen(str) - 1) = 0;
        }
        if ((obtab + Ob)->adj != -1)
            sprintf(spc, " The %s %s is on fire.\n", (adtab + ((obtab + Ob)->adj * (IDL + 1))),
                    (obtab + Ob)->id);
        else
            sprintf(spc, " The %s is on fire.\n", (obtab + Ob)->id);
        strcat(str, spc);
    }
    if ((obtab + Ob)->contains <= 0) {
        tx(str);
        return;
    }
    if (*(str + strlen(str) - 1) == '\n' || *(str + strlen(str) - 1) == '{') {
        *(str + strlen(str) - 1) = 0;
    }
    strcat(str, " ");
    showin(Ob, false);
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
        me->sex = 1 - me->sex;
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

int
isaroom(char *s)
{
    roomtab = rmtab;
    for (int r = 0; r < rooms; r++, roomtab++)
        if (stricmp(roomtab->id, s) == 0)
            return r;
    return -1;
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
PutRankInto(char *s)
{
	PutARankInto(s, Af);
}

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
    char *p = (you->sex == 0) ? g_ranks[you->rank].male : g_ranks[you->rank].female;
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
    for (int i = 0; i < nouns && me2->numobj > 0; i++)
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
    for (int i = 0; i < nouns; i++, objtab++)
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
