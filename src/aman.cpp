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
    ((sizeof(*usr) + sizeof(*linestat)) * MAXNODE) + (g_gameData.numRooms * sizeof(short)) +       \
            (sizeof(mob) * g_gameData.numMobs)

#include <cassert>
#include <cctype>
#include <cstdio>
#include <ctime>
#include <thread>

#if !defined(_MSC_VER)
#    include <unistd.h>
#else
#    include <direct.h>
#endif

#include <h/aman.h>
#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.defs.h>
#include <h/amul.gcfg.h>
#include <h/amul.type.h>
#include <h/amul.vars.h>
#include <h/amul.vmop.h>
#if defined(__AMIGA__)
#    include <devices/timer.h>
TRQ ResReq;  // Reset request & Incoming from timer
#else
#    include <h/amigastubs.h>
#endif

#include "filesystem.h"
#include "msgports.h"
#include "system.h"

FILE *   ifp;
GameData g_gameData;

char      lastres[24], lastcrt[24], bid[MAXNODE], busy[MAXNODE];
char      vername[128];
bool      globflg[MAXD];               // Is daemon global?
long      reslp, TDBase, daemons;      // Daemons to process!
int       count[MAXD], num[MAXD];      // Daemon counters in minutes! & No.s
char      own[MAXD];                   // And their owners...
long      val[MAXD][2], typ[MAXD][2];  // Values and types...
Aport *   am;
MsgPort * trport;                // Timer port
long      invis, invis2, calls;  // Invisibility Stuff & # of calls
long      nextdaem, ded;         // nextdaem = time to next daemon, ded = deduct
roomid_t *ormtab;

long online;
char resety, forcereset, quiet;

// Prevent CTRL-C'ing
int
CXBRK()
{
    return 0;
}

// Get current time/date as string
static char *
now()
{
    time_t timenow = time(nullptr);
    char * ttxt = (char *)ctime(&timenow) + 4;
    *(ttxt + strlen(ttxt) - 1) = 0;  // Strip cr/lf
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

[[noreturn]] void
quit()
{
    if (ifp != NULL)
        fclose(ifp);
    if (reply != NULL)
        DeletePort(reply);
    if (port != NULL)
        DeletePort(port);
    if (trport != NULL)
        DeletePort(trport);
    givebackmemory();
    exit(0);
}

// Report memory allocation error
[[noreturn]]
static void
memfail(const char *s)
{
    afatal("Out of memory for %s");
}

// Report a read failure
[[noreturn]]
static void
readfail(const char *s, size_t got, size_t wanted)
{
    afatal("Expected %zu %s entries, got %zu", wanted, s, got);
}

// report open error
[[noreturn]]
static void
openError(const char *filepath, const char *activity)
{
    afatal("Unable to open '%s' for %sing", filepath, activity);
}

// Open a file for reading
static void
fopenr(const char *s)
{
    char filepath[MAX_PATH_LENGTH];
    safe_gamedir_joiner(s);
    ifp = fopen(filepath, "rb");
    if (!ifp)
        openError(filepath, "read");
}

// Size/Read data files
char *
xread(const char *s, size_t *countInto, const char *t)
{
    char *p{nullptr};

    fopenr(s);
    fseek(ifp, 0, 2L);
    *countInto = ftell(ifp);
    fseek(ifp, 0, 0L);
    if (*countInto != 0) {
        if ((p = (char *)AllocateMem(*countInto)) == NULL)
            memfail(t);
        if (int i = fread(p, 1, *countInto, ifp); i != *countInto)
            readfail(t, i, *countInto);
    }
    return p;
}

char *
readf(const char *s, char *p)
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
    static Aport s_am;
    am = &s_am;
    am->mn_ReplyPort = reply;
    am->from = -1;
}

// Remove daemon from list
static void
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
    globflg[i] = false;
    daemons--;
}

// Force users to log-out & kill extra lines
static void
reset_users()
{
    int i;

    online = 0;  // Allows for daemons & mobiles
    for (i = 0; i < MAXNODE; i++) {
        if ((linestat + i)->state <= 0)
            continue;
        online++;
        setam();
        am->type = MCLOSEING;
        am->mn_ReplyPort = port;
        (linestat + i)->rep->Put(MessagePtr(am));
    }
    while (online > 0) {
        MessagePtr amsg{port->Wait()};
        am = static_cast<Aport *>(amsg.get());
        if (am == nullptr)
            break;
        if (am->from != -'O') {
            printf("\x07 !! Invalid message!\n");
            am->type = am->data = -'R';
            ReplyMsg(std::move(amsg));
            continue;
        } else {
            online--;
        }
    }

    for (;;) {
        MessagePtr amsg{reply->Get()};
        am = static_cast<Aport *>(amsg.get());
        if (am == nullptr)
            break;
        am->type = am->data = -'R';
        ReplyMsg(std::move(amsg));
    }
    port->Clear();
    reply->Clear();
    online = 0;
}

