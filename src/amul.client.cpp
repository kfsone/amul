/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########

                   AMUL Client Role

Amiga AMUL loaded data into "AMan" and then clients were launched that
either used a serial port or a console screen, and they got pointers to
the data via the Amiga's MessagePort system.

New AMUL will merge AMan and AMUL into one binary and use threading to
achieve the previous compartmentalization.

I've moved all the original "amul.cpp" code into amul.wholeoriginal.cpp
and the plan is to gradually migrate it back into this file.
*/

#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <thread>

#include "amul.actions.h"
#include "amul.stct.h"
#include "amul.vars.h"
#include "amulinc.h"
#include "amullib.h"
#include "client.io.h"
#include "exceptions.h"
#include "game.h"
#include "logging.h"
#include "message.common.h"
#include "message.execdemon.h"
#include "modules.h"
#include "msgports.h"
#include "system.h"
#include "amul.typedefs.h"
#include "workertype.h"

using WorkerFn = void (*)(void *);

using namespace std::chrono;

// For a thread to track that it is shutting down
thread_local std::atomic_bool t_terminate{ false };

struct WorkerInstance {
    WorkerInstance(string_view name_, std::thread &&thread_, slotid_t slotId_)
        : name(name_), thread(move(thread_)), slotId(slotId_)
    {
    }
    string_view name;
    std::thread thread;
    slotid_t slotId;
};
using WorkerList = std::list<WorkerInstance>;
static WorkerList s_workers;

thread_local MsgPortPtr t_managerPort;
thread_local MsgPortPtr t_replyPort;
thread_local Message *amul;
thread_local slotid_t t_slotId;

Avatar::Avatar() : m_outputBuffer(static_cast<char*>(AllocateMem(4096))) {}

extern error_t PlayerClientUp();
extern void PlayerClientDown();
extern void amul_main();

////////////////////////////////////////////////////////////////////////////////

static void
playerMain(void * /*context*/)
{
    try {
        if (error_t err = PlayerClientUp(); err != 0) {
            LogError("Unable to initiate client %d", t_slotId);
            throw EndThread{};
        }
        amul_main();
    } catch (const EndThread &ex) {
        if (ex.what() != nullptr) {
            LogError("Disconnecting ", t_slotId, " with error: ", ex.what());
            Printf("Internal Error: %s", ex.what());
        } else
            LogError("Disconnecting %d", t_slotId);
    }

    PlayerClientDown();
}

void
demonMain(void * /*context*/)
{
    if (verbid_t bootId = IsVerb("\"boot"); bootId != -1) {
        lang_proc(bootId, 0);
    }

    LogInfo("-- Entered demon processor's main()");
    while (g_game.m_runMode == RunMode::Normal) {
        std::this_thread::sleep_for(seconds(1));
    }
}

void
npcMain(void * /*context*/)
{
    LogInfo("-- Entered npc processor's main()");

    /// NOTE: This snapshot is from right before I implemented the npc
    /// system in 0.9.00, so there's no actual npc implementation :(

    // Most npcs will probably want an easily accessible list of travel.
    std::vector<verbid_t> travelVerbs{};
    for (verbid_t i = 0; i < verbid_t(g_game.numVerbs); ++i) {
        if (GetVerb(i).flags & VB_TRAVEL) {
            travelVerbs.push_back(i);
        }
    }

    while (g_game.m_runMode == RunMode::Normal) {
        std::this_thread::sleep_for(seconds(1));
    }
}

////////////////////////////////////////////////////////////////////////////////

struct WorkerDef {
    const char *name;
    WorkerFn mainFn;
};
constexpr WorkerDef s_workerDefs[NUM_WORKER_TYPES]{
    // Lists the basic characteristics of each worker type.
    { "player", playerMain },
    { "demon", demonMain },
    { "npc", npcMain }
};

void
startThread(MsgPortPtr managerPort,
            MsgPortPtr replyPort,
            slotid_t slotId,
            WorkerFn workerFn,
            void *context)
{
    t_managerPort = managerPort;
    t_replyPort = replyPort;
    t_slotId = slotId;

    t_managerPort->Put(make_unique<MsgConnectClient>());

    Action::AbortParse();

#ifdef MESSAGE_CODE
    /// TODO: Need to make this is timed wait
#endif
    if (auto msg = t_replyPort->Wait(); msg->m_sender == -1) {
        Print("Connection refused by server.");
    } else {
        workerFn(context);
    }
}

void
StartClient(WorkerType type, slotid_t slotId, void *context)
{
    /// TODO: Find a slotId for this worker *before* we we trigger a thread.
    auto portName = std::to_string(slotId);
    auto clientReplyPort = CreatePort(portName);
    if (clientReplyPort == nullptr)
        LogFatal("Unable to spin up worker's reply port");
    LogDebug(
            "launching ", s_workerDefs[type].name, "#", slotId, " thread (", s_workers.size(), ")");
    std::thread worker(startThread,
                       t_managerPort,
                       clientReplyPort,
                       slotId,
                       s_workerDefs[type].mainFn,
                       context);
    s_workers.emplace_back(s_workerDefs[type].name, move(worker), slotId);
}

void
JoinClient(slotid_t slot)
{
    for (auto &thread : s_workers) {
        if (thread.slotId == slot) {
            thread.thread.join();
        }
    }
}

void
KillClients() noexcept
{
    g_game.m_runMode = RunMode::Terminate;

    for (auto &it : s_workers) {
        try {
            it.thread.join();
        } catch (...) {
            std::cerr << "exception terminating " << it.name << "#" << it.slotId << " thread\n";
        }
    }
    s_workers.clear();
    std::cout << "Workers terminated.\n";
}

error_t
clientModuleInit(Module * /*module*/)
{
    return 0;
}

error_t
clientModuleStart(Module * /*module*/)
{
    // Activate the demon and npc processor
    StartClient(am_DAEM, DEMON_SLOT, nullptr);
    StartClient(am_MOBS, NPC_SLOT, nullptr);

    return 0;
}

error_t
clientModuleClose(Module * /*module*/, error_t /*err*/)
{
    /// TODO: 'module' should have context telling us *which* client.
    return 0;
}

error_t
InitClient()
{
    NewModule(MOD_CLIENT, clientModuleInit, clientModuleStart, clientModuleClose, nullptr, nullptr);
    return 0;
}

