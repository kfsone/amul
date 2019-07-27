/*
          ####        ###     ###  ###   ### ####
         ##  ##        ###   ###   ##     ##  ##            Amiga
        ##    ##       #########   ##     ##  ##            Multi
        ##    ##       #########   ##     ##  ##            User
        ########  ---  ## ### ##   ##     ##  ##            adventure
        ##    ##       ##     ##    ##   ##   ##     #      Language
        ###  ###      ####   ####   #######  #########


  Copyright (C) Oliver Smith, 1990/1. Copyright (C) Kingfisher s/w 1990/1
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike

                                       */

#define MAXD 150
#define XR 16668

#define AMAN 1
#define PORTS 1
#define AMAN 1
#define TRQ timerequest

#define UINFO                                                                                      \
    ((sizeof(*usr) + sizeof(*linestat)) * MAXNODE) + (rooms * sizeof(short)) + (sizeof(mob) * mobs)

#include <h/aman.h>
#include <h/amul.cons.h>
#include <h/amul.defs.h>
#include <h/amul.vars.h>
#if defined(__AMIGA__)
#    include <devices/timer.h>
#else
#    include <h/amigastubs.h>
#endif
#include <time.h>

char *          ttxt, lastres[24], lastcrt[24], bid[MAXNODE], busy[MAXNODE], *xread(), *now();
char            globflg[MAXD];                /* Is daemon global?   */
long            reslp, TDBase, daemons, mins; /* Daemons to process! */
short int       count[MAXD], num[MAXD];       /* Daemon counters in minutes! & No.s */
char            own[MAXD];                    /* And their owners... */
long            val[MAXD][2], typ[MAXD][2];   /* Values and types... */
struct Aport *  am;
struct TRQ      ResReq;                /* Reset request & Incoming from timer */
struct MsgPort *trport;                /* Timer port */
long            invis, invis2, calls;  /* Invisibility Stuff & # of calls */
long            obtlen, nextdaem, ded; /* nextdaem = time to next daemon, ded = deduct */

long online, clock, cclock;
char resety, forcereset, quiet;

int CXBRK() /* Prevent CTRL-C'ing */ { return 0; }

char *now() /* Get current time/date */
{
    if (clock == 0)
        time(&clock);
    ttxt = (char *)ctime(&clock) + 4;
    clock = 0;
    *(ttxt + strlen(ttxt) - 1) = 0; /* Strip cr/lf */
    return ttxt;
}

// Release all memory AllocMem'd
static void
givebackmemory()
{
    ReleaseMem(&obtab);
    ReleaseMem(&rktab);
    ReleaseMem(&rmtab);
    ReleaseMem(&slottab);
    ReleaseMem(&synp);
    ReleaseMem(&ttp);
    ReleaseMem(&ttpp);
    ReleaseMem(&usr);
    ReleaseMem(&vbtab);

    adtab = NULL;
    linestat = NULL;
    ormtab = NULL;
    rctab = NULL;
    statab = NULL;
    synip = NULL;
    vtp = NULL;
    vtpp = NULL;
}

void
quit() /* Exit program tidily */
{
    if (ifp != NULL)
        fclose(ifp);
    if (reply != NULL)
        DeletePort(reply);
    if (port != NULL)
        DeletePort(port);
    if (TDBase != NULL) {
        AbortIO((struct IORequest *)&ResReq);
        CloseDevice((struct IORequest *)&ResReq);
    }
    if (trport != NULL)
        DeletePort(trport);
    givebackmemory();
    exit(0);
}

// Report memory allocation error
static void
memfail(char *s)
{
    sprintf(block, "** Unable to allocate memory for %s! **\n", s);
    log(block);
    quit();
}

// Report a read failure
static void
readfail(char *s, int got, int wanted)
{
    printf("** Error: Expected %d ", wanted);
    printf(s);
    printf(" entries, only got %d!\n", got);
    quit();
}

