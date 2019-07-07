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
#define TRQ timerequest

#define UINFO ((sizeof(*usr) + sizeof(*lstat)) * MAXNODE) + (rooms * sizeof(short)) + (sizeof(mob) * mobs)

#include "h/aman.h"
#include "h/amul.cons.h"
#include "h/amul.defs.h"
#include "h/amul.incs.h"
#include "h/amul.vars.h"
#include "h/os.h"
#include <time.h>

char *    ttxt, lastres[24], lastcrt[24], bid[MAXNODE], busy[MAXNODE], *xread(), *now();
char      globflg[MAXD];                 // Is daemon global?
int32_t   reslp, TDBase, daemons, mins;  // Daemons to process!
short int count[MAXD], num[MAXD];        // Daemon counters in minutes! & No.s
char      own[MAXD];                     // And their owners...
int32_t   val[MAXD][2], typ[MAXD][2];    // Values and types...
Aport *   am;
struct TRQResReq;                       // Reset request & Incoming from timer
Amiga::MsgPort *timerPort;              // Timer port
int32_t         invis, invis2, calls;   // Invisibility Stuff & # of calls
int32_t         obtlen, nextdaem, ded;  // nextdaem = time to next daemon, ded = deduct

static bool quiet{false};
int32_t     online, clock, cclock;
char        resety, forcereset;

// Prevent CTRL-C'ing
int
CXBRK()
{
    return 0;
}

// Get current time/date
char *
now()
{
    if (clock == 0)
        time(&clock);
    ttxt = (char *)ctime(&clock) + 4;
    clock = 0;
    *(ttxt + strlen(ttxt) - 1) = 0;  // Strip cr/lf
    return ttxt;
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

int
fsize(char *s)
{
    fopenr(s);
    fseek(ifp, 0, SEEK_END);
    int n = ftell(ifp);
    fclose(ifp);
    ifp = NULL;
    return n;
}

void
usage(const char *argv[], int err = EINVAL)
{
    fprintf(stderr, "Usage: %s [<-kill | -reset | -xtend> time] [-q] [gamepath]\n");
    exit(err);
}

void
parseArguments(int argc, const char *argv[])
{
    char cmd{0};
    int  cmdTime{-1};

    // Clear the directory path
    dir[0] = 0;

    for (int argn = 1; argn < argc; ++argn) {
        const char *arg = argv[argn];
        if (arg[0] == '-' && arg[1] != 0) {
            switch (tolower(arg[1])) {
            case 'q':  // quiet
                quiet = true;
                break;

            case 'k':  // kill
            case 'r':  // restart
            case 'x':  // extend
                if (cmd) {
                    fprintf(stderr, "ERROR: Command '%s' conflicts with previous command '-%c'.\n", arg, cmd);
                    usage(argv);
                }
                cmd = tolower(arg[1]);
                break;

            case 'h': usage(argv, 0);

            default: usage(argv);
            }
        } else if (cmd) {
            if (cmdTime) {
                fprintf(stderr, "ERROR: Unrecognized argument, '%s'\n", arg);
                usage(argv);
            }
            cmdTime = atoi(arg);
            if (cmdTime < 0) {
                fprintf(stderr, "ERROR: Invalid -%c time value: '%s'\n", cmd, arg);
                usage(argv);
            }
        } else {
            // presumed game path.
            if (dir[0]) {
                fprintf(stderr, "ERROR: Unrecognized argument, '%s'\n", arg);
                usage(argv);
            }
            strncpy(dir, arg, sizeof(dir));
        }
    }

    amanPort = FindPort(managerName);
    if (cmd) {
        if (!amanPort) {
            fprintf(stderr, "ERROR: Could not find a running manage to control.\n");
            exit(ESRCH);
        }

        switch (cmd) {
        case 'k':
            count[0] = cmdTime;
            shutreq(0);
            break;
        case 'r':
            count[0] = cmdTime;
            shutreq(1);
            break;
        case 'x':
            if (!cmdTime) {
                fprintf(stderr, "ERROR: '-x' command requires a time value.\n");
                usage(argv);
            }
            sendext(cmdTime);
            break;
        }
    } else {
        if (amanPort) {
            printf("AMAN %s running!\n", "already");
            exit(0);
        }
    }
}

int
main(int argc, char *argv[])
{
    sprintf(vername, "AMUL Manager v%d.%d (%s)", VERSION, REVISION, DATE);
    OS::SetProcessName(vername);

    // Find an existing port for an amul manager.
    parseArguments(argc, argv);
    amanPort = Amiga::CreatePort(managerName, 0);
    if (!amanPort) {
        printf("Unable to create %s port!\n", "AMUL Manager");
        quit();
    }
    replyPort = Amiga::CreatePort(0L, 0L);
    if (!replyPort) {
        printf("Unable to create %s port!\n", "Returns");
        quit();
    }
    timerPort = CreatePort(0L, 0L);
    if (!timerPort) {
        printf("Unable to create %s port!\n", "Timer");
        quit();
    }

    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (struct IORequest *)&ResReq, 0L) != NULL) {
        printf("Can't open timer.device!\n");
        quit();
    }
    ResReq.tr_node.io_Message.mn_ReplyPort = timerPort;
    TDBase = 1;

    setup();

    if (!quiet) {
        printf("\n[ %s %s ]\n", vername, "LOADED");
    }
    do {
        kernel();
    } while (resety != -1);

    if (!quiet) {
        printf("\n[ %s %s ]\n", vername, "KILLED");
    }
    quit();
}