template <typename... Args>
void
warn(const char *fmt, Args... args)
{
    alog(AL_WARN, fmt, args...);

    char message[512];
    snprintf(message, sizeof(message), fmt, args...);
    for (size_t i = 0; i < MAXU; i++) {
        if ((linestat + i)->state != OFFLINE) {
            auto amsg = std::make_unique<Aport>(reply, MRWARN);
            amsg->opaque = strdup(message);
            (linestat + i)->rep->Put(std::move(amsg));
        }
    }
}

// Shutdown receiver
static void
kill()
{
    char source[32];
    if (Af != -1)
        sprintf(source, "line %u.", uint32_t(Af + 1));
    else
        strcpy(source, "external");
    alog(AL_WARN, "!! (X) %s: shutdown request from %s", now(), source);

    if (online != 0) {
        alog(AL_NOTE, "&&%25s: Request denied: %u users on-line", " ", uint32_t(online));
        Ad = At = 'X';
    } else {
        warn("Game shutdown initiated by administrator.");
        reset_users();
        Ad = At = 'O';
        resety = -1;
    }
}

// User connecting
static void
cnct()
{
    int i;

    amul->data = (long)linestat;
    amul->opaque = (char *)usr;
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
    for (i = 0; i < MAXU; i++) {
        if ((linestat + i)->state != 0)
            continue;
        Af = i;
        (linestat + i)->state = LOGGING;
        online++;
        calls++;
        break;
    }
}

// Cancel demon
void
dkill(short int d)
{
    int i;

    nextdaem = g_gameData.gameDuration_m * 60;
    for (i = 1; i < daemons; i++) {
        if (((d != -1 && globflg[i]) || own[i] == Af) && (num[i] == d || d == -1))
            pack(i);
        if (i != daemons && count[i] < nextdaem)
            nextdaem = count[i];
    }
}

// Initiate daemon
void
start(char owner)
{
    // Ad=#, p1=inoun1, p2=inoun2, p3=wtype[2], p4=wtype[5], Ap->opaque=count
    val[daemons][0] = Ap1;
    val[daemons][1] = Ap2;
    typ[daemons][0] = Ap3;
    typ[daemons][1] = Ap4;
    own[daemons] = owner;
    count[daemons] = (int)amul->opaque;
    num[daemons] = Ad;
    daemons++;
    if (count[daemons - 1] < nextdaem)
        nextdaem = count[daemons - 1];
}

// Initiate global daemon
void
gstart()
{
    globflg[daemons] = true;
    start(MAXU);  // Set global flag & go!
}

// Initiate private daemon
void
pstart()
{
    globflg[daemons] = false;
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
        if ((own[i] == Af || globflg[i]) && num[i] == d) {
            Ad = i;
            Ap1 = count[i];
            break;
        }
}

// User disconnection
static void
discnct()
{
    if (Af < MAXU && (linestat + Af)->state == PLAYING) {
        alog(AL_INFO, "<- (%u) %s: user disconnected", uint32_t(Af), now());
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
        amul->opaque = usr;
        Ap1 = calls;
        Ap2 = (uintptr_t)vername;
        Ap3 = (uintptr_t)g_gameData.gameName;
        Ap4 = (uintptr_t)linestat;
        break;
    case 0:
        amul->opaque = gameDir;
        amul->p1 = count[0];  /// TODO: Fix
        break;
    case 1:
        amul->opaque = &g_gameData;
        break;
    default:
        amul->opaque = nullptr;
    }
}

// Receive & log a player login
static void
login()
{
    alog(AL_INFO, "-> (%d) %s: \"%s\" logged in.", Af, now(), (usr + Af)->name);
    (linestat + Af)->state = PLAYING;
}

