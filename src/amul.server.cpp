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

#include <cassert>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <future>
#include <list>
#include <string_view>
#include <thread>
#include <vector>

#include "amigastubs.h"
#include "amul.cons.h"
#include "amul.defs.h"
#include "amul.file.h"
#include "amul.h"
#include "amul.stringmanip.h"
#include "amul.vars.h"
#include "amul.version.h"
#include "amul.vmop.h"
#include "demon.h"
#include "demon.inl.h"
#include "filesystem.h"
#include "game.h"
#include "logging.h"
#include "message.common.h"
#include "message.execdemon.h"
#include "modules.h"
#include "msgports.h"
#include "roomflag.h"
#include "system.h"
#include "typedefs.h"
#include "users.h"

extern thread_local FILE *ifp;
Game g_game;
time_t g_nextReset;

using DemonList = std::list<Demon>;
DemonList demons{};
demonid_t Demon::s_nextID{ 1 };

char bid[MAXNODE];
bool g_busy[MAXNODE];

constexpr string_view serverVersion{ AMUL_VSTRING };

Message *am;
long invis, invis2, calls;  // Invisibility Stuff & # of calls
long ded;                   // ded = deduct

long online;
bool g_quiet{ false };  // replaces: quiet

std::future<void> s_serverState{};
std::thread s_serverThread;

void
MsgPingServer::Dispatch()
{
    LogInfo("Ping RX from ", m_sender);
}

#ifdef MESSAGE_CODE
// Force users to log-out & kill extra lines
static void
reset_users()
{
    online = 0;  // Allows for demons & npcs
    for (int i = 0; i < MAXNODE; i++) {
        if (g_game.m_avatars[i].state <= 0)
            continue;
        online++;
        auto msg = GetNewMessage(MCLOSEING, 0, t_managerPort);
        g_game.m_avatars[i].m_replyPort->Put(move(msg));
    }
    while (online > 0) {
        MessagePtr amsg{ t_managerPort->Wait() };
        am = amsg.get();
        if (am == nullptr)
            break;
        if (am->m_from != -'O') {
            LogError("invalid amsg: ", am->m_from);
            am->m_type = am->m_data = -'R';
            ReplyMsg(move(amsg));
            continue;
        } else {
            online--;
        }
    }

    for (;;) {
        MessagePtr amsg{ t_replyPort->Get() };
        am = amsg.get();
        if (!am)
            break;
            /// TODO: Need to notify the client we're shutting down
        ReplyMsg(move(amsg));
    }
    t_managerPort->Clear();
    t_replyPort->Clear();
    online = 0;
}
#endif

template<typename... Args>
void
warn(const char *fmt, Args... args)
{
    char message[512];
    snprintf(message, sizeof(message), fmt, args...);
    LogWarn(message);

    for (size_t i = 0; i < MAXU; i++) {
        if (g_game.m_avatars[i].state != OFFLINE) {
#ifdef MESSAGE_CODE
            auto amsg = make_unique<Message>(t_replyPort, MRWARN);
            amsg->m_ptrs[0] = strdup(message);  /// TODO: bad.
            g_game.m_avatars[i].m_replyPort->Put(move(amsg));
#endif
        }
    }
}

// User connecting
void
MsgConnectClient::Dispatch()
{
    if (m_sender < 0 || m_sender >= MAXNODE) {
        LogError("Received Connect from slot ", m_sender);
        m_sender = -1;
        return;
    }
    auto &avatar = GetAvatar(m_sender);
    switch (m_sender) {
        case DEMON_SLOT:
            if (avatar.state != OFFLINE) {
                LogError("Worker attempted to attach to occupied demon slot.");
                m_sender = -1;
                return;
            }
            LogInfo("Demon worker attached.");
            break;
        case NPC_SLOT:
            if (avatar.state != OFFLINE) {
                LogError("Worker attempted to attach to occupied npc slot.");
                m_sender = -1;
                return;
            }
            LogInfo("NPC worker attached.");
            break;
        default:
            if (avatar.state != OFFLINE) {
                LogError("Connection to slot ", m_sender, " in state ", avatar.state, " rejected.");
                m_sender = -1;
                return;
            }
            LogInfo("Client connected to slot ", m_sender);
            break;
    }

    avatar.state = LOGGING;
}