void
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
        (lstat + i)->IOlock = -1;
        (lstat + i)->room = bid[i] = -1;
        busy[i] = 0;
        (lstat + i)->helping = -1;
        (lstat + i)->following = -1;
    }
    ResReq.tr_time.tv_secs = 1;
    ResReq.tr_time.tv_micro = XR;
    ResReq.tr_node.io_Command = TR_ADDREQUEST;
    SendIO(&ResReq.tr_node);
    sprintf(block, "== (=) %s: [3mLoaded '%s'.[0m\n", now(), adname);
    log(block);
    forcereset = ded = 0;

    // Activate the daemon processor
    OS::Run("amul -%c", 3);

    while (resety == 0) {
        Amiga::Wait(-1);
    readport:
        while ((am = (Aport *)Amiga::GetMsg(replyPort)) != NULL) {
            OS::Free(am, sizeof(*amul));
        }
        while (Amiga::GetMsg(timerPort) != NULL) {
            // Process counter table
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
                    am->type = MSG_DAEMON;
                    Amiga::PutMsg((lstat + own[i])->rep, am);
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
        if ((amul = (Aport *)Amiga::GetMsg(amanPort)) == NULL)
            continue;
        switch (At) {
        case MSG_KILL: kill(); break;
        case MSG_CONNECT: cnct(); break;
        case MSG_DISCONNECT: discnct(); break;
        case MSG_DATA_REQUEST: data(); break;
        case MSG_LOGGED_IN: login(); break;
        case MSG_RESET: rest(); break;
        case MSG_LOCK: lock(); break;
        case MSG_BUSY: busy[Af] = 1; break;
        case MSG_FREE: busy[Af] = 0; break;
        case MSG_DAEMON_START: pstart(); break;  // Priv. daemon
        case MSG_DAEMON_CANCEL: dkill(Ad); break;
        case MSG_DAEMON_STATUS: check(Ad); break;
        case MSG_MADE_ADMIN: logwiz(Af); break;
        case MSG_LOG: logit(Ap); break;
        case MSG_EXTENDED:
            extend(Ad);
            forcereset = 0;
            break;
        case MSG_GDAEMON_START: gstart(); break;  // Global daemon
        default:
            At = -1;
            sprintf(block, "$$ (X) %s: *INVALID Message Type, %ld!*\n", now(), At);
            log(block);
            break;
        }
        Amiga::ReplyMsg(amul);
        if (resety != 0)
            break;
        goto readport;
    }
    while ((amul = (Aport *)Amiga::GetMsg(amanPort)) != NULL) {
        Ad = At = -'R';
        Amiga::ReplyMsg(amul);
    }
    AbortIO(&ResReq.tr_node);

    if (resety == 1) {
        res();
        givebackmemory();
        setup();
        if (!quiet)
            printf("\n[ %s %s ]\n", vername, "RESET");
        if ((fp = fopen("reset.bat", "rb")) != NULL) {
            fclose(fp);
            OS::Run("execute reset.bat", 0L, 0L);
        }
        online = resety = 0;
    } else
        resety = -1;
}

void kill()  // Shutdown receiver
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