// report Open ERror
static void
openError(char *s, char *t)
{
    printf("\x07** Error: Can't open file %s for %sing!\n\n", s, t);
    quit();
}

// Open a file for reading
static void
fopenr(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL)
        openError(block, "read");
}

// Size/Read data files
char *
xread(char *s, long *n, char *t)
{
    char *p;
    int   i;

    fopenr(s);
    fseek(ifp, 0, 2L);
    *n = ftell(ifp);
    fseek(ifp, 0, 0L);
    if (*n != 0) {
        if ((p = (char *)AllocateMem(*n)) == NULL)
            memfail(t);
        if ((i = fread(p, 1, *n, ifp)) != *n)
            readfail(t, i, *n);
    }
    return p;
}

char *
readf(char *s, char *p)
{
    fopenr(s);
    fread(p, 32767, 32767, ifp);
    fclose(ifp);
    ifp = NULL;
    return p;
}

static void
execute(const char*)
{
	/// TODO: Remove
	// run a command in the background
}

static void
setam()
{
    /// TODO: On the Amiga, it was the caller's responsibility to free messages
    am = (struct Aport *)AllocateMem(sizeof(*amul));
    am->msg.mn_Node.ln_Type = NT_MESSAGE;
    am->msg.mn_ReplyPort = reply;
    am->msg.mn_Length = (UWORD)sizeof(*am);
    am->from = -1;
}

// Force users to log-out & kill extra lines
static void
reset_users()
{
    int    i;
    struct Message *xx;

    online = 0; // Allows for daemons & mobiles
    for (i = 0; i < MAXNODE; i++) {
        if ((linestat + i)->state <= 0)
            continue;
        online++;
        setam();
        am->type = MCLOSEING;
        am->msg.mn_ReplyPort = port;
        PutMsg((linestat + i)->rep, am);
    }
    while (online > 0) {
        WaitPort(port);
        am = (struct Aport *)GetMsg(port);
    loop:
        if (am->from != -'O') {
            printf("\x07 !! Invalid message!\n");
            am->type = am->data = -'R';
            ReplyMsg((struct Message *)am);
            goto skip;
        }
        ReleaseMem(&am);
        online--;
    skip:
        if ((am = (struct Aport *)GetMsg(port)) != NULL)
            goto loop;
    }
loop2:
    if ((am = (struct Aport *)GetMsg(reply)) != NULL) {
        am->type = am->data = -'R';
        ReplyMsg(am);
        goto loop2;
    }
    while ((xx = GetMsg(port)) != NULL)
        ReplyMsg(xx);
    while ((xx = GetMsg(reply)) != NULL)
        ReplyMsg(xx);
    while (GetMsg((struct MsgPort *)trport) != NULL)
        ;
    online = 0;
}

static void
warn(char *s)
{
    for (size_t i = 0; i < MAXU; i++)
        if ((linestat + i)->state != OFFLINE) {
            setam();
            am->ptr = s;
            am->type = MRWARN;
            PutMsg((linestat + i)->rep, am);
        }
}

// Receive shutdown request
static void
kill()
{
    sprintf(block, "!! (X) %s: shutdown request, from ", now());
    log(block);
    if (Af != -1) {
        sprintf(block, "line %ld.\n", Af + 1);
        log(block);
    } else
        log("the void!\n");
    if (online != 0) {
        sprintf(block, "&&%25s: Request denied, %ld users on-line!\n", " ", online);
        log(block);
        Ad = At = 'X';
    } else {
        reset_users();
        Ad = At = 'O';
        resety = -1;
    }
}