demonid_t
Demon::Start(slotid_t owner, time_t seconds, Parse::Expression expression)
{
    const demonid_t id = s_nextID++;
    const time_t trigger = time(nullptr) + seconds;
    if (demons.empty() || demons.back().m_trigger <= trigger) {
        demons.emplace_back(id, owner, trigger, expression);
    } else {
        demons.emplace_front(id, owner, trigger, expression);
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
Demon::Kill(slotid_t owner, verbid_t action)
{
    demons.remove_if([=](const Demon &demon) {
        return demon.m_owner == owner && (action == WNONE || demon.m_expression.m_verb == action);
    });
}

uint32_t
Demon::GetSecondsRemaining() const noexcept
{
    if (auto now = time(nullptr); now >= m_trigger)
        return uint32_t(now - m_trigger);
    return 0;
}

uint32_t
GetResetCountdown()
{
    if (auto seconds = g_nextReset - time(nullptr); seconds >= 0)
        return uint32_t(seconds);

    return 0;
}

#ifdef MESSAGE_CODE
// Shutdown receiver
static void
kill()
{
    /// NOTE: This should be coming from the console.
    char source[32];
    if (amul->m_from != -1)
        sprintf(source, "line %u.", uint32_t(amul->m_from + 1));
    else
        strcpy(source, "external");
    LogWarn("!! (X) ", GetTimeStr(), ": shutdown request from ", source);

    if (online != 0) {
        LogNote("Kill request denied: ", online, " users on-line");
        amul->data = amul->type = 'X';
    } else {
        warn("%s", "Game shutdown initiated by administrator.");
        reset_users();
        amul->data = amul->type = 'O';
        g_game.m_runMode = RunMode::Reset;
    }
}

// Check if demon is active
void
checkDemon(verbid_t action)
{
    int i = 0;
    for (auto &&demon : demons) {
        if (demon.m_action == action &&
            (demon.m_owner == Demon::GlobalOwner || demon.m_owner == amul->m_from)) {
            amul->data = i;
            amul->p1 = demon.GetSecondsRemaining();
            return;
        }
        ++i;
    }
    amul->data = -1, amul->p1 = -1;
}

void
asend(int type, int data)
{
    if ((t_replyPort = CreatePort("Killer!")) == nullptr) {
        LogError("Unable to create ''killer' port");
        return;
    }
    auto aptr = make_unique<Message>(t_replyPort, type, -1, data);
    t_managerPort->Put(move(aptr));
    t_managerPort->Wait();
    ////TODO: have to capture wait -> amul
    if (!g_quiet) {
        switch (amul->m_data) {
            case 'R':
                LogNote("*-- Reset Invoked --*");
                break;
            case 'O':
                LogNote("AMUL Server removed");
                break;
            case 'X':
                LogError("Cannot reset AMUL Server with users connected");
                break;
            case 'U':
                LogError("AMUL Server error at other end");
                break;
            case -'X':
                LogNote("... Reset set for ", amul->p1, " seconds ...");
                break;
            case -'R':
                LogNote("... Reset in progress ...");
                break;
            case 'E':
                LogNote("... Reset delayed by ", amul->p1, " seconds ...");
                break;
            default:
                LogError("** Internal AMUL error: Return '", char(amul->data), "')");
                break;
        }
    }
    ReleaseMem(&amul);
    DeletePort(t_replyPort);
}

// RESeT in progress
static void
rest()
{
    g_game.m_forcedReset = true;
    if (amul->data > 0) {
        amul->p1 = amul->data;
        g_nextReset = time(nullptr) + amul->data;
        warn("** System reset invoked - %zu seconds remaining...\n", size_t(amul->data));
        amul->data = amul->type = -'X';
        return;
    }
    amul->data = amul->type = 'R';
    g_nextReset = time(nullptr);
}

// Extend by this many ticks
static void
extend(int32_t seconds)
{
    amul->data = amul->type = 'U';
    if (seconds == 0)
        return;

    g_nextReset += seconds;
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
    amul->p1 = seconds;
    amul->data = 'E';
}

// Reset <receiver>
static void
res()
{
    warn("][ (%c) %s: Reset requested! %ld user(s) online...\n",
         (amul->m_from >= 0 && amul->m_from < 11) ? '0' + amul->m_from : '#',
         notimestr(),
         online);
    int onwas = online;
    reset_users();
    if (onwas != 0)
        LogNote("== (#) ", GetTimeStr(), ": All users disconnected...");
    else
        LogNote("== (#) ", GetTimeStr(), ": Reset completed!");
    Delay(100);  // Always wait atleast a few seconds
}

void
logwiz(int who)
{
    LogNote("@@ ]",
            char(amul->m_from + '0'),
            "[ ",
            GetTimeStr(),
            ": User \"",
            g_game.m_players[who].name,
            "\" achieved top rank (",
            g_game.m_players[who].rank + 1,
            ")");
}
#endif  // MESSAGE_CODE

// Lock a users IO system
void
MsgLockUser::Dispatch()
{
    bid[m_sender] = m_param;
    if (g_game.m_avatars[m_param].IOlock != -1 ||
        (g_busy[m_param] && m_param != m_sender && bid[m_param] != m_sender)) {
        m_param = -1;
        return;
    }
    g_game.m_avatars[m_param].IOlock = m_sender;
    bid[m_sender] = -1;
}

// Receive & log a player login
void
MsgLoggedIn::Dispatch()
{
    LogInfo("-> ", m_sender, " ", GetTimeStr(), " '", GetCharacter(m_sender).name, "' logged in");
    GetAvatar(m_sender).state = PLAYING;
}

// User disconnection
void
MsgDisconnectClient::Dispatch()
{
    auto &avatar = GetAvatar(m_sender);
    if (m_sender < MAXU && avatar.state == PLAYING) {
        LogInfo("<- ", m_sender, " ", GetTimeStr(), ": user disconnected");
    }
    if (avatar.state != OFFLINE) {
        online--;
    }
    auto &player = GetCharacter(m_sender);
    player.name[0] = 0;
    avatar.room = -1;
    avatar.helping = -1;
    avatar.following = -1;
    Demon::Kill(m_sender, -1);
    avatar.state = OFFLINE;
    m_sender = -1;
}

void
MsgLog::Dispatch()
{
    LogInfo("@@ (", m_sender, ") ", GetTimeStr(), ": ", m_param);
}

void
demonTicker()
{
    // Ensure the demons are ordered by firing time and consume all the ones that have come due.
    demons.sort([](auto &lhs, auto &rhs) noexcept { return lhs.m_trigger < rhs.m_trigger; });
    while (!demons.empty() && demons.front().m_trigger <= time(nullptr)) {
        auto demon = demons.front();
        demons.pop_front();
        if (auto port = FindPort(demon.m_owner); port) {
            port->Put(make_unique<MsgExecDemon>(demon.m_expression));
        }
    }
}

void
resetTicker()
{
    auto timeToReset = GetResetCountdown();
    if (timeToReset == 300) {
        warn("%s", "--+ Next reset in 5 minutes +--");
    }
    if (timeToReset == 120) {
        warn("%s", "--+ 120 seconds until next reset +--");
    }
    if (timeToReset == 60) {
        warn("%s", "--+ Final warning - 60 seconds to reset +--");
    }
    if (timeToReset <= 0) {
        g_game.m_runMode = RunMode::Reset;
        if (!g_game.m_forcedReset)
            warn("%s", "[ Automatic Reset ]");
    }
}

void
MsgSetBusy::Dispatch(void)
{
    LogDebug("set busy ", m_sender, " to ", m_param ? "true" : "false");
    g_busy[m_sender] = m_param;
}

void
kernel()
{
    while (g_game.m_runMode == RunMode::Normal) {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));

        t_replyPort->Clear();

        demonTicker();
        resetTicker();

        /// TODO: timer/count
        for (size_t i = 0; i < 10; ++i) {
            MessagePtr amsg{ t_managerPort->Get() };
            // If there's nothing to get, we'll get a nullptr
            if (!amsg)
                break;
            amsg->Dispatch();
            ReplyMsg(move(amsg));
        }

/// TODO: Replace with custom packets that support 'dispatch'
#if defined(OLD_METHOD)
        switch (amul->type) {
            case MKILL:
                kill();
                break;
            case MRESET:
                rest();
                break;
            case MLOCK:
                lock();
                break;
            case MDCANCEL:
                Demon::Kill(amul->m_from, amul->data);
                break;
            case MCHECKD:
                checkDemon(amul->data);
                break;
            case MMADEWIZ:
                logwiz(amul->m_from);
                break;
            case MEXTEND:
                extend(amul->data);
                g_game.m_forcedReset = false;
                break;
            case MGDSTART:
                Demon::Start(Demon::GlobalOwner,
                             time_t(amul->m_ptrs[0]),
                             amul->data,
                             { amul->p1, amul->p2 },
                             { amul->p3, amul->p4 });
                break;  // Global demon
            default:
                amul->type = -1;
                LogError("$$ (X) ", GetTimeStr(), ": *INVALID Message Type: ", amul->type);
                break;
        }
#endif
    }
}