// User connection!
void
cnct()
{
    int i;

    Ad = (int32_t)lstat;
    Ap = (char *)usr;
    if (Af >= MAXU) {
        if (Af == MAXU + 1)
            printf("** Mobile processor connected.\n");
        if ((lstat + Af)->state != 0)
            Af = -1;
        else
            (lstat + Af)->state = US_CONNECTED;
        return;
    }
    Af = -1;
    for (i = 0; i < MAXU; i++)  // Allow for daemons & mobiles
    {
        if ((lstat + i)->state != 0)
            continue;
        Af = i;
        (lstat + i)->state = US_LOGGING_IN;
        online++;
        calls++;
        break;
    }
}

// User disconnection
void
discnct()
{
    if (Af < MAXU && (lstat + Af)->state == US_CONNECTED) {
        sprintf(block, "<- (%d) %s: user disconnected.\n", Af, now());
        log(block);
    }
    if (Af < MAXU)
        online--;
    (usr + Af)->name[0] = 0;
    (lstat + Af)->room = -1;
    (lstat + Af)->helping = -1;
    (lstat + Af)->following = -1;
    dkill(-1);
    (lstat + Af)->state = 0;
    Af = -1;
    Ad = -1;
}

// Sends pointers to database
void
data()
{
    At = MSG_DATA_REQUEST;
    switch (Ad) {
    case -1:
        Ad = online;
        Ap = (char *)usr;
        Ap1 = calls;
        Ap2 = (int32_t)vername;
        Ap3 = (int32_t)adname;
        Ap4 = (int32_t)lstat;
        break;
    case 0:
        strcpy(Ap, dir);
        amul->p1 = count[0];
        Ap1 = (int32_t)&count[0];
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
    case 5: Ap = (char *)desctab; break;
    case 6: Ap = (char *)ormtab; break;
    case 7: Ap = (char *)statab; break;
    case 8: Ap = (char *)adtab; break;
    case 9: Ap = (char *)ttp; break;
    case 10: Ap = (char *)umsgip; break;
    case 11: Ap = (char *)umsgp; break;
    case 12: Ap = (char *)ttpp; break;
    case 13: Ap = (char *)rctab; break;
    case 14: Ap = (char *)slottab; break;
    case 15: Ap = (char *)vtp; break;
    case 16: Ap = (char *)vtpp; break;
    case 17:
        Ap = (char *)synp;
        Ad = (int32_t)synip;
        break;
    case 18:
        Ap = lastres;
        Ad = (int32_t)lastcrt;
        break;
    default: Ap = (char *)-1;
    }
}

// Receive & log a player login
void
login()
{
    sprintf(block, "-> (%d) %s: \"%s\" logged in.\n", Af, now(), (usr + Af)->name);
    (lstat + Af)->state = US_CONNECTED;
    log(block);
}

void
shutreq(int x)
{
    asend((x == 0) ? MSG_KILL : MSG_RESET, count[0]);
}

void
sendext(int t)
{
    asend(MSG_EXTENDED, t);
}

// Shutdown request
int
asend(int type, int data)
{
    if ((replyPort = Amiga::CreatePort("Killer!", 1L)) == NULL) {
        printf("Unable to create killer port!\n");
        return 0;
    }
    amul = (Aport *)OS::AllocateClear(sizeof(*amul));
    if (amul == NULL) {
        printf("Unable to allocate AMUL port memory!\n");
        Amiga::DeletePort(replyPort);
        return 0;
    }
    At = type;
    Ad = data;
    Af = -1;
    Am.mn_Node.ln_Type = NT_MESSAGE;
    Am.mn_ReplyPort = replyPort;
    Am.mn_Length = (UWORD)sizeof(*amul);
    PutMsg(amanPort, amul);
    WaitPort(replyPort);
    GetMsg(replyPort);
    if (!quiet)
        switch (Ad) {
        case 'R': printf("\x07*-- Reset Invoked --*\n\n"); break;
        case 'O': printf("AMUL Manager removed!\n"); break;
        case 'X': printf("Cannot remove with users connected.\n"); break;
        case 'U': printf("AMAN error at other end!\n");
        case -'X': printf("... Reset set for %ld seconds ...\n", Ap1); break;
        case -'R': printf("... Reset in progress ...\n"); break;
        case 'E': printf("... Game extended by %ld seconds ...\n", Ap1); break;
        default: printf("** Internal AMUL error ** (Returned '%c')\n", Ad); break;
        }
    OS::Free(amul, sizeof(*amul));
    DeletePort(replyPort);
    printf("\n");
    return 0;
}

