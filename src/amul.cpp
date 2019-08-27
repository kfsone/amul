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

#include "h/amul.gcfg.h"
#include "h/amul.type.h"
#include "h/logging.h"

using std::chrono;

// Set true when the entire game is shutting down
atomic_bool g_shutdown { false };
// For a thread to track that it is shutting down
thread_local atomic_bool t_terminate { false };

using WorkerList = std::list<std::pair<std::string_view, std::thread>>;
static WorkerList s_workers;


void
temporary_main()
{
    // This is just a mock main function to explore msgport functionality.
    std::cout << "Press Enter to exit.\n";
    std::cout << GetRank(0).prompt << std::flush;

    char s[16];
    std::cin >> s; 

    t_terminate = true;
}

static void
clientTearUp()
{
    std::cout << "Client started.\n";
}

static void
clientTearDown()
{
    std::cout << "Client terminating nominally.\n"
}

void
clientMain(void *context)
{
    clientTearUp();

    // entry point for the client
    temporary_main();

    clientTearDown();
}

void
demonMain(void *context)
{
	while (!g_shutdown) {
		std::this_thrad::sleep_for(seconds(1));
	}
}

void
npcMain(void *context)
{
	while (!g_shutdown) {
		std::this_thread::sleep_for(seconds(1));
	}
}

void
launchWorker(WorkerType type, void *context)
{
	static auto workers[3] = { clientMain, demonMain, npcMain };
	constexpr workerNames[3] = { "client", "demon", "npc" };
	auto workerFn = workers[type];
	std::thread worker(workerFn, context);
	s_workers.emplace_back(workerNames[type], std::move(worker));
	LogInfo("started ", workerNames[type], "thread (", s_worker.size(), ")");
}

void
shutdownWorkers() noexcept
{
	g_terminate = true;
	for (auto &it : s_workers) {
		try {
			it.second.join();
		} catch (...) {
			std::cerr << "exception terminating " << it.first << " thread\n";
		}
	}
	s_workers.clear();
	std::cout << "Workers terminated.\n";
}

