/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL2.C......Adventure System      ****
              ****            The Saga Continues!           ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.


             Secondary/low-level functions, macros and routines
*/

#include "amulinc.h"
#include "h/amul.vars.h"

#define dtx(x)      \
    if (debug != 0) \
    tx(x)
short int debug;  // Is debug mode on?
char      llen;

struct Screen *  sC;
struct Window *  wG;
struct IOExtSer *serio, *wserio;
Amiga::Task *    MyTask;
struct ViewPort  vP;
struct RastPort *rpG;
Amiga::MsgPort * ReadRep, *WriteRep, *repbk;
struct IOStdReq  ReadIo, WriteIo;

// Frame specific variables
char    serop, MyFlag;                          // SerPort open? What am I?
char *  input;                                  // 400 bytes, 5 lines
char    str[800], spc[200], mxx[40], mxy[60];   // Output string
char    wtil[80];                               // Window title
char    inc, forced, failed, died, addcr, fol;  // For parsers use
char    actor, last_him, last_her;              // People we talked about
char    autoexits, needcr;                      // General flags
int32_t iverb, overb, iadj1, inoun1, iprep, iadj2, inoun2, lverb, ldir, lroom;
int32_t wtype[6], word, mins;                // Type of word...
unsigned short int *rescnt;                  // Reset counter from AMan
short int           donev, skip;             // No. of vb's/TT's done
char                exeunt, more, link;      // If we've linked yet
int32_t             ml, donet, it;           // Maximum lines
Aport *             ap, *amanp, *intam;      // The message pointers
Amiga::MsgPort *    amanrep;                 // AMAN reply port
char *    ob, *gp, *ow, *lastres, *lastcrt;  // Pointer to output buffers etc
short int rset, rclr, ip, csyn;              // Masks for Room Counter

// Various low-level macros/functions for AMUL...

void
reset()
{
    SendIt(MSG_DATA_REQUEST, 0, dir);  // Get basic data

    fopenr(Resources::Compiled::gameProfile());
    fgets(adname, sizeof(adname), ifp);
    nulTerminate(adname);
    fscanf(ifp,
           "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld "
           "%ld",
           &rooms, &ranks, &verbs, &syns, &nouns, &adjs, &ttents, &umsgs, &word,
           &mins, &invis, &invis2, &minsgo, &mobs, &rscale, &tscale, &mobchars);

    SendIt(MSG_DATA_REQUEST, 1, NULL);  // Get rooms data
    rooms = Ad;
    rmtab = (struct room *)Ap;
    rescnt = (short *)Ap1;

    SendIt(MSG_DATA_REQUEST, 2, NULL);  // Get ranks data
    ranks = Ad;
    rktab = (struct rank *)Ap;

    SendIt(MSG_DATA_REQUEST, 3, NULL);  // Get object headers
    nouns = Ad;
    obtab = (struct obj *)Ap;

    SendIt(MSG_DATA_REQUEST, 4, NULL);  // Get verbs
    verbs = Ad;
    vbtab = (struct verb *)Ap;

    SendIt(MSG_DATA_REQUEST, 5, NULL);  // Get descriptions
    desctab = Ap;

    SendIt(MSG_DATA_REQUEST, 6, NULL);  // Get room table data
    ormtab = (int32_t)Ap;

    SendIt(MSG_DATA_REQUEST, 7, NULL);  // Get states!
    statab = (_OBJ_STATE *)Ap;

    SendIt(MSG_DATA_REQUEST, 8, NULL);  // Get adjectives
    adtab = Ap;

    SendIt(MSG_DATA_REQUEST, 9, NULL);  // Get travel table
    ttp = (_TT_ENT *)Ap;

    SendIt(MSG_DATA_REQUEST, 10, NULL);  // Get UMsg Indexes
    umsgip = (int32_t *)Ap;

    SendIt(MSG_DATA_REQUEST, 11, NULL);  // Get UMsg Text
    umsgp = Ap;

    SendIt(MSG_DATA_REQUEST, 12, NULL);  // Get TT Params
    ttpp = (int32_t *)Ap;

    SendIt(MSG_DATA_REQUEST, 13, NULL);  // Get roomcount table
    rctab = (short *)Ap;

    SendIt(MSG_DATA_REQUEST, 14, NULL);  // Get slot table
    slottab = (struct vbslot *)Ap;

    SendIt(MSG_DATA_REQUEST, 15, NULL);  // Get vt table
    vtp = (struct vt *)Ap;

    SendIt(MSG_DATA_REQUEST, 16, NULL);  // Get vtp table
    vtpp = (int32_t *)Ap;

    SendIt(MSG_DATA_REQUEST, 17, NULL);  // Get syn data
    synp = Ap;
    synip = (short int *)Ad;

    SendIt(MSG_DATA_REQUEST, 18, NULL);  // Get last reset & create times
    lastres = Ap;
    lastcrt = (char *)Ad;
}

void
look(const char *roomName, int visitFlag)
{
    int roomno, mod;

    /* Some complex stuff here!
      if f==0 (rdmode=RoomCount) and we have been here before,
        look(brief)
      if f==0 (rdmode=RoomCount) and this is our first visit,
        look(verbose)
      if f==0 visit the room
                                   */

    if ((roomno = isroom(roomName)) == -1)
        return;
    roomtab = rmtab + roomno;
    if (visitFlag == RD_VERBOSE_ONCE && ((*(rctab + roomno) & rset) != rset))
        mod = RD_VERBOSE;
    else
        mod = visitFlag;
    if (visitFlag != 2)
        *(rctab + roomno) = *(rctab + roomno) | rset;

    look_here(mod, roomno);
}

// Add object to players inventory
void
agive(int obj, int to)
{
    int own, orm;

    objtab = obtab + obj;

    if ((objtab->flags & OF_SCENERY) || (STATE->flags & SF_ALIVE) ||
        objtab->nrooms != 1)
        return;
    if ((own = owner(obj)) != -1)
        rem_obj(own, obj);
    orm = *objtab->rmlist;
    add_obj(to);

    /*== The lighting conditions for transfering an object between a
         variable source and destination are complex! See below!	*/
    if (STATE->flags & SF_LIT) {
        if (own == -1)  //== Did I just pick and was it from here?
        {
            if (orm == (lstat + own)->room)
                return;
        } else {  //== If I got it from someone, and he is near me...
            if ((lstat + own)->room == (lstat + to)->room)
                return;
            lighting(own, AHERE);  //== Else check his lights!
        }
        lighting(to, AHERE);
    }
}

// Drop the object (to a room)
void
adrop(int ob, int r, int f)
{
    objtab = obtab + ob;
    *objtab->rmlist = r;
    rem_obj(Af, ob);
    lighting(Af, AHERE);

    // If the room IS a 'swamp', give em points
    if ((rmtab + me2->room)->flags & RF_SINKHOLE) {
        //== Only give points if player hasn't quit.
        if (exeunt == 0)
            aadd(scaled(STATE->value, STATE->flags), STSCORE, Af);
        *objtab->rmlist = -1;
    }
}

int
owner(int objectNo)
{
    int r = -5 - *(obtab + objectNo)->rmlist;
    if (r >= MAXU || r < 0)
        return -1;
    return r;
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
    if (*(lstat + p)->pre != 0) {
        strcat(str, (lstat + p)->pre);
        strcat(str, " ");
    }
    strcat(str,
           (gender == 0) ? (rktab + rankn)->male : (rktab + rankn)->female);
    if (*(lstat + p)->post != 0) {
        strcat(str, " ");
        strcat(str, (lstat + p)->post);
    }
}