// RESeT in progress
void
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
void
extend(short int tics)
{
    short int newtime;

    Ad = At = 'U';
    if (tics == 0)
        return;

    newtime = count[0] + tics + 1;
    if (count[0] > 120)
        sprintf(block, "...Game time extended - reset will now occur in %ld %s and %ld %s...\n", newtime / 60,
                "minutes", newtime - ((newtime / 60) * 60), "seconds");
    else
        sprintf(block, "...Reset postponed - it will now occur in %ld %s...\n", newtime, "seconds");
    warn(block);
    Ap1 = tics;
    count[0] = newtime;
    Ad = 'E';
}

// Reset <receiver>
void
res()
{
    int onwas;
    sprintf(block, "][ (%c) %s: Reset requested! %ld user(s) online...\n", (Af >= 0 && Af < 11) ? '0' + Af : '#', now(),
            online);
    log(block);
    onwas = online;
    reset_users();
    if (onwas != 0)
        sprintf(block, "== (#) %s: All users disconnected...\n", now());
    else
        sprintf(block, "== (#) %s: Reset completed!\n", now());
    log(block);
    Amiga::Delay(90);  // Always wait atleast a few seconds
}

// Force users to log-out & kill extra lines
void
reset_users()
{
    int             i;
    Amiga::Message *xx;

    online = 0;  // Allows for daemons & mobiles
    for (i = 0; i < MAXNODE; i++) {
        if ((lstat + i)->state <= 0)
            continue;
        online++;
        setam();
        am->type = MSG_CLOSEING;
        am->msg.mn_ReplyPort = amanPort;
        PutMsg((lstat + i)->rep, am);
    }
    while (online > 0) {
        Amiga::WaitPort(amanPort);
        am = (Aport *)GetMsg(amanPort);
    loop:
        if (am->from != -'O') {
            printf("\x07 !! Invalid message!\n");
            am->type = am->data = -'R';
            Amiga::ReplyMsg(am);
            goto skip;
        }
        OS::Free((char *)am, (int32_t)sizeof(*amul));
        online--;
    skip:
        if ((am = (Aport *)GetMsg(amanPort)) != NULL)
            goto loop;
    }
loop2:
    if ((am = (Aport *)Amiga::GetMsg(replyPort)) != NULL) {
        am->type = am->data = -'R';
        Amiga::ReplyMsg(am);
        goto loop2;
    }
    while ((xx = Amiga::GetMsg(amanPort)) != NULL)
        Amiga::ReplyMsg(xx);
    while ((xx = Amiga::GetMsg(replyPort)) != NULL)
        Amiga::ReplyMsg(xx);
    while (Amiga::GetMsg(timerPort) != NULL)
        ;
    online = 0;
}

