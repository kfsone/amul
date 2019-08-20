#include "amulconfig.h"

#include <iostream>
#include <map>
#include <string>
#include <string_view>
#include <thread>

#include "amul.stringmanip.h"
#include "game.h"
#include "logging.h"
#include "typedefs.h"
#include "workertype.h"

using CommandMap = std::map<string_view, pair<void (*)(string_view), const char *>>;
static void consoleHelp(string_view);
static void consoleStats(string_view);
static void consolePlay(string_view);
static bool s_consoleShutdown{ false };

static CommandMap s_consoleCommands = {
    // Console command definition
    { "help", { &consoleHelp, "Describes console commands/usage." } },
    { "play", { &consolePlay, "Attempt to launch a client on the console." } },
    { "quit", { [](string_view) { s_consoleShutdown = true; }, "Shutdown AMUL Server/Clients" } },
    { "stats", { &consoleStats, "Show assorted game stats." } },
};

static void consoleHelp(string_view)
{
    std::cout << "AMUL Server Console commands:\n";
    for (auto &it : s_consoleCommands) {
        std::cout << "  " << it.first << ": " << it.second.second << "\n";
    }
}

static void consoleStats(string_view)
{
    std::cout << "Strings: " << g_game.m_stringIndex.size() << "\n"
              << "Text: " << g_game.m_strings.size() << "\n"
              << "Rooms: " << g_game.m_rooms.size() << "\n"
              << "Objects: " << g_game.m_objects.size() << "\n";
}

extern void StartClient(WorkerType type, slotid_t slotId, void *context);
extern void JoinClient(slotid_t slot);

static void consolePlay(string_view)
{
    StartClient(am_USER, 0, nullptr);
    JoinClient(0);
}

void
ConsoleMain()
{
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // This is just a mock main function to explore msgport functionality.
    std::cout << "----------\n";
    std::cout << "AMUL Server Console. Type 'quit' to exit.\n";

    while (g_game.m_runMode != RunMode::Terminate && !s_consoleShutdown) {
        std::string input{};
        std::cout << g_game.m_ranks.back().prompt << std::flush;
        std::getline(std::cin, input);

        auto it = s_consoleCommands.find(input);
        if (it == s_consoleCommands.end()) {
            std::cout << "Unknown command: " << input << ". See 'help' for assistance.\n";
            continue;
        }
        it->second.first(input);
    }

    std::cout << "Exiting.\n";

    g_game.m_runMode = RunMode::Terminate;
}
