/////////////////////////////////////////////////////////////////////////////
// SMUGL - Simple Multi-User Gaming Language
// Copyright (C) Oliver Smith, 1990-1997. Copyright (C) KingFisher Software
//---------------------------------------------------------------------------
// SMUGL is based on AMUL, the Amiga Multi-User Language.
// Development started in mid-1990, and finished in 1993.
// In late 1996, development started as re-implementation under Unix,
// and AMUL became SMUGL.
// Thanks to Richard Pike for his assistance with some parts of AMUL.
//---------------------------------------------------------------------------
// AMUL was written (badly) in 'C', SMUGL will be written (badly) in C++
/////////////////////////////////////////////////////////////////////////////

// Actually, this is just an experiment to see if I can convert
// AMUL to C++ ;-)

#define DEF
#include "smugl/smugl.hpp"
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include "include/consts.hpp"
#include "include/syslog.hpp"
#include "smugl/client.hpp"
#include "smugl/loaders.hpp"

// Proto files
#include "include/libprotos.hpp"
#include "smugl/io.hpp"
#include "smugl/ipc.hpp"
#include "smugl/lang.hpp"
#include "smugl/login.hpp"
#include "smugl/manager.hpp"
#include "smugl/misc.hpp"
#include "smugl/mobiles.hpp"
#include "smugl/objects.hpp"
#include "smugl/parser.hpp"
#include "smugl/ranks.hpp"
#include "smugl/rooms.hpp"
#include "smugl/travel.hpp"

char g_fork_on_load;  // Do we detach on startup?
int g_debug;          // What debug level?

int g_exiting = ecFalse;    // If we're exiting
bool g_heavyDebug = false;  // Enable heavy debugging

char* program;                // Program name (argv[0])
extern const char vername[];  // Version name from version.C

// Local forward-protos
static void argue(int argc, char* argv[]);

int
main(int argc, char* argv[])
{
    program = argv[0];

    // For safeties sake, we kick the random seed around some
    srand((unsigned int) time(
            NULL));  /// TODO: Not good enough; two people can log in the same second.

    argue(argc - 1, argv + 1);  // Process the command line arguments

    // Print the banner
    printf("===============================================================\n");
    printf("== %-57s ==\n", vername);
    printf("== %-57s ==\n", "SMUGL Copyright (C) KingFisher Software 1996/97");
    printf("== %-57s ==\n", "AMUL  Copyright (C) KingFisher Software 1991-1996");
    printf("===============================================================\n");

    // openlog("smugl", LOG_PID, LOG_LOCAL0);

    init_ipc(memory_required());   // Initialise the IPC and shared memory
    load_database(data->shmbase);  // Load initial database into shared memory
    init_sockets();                // Prepare the listening socket

    // Now we invoke the manager's main loop.
    // This perpetuates, and only ever returns when it has fork()ed a
    // client process. So when we return from this call, we are a client
    run_the_game();

    // From here on, we are a client
    client_initialise();

    ShowFile("title.text");  // Display title screen
    login();                 // Log the player in
    sysLog.Write(_FLI, "player logged in: %s", me->name());

    while (g_exiting == ecFalse) {
        prompt(myRank->prompt);
        fetch_input(g_input, MAX_PHRASE_SIZE);
        sanitise_input();  // Tidy up what we read
        if (g_input[0] == 0)
            continue;
        if (strncmp(g_input, "*debug", 6) == 0) {
            txprintf("- Heavy Debugging is %s\n", (g_heavyDebug = !g_heavyDebug) ? "on" : "off");
            continue;
        }
        if (g_debug)
            txprintf("You typed: [%s]\n", g_input);
        parse(g_input);
    }

    exit(0);
}

// Describe how to run the program
static void
usage(int code)
{
    printf("Usage: %s [-d] [-f] [path]\n"
           "Loads and runs a pre-compiled SMUGL game (see smuglcom)\n"
           "Options:\n"
           "  -d       Enable debugging output\n"
           "  -f       Fork after initialising the game\n"
           "  path     Specify the game path (default is pwd)\n",
           program);
    exit(code);
}

// Process the arguments
static void
argue(int argc, char* argv[])
{
    struct stat sbuf;
    int arg = 0;

    g_dir[0] = 0;
    // NOTE: This isn't real argv - argv[0] is the first ARGUMENT
    // the program was called with. Don't get confused ;-)
    if (argc >= 1)
    // We have some arguments
    {
        if (strcmp(argv[0], "?") == 0)
            usage(0);  // Allow "smugl ?" for usage
        for (; arg < argc; arg++) {
            if (argv[arg][0] == '-' || argv[arg][0] == '/') {
                switch (argv[arg][1]) {
                    case '?':  // Things that look like usage requests
                    case 'h':
                    case 'u':
                        usage(0);
                        // NOTREACHED

                    case 'f':  // fork after loading everything
                        g_fork_on_load = 1;
                        break;

                    case 'd':  // debug mode (incremental)
                        g_debug++;
                        break;

                    default:
                        usage(0);
                        // NOTREACHED
                }
            } else {
                if (arg + 1 < argc) {
                    sysLog.Write(_FLE,
                                 "Don't understand arguments at: %s %s ...",
                                 argv[arg],
                                 argv[arg + 1]);
                    usage(1);
                    // NOTREACHED
                }
                strcpy(g_dir, argv[arg]);
            }
        }
    }
    if (g_dir[0] == 0) {
        if (_getcwd(g_dir, MAXPATHLEN) == NULL) {
            sysLog.Write(_FLE, "Can't get current directory: %d", errno);
            usage(1);
        }
    }
    if (g_dir[strlen(g_dir) - 1] != '/')
        strcat(g_dir, "/");
    if (stat(g_dir, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode)) {
        sysLog.Write(_FLE, "Can't access directory %s", g_dir);
        usage(1);
    }
}