// Shutdown request
static void
asend(int type, int data)
{
    if ((reply = CreatePort("Killer!")) == NULL) {
        printf("Unable to create killer port!\n");
        return;
    }
    auto aptr = std::make_unique<Aport>(reply, type, -1, data);
    port->Put(std::move(aptr));
    port->Wait();
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
            printf("... Reset set for %" PRId64 " seconds ...\n", Ap1);
            break;
        case -'R':
            printf("... Reset in progress ...\n");
            break;
        case 'E':
            printf("... Game extended by %" PRId64 " seconds ...\n", Ap1);
            break;
        default:
            printf("** Internal AMUL error ** (Returned '%c')\n", static_cast<char>(Ad));
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
        warn("** System reset invoked - %zu seconds remaining...\n", size_t(Ad));
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
        alog(AL_NOTE, "...Game time extended - reset will now occur in %ld %s and %ld %s...\n",
             newtime / 60, "minutes", newtime - ((newtime / 60) * 60), "seconds");
    else
        alog(AL_NOTE, "...Reset postponed - it will now occur in %ld %s...\n", newtime, "seconds");
    Ap1 = tics;
    count[0] = newtime;
    Ad = 'E';
}

// Reset <receiver>
static void
res()
{
    int onwas;
    warn("][ (%c) %s: Reset requested! %ld user(s) online...\n",
         (Af >= 0 && Af < 11) ? '0' + Af : '#', now(), online);
    onwas = online;
    reset_users();
    if (onwas != 0)
        alog(AL_NOTE, "== (#) %s: All users disconnected...\n", now());
    else
        alog(AL_NOTE, "== (#) %s: Reset completed!\n", now());
    Delay(100);  // Always wait atleast a few seconds
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

void
logwiz(int who)
{
    alog(AL_NOTE, "@@ ]%c[ %s: User \"%s\" achieved top rank (%ld)", Af + '0', now(),
         (usr + Af)->name, (usr + Af)->rank + 1);
}

void
logit(const char *s)
{
    alog(AL_INFO, "@@ (%c) %s: %s", Af + '0', now(), s);
}

// Read in & evaluate data files
static void
setup()
{
    long  rc = 0;
    long  i, l, act, j, k;
    long *pt;
    char *p;

    if ((p = (char *)AllocateMem(UINFO)) == NULL)
        memfail("User tables");
    usr = (_PLAYER *)p;
    p += sizeof(*usr) * MAXNODE;
    linestat = (LS *)p;
    p += sizeof(*linestat) * MAXNODE;
    rctab = (short *)p;

    fopenr(roomDataFile);  // 1: Open room block file
    if ((rmtab = (_ROOM_STRUCT *)AllocateMem(g_gameData.numRooms * sizeof(room))) == NULL)
        memfail("room table");  // Allocate memory
    if ((i = fread((char *)rmtab, sizeof(room), g_gameData.numRooms, ifp)) != g_gameData.numRooms)
        readfail("room table", i, g_gameData.numRooms);

    fopenr(rankDataFile);  // 2: Read player g_gameData.numRanks
    if ((rktab = (_RANK_STRUCT *)AllocateMem(g_gameData.numRanks * sizeof(rank))) == NULL)
        memfail("player g_gameData.numRanks");  // Allocate memory
    if ((i = fread((char *)rktab, sizeof(rank), g_gameData.numRanks, ifp)) != g_gameData.numRanks)
        readfail("player g_gameData.numRanks", i, g_gameData.numRanks);

    fopenr(verbDataFile);  // 4: Read the g_gameData.numVerbs in
    if ((vbtab = (_VERB_STRUCT *)AllocateMem(g_gameData.numVerbs * sizeof(_VERB_STRUCT))) == NULL)
        memfail("verb table");
    if ((i = fread(vbtab->id, sizeof(_VERB_STRUCT), g_gameData.numVerbs, ifp)) !=
        g_gameData.numVerbs)
        readfail("verb table", i, g_gameData.numVerbs);

    // 3, 5, 6 & 7: Read objects
    size_t obtlen, ormtablen, statablen;
    size_t stlen, vtlen, vtplen;
    size_t synlen, synilen, adtablen;

    GetFilesSize(objectDataFile, &obtlen, true);
    GetFilesSize(objectRoomFile, &ormtablen, true);
    GetFilesSize(objectStateFile, &statablen, true);
    GetFilesSize(verbSlotFile, &stlen, true);
    GetFilesSize(verbTableFile, &vtlen, true);
    GetFilesSize(verbParamFile, &vtplen, true);
    GetFilesSize(synonymDataFile, &synlen, true);
    GetFilesSize(synonymIndexFile, &synilen, true);
    GetFilesSize(adjectiveDataFile, &adtablen, true);

    if ((p = (char *)AllocateMem(obtlen + ormtablen + statablen)) == NULL)
        memfail("object data");
    obtab = (_OBJ_STRUCT *)readf(objectDataFile, p);
    ormtab = (roomid_t *)readf(objectRoomFile, (p = p + obtlen));
    statab = (_OBJ_STATE *)readf(objectStateFile, p + ormtablen);

    // Update the object room list ptrs and the state ptrs
    statep = statab;
    for (i = 0; i < g_gameData.numObjects; i++) {
        objtab = obtab + i;
        objtab->rmlist = ormtab + rc;
        rc += objtab->nrooms;
        objtab->states = statep;
        statep = statep + (long)objtab->nstates;
    }

    // 9: Read the travel table
    size_t ttlen{0};
    ttp = (_TT_ENT *)xread(travelTableFile, &ttlen, "travel table");
    assert(ttlen / sizeof(_TT_ENT) == g_gameData.numTTEnts);

    // 12: Read parameters
    size_t ttplen{0};
    ttpp = (long *)xread(travelParamFile, &ttplen, "TT parameter table");
    ttabp = ttp;
    pt = ttpp;
    for (i = 0; i < g_gameData.numTTEnts; i++) {
        ttabp = ttp + i;
        k = (long)ttabp->pptr;
        ttabp->pptr = (int *)pt;
        if (k == -2)
            continue;
        act = ttabp->condition;
        if (act < 0)
            act = -1 - act;
        pt += conditions[act].parameterCount;
        act = ttabp->action;
        if (act < 0) {
            act = -1 - act;
            pt += actions[act].parameterCount;
        }
    }

    // 14: Load Slot table
    if ((p = (char *)AllocateMem(stlen + vtlen + vtplen)) == NULL)
        memfail("language data");
    slottab = (_SLOTTAB *)readf(verbSlotFile, p);
    vtp = (_VBTAB *)readf(verbTableFile, p + stlen);
    vtpp = (long *)readf(verbParamFile, p + stlen + vtlen);

    // 17: Get the Synonym data & adjectives
    if ((p = (char *)AllocateMem(synlen + synilen + adtablen)) == NULL)
        memfail("synonym data");
    synp = (char *)readf(synonymDataFile, p);
    synip = (short int *)readf(synonymIndexFile, (p = p + synlen));
    adtab = (char *)readf(adjectiveDataFile, p + synilen);

    // 18: Get last reset time
    strcpy(lastres, now());
    strcpy(lastcrt, now());

    // Adjust the verb-related pointers
    vbptr = vbtab;
    stptr = slottab;
    vtabp = vtp;
    l = 0;
    for (i = 0; i < g_gameData.numVerbs; i++, vbptr++) {
        vbptr->ptr = stptr;
        for (j = 0; j < vbptr->ents; j++, stptr++) {
            stptr->ptr = vtabp;
            for (k = 0; k < stptr->ents; k++, vtabp++) {
                vtabp->pptr = (int *)vtpp + l;
                act = vtabp->condition;
                if (act < 0)
                    act = -1 - act;
                l += conditions[act].parameterCount;
                act = vtabp->action;
                if (act < 0) {
                    act = -1 - act;
                    l += actions[act].parameterCount;
                }
            }
        }
    }

    // Fix the object 'inside' flags
    objtab = obtab;
    for (i = 0; i < g_gameData.numObjects; i++, objtab++) {
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
    int i;

    alog(AL_INFO, "------------------------------------------------------------");
    online = resety = forcereset = 0;
    for (i = 0; i < MAXD; i++) {
        count[i] = -1;
        own[i] = -1;
        val[i][0] = val[i][1] = typ[i][0] = typ[i][1] = 0;
        globflg[i] = false;
    }
    daemons = 1;
    nextdaem = count[0] = g_gameData.gameDuration_m * 60;
    for (i = 0; i < MAXNODE; i++) {
        (linestat + i)->IOlock = -1;
        (linestat + i)->room = bid[i] = -1;
        busy[i] = 0;
        (linestat + i)->helping = -1;
        (linestat + i)->following = -1;
    }
    alog(AL_INFO, "== (=) %s: Loaded '%s'.", now(), g_gameConfig.gameName);
    forcereset = ded = 0;

    // Activate the daemon processor

    execute("amul -\03");

    while (resety == 0) {
        std::this_thread::yield();

        reply->Clear();

#ifdef NEVER  /// TODO:: Restore
        while (GetMsg((MsgPort *)trport) != NULL) {
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
                    am->type = MDAEMON;
                    PutMsg((linestat + own[i])->rep, am);
                    pack(i);
                    i--;
                }
            }
            if (count[0] == 300) {
                warn("--+ Next reset in 5 minutes +--");
            }
            if (count[0] == 120) {
                warn("--+ 120 seconds until next reset +--");
            }
            if (count[0] == 60) {
                warn("--+ Final warning - 60 seconds to reset +--");
            }
            if (count[0] <= 0) {
                count[0] = -10;
                resety = 1;
                if (forcereset == 0)
                    warn("[ Automatic Reset ]\n");
            }
        }
#endif

        MessagePtr amsg{port->Get()};
        if (amul = static_cast<Aport *>(amsg.get()); !amul)
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
            break;  // Priv. daemon
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
            logit(static_cast<const char *>(amul->opaque));
            break;
        case MEXTEND:
            extend(Ad);
            forcereset = 0;
            break;
        case MGDSTART:
            gstart();
            break;  // Global daemon
        default:
            At = -1;
            alog(AL_ERROR, "$$ (X) %s: *INVALID Message Type, %ld!*", now(), At);
            break;
        }

        ReplyMsg(std::move(amsg));

        if (resety != 0)
            break;
    }
    for (;;) {
        MessagePtr amsg{port->Get()};
        amul = static_cast<Aport *>(amsg.get());
        if (!amul)
            break;
        Ad = At = -'R';
        ReplyMsg(std::move(amsg));
    }

    if (resety == 1) {
        res();
        givebackmemory();
        setup();
        if (quiet == 0)
            printf("\n[ %s %s ]\n", vername, "RESET");
#ifdef REPAIR
        if (GetFilesSize("reset.bat") > 0) {
            Execute("execute reset.bat", 0L, 0L);
        }
#endif
        online = resety = 0;
    } else
        resety = -1;
}