// User connecting
// static void
cnct()
{
    int i;

    Ad = (long)linestat;
    Ap = (char *)usr;
    if (Af >= MAXU) {
        if (Af == MAXU + 1)
            printf("** Mobile processor connected.\n");
        if ((linestat + Af)->state != 0)
            Af = -1;
        else
            (linestat + Af)->state = PLAYING;
        return;
    }
    Af = -1;
	// Allow for daemons & mobiles
    for (i = 0; i < MAXU; i++)
    {
        if ((linestat + i)->state != 0)
            continue;
        Af = i;
        (linestat + i)->state = LOGGING;
        online++;
        calls++;
        break;
    }
}

// Remove daemon from list
void
pack(int i)
{
    int j;
    if (i != (daemons - 1)) {
        j = daemons - 1;
        num[i] = num[j];
        own[i] = own[j];
        count[i] = count[j];
        globflg[i] = globflg[j];
        val[i][0] = val[j][0];
        val[i][1] = val[j][1];
        typ[i][0] = typ[j][0];
        typ[i][1] = typ[j][1];
        i = j;
    }
    own[i] = -1;
    count[i] = -1;
    val[i][0] = -1;
    val[i][1] = -1;
    typ[i][0] = -1;
    typ[i][1] = -1;
    globflg[i] = -1;
    daemons--;
}

// Cancel daemon...
void
dkill(short int d)
{
    int i;

    nextdaem = mins * 60;
    for (i = 1; i < daemons; i++) {
        if (((d != -1 && globflg[i] == TRUE) || own[i] == Af) && (num[i] == d || d == -1))
            pack(i);
        if (i != daemons && count[i] < nextdaem)
            nextdaem = count[i];
    }
}

// Initiate daemon
void
start(char owner)
{
    /* Ad=#, p1=inoun1, p2=inoun2, p3=wtype[2], p4=wtype[5], Ap=count */

    val[daemons][0] = Ap1;
    val[daemons][1] = Ap2;
    typ[daemons][0] = Ap3;
    typ[daemons][1] = Ap4;
    own[daemons] = owner;
    count[daemons] = (short int)Ap;
    num[daemons] = Ad;
    daemons++;
    if (count[daemons - 1] < nextdaem)
        nextdaem = count[daemons - 1];
}

// Initiate global daemon
void
gstart()
{
    globflg[daemons] = TRUE;
    start(MAXU); /* Set global flag & go! */
}

// Initiate private daemon
void
pstart()
{
    globflg[daemons] = FALSE;
    start(Af);
}

// Check if daemon is active
void
check(int d)
{
    int i;
    Ad = -1;
    Ap1 = -1;
    for (i = 1; i < daemons; i++)
        if ((own[i] == Af || globflg[i] == TRUE) && num[i] == d) {
            Ad = i;
            Ap1 = count[i];
            break;
        }
}

// User disconnection
void
discnct()
{
    if (Af < MAXU && (linestat + Af)->state == PLAYING) {
        sprintf(block, "<- (%d) %s: user disconnected.\n", Af, now());
        log(block);
    }
    if (Af < MAXU)
        online--;
    (usr + Af)->name[0] = 0;
    (linestat + Af)->room = -1;
    (linestat + Af)->helping = -1;
    (linestat + Af)->following = -1;
    dkill(-1);
    (linestat + Af)->state = 0;
    Af = -1;
    Ad = -1;
}