constexpr auto checkedRead = [](int fd, auto *into, size_t count) noexcept
{
    auto expected = sizeof(*into) * count;
    if (auto bytes = read(fd, into, expected); size_t(bytes) != expected)
        LogFatal("Read error: expected ", expected, " bytes, got ", bytes);
};

constexpr auto checkedLoad = [](string_view label, int fd, auto &into, size_t count) noexcept
{
    char desc[64]{};
    checkedRead(fd, desc, sizeof(desc));
    if (desc != label)
        LogFatal("Expecting ", label, " but found ", desc, "\n");
    size_t size{ 0 };
    checkedRead(fd, &size, 1);
    if (size != count)
        LogFatal("Expecting ", count, " but found ", size, "\n");
    into.resize(count);
    LogDebug("reading ", label, ": ", count, ": ", sizeof(*into.data()) * count);
    checkedRead(fd, into.data(), count);
};

error_t
Game::Load()
{
    LogInfo("[", serverVersion, " LOADING]");

    std::string filepath{};
    safe_gamedir_joiner(gameFile);
    int fd = open(filepath.c_str(), READ_FLAGS);
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
    checkedLoad("travel", fd, m_travel, numTTEnts);

    checkedLoad("adjectives", fd, m_adjectives, numAdjectives);

    checkedLoad("objects", fd, m_objects, numObjects);
    checkedLoad("object locations", fd, m_objectLocations, numObjLocations);
    checkedLoad("object states", fd, m_objectStates, numObjStates);

    checkedLoad("synonyms", fd, m_synonyms, numSynonyms);

    checkedLoad("verbs", fd, m_verbs, numVerbs);
    checkedLoad("syntax lines", fd, m_verbSlots, numVerbSlots);
    checkedLoad("language code", fd, m_vmLines, numVerbOps);
    checkedLoad("npc classes", fd, m_npcClasses, numNPCClasses);

    close(fd);

    // Fix the object 'inside' flags
    for (auto &object : m_objects) {
        object.rooms = m_objectLocations.data() + object.roomsOffset;
        object.states = m_objectStates.data() + object.stateOffset;
        for (size_t i = 0; i < size_t(object.nrooms); ++i) {
            objid_t container = object.Room(i);
            // Look for objects that have a negative room id which is below the 'INS' value
            if (container <= -INS) {
                container = -(container + INS);
                m_objects[container].inside++;
            }
        }
    }

    // Update the verb table so each record points to it's syntaxes lines
    for (auto &it : m_verbs) {
        it.ptr = it.ents ? m_verbSlots.data() + it.slotOffset : nullptr;
    }

    // Update all the syntaxes lines to point to the vm lines
    for (auto &it : m_verbSlots) {
        it.ptr = it.ents ? m_vmLines.data() + it.lineOffset : nullptr;
    }

    // Update each room so it has a pointer to its travel entries
    roomid_t roomNo{ 0 };
    std::vector<string_view> startRooms;
    for (auto &it : m_rooms) {
        it.ptr = it.ttlines ? m_travel.data() + it.ttOffset : nullptr;
        // Build a list of starting rooms.
        if (it.flags & STARTL) {
            g_game.m_startRooms.push_back(roomNo);
            startRooms.emplace_back(it.id);
        }
        ++roomNo;
    }
    LogInfo("Start Rooms: (",
            startRooms.size(),
            "): ",
            JoinStrings(startRooms.begin(), startRooms.end()));

    // Allocate bitfields for all the players
    for (auto &playerBits : m_visited) {
        playerBits.resize(m_rooms.size());
    }

    constexpr const char playerDataFile[] = "players.amulo";
    safe_gamedir_joiner(playerDataFile);
    if (error_t err = LoadPlayers(playerDataFile); err != 0 && err != ENOENT)
        LogFatal("Unable to load player data file");

    return 0;
}