// Read in & evaluate data files
void
setup()
{
    int32_t  i, rc, l, act, j, k;
    int32_t *pt;
    char *   p;

    rc = 0;
    fopenr(Resources::Compiled::gameProfile());
    fgets(adname, sizeof(adname), ifp);
    nulTerminate(adname);
    fscanf(ifp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &rooms, &ranks, &verbs, &syns,
            &nouns, &adjs, &ttents, &umsgs, &cclock, &mins, &invis, &invis2, &minsgo, &mobs, &rscale, &tscale,
            &mobchars);
    if ((p = OS::AllocateClear(UINFO)) == NULL)
        memfail("User tables");
    usr = (struct _PLAYER *)p;
    p += sizeof(*usr) * MAXNODE;
    lstat = (struct LS *)p;
    p += sizeof(*lstat) * MAXNODE;
    rctab = (short *)p;

    fopenr(Resources::Compiled::roomData());  // 1: Open room block file
    if ((rmtab = (struct room *)OS::Allocate(rooms * sizeof(room))) == NULL)
        memfail("room table");  // Allocate memory
    if ((i = fread((char *)rmtab, sizeof(room), rooms, ifp)) != rooms)
        readfail("room table", i, rooms);

    fopenr(Resources::Compiled::rankData());  // 2: Read player ranks
    if ((rktab = (struct rank *)OS::Allocate(ranks * sizeof(rank))) == NULL)
        memfail("player ranks");  // Allocate memory
    if ((i = fread((char *)rktab, sizeof(rank), ranks, ifp)) != ranks)
        readfail("player ranks", i, ranks);

    fopenr(lang1fn);  // 4: Read the verbs in
    if ((vbtab = (struct verb *)OS::Allocate(verbs * sizeof(verb))) == NULL)
        memfail("verb table");
    if ((i = fread(vbtab->id, sizeof(verb), verbs, ifp)) != verbs)
        readfail("verb table", i, verbs);

    // 3, 5, 6 & 7: Read objects
    obtlen = fsize(Resources::Compiled::objData());
    desctlen = fsize(Resources::Compiled::objDesc());
    ormtablen = fsize(Resources::Compiled::objLoc());
    statablen = fsize(Resources::Compiled::objState());
    if ((p = OS::Allocate(obtlen + desctlen + ormtablen + statablen)) == NULL)
        memfail("object data");
    obtab = (struct obj *)readf(Resources::Compiled::objData(), p);
    desctab = (char *)readf(Resources::Compiled::objDesc(), (p = p + obtlen));
    ormtab = (int32_t)readf(Resources::Compiled::objLoc(), (p = p + desctlen));
    statab = (struct state *)readf(Resources::Compiled::objState(), p + ormtablen);

    // Update the object room list ptrs and the state ptrs
    statep = statab;
    for (i = 0; i < nouns; i++) {
        objtab = obtab + i;
        objtab->rmlist = (int32_t *)(ormtab + rc);
        rc += objtab->nrooms * sizeof(int32_t);
        objtab->states = statep;
        statep = statep + (int32_t)objtab->nstates;
    }

    // 10 & 11: User Messages

    umsgil = fsize(Resources::Compiled::umsgIndex());
    umsgl = fsize(Resources::Compiled::umsgData());
    if ((p = OS::Allocate(umsgil + umsgl)) == NULL)
        memfail("user messages");
    umsgip = (int32_t *)readf(Resources::Compiled::umsgIndex(), p);
    umsgp = (char *)readf(Resources::Compiled::umsgData(), p + umsgil);

    // 9: Read the travel table
    ttp = (struct tt *)xread(Resources::Compiled::travelTable(), &ttlen, "travel table");

    // 12: Read parameters
    ttpp = (int32_t *)xread(Resources::Compiled::travelParams(), &ttplen, "TT parameter table");
    ttents = ttlen / sizeof(tt);
    ttabp = ttp;
    pt = ttpp;
    for (i = 0; i < ttents; i++) {
        ttabp = ttp + i;
        k = (int32_t)ttabp->pptr;
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

    // 14: Load Slot table
    stlen = fsize(lang2fn);
    vtlen = fsize(lang3fn);
    vtplen = fsize(lang4fn);
    if ((p = OS::Allocate(stlen + vtlen + vtplen)) == NULL)
        memfail("language data");
    slottab = (struct _SLOTTAB *)readf(lang2fn, p);
    vtp = (struct _VBTAB *)readf(lang3fn, p + stlen);
    vtpp = (int32_t *)readf(lang4fn, p + stlen + vtlen);

    // 17: Get the Synonym data & adjectives
    synlen = fsize(Resources::Compiled::synonymData());
    synilen = fsize(Resources::Compiled::synonymIndex());
    adtablen = fsize(Resources::Compiled::adjTable());
    if ((p = OS::Allocate(synlen + synilen + adtablen)) == NULL)
        memfail("synonym data");
    synp = (char *)readf(Resources::Compiled::synonymData(), p);
    synip = (short int *)readf(Resources::Compiled::synonymIndex(), (p = p + synlen));
    adtab = (char *)readf(Resources::Compiled::adjTable(), p + synilen);

    // 18: Get last reset time
    strcpy(lastres, now());
    clock = cclock;
    strcpy(lastcrt, now());

    // Adjust the verb-related pointers
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

    // Fix the object 'inside' flags
    objtab = obtab;
    for (i = 0; i < nouns; i++, objtab++) {
        if (*(objtab->rmlist) <= -INS)
            (obtab + (-(INS + *(objtab->rmlist))))->inside++;
    }
    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
}

// Report memory allocation error
void
memfail(char *s)
{
    sprintf(block, "** Unable to allocate memory for %s! **\n", s);
    log(block);
    quit();
}

// Report a read failure
void
readfail(char *s, int got, int wanted)
{
    printf("** Error: Expected %d ", wanted);
    printf(s);
    printf(" entries, only got %d!\n", got);
    quit();
}

// Release all memory AllocMem'd
void
givebackmemory()
{
    OS::Free(usr, UINFO);
    OS::Free(rmtab, sizeof(room) * rooms);
    OS::Free(rktab, sizeof(rank) * ranks);
    OS::Free(vbtab, verbs * sizeof(verb));
    OS::Free(obtab, obtlen + desctlen + ormtablen + statablen);
    OS::Free(umsgip, umsgil + umsgl);
    OS::Free(ttp, ttlen);
    OS::Free(ttpp, ttplen);
    OS::Free(slottab, stlen + vtlen + vtplen);
    OS::Free(synp, synlen + synilen + adtablen);

    lstat = NULL;
    umsgp = NULL;
    vtp = NULL;
    vtpp = NULL;
    adtab = NULL;
    statab = NULL;
    ormtab = NULL;
    desctab = NULL;
    synp = NULL;
    synip = NULL;
}

// Open a file for reading
void
fopenr(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL)
        oer(block, "read");
}