// Sends pointers to database
static void
data()
{
    At = MDATAREQ;
    switch (Ad) {
    case -1:
        Ad = online;
        Ap = (char *)usr;
        Ap1 = calls;
        Ap2 = (long)vername;
        Ap3 = (long)adname;
        Ap4 = (long)linestat;
        break;
    case 0:
        strcpy(Ap, dir);
        amul->p1 = count[0];
        Ap1 = (long)&count[0];
        break;
    case 1:
        Ad = rooms;
        Ap = (char *)rmtab;
        break;
    case 2:
        Ad = ranks;
        Ap = (char *)rktab;
        break;
    case 3:
        Ad = nouns;
        Ap = (char *)obtab;
        break;
    case 4:
        Ad = verbs;
        Ap = (char *)vbtab;
        break;
    case 5:
        Ap = (char *)desctab;
        break;
    case 6:
        Ap = (char *)ormtab;
        break;
    case 7:
        Ap = (char *)statab;
        break;
    case 8:
        Ap = (char *)adtab;
        break;
    case 9:
        Ap = (char *)ttp;
        break;
    case 10:
        Ap = (char *)umsgip;
        break;
    case 11:
        Ap = (char *)umsgp;
        break;
    case 12:
        Ap = (char *)ttpp;
        break;
    case 13:
        Ap = (char *)rctab;
        break;
    case 14:
        Ap = (char *)slottab;
        break;
    case 15:
        Ap = (char *)vtp;
        break;
    case 16:
        Ap = (char *)vtpp;
        break;
    case 17:
        Ap = (char *)synp;
        Ad = (long)synip;
        break;
    case 18:
        Ap = lastres;
        Ad = (long)lastcrt;
        break;
    default:
        Ap = (char *)-1;
    }
}

// Receive & log a player login
static void
login()
{
    sprintf(block, "-> (%d) %s: \"%s\" logged in.\n", Af, now(), (usr + Af)->name);
    (linestat + Af)->state = PLAYING;
    log(block);
}

// Shutdown request
static void
asend(int type, int data)
{
    if ((reply = CreatePort("Killer!", 1L)) == NULL) {
        printf("Unable to create killer port!\n");
        return;
    }
    amul = (struct Aport *)AllocateMem(sizeof(*amul));
    if (amul == NULL) {
        printf("Unable to allocate AMUL port memory!\n");
        DeletePort(reply);
        return;
    }
    At = type;
    Ad = data;
    Af = -1;
    Am.mn_Node.ln_Type = NT_MESSAGE;
    Am.mn_ReplyPort = reply;
    Am.mn_Length = (UWORD)sizeof(*amul);
    PutMsg(port, amul);
    WaitPort(reply);
    GetMsg((struct MsgPort *)reply);
    if (quiet == 0)
        switch (Ad) {
        case 'R':
            printf("\x07*-- Reset Invoked --*\n\n");
            break;
        case 'O':
            printf("AMUL Manager removed!\n");
            break;
        case 'X':
            printf("Cannot remove with users connected.\n");
            break;
        case 'U':
            printf("AMAN error at other end!\n");
        case -'X':
            printf("... Reset set for %ld seconds ...\n", Ap1);
            break;
        case -'R':
            printf("... Reset in progress ...\n");
            break;
        case 'E':
            printf("... Game extended by %ld seconds ...\n", Ap1);
            break;
        default:
            printf("** Internal AMUL error ** (Returned '%c')\n", Ad);
            break;
        }
    ReleaseMem(&amul);
    DeletePort(reply);
    printf("\n");
    return;
}

static void
shutreq(int x)
{
	asend((x == 0) ? MKILL : MRESET, count[0]);
}

static void
sendext(int t)
{
	asend(MEXTEND, t);
}

// RESeT in progress
static void
rest()
{
    forcereset = 1;
    if (Ad > 0) {
        Ap1 = Ad;
        count[0] = Ad + 1;
        sprintf(block, "** System reset invoked - %ld seconds remaining...\n", Ad);
        warn(block);
        Ad = At = -'X';
        return;
    }
    Ad = At = 'R';
    count[0] = 1;
}

// Extend by this many ticks
static void
extend(short int tics)
{
    short int newtime;

    Ad = At = 'U';
    if (tics == 0)
        return;

    newtime = count[0] + tics + 1;
    if (count[0] > 120)
        sprintf(block, "...Game time extended - reset will now occur in %ld %s and %ld %s...\n",
                newtime / 60, "minutes", newtime - ((newtime / 60) * 60), "seconds");
    else
        sprintf(block, "...Reset postponed - it will now occur in %ld %s...\n", newtime, "seconds");
    warn(block);
    Ap1 = tics;
    count[0] = newtime;
    Ad = 'E';
}