error_t
serverModuleInit(Module * /*module*/)
{
    return 0;
}

error_t
serverMain(Module * /*module*/)
{
    LogInfo("[", serverVersion, " STARTED]");

    g_nextReset = time(nullptr) + g_game.gameDuration_m * 60;
    g_game.m_runMode = RunMode::Normal;
    g_game.m_forcedReset = false;
    online = 0;
    demons.clear();

    for (int i = 0; i < MAXNODE; i++) {
        g_game.m_avatars[i].IOlock = -1;
        g_game.m_avatars[i].room = bid[i] = -1;
        g_busy[i] = false;
        g_game.m_avatars[i].helping = -1;
        g_game.m_avatars[i].following = -1;
    }

    // 18: Get last reset time
    strcpy(g_game.lastStartupTime, GetTimeStr());
    strcpy(g_game.lastResetTime, g_game.lastStartupTime);
    strcpy(g_game.lastCompileTime, GetTimeStr(g_game.compiled));

    // Where we receive replies to messages.
    t_replyPort = CreatePort();
    if (!t_replyPort) {
        LogFatal("Unable to create AMUL Server reply port");
    }

    // The message port we originally receive messages on
    t_managerPort = CreatePort(managerPortName);
    if (!t_managerPort) {
        LogFatal("Unable to create AMUL Server port");
    }

    kernel();

#ifdef MESSAGE_CODE
    for (;;) {
        MessagePtr amsg{ t_managerPort->Get() };
        amul = amsg.get();
        if (!amul)
            break;
        amul->m_data = amul->m_type = -'R';

        ReplyMsg(move(amsg));
    }
#endif

    if (g_game.m_runMode == RunMode::Reset) {
        /// TODO: Support it. (move to serverThread)
        LogFatal("RESET NOT SUPPORTED YET");
#ifdef REPAIR
        res();
        // setup();
        LogInfo("[ ", serverVersion, " RESET ]");
        if (GetFilesSize("reset.bat") > 0) {
            Execute("execute reset.bat", 0L, 0L);
        }
#endif
        online = 0;
    }

    return 0;
}