[[noreturn]] void
executeCommand(int argc, const char *argv[])
{
    if (!port) {
        afatal("AMAN is not running");
    }
    size_t mins{0};
    if (argc == 3)
        sscanf(argv[2], "%zu", &mins);
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
            afatal("Missing minute value after -x option");
        sendext(count[0]);
        break;
    }
    exit(0);
}

void
parseArguments(int argc, const char *argv[])
{
    if (argc > 4) {
        printf("Invalid arguments!\n");
        exit(0);
    }

    if (argc == 1) {
        strcpy(gameDir, ".");
        return;
    }

    if (argv[1][0] == '-' && strchr("krc", argv[1][1])) {
        executeCommand(argc, argv);
    }

    int argn = 1;
    if (!stricmp(argv[argn], "-q")) {
        ++argn;
        quiet = 1;
    }
    if (argn < argc) {
        error_t err = path_copier(gameDir, argv[argn]);
        if (err != 0)
            afatal("Invalid game path: %s", argv[argn]);
    } else
        strcpy(gameDir, ".");
}

error_t
GameData::Load()
{
    char filepath[MAX_PATH_LENGTH];
    safe_gamedir_joiner(gameDataFile);
    FILE *fp = fopen(filepath, "rb");
    if (!fp)
        afatal("Unable to open game data file: %s", filepath);
    fread(dynamic_cast<GameConfig *>(&g_gameData), sizeof(GameConfig), 1, fp);
    fclose(fp);

    return 0;
}