// Reset <receiver>
static void
res()
{
    int onwas;
    sprintf(block, "][ (%c) %s: Reset requested! %ld user(s) online...\n",
            (Af >= 0 && Af < 11) ? '0' + Af : '#', now(), online);
    log(block);
    onwas = online;
    reset_users();
    if (onwas != 0)
        sprintf(block, "== (#) %s: All users disconnected...\n", now());
    else
        sprintf(block, "== (#) %s: Reset completed!\n", now());
    log(block);
    Delay(90);  // Always wait atleast a few seconds
}

// Lock a users IO system
static void
lock()
{
    bid[Af] = Ad;
    if ((linestat + Ad)->IOlock != -1 || (busy[Ad] != 0 && Ad != Af && bid[Ad] != Af)) {
        Ad = -1;
        return;
    }
    (linestat + Ad)->IOlock = Af;
    bid[Af] = -1;
}

// Log data to AMUL.Log
static void
log(char *s)
{
    FILE *fp;

    if ((fp = fopen("AMUL.Log", "ab+")) == NULL)
        openError("AMUL.Log", "logg");
    fseek((FILE *)fp, 0, 2L);
    fprintf((FILE *)fp, s);
    fclose((FILE *)fp);
}

static void
logwiz(int who)
{
    sprintf(block, "@@ ]%c[ %s: User \"%s\" achieved top rank (%ld)!!!\n", Af + '0', now(),
            (usr + Af)->name, (usr + Af)->rank + 1);
    log(block);
}

static void
logit(char *s)
{
    sprintf(block, "@@ (%c) %s: %s\n", Af + '0', now(), s);
    log(block);
}

static int
fsize(char *s)
{
    int n;
    fopenr(s);
    fseek(ifp, 0, 2L);
    n = ftell(ifp);
    fclose(ifp);
    ifp = NULL;
    return n;
}