// report Open ERror
void
oer(char *s, char *t)
{
    printf("\x07** Error: Can't open file %s for %sing!\n\n", s, t);
    quit();
}

// Exit program tidily
void
quit()
{
    if (ifp != NULL)
        fclose(ifp);
    if (replyPort != NULL)
        DeletePort(replyPort);
    if (amanPort != NULL)
        DeletePort(amanPort);
    if (TDBase != NULL) {
        AbortIO((struct IORequest *)&ResReq);
        CloseDevice((struct IORequest *)&ResReq);
    }
    if (timerPort != NULL)
        DeletePort(timerPort);
    givebackmemory();
    exit(0);
}

// Size/Read data files
char *
xread(const char *s, int32_t *n, char *t)
{
    char *p;
    int   i;

    fopenr(s);
    fseek(ifp, 0, SEEK_END);
    *n = ftell(ifp);
    fseek(ifp, 0, 0L);
    if (*n != 0) {
        if ((p = (char *)OS::Allocate(*n)) == NULL)
            memfail(t);
        if ((i = fread(p, 1, *n, ifp)) != *n)
            readfail(t, i, *n);
    }
    return p;
}

// Lock a users IO system
void
lock()
{
    bid[Af] = Ad;
    if ((lstat + Ad)->IOlock != -1 || (busy[Ad] != 0 && Ad != Af && bid[Ad] != Af)) {
        Ad = -1;
        return;
    }
    (lstat + Ad)->IOlock = Af;
    bid[Af] = -1;
}

// Log data to AMUL.Log
void
log(char *s)
{
    FILE *fp;

    if ((fp = fopen("AMUL.Log", "ab+")) == NULL)
        oer("AMUL.Log", "logg");
    fseek(fp, 0, SEEK_END);
    fprintf(fp, s);
    fclose(fp);
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
    // Ad=#, p1=inoun1, p2=inoun2, p3=wtype[2], p4=wtype[5], Ap=count

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
    start(MAXU);  // Set global flag & go!
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

void
setam()
{
    am = (Aport *)OS::AllocateClear((int32_t)sizeof(*amul));
    am->mn_Node.ln_Type = NT_MESSAGE;
    am->mn_ReplyPort = replyPort;
    am->mn_Length = (UWORD)sizeof(*am);
    am->from = -1;
}

void
logwiz(int who)
{
    sprintf(block, "@@ ]%c[ %s: User \"%s\" achieved top rank (%ld)!!!\n", Af + '0', now(), (usr + Af)->name,
            (usr + Af)->rank + 1);
    log(block);
}

void
logit(char *s)
{
    sprintf(block, "@@ (%c) %s: %s\n", Af + '0', now(), s);
    log(block);
}

void
warn(char *s)
{
    for (int i = 0; i < MAXU; i++) {
        if ((lstat + i)->state != US_OFFLINE) {
            setam();
            am->ptr = s;
            am->type = MSG_RESET_WARNING;
            PutMsg((lstat + i)->rep, am);
        }
    }
}
