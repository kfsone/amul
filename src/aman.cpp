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

#define PORTS 1
#define TRQ timerequest

#define UINFO                                                                                      \
    (((sizeof(*usr) + sizeof(*linestat)) * MAXNODE) + (g_game.numRooms * sizeof(short)) +          \
     (sizeof(mob) * g_game.numMobs))

#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <list>
#include <thread>
#include <vector>

#include "h/aman.version.h"
#include "h/amigastubs.h"
#include "h/amul.cons.h"
#include "h/amul.defs.h"
#include "h/amul.file.h"
#include "h/amul.gcfg.h"
#include "h/amul.type.h"
#include "h/amul.vars.h"
#include "h/amul.vmop.h"
#include "h/demon.h"
#include "h/demon.inl.h"
#include "h/filesystem.h"
#include "h/logging.h"
#include "h/msgports.h"
#include "h/system.h"

FILE *ifp;
Game g_game;
time_t s_nextReset;

using DemonList = std::list<Demon>;
DemonList demons{};
demonid_t Demon::s_nextID{ 1 };

char lastres[24], lastcrt[24], bid[MAXNODE], busy[MAXNODE];
char vername[128];

Aport *am;
MsgPort *trport;            // Timer port
long invis, invis2, calls;  // Invisibility Stuff & # of calls
long ded;                   // ded = deduct

long online;
bool g_resetInProgress{ false };  // replaces: resety
bool g_forceReset{ false };       // replaces: forcereset
bool g_quiet{ false };            // replaces: quiet

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
    char *ttxt = (char *) ctime(&timenow) + 4;
    *(ttxt + strlen(ttxt) - 1) = 0;  // Strip cr/lf
    return ttxt;
}

// Release all memory AllocMem'd
static void
givebackmemory()
{
    ReleaseMem(&slottab);
    ReleaseMem(&ttp);
    ReleaseMem(&ttpp);
    ReleaseMem(&usr);
    ReleaseMem(&vbtab);

    linestat = nullptr;
    rctab = nullptr;
    vtp = nullptr;
    vtpp = nullptr;
}

[[noreturn]] void
quit()
{
    if (ifp != nullptr)
        fclose(ifp);
    if (reply != nullptr)
        DeletePort(reply);
    if (port != nullptr)
        DeletePort(port);
    if (trport != nullptr)
        DeletePort(trport);
    givebackmemory();
    exit(0);
}

// Report memory allocation error
[[noreturn]] static void
memfail(const char *s)
{
    LogFatal("Out of memory for ", s);
}

// Report a read failure
[[noreturn]] static void
readfail(const char *s, size_t got, size_t wanted)
{
    LogFatal("Expected ", wanted, " ", s, ", got ", got);
}