// Read in & evaluate data files
static void
setup()
{
    long  i, rc, l, act, j, k;
    long *         pt;
    char *p;

    rc = 0;
    fopenr(gameDataFile);
    fgets(adname, 41, ifp);
    adname[strlen(adname) - 1] = 0;
    fscanf(ifp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &rooms,
           &ranks, &verbs, &syns, &nouns, &adjs, &ttents, &umsgs, &cclock, &mins, &invis, &invis2,
           &minsgo, &mobs, &rscale, &tscale, &mobchars);
    if ((p = AllocateMem(UINFO)) == NULL)
        memfail("User tables");
    usr = (struct _PLAYER *)p;
    p += sizeof(*usr) * MAXNODE;
    linestat = (struct LS *)p;
    p += sizeof(*linestat) * MAXNODE;
    rctab = (short *)p;

    fopenr(roomDataFile); /* 1: Open room block file */
    if ((rmtab = (struct room *)AllocateMem(rooms * sizeof(room))) == NULL)
        memfail("room table"); /* Allocate memory */
    if ((i = fread((char *)rmtab, sizeof(room), rooms, ifp)) != rooms)
        readfail("room table", i, rooms);

    fopenr(rankDataFile); /* 2: Read player ranks */
    if ((rktab = (struct rank *)AllocateMem(ranks * sizeof(rank))) == NULL)
        memfail("player ranks"); /* Allocate memory */
    if ((i = fread((char *)rktab, sizeof(rank), ranks, ifp)) != ranks)
        readfail("player ranks", i, ranks);

    fopenr(verbDataFile); /* 4: Read the verbs in */
    if ((vbtab = (struct verb *)AllocateMem(verbs * sizeof(verb))) == NULL)
        memfail("verb table");
    if ((i = fread(vbtab->id, sizeof(verb), verbs, ifp)) != verbs)
        readfail("verb table", i, verbs);

    /* 3, 5, 6 & 7: Read objects */
    obtlen = fsize(objectDataFile);
    desctlen = fsize(obdsfn);
    ormtablen = fsize(objectRoomFile);
    statablen = fsize(objectStateFile);
    if ((p = AllocateMem(obtlen + desctlen + ormtablen + statablen)) == NULL)
        memfail("object data");
    obtab = (struct obj *)readf(objectDataFile, p);
    desctab = (char *)readf(obdsfn, (p = p + obtlen));
    ormtab = (long)readf(objectRoomFile, (p = p + desctlen));
    statab = (struct state *)readf(objectStateFile, p + ormtablen);

    /* Update the object room list ptrs and the state ptrs */
    statep = statab;
    for (i = 0; i < nouns; i++) {
        objtab = obtab + i;
        objtab->rmlist = (long *)(ormtab + rc);
        rc += objtab->nrooms * sizeof(long);
        objtab->states = statep;
        statep = statep + (long)objtab->nstates;
    }

    /* 10 & 11: User Messages */

    umsgil = fsize(umsgifn);
    umsgl = fsize(umsgfn);
    if ((p = AllocateMem(umsgil + umsgl)) == NULL)
        memfail("user messages");
    umsgip = (long *)readf(umsgifn, p);
    umsgp = (char *)readf(umsgfn, p + umsgil);

    /* 9: Read the travel table */
    ttp = (struct tt *)xread(travelTableFile, &ttlen, "travel table");

    /* 12: Read parameters */
    ttpp = (long *)xread(travelParamFile, &ttplen, "TT parameter table");
    ttents = ttlen / sizeof(tt);
    ttabp = ttp;
    pt = ttpp;
    for (i = 0; i < ttents; i++) {
        ttabp = ttp + i;
        k = (long)ttabp->pptr;
        ttabp->pptr = pt;
        if (k == -2)
            continue;
        act = ttabp->condition;
        if (act < 0)
            act = -1 - act;
        pt += ncop[act];
        act = ttabp->action;
        if (act < 0) {
            act = -1 - act;
            pt += nacp[act];
        }
    }

    /* 14: Load Slot table */
    stlen = fsize(verbSlotFile);
    vtlen = fsize(verbTableFile);
    vtplen = fsize(verbParamFile);
    if ((p = AllocateMem(stlen + vtlen + vtplen)) == NULL)
        memfail("language data");
    slottab = (struct _SLOTTAB *)readf(verbSlotFile, p);
    vtp = (struct _VBTAB *)readf(verbTableFile, p + stlen);
    vtpp = (long *)readf(verbParamFile, p + stlen + vtlen);

    /* 17: Get the Synonym data & adjectives */
    synlen = fsize(synonymDataFile);
    synilen = fsize(synonymIndexFile);
    adtablen = fsize(adjectiveDataFile);
    if ((p = AllocateMem(synlen + synilen + adtablen)) == NULL)
        memfail("synonym data");
    synp = (char *)readf(synonymDataFile, p);
    synip = (short int *)readf(synonymIndexFile, (p = p + synlen));
    adtab = (char *)readf(adjectiveDataFile, p + synilen);

    /* 18: Get last reset time */
    strcpy(lastres, now());
    clock = cclock;
    strcpy(lastcrt, now());

    /* Adjust the verb-related pointers */
    vbptr = vbtab;
    stptr = slottab;
    vtabp = vtp;
    l = 0;
    for (i = 0; i < verbs; i++, vbptr++) {
        vbptr->ptr = stptr;
        for (j = 0; j < vbptr->ents; j++, stptr++) {
            stptr->ptr = vtabp;
            for (k = 0; k < stptr->ents; k++, vtabp++) {
                vtabp->pptr = vtpp + l;
                act = vtabp->condition;
                if (act < 0)
                    act = -1 - act;
                l += ncop[act];
                act = vtabp->action;
                if (act < 0) {
                    act = -1 - act;
                    l += nacp[act];
                }
            }
        }
    }

    /* Fix the object 'inside' flags */
    objtab = obtab;
    for (i = 0; i < nouns; i++, objtab++) {
        if (*(objtab->rmlist) <= -INS)
            (obtab + (-(INS + *(objtab->rmlist))))->inside++;
    }
    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
}