// Move player to room, testing ligthing!
void
moveto(int r)
{
    int i;

    /*
      Set the players current lighting to NONE and then test lighting for
      the room. Then move the player and restore his lighting. Now test
      again!
                                                                            */
    StopFollow();
    me2->flags = me2->flags | PFMOVING;
    lroom = me2->room;
    i = me2->light;
    me2->light = 0;
    lighting(Af, AOTHERS);
    me2->room = r;
    me2->light = i;
    me2->hadlight = 0;
    lighting(Af, AOTHERS);
    look((rmtab + me2->room)->id, me->rdmode);
    me2->flags = me2->flags & -(1 + PFMOVING);
}

// n MODulo X
int32_t
mod(int32_t n, int32_t x)
{
    if (n < 0) {
        return -(-n % x);
    } else {
        return n % x;
    }
}

// Return pseudo-random number
int32_t
rnd()
{
    int32_t x;

    time(&x);
    srand(x);
    return rand();
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

// The next two commands are ACTIONS but work like conditions
// Check player is near object, else msg + endparse
int
achecknear(int obj)
{
    if (nearto(obj) == NO) {
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
    if ((objtab->flags & OF_SCENERY) || (STATE->flags & SF_ALIVE) ||
        objtab->nrooms != 1) {
        tx("You can't move that!\n");
        donet = ml + 1;
        return -1;
    }
    if ((rktab + me->rank)->numobj <= 0) {
        tx("You can't manage it!\n");
        donet = ml + 1;
        return -1;
    }
    if (STATE->weight > (rktab + me->rank)->maxweight) {
        tx("You aren't strong enough to lift that!\n");
        donet = ml + 1;
        return -1;
    }
    if (me2->numobj + 1 > (rktab + me->rank)->numobj) {
        tx("You can't carry any more!");
        tx(" You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    if (STATE->weight + me2->weight > (rktab + me->rank)->maxweight) {
        tx("It's too heavy.");
        tx(" You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    inc = 0;
    return 0;
}

void
look_here(int f, int rm)
{
    char il;

    // Can I see?
    if (me2->flags & PFBLIND) {
        list_what(rm, 0);
        return;
    }
    // Can we see in here?
    if ((il = lit(me2->room)) == NO) {
        sys(TOODARK);
        *(rctab + rm) = *(rctab + rm) & rclr;
        goto die;
    }
    desc_here(f);
    list_what(rm, 0);
die:
    if ((roomtab->flags & RF_LETHAL) && me->rank != ranks - 1) {
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
    char *p, c;
    fseek(ifp, roomtab->desptr, 0L);
    if (roomtab->flags & RF_CEMETERY)  // A dmove room?
        fgets(dmove, IDL, ifp);
    else
        dmove[0] = 0;

    // Print short description
    p = block;
    if (!(roomtab->flags & RF_LETHAL))
        ans("1m");
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
        tx(block);  // Make sure we dump it!
    // If I am the toprank show me the room id too!
    ans("0;37m");
    if (me->rank == ranks - 1) {
        sprintf(block, "   (%s)", roomtab->id);
        tx(block);
        block[0] = 0;
    }
    txc('\n');
    if (c == '\n' && f == RD_VERBOSE) {
        // Display LONG description!
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
    int o, or, f;

    f = -1;
    if (lit(me2->room) == NO) {
        sys(TOOMAKE);
        return;
    }

    if (me2->flags & PFBLIND) {
        sys(YOURBLIND);
    }

    if (((rmtab + r)->flags & RF_HIDE_OBJECTS) && i != 0 &&
        me->rank != ranks - 1) {
        sys(NOWTSPECIAL);  // Wizards can see in hideaways!
    }
    for (o = 0; o < nouns; o++)  // All objects
    {
        // Only let the right people see the object
        if (canseeobj(o, Af) == NO)
            continue;
        if (((rmtab + r)->flags & RF_HIDE_OBJECTS) &&
            (i == 0 || (i == 1 && me->rank != ranks - 1)) &&
            !((obtab + o)->flags & OF_SCENERY))
            continue;
        if (lit(me2->room) == NO && !((obtab + o)->flags & OF_SMELL))
            continue;
        obj = *(obtab + o);
        for (or = 0; or < obj.nrooms; or ++) {
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
    obj.states = (obtab + Ob)->states + (int32_t)(obtab + Ob)->state;
    if (obj.states->descrip == 0)
        return;
    sprintf(str, desctab + obj.states->descrip,
            adtab + ((obtab + Ob)->adj * (IDL + 1)));
    if (((obtab + Ob)->flags & OF_SHOWFIRE) && (obj.states->flags & SF_LIT)) {
        if (*(str + strlen(str) - 1) == '\n' ||
            *(str + strlen(str) - 1) == '{') {
            *(str + strlen(str) - 1) = 0;
        }
        if ((obtab + Ob)->adj != -1)
            sprintf(spc, " The %s %s is on fire.\n",
                    (adtab + ((obtab + Ob)->adj * (IDL + 1))),
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
    showin(Ob, NO);
}

void
inflict(int x, int s)
{
    you2 = lstat + x;
    if (you2->state != US_CONNECTED)
        return;
    switch (s) {
    case SPELL_GLOW:
        if (!(you2->flags & PFGLOW)) {
            you2->flags = (you2->flags | PFGLOW);
            you2->light++;
        }
        break;
    case SPELL_INVISIBLE: you2->flags = you2->flags | PFINVIS; break;
    case SPELL_DEAFEN: you2->flags = you2->flags | PFDEAF; break;
    case SPELL_BLIND: you2->flags = you2->flags | PFBLIND; break;
    case SPELL_CRIPPLE: you2->flags = you2->flags | PFCRIP; break;
    case SPELL_MUTE: you2->flags = you2->flags | PFDUMB; break;
    case SPELL_SLEEP: you2->flags = you2->flags | PFASLEEP; break;
    case SPELL_SUPER_INVIS:
        you2->flags = you2->flags | PFSPELL_INVISIBLE;
        break;
    }
    calcdext();
    lighting(x, AHERE);
}

void
cure(int x, int s)
{
    you2 = lstat + x;
    if (you2->state != US_CONNECTED)
        return;
    switch (s) {
    case SPELL_GLOW:
        if (you2->flags & PFGLOW) {
            you2->flags = (you2->flags & (-1 - PFGLOW));
            you2->light--;
        }
        break;
    case SPELL_INVISIBLE: you2->flags = you2->flags & -(1 + PFINVIS); break;
    case SPELL_DEAFEN: you2->flags = you2->flags & -(1 + PFDEAF); break;
    case SPELL_BLIND: you2->flags = you2->flags & -(1 + PFBLIND); break;
    case SPELL_CRIPPLE: you2->flags = you2->flags & -(1 + PFCRIP); break;
    case SPELL_MUTE: you2->flags = you2->flags & -(1 + PFDUMB); break;
    case SPELL_SLEEP: you2->flags = you2->flags & -(1 + PFASLEEP); break;
    case SPELL_SUPER_INVIS:
        you2->flags = you2->flags & -(1 + PFSPELL_INVISIBLE);
        break;
    }
    calcdext();
    lighting(x, AHERE);
}

void
summon(int plyr)
{
    if ((lstat + plyr)->room == me2->room) {
        txs(acp(CANTSUMN), (usr + plyr)->name);
        return;
    }
    interact(MSG_SUMMONED, plyr, me2->room);
}

void
adestroy(int obj)
{
    Amiga::ScheduleGuard guard;
    loseobj(obj);
    for (int i = 0; i < (obtab + obj)->nrooms; i++) {
        *((obtab + obj)->rmlist + i) = -1;
    }
    (obtab + obj)->flags = (obtab + obj)->flags | OF_ZONKED;
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

// Restore players details
void
refresh()
{
    if (me->strength <= 0)
        me->strength = (rktab + me->rank)->strength;
    me2->strength = me->strength;
    if (me->stamina <= 0)
        me->stamina = (rktab + me->rank)->stamina;
    me2->stamina = me->stamina;
    if (me->dext <= 0)
        me->dext = (rktab + me->rank)->dext;
    me2->dext = me->dext;
    me2->dextadj = 0;
    if (me->wisdom <= 0)
        me->wisdom = (rktab + me->rank)->wisdom;
    me2->wisdom = me->wisdom;
    if (me->experience <= 0)
        me->experience = (rktab + me->rank)->experience;
    if (me->magicpts <= 0)
        me->magicpts = (rktab + me->rank)->magicpts;
    me2->magicpts = me->magicpts;
    calcdext();
}

void
zapme()
{
    char *p;
    int   i;

    {
        Amiga::ScheduleGuard guard;
        p = (char *)me->name;
        exeunt = 1;
        for (i = 0; i < sizeof(usr); i++)
            *(p++) = 0;
    }

    save_me();
    nohelp();
}

void
send(int o, int to)
{
    short int x, i;

    x = lit(to);
    loseobj(o);
    for (i = 0; i < objtab->nrooms; i++)
        *(objtab->rmlist + i) = to;
    if (lit(to) != x)
        actionin(to, acp(NOWLIGHT));
}

void
achange(int u)
{
    if (u == Af) {
        me->sex = 1 - me->sex;
        sys(CHANGESEX);
    } else {
        sendex(u, ACHANGESEX, u, 0, 0, 0);  // Tell them to clear up
    }
}

//== Fixed to allow increase/decrease
void
newrank(int plyr, int r)
{
    int or = me->rank;

    if ((rktab + r)->tasks != 0) {
        if ((me->tasks & (1 << ((rktab + r)->tasks) - 1)) == NULL) {
            sys(NOTASK);
            return;
        }
    }

    me->rank = r;
    sys(MADERANK);

    // Update Current Line Stats
    me2->strength += (rktab + r)->strength - (rktab + or)->strength;
    if (me2->strength > (rktab + r)->strength)
        me2->strength = (rktab + r)->strength;
    me2->stamina += (rktab + r)->stamina - (rktab + or)->stamina;
    if (me2->stamina > (rktab + r)->stamina)
        me2->stamina = (rktab + r)->stamina;
    me2->wisdom += (rktab + r)->wisdom - (rktab + or)->wisdom;
    if (me2->wisdom > (rktab + r)->wisdom)
        me2->wisdom = (rktab + r)->wisdom;
    me->experience += (rktab + r)->experience - (rktab + or)->experience;
    if (me->experience > (rktab + r)->experience)
        me->experience = (rktab + r)->experience;
    me2->magicpts += (rktab + r)->magicpts - (rktab + or)->magicpts;
    if (me2->magicpts > (rktab + r)->magicpts)
        me2->magicpts = (rktab + r)->magicpts;
    calcdext();

    // Update File Stats
    me->strength = (rktab + r)->strength;
    me->stamina = (rktab + r)->stamina;
    me->wisdom = (rktab + r)->wisdom;
    me->dext = (rktab + r)->dext;
    me->experience += (rktab + r)->experience - (rktab + or)->experience;
    me->magicpts = (rktab + r)->magicpts;

    if (r == ranks - 1) {
        sys(TOPRANK);
        SendIt(MSG_MADE_ADMIN, 0, me->name);
    }
}

void
aadd(int howmuch, int stat, int plyr)
{
    int r;
    if (howmuch < 0) {
        asub(-howmuch, stat, plyr);
        return;
    }
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
            for (r = ranks - 1; r >= 0; r--) {
                if (me->score >= (rktab + r)->score) {
                    if (me->rank == r)
                        break;
                    newrank(plyr, r);
                    break;
                }
            }
            break;
        case STSTR: me2->strength += howmuch; break;
        case STSTAM:
            me2->stamina += howmuch;
            sprintf(block, "<STAM: %ld/%ld>\n", me2->stamina, me->stamina);
            ans("1m");
            tx(block);
            ans("0;37m");
            break;
        case STDEX: me2->dextadj += howmuch; break;
        case STEXP: me->experience += howmuch; break;
        case STWIS: me2->wisdom += howmuch; break;
        case STMAGIC: me2->magicpts += howmuch; break;
        }
    } else
        sendex(plyr, AADD, howmuch, stat, plyr);  // Tell them to clear up!
}

void
asub(int howmuch, int stat, int plyr)
{
    int r;
    if (howmuch < 0) {
        aadd(-howmuch, stat, plyr);
        return;
    }
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
            for (r = 0; r < ranks - 1; r++) {
                if (me->score < (rktab + (r + 1))->score && me->rank == r)
                    break;
                if (me->score < (rktab + (r + 1))->score) {
                    newrank(plyr, r);
                    break;
                }
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
        case STDEX: me2->dextadj -= howmuch; break;
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
        sendex(plyr, ASUB, howmuch, stat, plyr);  // Tell them to clear up!
}

void
afix(int stat, int plyr)
{
    if (plyr == Af) {
        switch (stat) {
        case STSTR:
            me2->strength = ((rktab + me->rank)->strength *
                                     (rktab + me->rank)->maxweight -
                             me2->weight) /
                            (rktab + me->rank)->maxweight;
            break;
        case STSTAM: me2->stamina = (rktab + me->rank)->stamina; break;
        case STDEX:
            me2->dextadj = 0;
            calcdext();
            break;
        case STWIS: me2->wisdom = (rktab + me->rank)->wisdom; break;
        case STEXP: me->experience = (rktab + me->rank)->experience; break;
        case STMAGIC: me2->magicpts = (rktab + me->rank)->magicpts; break;
        }
    } else
        sendex(plyr, AFIX, stat, plyr, 0);  // Tell them to clear up!
}

// Loud noises/events
void
announce(const char *s, int towho)
{
    int i, x;

    for (i = 0; i < MAXU; i++) {
        // If the player is deaf, ignore him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & PFDEAF))
            continue;
        /*
           The next line says:
            if this is another player, and he's in another room,
            and the room is a silent room, ignore him.
        */
        if (i != Af && (lstat + i)->room != me2->room &&  // --v
            ((rmtab + (lstat + i)->room)->flags & RF_SILENT))
            continue;
        x = 0;
        switch (towho) {
        case AALL:
        case AEVERY1: x = 1; break;
        case AGLOBAL:
            if (i != Af)
                x = 1;
            break;
        case AOTHERS:
            if (i == Af)
                break;
        case AHERE:
            if ((lstat + i)->room == me2->room)
                x = 1;
            break;
        case AOUTSIDE:
            if ((lstat + i)->room != me2->room)
                x = 1;
            break;
        }
        if (x == 1) {
            setmxy(PC_NOISE, i);
            utx(i, s);
        }
    }
}

// Loud noises/events
void
announcein(int toroom, const char *s)
{
    int i;
    for (i = 0; i < MAXU; i++) {
        // If the player is deaf, ignore him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & PFDEAF) || (lstat + i)->room != toroom)
            continue;
        setmxy(PC_NOISE, i);
        utx(i, s);
    }
}

// Loud noises/events
void
announcefrom(int obj, const char *s)
{
    int i, o;
    for (i = 0; i < MAXU; i++) {
        // If the player is deaf or can see me, ignore him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & PFDEAF) || (lstat + i)->room == me2->room)
            continue;
        // Check if the player is NEAR to someone carrying the object
        if ((o = owner(obj)) != -1 && (lstat + o)->room != (lstat + i)->room)
            continue;
        if (o == -1 && isin(obj, (lstat + o)->room) == NO)
            continue;
        setmxy(PC_NOISE, i);
        utx(i, s);
    }
}

void objannounce(int obj, char *s)  // Loud noises/events
{
    int i, o;
    for (i = 0; i < MAXU; i++) {
        // If the player is deaf or can see me, ignore him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & PFDEAF))
            continue;
        // Check if the player is NEAR to someone carrying the object
        if ((o = owner(obj)) != -1 && (lstat + o)->room != (lstat + i)->room)
            continue;
        if (o == -1 && isin(obj, (lstat + o)->room) == NO)
            continue;
        setmxy(PC_NOISE, i);
        utx(i, s);
    }
}

// Quiet actions/notices
void
action(const char *s, int towho)
{
    int i, x;
    for (i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & (PFBLIND + PFASLEEP)) != 0)
            continue;
        x = 0;
        switch (towho) {
        case AALL:
        case AEVERY1: x = 1; break;
        case AGLOBAL:
            if (i != Af)
                x = 1;
            break;
        case AOTHERS:
            if (i == Af)
                break;
        case AHERE:
            if ((lstat + i)->room == me2->room && cansee(i, Af) == YES)
                x = 1;
            break;
        case AOUTSIDE:
            if ((lstat + i)->room != me2->room)
                x = 1;
            break;
        }
        if (x == 1) {
            setmxy(PC_ACTION, i);
            utx(i, s);
        }
    }
}

// Quiet actions/notices
void
actionin(int toroom, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || ((lstat + i)->state < US_CONNECTED) ||
            ((lstat + i)->flags & (PFBLIND + PFASLEEP)) ||
            (lstat + i)->room != toroom)
            continue;
        setmxy(PC_ACTION, i);
        utx(i, s);
    }
}

// Quiet actions/notices
void
actionfrom(int obj, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & (PFBLIND + PFASLEEP)) ||
            (lstat + i)->room == me2->room)
            continue;
        // Check if the player is NEAR to someone carrying the object
        int o = owner(obj);
        if (o != -1)
            if ((lstat + o)->room != (lstat + i)->room)
                continue;
        if (o == -1 && isin(obj, (lstat + i)->room) == NO)
            continue;
        setmxy(PC_ACTION, i);
        utx(i, s);
    }
}

// Quiet actions/notices
void
objaction(int obj, const char *s)
{
    int i, o;
    for (i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || ((lstat + i)->state < 2) ||
            ((lstat + i)->flags & (PFBLIND + PFASLEEP)))
            continue;
        // Check if the player is NEAR to someone carrying the object
        if ((o = owner(obj)) != -1)
            if ((lstat + o)->room != (lstat + i)->room)
                continue;
        if (o == -1 && isin(obj, (lstat + i)->room) == NO)
            continue;
        setmxy(PC_ACTION, i);
        utx(i, s);
    }
}

void
fwait(int32_t n)
{
    int i;

    if (n < 1)
        n = 1;
    for (i = 0; i < 7; i++) {
        Amiga::Delay(n * 7);
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

//== twho - tell who
void
lighting(int x, int twho)
{
    if ((lstat + x)->light == (lstat + x)->hadlight ||
        !((rmtab + (lstat + x)->room)->flags & RF_DARK))
        return;
    if ((lstat + x)->light <= 0) {
        if ((lstat + x)->hadlight <= 0)
            return;
        (lstat + x)->hadlight = (lstat + x)->light = 0;
        if (lit((lstat + x)->room) == NO)
            action(acp(NOWTOODARK), twho);
    } else {
        if ((lstat + x)->hadlight != 0)
            return;
        if (lit((lstat + x)->room) == NO)
            action(acp(NOWLIGHT), twho);
        (lstat + x)->hadlight = (lstat + x)->light;
    }
}

// Remove object from its owners inventory
void
loseobj(int obj)
{
    int o, i;

    objtab = obtab + obj;

    if ((o = owner(obj)) != -1) {
        for (i = 0; i < objtab->nrooms; i++)
            *(objtab->rmlist + i) = -1;
        rem_obj(o);
        lighting(o, AHERE);
    }
}

void
nohelp()
{
    int i;

    if (me2->helping != -1)
        utx(me2->helping, "@me is no-longer able to help you...\n");
    (lstat + me2->helping)->helped--;
    me2->helping = -1;
    you2 = lstat;
    for (i = 0; i < MAXU; i++, you2++)
        if (you2->helping == Af) {
            utx(i, "You are no longer able to help @me.\n");
            you2->helping = -1;
        }
    me2->helping = me2->helped = -1;
}

void
aforce(int whom, const char *cmd)
{
    DoThis(whom, cmd, 0);
}

void
afight(int plyr)
{
    if (plyr == Af)
        return;
    if ((rmtab + me2->room)->flags & RF_SANCTUARY) {
        sys(NOFIGHT);
        return;
    }
    if ((lstat + plyr)->fighting == Af) {
        txs("You are already fighting %s!\n", (usr + plyr)->name);
        donet = ml + 1;
        return;
    }
    if ((lstat + plyr)->fighting != -1) {
        txs("%s is already in a fight!\n", (usr + plyr)->name);
        donet = ml + 1;
        return;
    }
    you2 = lstat + plyr;
    you2->flags = you2->flags | PFFIGHT;
    me2->flags = me2->flags | PFFIGHT | PFATTACKER;
    you2->fighting = Af;
    me2->fighting = plyr;
    Permit();
}

void
clearfight()
{
    Amiga::ScheduleGuard guard;
    if (me2->fighting != -1 && me2->fighting != Af) {
        finishfight(me2->fighting);
    }
    finishfight(Af);
}

void
finishfight(int plyr)
{
    you2 = lstat + plyr;
    you2->flags = you2->flags & (-1 - (PFFIGHT | PFATTACKER));
    you2->fighting = -1;
}

void
acombat()
{
    /*
        To hit:  Str=40 Exp=50 Dext=10
        Defence: Dext=70 Exp=20 Str=10
        No hits: Players level different by 2 double attacks etc.
        Damage:  Str / 10 + weapon.		<--- made this random!

    == Should ALSO go on how crippled a player is... A cripple can't
    strike out at another player. Also, blindness should affect your
    attack/defence.
    */

    int aatt, adef, adam, datt, ddef, str;
    int oldstr, minpksl;

    calcdext();

    if (me2->fighting == Af || me2->fighting == -1 ||
        me2->state < US_CONNECTED || me2->stamina <= 0) {
        donet = ml + 1;  // End parse
        finishfight(Af);
        return;  // Macro : Permit(); return
    }

    you = usr + me2->fighting;
    you2 = lstat + me2->fighting;
    minpksl = (rktab + you->rank)->minpksl;

    if (you2->state < US_CONNECTED || you2->room != me2->room ||
        you2->stamina <= 0) {
        donet = ml + 1;
        finishfight(Af);
        return;
    }

    if (me2->wield != -1) {
        objtab = obtab + me2->wield;
        str = (20 * me2->strength) + STATE->damage;
    } else
        str = (20 * me2->strength);

    if (me->dext == 0)
        aatt = 5;
    else
        aatt = (50 * me->experience) / (rktab + ranks - 1)->experience +
               (40 * me2->strength) / (rktab + ranks - 1)->strength +
               (10 * me2->dext) / (rktab + ranks - 1)->dext;

    if (me->dext == 0)
        adef = 5;
    else
        adef = (5 * me->experience) / (rktab + ranks - 1)->experience +
               (15 * me2->strength) / (rktab + ranks - 1)->strength +
               (80 * me2->dext) / (rktab + ranks - 1)->dext;

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
        datt = (50 * you->experience) / (rktab + ranks - 1)->experience +
               (40 * you2->strength) / (rktab + ranks - 1)->strength +
               (10 * you2->dext) / (rktab + ranks - 1)->dext;

    if (you->dext == 0)
        ddef = 5;
    else
        ddef = (5 * you->experience) / (rktab + ranks - 1)->experience +
               (15 * you2->strength) / (rktab + ranks - 1)->strength +
               (80 * you2->dext) / (rktab + ranks - 1)->dext;

    /*	if(you2->flags & PFCRIP)  { datt = datt / 5; ddef = ddef / 10; }
        if(you2->flags & PFBLIND) { datt = datt / 2; ddef = ddef / 4;  } */

    oldstr = you2->stamina;
    adam = -1;
    if (mod(rand(), 100) < aatt || (ddef <= 0 && mod(rand(), 100) < 50)) {
        if (mod(rand(), 100) < ddef) {
            if (you2->wield != -1) {
                sys(WBLOCK);
                utx(me2->fighting, acp(WDEFEND));
            } else {
                sys(BLOCK);
                utx(me2->fighting, acp(DEFEND));
            }
            //			if((i=isverb("\"block"))!=-1) lang_proc(i,0);
        } else {
            if (me2->wield != -1) {
                sys(WATTACK);
                objtab = obtab + me2->wield;
                adam = (me2->strength / 10) + 1 + mod(rand(), STATE->damage);
                utx(me2->fighting, acp(WHIT));
            } else {
                adam = (me2->strength / 10) + 1;
                sys(ATTACK);
                utx(me2->fighting, acp(AMHIT));
            }
            asub(adam, STSTAM, me2->fighting);
            //			if((i=isverb("\"hit"))!=-1) lang_proc(i,0);
        }
    } else {
        sys(MISSED);
        utx(me2->fighting, acp(HEMISSED));
        //		if((i=isverb("\"miss"))!=-1) lang_proc(i,0);
    }
    oldstr -= adam;
    if ((me2->flags & PFATTACKER) && oldstr > 0) {
        //		if(me2->helped != -1 && (lstat+me2->helped)->room==me2->room)
        //Well?

        sendex(me2->fighting, ACOMBAT, -1, 0, 0);  // Tell them to clear up!
    }
    if (oldstr <= 0) {
        donet = ml + 1;  // End parse
        tx("You have defeated @pl!\n");
        aadd(minpksl, STSCORE, Af);
        finishfight(Af);
    }
}

void
exits()
{
    int             v, i, maxl, x, ac, brk, l;
    struct _TT_ENT *tp, *otp;
    int             c, a;
    int32_t *       pptr;

    roomtab = rmtab + me2->room;
    if (roomtab->tabptr == -1) {
        tx("There are no visible exits.\n");
        return;
    }

    vbptr = vbtab;
    x = 0;
    c = tt.condition;
    a = tt.action;
    otp = ttabp;
    pptr = tt.pptr;

    for (v = 0; v < verbs; v++, vbptr++) {
        if (vbptr->flags & VB_TRAVEL)
            continue;  // Not a trav verb

        roomtab = rmtab + me2->room;
        l = -1;
        maxl = roomtab->ttlines;
        tp = ttp + roomtab->tabptr;
        brk = 0;
        for (i = 0; i < maxl && brk == 0; i++) {
            ttabp = tp + i;
            tt.condition = ttabp->condition;
            tt.pptr = ttabp->pptr;
            if (ttabp->verb == v && (l = cond(ttabp->condition, l)) != -1) {
                if (ttabp->action >= 0) {
                    txs("%-10s ", vbptr->id);
                    brk = 1;
                    roomtab = rmtab + (ttabp->action);
                    if (roomtab->flags & RF_LETHAL)
                        sys(CERTDEATH);
                    else
                        desc_here(RD_TERSE);
                    break;
                }
                ac = -1 - ttabp->action;
                switch (ac) {
                case AKILLME: txs("%s: It's difficult to tell...\n", vbptr->id);
                case AENDPARSE:
                case AFAILPARSE:
                case AABORTPARSE:
                case ARESPOND: maxl = -1; break;
                case ASKIP: i += TP1;
                }
                if (tt.condition == CANTEP || tt.condition == CALTEP ||
                    tt.condition == CELTEP)
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
isaroom(const char *s)
{
    int r;

    roomtab = rmtab;
    for (r = 0; r < rooms; r++, roomtab++)
        if (stricmp(roomtab->id, s) == 0)
            return r;
    return -1;
}

void
follow(int subject, const char *cmd)
{
    lockusr(subject);
    if ((intam = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm->mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm->mn_Node.ln_Type = NT_MESSAGE;
    IAm->mn_ReplyPort = repbk;
    IAt = MSG_FORCE;
    IAd = 1;
    IAp = cmd;
    Amiga::PutMsg((lstat + subject)->rep, intam);
    (lstat + subject)->IOlock = -1;
}

void
log(const char *s)
{
    ioproc(s);
    char *p = ow;
    while (*p != 0) {
        if (*p == '\n') {
            *p = ' ';
        }
        ++p;
    }
    SendIt(MSG_LOG, NULL, ow);
}

void
PutRankInto(char *s)
{
    PutARankInto(s, Af);
}

void
PutARankInto(char *s, int rankNo)
{
    char *p;

    you = (usr + rankNo);
    you2 = (lstat + rankNo);

    if (you2->pre[0] != 0) {
        p = you2->pre;
        while (*p != 0)
            *(s++) = *(p++);
        *(s++) = ' ';
    }
    p = (you->sex == 0) ? (rktab + you->rank)->male
                        : (rktab + you->rank)->female;
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
        int  i;
        char tsk[6];
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
    int i;

    for (i = 0; i < nouns && me2->numobj > 0; i++) {
        if (*(obtab + i)->rmlist == -(5 + Af))
            adrop(i, torm, NO);
    }
    me2->numobj = 0;
}

void
invent(int plyr)
{
    int   i, pr, j;
    char *p;

    p = block + strlen(block);

    strcpy(p, "carrying ");
    *(p += 9) = 0;
    if ((lstat + plyr)->numobj == 0) {
        strcat(p, "nothing.\n");
        tx(block);
        return;
    }
    objtab = obtab;
    j = 0;
    pr = -(5 + plyr);
    for (i = 0; i < nouns; i++, objtab++)
        if (*objtab->rmlist == pr && canseeobj(i, Af) == YES) {
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

    if (type == VERBOSE) {
        sprintf(block, "Recorded details:		%s\n\n", vername);
        tx(block);
        tx("Name: @m! Sex  : @gn		Played   : @gp times\n");
        ioproc("@mr");
        txs("Rank: %-20s  Score: @sc points	This game: @sg points\n", ow);
        sprintf(block,
                "Strength: @sr/%ld. Stamina: @st/%ld. Dexterity %ld/%ld.\n",
                (rktab + me->rank)->strength, (rktab + me->rank)->stamina,
                me2->dext, (rktab + me->rank)->dext);
        tx(block);
        sprintf(block, "Magic Points: @mg/%ld. Wisdom:  @wi.\n",
                (rktab + me->rank)->magicpts);
        tx(block);

        sprintf(block,
                "\nCurrent Info:\nObjects Carried: %ld/%ld,	Weight Carried: "
                "%ld/%ldg\n",
                me2->numobj, (rktab + me->rank)->numobj, me2->weight,
                (rktab + me->rank)->maxweight);
        tx(block);
        tx("Following: @mf.	");
        if (me2->helping != -1)
            tx("Helping: @fr.  ");
        if (me2->helped != -1)
            tx("Helped by: @he.");
        if (me2->helping != -1 || me2->helped != -1)
            txc('\n');
        //== Current weapon
        if (me2->wield != -1)
            txs("Currently wielding: %s.\n", (obtab + me2->wield)->id);
        show_tasks(Af);
    } else {
        txs("Score: @sc. ", ow);
        sprintf(block,
                "Strength: @sr/%ld. Stamina: @st/%ld. Dexterity: %ld/%ld. "
                "Magic: @mg/%ld\n",
                (rktab + me->rank)->strength, (rktab + me->rank)->stamina,
                me2->dext, (rktab + me->rank)->dext,
                (rktab + me->rank)->magicpts);
        tx(block);
    }
}

void
calcdext()
{
    me2->dext = (rktab + me->rank)->dext;

    if (me2->flags & PFSITTING)
        me2->dext = me2->dext / 2;
    else if (me2->flags & PFLYING)
        me2->dext = me2->dext / 3;
    if ((LightHere == NO) || (me2->flags & PFBLIND))
        me2->dext = me2->dext / 5;

    me2->dext -=
            ((me2->dext / 10) -
             (((me2->dext / 10) *
               ((rktab + me->rank)->maxweight - (me2->weight))) /
              (rktab + me->rank)->maxweight));

    if (me2->flags & PFINVIS)
        me2->dext += (me2->dext / 3);
    if (me2->flags & PFSPELL_INVISIBLE)
        me2->dext += (me2->dext / 2);
    if (me->flags & PFCRIP)
        me2->dext = 0;
    me2->dext += me2->dextadj;
}

void
toprank()
{
    int i;
    for (i = 0; i < ranks - 1; i++) {
        if ((rktab + i)->tasks != 0) {
            me->tasks = me->tasks | (1 << ((rktab + i)->tasks - 1));
        }
    }
    aadd((rktab + ranks - 1)->score - me->score + 1, STSCORE, Af);
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
    fwritesafe(*me, afp);
    fclose(afp);
    afp = NULL;
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
    int i;

    if (lit(me2->room) == NO)
        return;
    if (((rmtab + me2->room)->flags & RF_HIDE_PLAYERS) != NULL &&
        me->rank != ranks - 1) {
        sys(WHO_HIDE);
        return;
    }
    for (i = 0; i < MAXU; i++) {
        if (i != Af && cansee(Af, i) == YES &&
            !((lstat + i)->flags & PFMOVING)) {
            PutARankInto(str, i);
            sprintf(block, acp(ISHERE), (usr + i)->name, str);
            if (((lstat + i)->flags & PFSITTING) != 0)
                strcat(block, ", sitting down");
            if (((lstat + i)->flags & PFLYING) != 0)
                strcat(block, ", lying down");
            if (((lstat + i)->flags & PFASLEEP) != 0)
                strcat(block, ", asleep");
            if ((lstat + i)->numobj == 0) {
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
    int i, j;

    if (type == VERBOSE) {
        for (i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (lstat + i)->state > 1 &&
                (!((lstat + i)->flags & PFSPELL_INVISIBLE))) {
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
        j = 0;
        str[0] = 0;
        for (i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (lstat + i)->state > 1 &&
                (!((lstat + i)->flags & PFSPELL_INVISIBLE))) {
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
    me->llen = UD_LINE_LENGTH;
    me->slen = UD_SCREEN_LINES;
    me->rchar = UD_REDO_CHAR;
    sprintf(spc,
            "Default settings are:\n\nScreen width = %ld, Lines = %ld, Redo = "
            "'%c', Flags = ANSI: "
            "%s, Add LF: "
            "%s\n\nChange settings (Y/n): ",
            (me->llen), (me->slen), me->rchar,
            (me->flags & UF_ANSI) ? "On" : "Off",
            (me->flags & UF_CRLF) ? "On" : "Off");
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
    sprintf(str, "\nEnter %s%s[%s]: ", "screen length",
            " (0 to disable MORE? prompting) ", input);
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
    char rchar;
    rchar = me->rchar;
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
    if (me->flags & UF_CRLF)
        tx("[Y/n]: ");
    else
        tx("[y/N]: ");
    Inp(str, 2);
    if (toupper(str[0]) == 'Y' || toupper(str[0]) == 'N')
        me->flags =
                me->flags & (toupper(str[0]) == 'Y') ? UF_CRLF : -(1 + UF_CRLF);
    tx("Use ANSI control codes?     ");
    if (me->flags & UF_ANSI)
        tx("[Y/n]: ");
    else
        tx("[y/N]: ");
    Inp(str, 2);
    if (toupper(str[0]) == 'Y' || toupper(str[0]) == 'N')
        me->flags =
                me->flags & (toupper(str[0]) == 'Y') ? UF_ANSI : -(1 + UF_ANSI);
}

//============= Please adhere to AutoDoc style docs herewith ================

/*===========================================================================*
 *
 * The following functions are low-level, and considered fairly constant.
 * However, some of them (or all of them) may be moved to the AMUL.Library
 * at a later date...
 *
 *===========================================================================*/

/****** amul.library/numb ******************************************
 *
 *   NAME
 *	numb -- mathematically compare two numbers, including <> and =.
 *
 *   SYNOPSIS
 *	ret = numb( Number, Value )
 *	d0            d0      d1
 *
 *	BOOLEAN numb( uint32_t, uint32_t );
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
 *	numb(10, ( LESS & 20 ));	Returns TRUE.
 *    Lang.Txt:
 *	numb ?brick >%wall
 *
 *   NOTES
 *	Remember to process the values using ACTUAL before passing them,
 *	thus: Numb(actual(n1), actual(n2));
 *
 ******************************************************************************/

int
numb(int32_t x, int32_t n)
{
    if (n == x) {
        return YES;
    }
    if ((n & MORE) == MORE) {
        n = n - MORE;
        if (n > x)
            return YES;
    }
    if ((n & LESS) == LESS) {
        n = n - LESS;
        if (n < x)
            return YES;
    }
    return NO;
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
 *	atreatas( NewVERB )
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
atreatas(uint32_t verbno)
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
    failed = YES;
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
ado(int verb)
{
    int32_t              old_ml, old_donet, old_verb, old_ttv, old_rm;
    struct _TT_ENT *     old_ttabp;
    struct _VERB_STRUCT *ovbptr;
    struct _SLOTTAB *    ostptr;

    old_ml = ml;
    old_donet = donet;
    old_verb = iverb;
    iverb = verb;
    old_ttv = tt.verb;
    old_rm = me2->room;
    old_ttabp = ttabp;
    ovbptr = vbptr;
    ostptr = stptr;

    lang_proc(verb, 1);

    iverb = old_verb;

    if (failed != NO || forced != 0 || died != 0 || exeunt != 0) {
        donet = ml + 1;
        ml = -1;
        failed = YES;
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

void add_obj(int to)  //== Add an object into a players inventory
{
    *objtab->rmlist = -(5 + to);  // It now belongs to them
    (lstat + to)->numobj++;
    (lstat + to)->weight += STATE->weight;
    (lstat + to)->strength -=
            ((rktab + (usr + to)->rank)->strength * STATE->weight) /
            (rktab + (usr + to)->rank)->maxweight;
    if (STATE->flags & SF_LIT)
        (lstat + to)->light++;
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
 *	The calling function MUST change the object's location, otherwise
 *	the player will effectively still own the object.
 *
 *   SEE ALSO
 *	add_obj()
 *
 ******************************************************************************
 *
 */
void rem_obj(int to, int ob)  //== Remove object from inventory
{
    (lstat + to)->numobj--;
    (lstat + to)->weight -= STATE->weight;
    (lstat + to)->strength +=
            ((rktab + (usr + to)->rank)->strength * STATE->weight) /
            (rktab + (usr + to)->rank)->maxweight;
    if (STATE->flags & SF_LIT)
        (lstat + to)->light--;
    if (me2->wield == ob)
        me2->wield = -1;  //== Don't let me wield it
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
    if ((lstat + who)->state == US_CONNECTED)
        actor = who;
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
    uint32_t t1, t2;

    inc = 0;
    // === N1 Handling ===
    if (n1 == TC_NONE)
        t1 = n1;
    else if ((n1 & IWORD))  // Is it an IWORD?
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
        t1 = TC_NOUN;
    }

    // === N2 Handling ===
    if (n2 == TC_NONE)
        t2 = n2;
    else if ((n2 & IWORD))  // Is it an IWORD?
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
        n2 = isnoun(
                (obtab + n2)->id, (obtab + n2)->adj, (vbtab + iverb)->sort2);
        t2 = TC_NOUN;
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
 *	void iocopy( uint8_t, uint8_t, uint32_t );
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
 *	Source - Contains the processed string, upto MaxLen bytes int32_t.
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

void
iocopy(char *dst, const char *src, uint32_t length)
{
    ioproc(src);
    qcopy(dst, ow, length);
}

// -- Quick copy - used by iocopy and others --

void
qcopy(char *dst, const char *src, uint32_t length)
{
    Amiga::ScheduleGuard guard;
    while (length > 0 && *src != 0 && *src != '\n') {
        *(dst++) = *(src++);
    }
    *dst = 0;
}

/****** AMUL3.C/DoThis ******************************************
 *
 *   NAME
 *	DoThis -- Tell another player to follow me or perform an action.
 *
 *   SYNOPSIS
 *	DoThis( Player, Command, Type )
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
DoThis(int x, const char *cmd, short int type)
{
    lockusr(x);
    if ((intam = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = MSG_FORCE;
    IAd = type;
    IAp = cmd;
    Amiga::PutMsg((lstat + x)->rep, intam);
    (lstat + x)->IOlock = -1;
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
    {
        Amiga::ScheduleGuard guard;
        if (fol != 0 || me2->following == -1 ||
            (vbtab + overb)->flags & VB_TRAVEL) {
            return;
        }
        if ((lstat + me2->following)->state != US_CONNECTED ||
            (lstat + me2->following)->followed != Af) {
            me2->following = -1;
            return;
        }
        (lstat + me2->following)->followed = -1;
    }

    tx("You are no-longer following @mf.\n");
    me2->following = -1;
}

#include "amulinc.h"

/****** AMUL3.C/internal ******************************************
 *
 *   NAME
 *	internal -- process internal control command.
 *
 *   SYNOPSIS
 *	internal( Command )
 *
 *	void internal( uint8_t );
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
internal(const char *s)
{
    char *p;

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

    p = s;
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
        me->flags = me->flags ^ UF_CRLF;
        txs("LineFeed follows carriage return %sABLED.\n",
            (me->flags & UF_CRLF) ? "EN" : "DIS");
        return;
    }

    if (*s == 'a') {
        switch (*(s + 1)) {
        case 'n':
            me->flags = me->flags ^ UF_ANSI;
            ans("1m");
            txs("ANSI control codes now %sABLED.\n",
                (me->flags & UF_ANSI) ? "EN" : "DIS");
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
    (lstat + (me2->followed))->following = -1;  // Unhook them
    utx(me2->followed, "You are no-longer able to follow @me.\n");
    me2->followed = -1;  // Unhook me
}

/****** AMUL3.C/ShowFile ******************************************
 *
 *   NAME
 *	ShowFile -- Send file to user (add extension)
 *
 *   SYNOPSIS
 *	ShowFile( FileName )
 *
 *	void ShowFile( uint8_t );
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
ShowFile(const char *s)
{
    int32_t fsize;
    char *  p;

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
    txs("\n--+ Please inform the dungeon master that file %s is missing.\n\n",
        s);
    return;
show:
    fseek(ifp, 0, SEEK_END);
    fsize = ftell(ifp);
    fseek(ifp, 0, 0L);
    if ((p = (char *)OS::AllocateClear(fsize + 2)) == NULL) {
        txs("\n--+ \x07System memory too low, exiting! +--\n");
        forced = 1;
        exeunt = 1;
        kquit("out of memory!\n");
    }
    fread(p, fsize, 1, ifp);
    tx(p);
    OS::Free(p, fsize + 2);
    pressret();
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

int32_t
scaled(int32_t value, short int flags)
{
    if (!(flags & SF_SCALED))
        return value;
    int32_t scalefactor = ((rscale * me->rank * 100 / ranks) +
                           (tscale * *rescnt * 100 / (mins * 60))) /
                          100;
    return value - (value * scalefactor / 100);
}

/****** AMUL3.C/showin ******************************************
 *
 *   NAME
 *	showin -- Display the contents of an object.
 *
 *   SYNOPSIS
 *	showin( Object, Mode )
 *
 *	void showin( int, int );
 *
 *   FUNCTION
 *	Displays the contents of an object, modified depending on the
 *	objects 'putto' flag. Mode determines whether output is given when
 *	the contents of the object cannot be seen or there it is empty.
 *
 *   INPUTS
 *	Object -- the object's id number.
 *	Mode   -- YES to force display of contents, or to inform the player
 *		  if the object is empty.
 *		  NO not to list the contents of the object if it is opaque,
 *		  and not to display anything when it is empty.
 *
 ******************************************************************************
 *
 */

void
showin(int o, int mode)
{
    int   i, j, k, l;
    char *p, *s;

    if (State(o)->flags & SF_OPAQUE && mode == NO) {
        tx(str);
        txc('\n');
        return;
    }
    p = str + strlen(str);
    if ((obtab + o)->inside <= 0) {
        if (mode == YES) {
            if ((obtab + o)->putto == 0)
                sprintf(p, "The %s contains ", (obtab + o)->id);
            else
                sprintf(p, "%s the %s you find: ", obputs[(obtab + o)->putto]);
            strcat(p, "nothing.\n");
        } else
            sprintf(p, "\n");
        tx(str);
        return;
    }

    if ((obtab + o)->putto == 0)
        sprintf(p, "The %s contains ", (obtab + o)->id);
    else
        sprintf(p, "%s the %s you find: ", obputs[(obtab + o)->putto],
                (obtab + o)->id);
    p += strlen(p);

    j = 0;
    k = -(INS + o);
    l = (obtab + o)->inside;
    for (i = 0; i < nouns && l > 0; i++) {
        if (*(obtab + i)->rmlist != k)
            continue;
        if (j != 0) {
            *(p++) = ',';
            *(p++) = ' ';
        }
        s = (obtab + i)->id;
        while (*s != 0)
            *(p++) = *(s++);
        *p = 0;
        j = 1;
        l--;
    }
    strcat(p, ".\n");
    tx(str);
}

/****** AMUL3.C/stfull ******************************************
 *
 *   NAME
 *	stfull -- Check if players property is at full
 *
 *   SYNOPSIS
 *	stfull( Stat, Player )
 *
 *	BOOLEAN stfull( uint16_t, uint16_t );
 *
 *   FUNCTION
 *	Tests to see if a players 'stat' is at full power and returns a
 *	TRUE or FALSE result (YES or NO).
 *
 *   INPUTS
 *	stat   -- a players stat number (see h/amul.defs.h)
 *	player -- number of the player to check
 *
 *   RESULT
 *	YES if it is
 *	NO  if it isn't
 *
 ******************************************************************************
 *
 */

int stfull(int st, int p)  // full <st> <player>
{
    you = (usr + p);
    you2 = (lstat + p);
    switch (st) {
    case STSCORE: return NO;
    case STSCTG: return NO;
    case STSTR:
        if (you2->strength < you->strength)
            return NO;
        break;
    case STDEX:
        if (you2->dext < you->dext)
            return NO;
        break;
    case STSTAM:
        if (you2->stamina < you->stamina)
            return NO;
        break;
    case STWIS:
        if (you2->wisdom < you->wisdom)
            return NO;
        break;
    case STMAGIC:
        if (you2->magicpts < you->magicpts)
            return NO;
        break;
    case STEXP:
        if (you->experience < (rktab + you->rank)->experience)
            return NO;
        break;
    }
    return YES;
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
    char i, j, x, w, f;

    objtab = obtab + obj;
    x = owner(obj);
    i = lit(loc(obj));  // WAS the room lit?
    // Remove from owners inventory
    if (x != -1) {
        w = (lstat + x)->wield;
        rem_obj(x, obj);
    }
    f = STATE->flags & SF_LIT;
    objtab->state = stat;
    if (objtab->flags & OF_SHOWFIRE) {
        if (f == 0)
            STATE->flags = STATE->flags & -(1 + SF_LIT);
        else
            STATE->flags = STATE->flags | SF_LIT;
        if (x != -1)
            add_obj(x);
        return;  // Don't need to check for change of lights!
    }

    if (x != -1) {
        add_obj(x);  // And put it back again
        //== Should check to see if its too heavy now
        lighting(x, AHERE);
        (lstat + x)->wield = w;
    }

    if ((j = lit(loc(obj))) == i)
        return;
    if (j == NO) {
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
    int      i, j, found, k;
    int32_t *rp;

    found = -1;
    for (i = 0; i < nouns; i++)  // Check all
    {
        if (stricmp((obtab + obj)->id, (obtab + i)->id) == NULL) {
            if (canseeobj(i, Af) == NO)
                continue;
            if ((j = owner(i)) != -1) {
                if (lit((lstat + j)->room) == YES) {
                    if (j != Af) {
                        tx("You see ");
                        ans("1m");
                        tx((usr + j)->name);
                        ans("0;37m");
                        tx(".\n");
                    } else
                        tx("There is one in your possesion.\n");
                    found++;
                }
                continue;
            }
            rp = (obtab + i)->rmlist;
            for (j = 0; j < (obtab + i)->nrooms; j++) {
                if (*(rp + j) == -1)
                    continue;
                if (*(rp + j) >= 0) {
                    if (*(rp + j) == -1 || lit(*(rp + j)) == NO)
                        continue;
                    roomtab = rmtab + *(rp + j);
                    desc_here(2);
                } else {
                    k = -(INS + *(rp + j));
                    sprintf(block, "There is one %s something known as %s!\n",
                            obputs[(obtab + k)->putto], (obtab + k)->id);
                    tx(block);
                }
                found++;
            }
        }
    }
    if (found == -1)
        sys(SPELLFAIL);
}

void
osflag(int o, int flag)
{
    int own, l;

    objtab = obtab + o;

    own = owner(o);
    if (own == -1)
        l = lit(loc(o));
    else
        rem_obj(own, o);
    STATE->flags = flag;
    if (own != -1) {
        add_obj(o);
        lighting(own, AHERE);
        return;
    }
    if (lit(loc(o)) != l)
        if (l == YES) {
            actionfrom(o, acp(NOWTOODARK));
            sys(NOWTOODARK);
        } else {
            actionfrom(o, acp(NOWLIGHT));
            sys(NOWLIGHT);
        }
}

// Set @xx and @xy corresponding to a specific player

void
setmxy(int Flags, int Them)
{
    if (Them == Af || cansee(Them, Af) == YES)  // If he can see me
    {
        ioproc("@me");
        strcpy(mxx, ow);
        ioproc("@me the @mr");
        strcpy(mxy, ow);
        return;
    }
    if (pROOM(Them) == me2->room) {
        switch (Flags) {
        case PC_ACTION:
        case PC_EVENT:
        case PC_TEXTS:
            strcpy(mxx, "Someone nearby");
            strcpy(mxy, "Someone nearby");
            return;
        case PC_NOISE:
            strcpy(mxx, "Someone nearby");
            ioproc("A @gn voice nearby");
            strcpy(mxy, ow);
            return;
        }
    }
    // They aren't in the same room
    switch (Flags) {
    case PC_ACTION:
    case PC_EVENT:
        strcpy(mxx, "Someone");
        if (me->rank == ranks - 1)
            strcpy(mxy, "Someone very powerful");
        else
            strcpy(mxy, "Someone");
        return;
    case PC_TEXTS:
        ioproc("@me");
        strcpy(mxx, ow);
        if (me->rank == ranks - 1)
            ioproc("@me the @mr");
        strcpy(mxy, ow);
        return;
    case PC_NOISE:
        strcpy(mxx, "Someone");
        if (me->rank == ranks - 1)
            ioproc("A powerful @gn voice somewhere in the distance");
        else
            ioproc("A @gn voice in the distance");
        strcpy(mxy, ow);
        return;
    default: strcpy(mxx, "Someone"); strcpy(mxy, "Someone");
    }
}