int
main(int argc, const char *argv[])
{
    snprintf(vername, sizeof(vername), "AMUL Manager v%d.%d (%s)", VERSION, REVISION, DATE);

#if defined(__AMIGA__)
    mytask = FindTask(0L);
    mytask->tc_Node.ln_Name = vername;
#else
    (void)vername;
#endif

    parseArguments(argc, argv);
    chdir(gameDir);

    if (error_t err = g_gameData.Load(); err != 0)
        afatal("Failed to load game data");

    port = FindPort(managerPortName);  // Check for existing port
    if (port != NULL) {
        printf("AMAN %s running!\n", "already");
        exit(0);
    }
    if ((port = CreatePort(managerPortName)) == NULL) {
        printf("Unable to create %s port!\n", "AMUL Manager");
        quit();
    }
    if ((reply = CreatePort(nullptr)) == NULL) {
        printf("Unable to create %s port!\n", "Returns");
        quit();
    }
    if ((trport = CreatePort(nullptr)) == NULL) {
        printf("Unable to create %s port!\n", "Timer");
        quit();
    }

#if defined(__AMIGA__)
    /// TODO: Replace
    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (IORequest *)&ResReq, 0L) != NULL) {
        printf("Can't open timer.device!\n");
        quit();
    }
    ResReq.tr_node.io_Message.mn_ReplyPort = trport;
    TDBase = 1;
#endif

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