static void
kernel()
{
    int   i;
    FILE *fp;

    strcpy(block, "--------------------------------------------------------------\n");
    log(block);
    online = resety = forcereset = 0;
    for (i = 0; i < MAXD; i++) {
        count[i] = -1;
        own[i] = -1;
        val[i][0] = val[i][1] = typ[i][0] = typ[i][1] = 0;
        globflg[i] = FALSE;
    }
    daemons = 1;
    nextdaem = count[0] = mins * 60;
    for (i = 0; i < MAXNODE; i++) {
        (linestat + i)->IOlock = -1;
        (linestat + i)->room = bid[i] = -1;
        busy[i] = 0;
        (linestat + i)->helping = -1;
        (linestat + i)->following = -1;
    }
    ResReq.tr_time.tv_secs = 1;
    ResReq.tr_time.tv_micro = XR;
    ResReq.tr_node.io_Command = TR_ADDREQUEST;
    SendIO(&ResReq.tr_node);
    sprintf(block, "== (=) %s: [3mLoaded '%s'.[0m\n", now(), adname);
    log(block);
    forcereset = ded = 0;

    /* Activate the daemon processor */

    sprintf(block, "run >NIL: amul -%c", 3); /* Request daemon processor */
    Execute(block, 0L, 0L);                  /* activate it! */

    while (resety == 0) {
        Wait(-1);
    readport:
        while ((am = (struct Aport *)GetMsg(reply)) != NULL) {
            ReleaseMem(&am);
        }
        while (GetMsg((struct MsgPort *)trport) != NULL) {
            /* Process counter table */
            for (i = 0; i < daemons; i++) {
                count[i]--;
                if (count[i] <= 0) {
                    if (i == 0)
                        break;
                    setam();
                    am->data = num[i];
                    am->p1 = val[i][0];
                    am->p2 = val[i][1];
                    am->p3 = typ[i][0];
                    am->p4 = typ[i][1];
                    am->type = MDAEMON;
                    PutMsg((linestat + own[i])->rep, am);
                    pack(i);
                    i--;
                }
            }
            if (count[0] == 300) {
                warn("--+ Next reset in 5 minutes +--\n");
            }
            if (count[0] == 120) {
                warn("--+ 120 seconds until next reset +--\n");
            }
            if (count[0] == 60) {
                warn("--+ Final warning - 60 seconds to reset +--\n");
            }
            if (count[0] <= 0) {
                count[0] = -10;
                resety = 1;
                if (forcereset == 0)
                    printf("[ Automatic Reset ]\n");
            } else {
                ResReq.tr_time.tv_secs = 1;
                ResReq.tr_time.tv_micro = XR;
                ResReq.tr_node.io_Command = TR_ADDREQUEST;
                SendIO(&ResReq.tr_node);
            }
        }
        if ((amul = (struct Aport *)GetMsg((struct MsgPort *)port)) == NULL)
            continue;
        switch (At) {
        case MKILL:
            kill();
            break;
        case MCNCT:
            cnct();
            break;
        case MDISCNCT:
            discnct();
            break;
        case MDATAREQ:
            data();
            break;
        case MLOGGED:
            login();
            break;
        case MRESET:
            rest();
            break;
        case MLOCK:
            lock();
            break;
        case MBUSY:
            busy[Af] = 1;
            break;
        case MFREE:
            busy[Af] = 0;
            break;
        case MDSTART:
            pstart();
            break; /* Priv. daemon */
        case MDCANCEL:
            dkill(Ad);
            break;
        case MCHECKD:
            check(Ad);
            break;
        case MMADEWIZ:
            logwiz(Af);
            break;
        case MLOG:
            logit(Ap);
            break;
        case MEXTEND:
            extend(Ad);
            forcereset = 0;
            break;
        case MGDSTART:
            gstart();
            break; /* Global daemon */
        default:
            At = -1;
            sprintf(block, "$$ (X) %s: *INVALID Message Type, %ld!*\n", now(), At);
            log(block);
            break;
        }
        ReplyMsg((struct Message *)amul);
        if (resety != 0)
            break;
        goto readport;
    }
    while ((amul = (struct Aport *)GetMsg((struct MsgPort *)port)) != NULL) {
        Ad = At = -'R';
        ReplyMsg((struct Message *)amul);
    }
    AbortIO(&ResReq.tr_node);

    if (resety == 1) {
        res();
        givebackmemory();
        setup();
        if (quiet == 0)
            printf("\n[ %s %s ]\n", vername, "RESET");
        if ((fp = fopen("reset.bat", "rb")) != NULL) {
            fclose(fp);
            Execute("execute reset.bat", 0L, 0L);
        }
        online = resety = 0;
    } else
        resety = -1;
}