void
serverThread(Module *module)
{
    /// while not exiting:
    error_t err = serverMain(module);
    if (err)
        LogFatal("server main returned error", err);
}

static bool
isServerRunning() noexcept
{
    using namespace std::chrono_literals;
    return s_serverState.valid() && s_serverState.wait_for(0ms) != std::future_status::ready;
}

error_t
serverModuleStart(Module *module)
{
    (void) module;  /// TODO: module should be the g_game context for this server instance.

    if (error_t err = g_game.Load(); err != 0)
        LogFatal("Failed to load game data");

    std::packaged_task<void(Module *)> task{ &serverThread };
    s_serverState = task.get_future();
    s_serverThread = std::thread(move(task), module);

    size_t ticks = 500;
    while (ticks && isServerRunning()) {
        t_managerPort = FindPort(managerPortName);
        if (t_managerPort)
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        --ticks;
    }
    if (ticks && isServerRunning()) {
        LogDebug("AMUL Server msgport registered");
        auto replyPort = CreatePort();
        t_managerPort->Put(make_unique<MsgPingServer>(replyPort));
        LogDebug("Server Ping Sent");

        while (ticks && isServerRunning()) {
            auto resp = replyPort->Get();
            if (resp.get() != nullptr)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            ticks--;
        }

        replyPort->Close();
    }
    if (!isServerRunning()) {
        LogError("Server thread terminated early");
        return ENOENT;
    }
    if (!ticks) {
        LogError("Server startup timed out");
        return EINVAL;
    }
    LogDebug("Server Pong received");

    return 0;
}

error_t
serverModuleClose(Module * /*module*/, error_t /*err*/)
{
    /// TODO: module should be the game context.

    LogInfo("[ ", serverVersion, " CLOSING]");

    if (s_serverThread.joinable())
        s_serverThread.join();

    /// TODO: This may actually cause a deadlock if a worker is waiting
    // on a server response...
    KillClients();

    return 0;
}

error_t
InitServer()
{
    NewModule(MOD_SERVER, serverModuleInit, serverModuleStart, serverModuleClose, nullptr, nullptr);
    return 0;
}