// report open error
[[noreturn]] static void
openError(const char *filepath, const char *activity)
{
    LogFatal("Unable to open '", filepath, "' for ", activity, "ing");
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
void *
xread(const char *s, size_t *countInto, const char *t)
{
    char filepath[MAX_PATH_LENGTH];
    safe_gamedir_joiner(s);
    int fd = open(filepath, READ_FLAGS);
    if (fd == -1) {
        LogFatal("Unable to open file: ", filepath, ": ", strerror(errno));
    }
    struct stat sb {
    };
    if (fstat(fd, &sb) == -1) {
        LogFatal("Error accessing file: ", filepath, ": ", strerror(errno));
    }
    if (sb.st_size == 0) {
        LogFatal("Empty/corrupt file: ", filepath);
    }
    void *data = AllocateMem(sb.st_size);
    if (data == nullptr)
        memfail(filepath);
    if (size_t bytes = read(fd, data, sb.st_size); bytes != sb.st_size) {
        LogFatal(
                "Unexpected end of file: ", filepath, ": Expected: ", sb.st_size, ", Got: ", bytes);
    }
    close(fd);
    *countInto = sb.st_size;
    return data;
}

char *
readf(const char *s, char *p)
{
    fopenr(s);
    fread(p, 32767, 32767, ifp);
    fclose(ifp);
    ifp = nullptr;
    return p;
}

static void
execute(const char *)
{
    /// TODO: Remove
    // run a command in the background
}

static void
setam()
{
    /// TODO: On the Amiga, it was the caller's responsibility to free messages
    new (am) Aport;
    am->mn_ReplyPort = reply;
    am->from = -1;
}

std::unique_ptr<Aport>
GetNewAport(uint32_t type, int64_t data)
{
    return std::make_unique<Aport>(reply, type, -1, data);
}

// Force users to log-out & kill extra lines
static void
reset_users()
{
    online = 0;  // Allows for demons & mobiles
    for (int i = 0; i < MAXNODE; i++) {
        if ((linestat + i)->state <= 0)
            continue;
        online++;
        setam();
        am->type = MCLOSEING;
        am->mn_ReplyPort = port;
        (linestat + i)->rep->Put(MessagePtr(am));
    }
    while (online > 0) {
        MessagePtr amsg{ port->Wait() };
        am = static_cast<Aport *>(amsg.get());
        if (am == nullptr)
            break;
        if (am->from != -'O') {
            LogError("invalid amsg: ", am->from);
            am->type = am->data = -'R';
            ReplyMsg(std::move(amsg));
            continue;
        } else {
            online--;
        }
    }

    for (;;) {
        MessagePtr amsg{ reply->Get() };
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

template<typename... Args>
void
warn(const char *fmt, Args... args)
{
    char message[512];
    snprintf(message, sizeof(message), fmt, args...);
    LogWarn(message);

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
    LogWarn("!! (X) ", now(), ": shutdown request from ", source);

    if (online != 0) {
        LogNote("Kill request denied: ", online, " users on-line");
        Ad = At = 'X';
    } else {
        warn("%s", "Game shutdown initiated by administrator.");
        reset_users();
        Ad = At = 'O';
        g_resetInProgress = true;
    }
}

// User connecting
static void
cnct()
{
    amul->data = (long) linestat;
    amul->opaque = (char *) usr;
    if (Af >= MAXU) {
        if (Af == MAXU + 1)
            LogInfo("Mobile processor connected.");
        if ((linestat + Af)->state != 0)
            Af = -1;
        else
            (linestat + Af)->state = PLAYING;
        return;
    }
    Af = -1;
    // Allow for demons & mobiles
    for (int i = 0; i < MAXU; i++) {
        if ((linestat + i)->state != 0)
            continue;
        Af = i;
        (linestat + i)->state = LOGGING;
        online++;
        calls++;
        break;
    }
}

demonid_t
Demon::Start(slot_t owner, time_t seconds, verbid_t action, Param param1, Param param2)
{
    const demonid_t id = s_nextID++;
    const time_t trigger = time(nullptr) + seconds;
    if (demons.empty() || demons.back().m_trigger <= trigger) {
        demons.emplace_back(id, owner, trigger, action, param1, param2);
    } else {
        demons.emplace_front(id, owner, trigger, action, param1, param2);
    }
    return id;
}

Demon::~Demon()
{
    if (m_id != -1) {
        LogDebug(*this, ": killed");
    }
}

void
Demon::Kill(slot_t owner, verbid_t action)
{
    demons.remove_if([=](const Demon &demon) {
        return demon.m_owner == owner && (action == -1 || demon.m_action == action);
    });
}

uint32_t
Demon::GetSecondsRemaining() const noexcept
{
    if (auto now = time(nullptr); now >= m_trigger)
        return uint32_t(now - m_trigger);
    return 0;
}

// Check if demon is active
void
checkDemon(verbid_t action)
{
    int i = 0;
    for (auto &&demon : demons) {
        if (demon.m_action == action &&
            (demon.m_owner == Demon::GlobalOwner || demon.m_owner == Af)) {
            Ad = i;
            Ap1 = demon.GetSecondsRemaining();
            return;
        }
        ++i;
    }
    Ad = -1, Ap1 = -1;
}

// User disconnection
static void
discnct()
{
    if (Af < MAXU && (linestat + Af)->state == PLAYING) {
        LogInfo("<- ", Af, " ", now(), ": user disconnected");
    }
    if (Af < MAXU)
        online--;
    (usr + Af)->name[0] = 0;
    (linestat + Af)->room = -1;
    (linestat + Af)->helping = -1;
    (linestat + Af)->following = -1;
    Demon::Kill(Af, -1);
    (linestat + Af)->state = 0;
    Af = -1;
    Ad = -1;
}

uint32_t
GetResetCountdown()
{
    if (auto seconds = s_nextReset - time(nullptr); seconds >= 0)
        return uint32_t(seconds);

    return 0;
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
            Ap2 = (uintptr_t) vername;
            Ap3 = (uintptr_t) g_game.gameName;
            Ap4 = (uintptr_t) linestat;
            break;
        case 0:
            amul->opaque = gameDir;
            amul->p1 = GetResetCountdown();
            break;
        case 1:
            amul->opaque = &g_game;
            break;
        default:
            amul->opaque = nullptr;
    }
}

// Receive & log a player login
static void
login()
{
    LogInfo("-> ", Af, " ", now(), " '", (usr + Af)->name, "' logged in");
    (linestat + Af)->state = PLAYING;
}

// Shutdown request
static void
asend(int type, int data)
{
    if ((reply = CreatePort("Killer!")) == nullptr) {
        LogError("Unable to create ''killer' port");
        return;
    }
    auto aptr = std::make_unique<Aport>(reply, type, -1, data);
    port->Put(std::move(aptr));
    port->Wait();
    if (!g_quiet) {
        switch (Ad) {
            case 'R':
                LogNote("*-- Reset Invoked --*");
                break;
            case 'O':
                LogNote("AMUL Manager removed");
                break;
            case 'X':
                LogError("Cannot reset manager with users connected");
                break;
            case 'U':
                LogError("AMAN error at other end");
                break;
            case -'X':
                LogNote("... Reset set for ", Ap1, " seconds ...");
                break;
            case -'R':
                LogNote("... Reset in progress ...");
                break;
            case 'E':
                LogNote("... Reset delayed by ", Ap1, " seconds ...");
                break;
            default:
                LogError("** Internal AMUL error: Return '", char(Ad), "')");
                break;
        }
    }
    ReleaseMem(&amul);
    DeletePort(reply);
}

static void
shutreq(bool kill, int seconds)
{
    asend(kill ? MKILL : MRESET, seconds);
}

static void
sendext(int seconds)
{
    asend(MEXTEND, seconds);
}

// RESeT in progress
static void
rest()
{
    g_forceReset = true;
    if (Ad > 0) {
        Ap1 = Ad;
        s_nextReset = time(nullptr) + Ad;
        warn("** System reset invoked - %zu seconds remaining...\n", size_t(Ad));
        Ad = At = -'X';
        return;
    }
    Ad = At = 'R';
    s_nextReset = time(nullptr);
}

// Extend by this many ticks
static void
extend(int32_t seconds)
{
    Ad = At = 'U';
    if (seconds == 0)
        return;

    s_nextReset += seconds;
    auto countdown = GetResetCountdown();
    if (countdown > 120) {
        LogNote("...Game time extended - reset will now occur in ",
                countdown / 60,
                " minutes and ",
                countdown % 60,
                " seconds");
    } else {
        LogNote("...Reset postponed - it will now occur in ", countdown, " seconds");
    }
    Ap1 = seconds;
    Ad = 'E';
}

// Reset <receiver>
static void
res()
{
    int onwas;
    warn("][ (%c) %s: Reset requested! %ld user(s) online...\n",
         (Af >= 0 && Af < 11) ? '0' + Af : '#',
         now(),
         online);
    onwas = online;
    reset_users();
    if (onwas != 0)
        LogNote("== (#) ", now(), ": All users disconnected...");
    else
        LogNote("== (#) ", now(), ": Reset completed!");
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
    LogNote("@@ ]",
            char(Af + '0'),
            "[ ",
            now(),
            ": User \"",
            (usr + Af)->name,
            "\" achieved top rank (",
            (usr + Af)->rank + 1,
            ")");
}

void
logit(const char *s)
{
    LogInfo("@@ (", char(Af + '0'), ") ", now(), ": ", s);
}

static void
filesize(const char *filename, size_t *intop)
{
    char filepath[MAX_PATH_LENGTH]{};
    safe_gamedir_joiner(filename);
    GetFilesSize(filepath, intop, true);
}

// Read in & evaluate data files
static void
setup()
{
    long l, act, j, k;
    long *pt;
    char *p;

    if ((p = (char *) AllocateMem(UINFO)) == nullptr)
        memfail("User tables");
    usr = (_PLAYER *) p;
    p += sizeof(*usr) * MAXNODE;
    linestat = (LS *) p;
    p += sizeof(*linestat) * MAXNODE;
    rctab = (short *) p;

    size_t verblen{ 0 };
    vbtab = (_VERB_STRUCT *) xread(verbDataFile, &verblen, "verb list");
    assert(verblen / sizeof(_VERB_STRUCT) == g_game.numVerbs);

    size_t stlen{ 0 }, vtlen{ 0 }, vtplen{ 0 };

    filesize(verbSlotFile, &stlen);
    filesize(verbTableFile, &vtlen);
    filesize(verbParamFile, &vtplen);

    // 9: Read the travel table
    size_t ttlen{ 0 };
    ttp = (_TT_ENT *) xread(travelTableFile, &ttlen, "travel table");
    assert(ttlen / sizeof(_TT_ENT) == g_game.numTTEnts);

    // 12: Read parameters
    size_t ttplen{ 0 };
    ttpp = (long *) xread(travelParamFile, &ttplen, "TT parameter table");
    ttabp = ttp;
    pt = ttpp;
    for (size_t i = 0; i < g_game.numTTEnts; i++) {
        ttabp = ttp + i;
        k = (long) ttabp->pptr;
        ttabp->pptr = (int *) pt;
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
    if ((p = (char *) AllocateMem(stlen + vtlen + vtplen)) == nullptr)
        memfail("language data");
    slottab = (_SLOTTAB *) readf(verbSlotFile, p);
    vtp = (_VBTAB *) readf(verbTableFile, p + stlen);
    vtpp = (long *) readf(verbParamFile, p + stlen + vtlen);

    // 18: Get last reset time
    strcpy(lastres, now());
    strcpy(lastcrt, now());

    // Adjust the verb-related pointers
    vbptr = vbtab;
    stptr = slottab;
    vtabp = vtp;
    l = 0;
    for (size_t i = 0; i < g_game.numVerbs; i++, vbptr++) {
        vbptr->ptr = stptr;
        for (j = 0; j < vbptr->ents; j++, stptr++) {
            stptr->ptr = vtabp;
            for (k = 0; k < stptr->ents; k++, vtabp++) {
                vtabp->pptr = (int *) vtpp + l;
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

    if (ifp != nullptr) {
        fclose(ifp);
        ifp = nullptr;
    }
}

void
demonTicker()
{
    demons.sort([](auto &lhs, auto &rhs) noexcept { return lhs.m_trigger < rhs.m_trigger; });
    time_t now = time(nullptr);
    for (auto cnt = 0; cnt < 10 && !demons.empty() && demons.front().m_trigger <= now; ++cnt) {
        auto demon = demons.front();
        demons.pop_front();
        auto amp = GetNewAport(MDAEMON, demon.m_trigger);

        amp->p1 = demon.m_params[0].m_type;
        amp->p2 = demon.m_params[0].m_value;
        amp->p3 = demon.m_params[1].m_type;
        amp->p4 = demon.m_params[1].m_value;

        auto owner = demon.m_owner;
        (linestat + owner)->rep->Put(std::move(amp));
    }
}

void
resetTicker()
{
    auto timeToReset = GetResetCountdown();
    if (timeToReset == 300) {
        warn("--+ Next reset in 5 minutes +--");
    }
    if (timeToReset == 120) {
        warn("--+ 120 seconds until next reset +--");
    }
    if (timeToReset == 60) {
        warn("--+ Final warning - 60 seconds to reset +--");
    }
    if (timeToReset <= 0) {
        g_resetInProgress = true;
        if (!g_forceReset)
            warn("[ Automatic Reset ]\n");
    }
}

void
kernel()
{
    LogInfo("------------------------------------------------------------");
    s_nextReset = time(nullptr) + g_game.gameDuration_m * 60;
    g_resetInProgress = false;
    g_forceReset = false;
    online = 0;
    demons.clear();

    for (int i = 0; i < MAXNODE; i++) {
        (linestat + i)->IOlock = -1;
        (linestat + i)->room = bid[i] = -1;
        busy[i] = 0;
        (linestat + i)->helping = -1;
        (linestat + i)->following = -1;
    }

    LogInfo("== (=) ", now(), ": Loaded ", g_game.gameName);
    ded = 0;

    // Activate the demon processor

    execute("amul -\03");

    while (!g_resetInProgress) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        reply->Clear();

        demonTicker();
        resetTicker();

        MessagePtr amsg{ port->Get() };
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
                Demon::Start(amul->from,
                             time_t(amul->opaque),
                             amul->data,
                             { amul->p1, amul->p2 },
                             { amul->p3, amul->p4 });
                break;  // Priv. demon
            case MDCANCEL:
                Demon::Kill(Af, Ad);
                break;
            case MCHECKD:
                checkDemon(Ad);
                break;
            case MMADEWIZ:
                logwiz(Af);
                break;
            case MLOG:
                logit(static_cast<const char *>(amul->opaque));
                break;
            case MEXTEND:
                extend(Ad);
                g_forceReset = false;
                break;
            case MGDSTART:
                Demon::Start(Demon::GlobalOwner,
                             time_t(amul->opaque),
                             amul->data,
                             { amul->p1, amul->p2 },
                             { amul->p3, amul->p4 });
                break;  // Global demon
            default:
                At = -1;
                LogError("$$ (X) ", now(), ": *INVALID Message Type: ", At);
                break;
        }

        ReplyMsg(std::move(amsg));
    }

    for (;;) {
        MessagePtr amsg{ port->Get() };
        amul = static_cast<Aport *>(amsg.get());
        if (!amul)
            break;
        Ad = At = -'R';
        ReplyMsg(std::move(amsg));
    }

    if (g_resetInProgress) {
        res();
        givebackmemory();
        setup();
        LogInfo("[ ", vername, " RESET ]");
#ifdef REPAIR
        if (GetFilesSize("reset.bat") > 0) {
            Execute("execute reset.bat", 0L, 0L);
        }
#endif
        g_resetInProgress = false;
        online = 0;
    }
}

[[noreturn]] void
executeCommand(int argc, const char *argv[])
{
    if (!port) {
        LogFatal("AMAN is not running");
    }
    size_t seconds{ 0 };
    if (argc == 3)
        sscanf(argv[2], "%zu", &seconds);
    switch (toupper(*(argv[1] + 1))) {
        case 'K':
            shutreq(true, seconds);
            break;
        case 'R':
            shutreq(false, seconds);
            break;
        case 'X':
            if (argc != 3)
                LogFatal("Missing minute value after -x option");
            sendext(seconds);
            break;
    }
    exit(0);
}

[[noreturn]] void
usage(const char *argv[], error_t err)
{
    printf("Usage: %s [-h|-?|--help] [-v|--verbose] [-q|--quiet] [game directory]\n", argv[0]);
    printf("Server for AMUL multi-player games.\n");
    printf("\n");
    printf("  --help               Displays this help information\n");
    printf("  --quiet              Decreases output verbosity\n");
    printf("  --verbose            Increases output verbosity\n");
    printf("  <game directory>     Option path containing game files\n");

    exit(err);
}

void
parseArguments(int argc, const char *argv[])
{
    if (argc == 1) {
        strcpy(gameDir, ".");
        return;
    }

    if (argv[1][0] == '-' && strchr("krc", argv[1][1])) {
        executeCommand(argc, argv);
    }

    int desiredLogLevel = LWARN;

    for (int argn = 1; argn < argc; ++argn) {
        std::string_view arg = argv[argn];
        if (arg[0] == '-') {
            if (arg == "-v" || arg == "--verbose") {
                if (desiredLogLevel > 0)
                    --desiredLogLevel;
                continue;
            }
            if (arg == "-q" || arg == "--quiet") {
                g_quiet = true;
                if (desiredLogLevel < MAX_LOG_LEVEL - 1)
                    ++desiredLogLevel;
                continue;
            }
            if (arg == "-h" || arg == "-?" || arg == "--help") {
                usage(argv, 0);
            }
            printf("ERROR: Invalid command line argument: %s", arg.data());
            usage(argv, EINVAL);
        }

        if (gameDir[0] != 0) {
            LogFatal("Invalid argument/multiple paths specified: %s", arg.data());
        }

        error_t err = path_copier(gameDir, arg.data());
        if (err != 0)
            LogFatal("Invalid game path: ", argv[argn]);
    }

    if (gameDir[0] == 0)
        strcpy(gameDir, ".");

    SetLogLevel((LogLevel) desiredLogLevel);
    LogDebug("Game Path: ", gameDir);
}

constexpr auto checkedRead = [](int fd, auto *into, size_t count) noexcept
{
    auto expected = sizeof(*into) * count;
    if (auto bytes = read(fd, into, expected); size_t(bytes) != expected)
        LogFatal("Read error: expected ", expected, " bytes, got ", bytes);
};

constexpr auto checkedLoad = [](const char *label, int fd, auto &into, size_t count) noexcept
{
    into.resize(count);
    LogDebug("reading ", label, ": ", count, ": ", sizeof(*into.data()) * count);
    checkedRead(fd, into.data(), count);
};

error_t
Game::Load()
{
    LogInfo("Loading game data");

    char filepath[MAX_PATH_LENGTH];
    safe_gamedir_joiner(gameFile);
    int fd = open(filepath, READ_FLAGS);
    if (fd == -1)
        LogFatal("Unable to open game data file: ", filepath, ": ", strerror(errno));

    LogDebug("data section");
    checkedRead(fd, dynamic_cast<GameConfig *>(&g_game), 1);
    if (g_game.version != GameConfig::CurrentVersion) {
        LogError("Game file was compiled with a different AMUL version -- cannot load.");
        return EINVAL;
    }

    checkedLoad("string index", fd, m_stringIndex, numStrings);
    checkedLoad("string bytes", fd, m_strings, stringBytes);

    checkedLoad("ranks", fd, m_ranks, numRanks);

    checkedLoad("rooms", fd, m_rooms, numRooms);

    checkedLoad("adjectives", fd, m_adjectives, numAdjectives);

    checkedLoad("objects", fd, m_objects, numObjects);
    checkedLoad("object locations", fd, m_objectLocations, numObjLocations);
    checkedLoad("object states", fd, m_objectStates, numObjStates);

    checkedLoad("synonyms", fd, m_synonyms, numSynonyms);

    close(fd);

    // Fix the object 'inside' flags
    for (auto &obj : m_objects) {
        // Look for objects that have a negative room id which is below the 'INS' value
        objid_t container = m_objectLocations[obj.rooms];
        if (container <= -INS)
            m_objects[container].inside++;
    }

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
    (void) vername;
#endif

    parseArguments(argc, argv);

    if (error_t err = g_game.Load(); err != 0)
        LogFatal("Failed to load game data");

    port = FindPort(managerPortName);  // Check for existing port
    if (port != nullptr) {
        printf("AMAN %s running!\n", "already");
        exit(0);
    }
    if ((port = CreatePort(managerPortName)) == nullptr) {
        printf("Unable to create %s port!\n", "AMUL Manager");
        quit();
    }
    if ((reply = CreatePort(nullptr)) == nullptr) {
        printf("Unable to create %s port!\n", "Returns");
        quit();
    }
    if ((trport = CreatePort(nullptr)) == nullptr) {
        printf("Unable to create %s port!\n", "Timer");
        quit();
    }

#if defined(__AMIGA__)
    /// TODO: Replace
    if (OpenDevice(TIMERNAME, UNIT_VBLANK, (IORequest *) &ResReq, 0L) != NULL) {
        printf("Can't open timer.device!\n");
        quit();
    }
    ResReq.tr_node.io_Message.mn_ReplyPort = trport;
    TDBase = 1;
#endif

    setup();

    LogInfo("[", vername, " LOADED]");
    while (!g_resetInProgress) {
        kernel();
    }
    LogInfo("[ ", vername, " KILLED]");

    quit();
}