int
main(int argc, char *argv[])
{
    sprintf(vername, "AMUL Manager v%d.%d (%s)", VERSION, REVISION, DATE);
    mytask = FindTask(0L);
    mytask->tc_Node.ln_Name = vername;
    if (argc > 4) {
        printf("Invalid arguments!\n");
        exit(0);
    }
    port = FindPort(mannam); /* Check for existing port */
    if (argc != 1 && (stricmp(argv[1], "-k") == NULL || stricmp(argv[1], "-r") == NULL ||
                      stricmp(argv[1], "-x") == NULL)) {
        if (port == NULL) {
            printf("AMAN %s running!\n", "not");
            exit(0);
        }
        mins = 0;
        if (argc == 3)
            sscanf(argv[2], "%ld", &mins);
        count[0] = mins;
        switch (toupper(*(argv[1] + 1))) {
        case 'K':
            shutreq(0);
            break;
        case 'R':
            shutreq(1);
            break;
        case 'X':
            if (argc != 3)
                exit(0 * printf("** Missing parameter after -x option.\n"));
            sendext(count[0]);
            break;
        }
        exit(0);
    }
    quiet = 0;

    if (argc != 1) {
        if (stricmp(argv[1], "-q") == NULL) {
            quiet = 1;
            if (argc > 2)
                strcpy(dir, argv[2]);
            else
                dir[0] = 0;
        } else
            strcpy(dir, argv[1]);
        if (dir[strlen(dir)] != '/' && dir[strlen(dir)] != ':')
            strcat(dir, "/");
        ;
    } else
        dir[0] = 0;
    if (port != NULL) {
        printf("AMAN %s running!\n", "already");
        exit(0);
    }
    if ((port = CreatePort(mannam, 0L)) == NULL) {
        printf("Unable to create %s port!\n", "AMUL Manager");
        quit();
    }
    if ((reply = CreatePort(0L, 0L)) == NULL) {
        printf("Unable to create %s port!\n", "Returns");
        quit();
    }
    if ((trport = (struct MsgPort *)CreatePort(0L, 0L)) == NULL) {
        printf("Unable to create %s port!\n", "Timer");
        quit();
    }
    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&ResReq, 0L) != NULL) {
        printf("Can't open timer.device!\n");
        quit();
    }
    ResReq.tr_node.io_Message.mn_ReplyPort = trport;
    TDBase = 1;
    setup();

    if (quiet == 0)
        printf("\n[ %s %s ]\n", vername, "LOADED");
    do {
        kernel();
    } while (resety != -1);

    if (quiet == 0)
        printf("\n[ %s %s ]\n", vername, "KILLED");
    quit();
}
