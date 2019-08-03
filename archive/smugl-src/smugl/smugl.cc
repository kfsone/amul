/////////////////////////////////////////////////////////////////////////////
// $Id: smugl.cc,v 1.14 1999/09/10 15:57:31 oliver Exp $
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

static const char rcsid[] = "$Id: smugl.cc,v 1.14 1999/09/10 15:57:31 oliver Exp $";

#include <cstring>

#define DEF
#include "smugl.hpp"
#include "consts.hpp"
#include "fileio.hpp"
#include "loaders.hpp"
#include "client.hpp"

// Proto files
#include "libprotos.hpp"
#include "lang.hpp"
#include "rooms.hpp"
#include "ranks.hpp"
#include "mobiles.hpp"
#include "objects.hpp"
#include "manager.hpp"
#include "travel.hpp"
#include "ipc.hpp"
#include "io.hpp"
#include "misc.hpp"
#include "login.hpp"
#include "parser.hpp"

#ifdef HAVE_SYS_STAT_H
#include "sys/stat.h"
#endif
#include "sys/param.h"

char fork_on_load;              // Do we detach on startup?
char debug;                     // What debug level?

int exiting = ecFalse;          // If we're exiting
int heavyDebug = 0;		// Enable heavy debugging

char *program;                  // Program name (argv[0])
extern char vername[];          // Version name from version.C

// Local forward-protos
static void argue(int argc, char *argv[]);
extern void set_version_string(void);

int
main(int argc, char *argv[])
    {
    set_version_string();
    program = argv[0];

    // For safeties sake, we kick the random seed around some
    srand(time(NULL) % getpid());

    argue(argc - 1, argv+1);    // Process the command line arguments

    // Print the banner
    printf("===============================================================\n");
    printf("== %-57s ==\n", vername);
    printf("== %-57s ==\n", "SMUGL Copyright (C) KingFisher Software 1996/97");
    printf("== %-57s ==\n", "AMUL  Copyright (C) KingFisher Software 1991-1996");
    printf("===============================================================\n");

    openlog("smugl", LOG_PID, LOG_LOCAL0);

    init_ipc(memory_required()); // Initialise the IPC and shared memory
    load_database(data->shmbase); // Load initial database into shared memory
    init_sockets();             // Prepare the listening socket

    // Now we invoke the manager's main loop.
    // This perpetuates, and only ever returns when it has fork()ed a
    // client process. So when we return from this call, we are a client
    run_the_game();

    // From here on, we are a client
    client_initialise();

    ShowFile("title.text");     // Display title screen
    login();                    // Log the player in
    syslog(LOG_NOTICE, "player logged in: %s", me->name());

    while (exiting == ecFalse)
        {
        prompt(myRank->prompt);
        fetch_input(input, MAX_PHRASE_SIZE);
        sanitise_input();       // Tidy up what we read
        if (input[0] == 0)
            continue;
	if (strncmp(input, "*debug", 6) == 0)
	    {
	    txprintf("- Heavy Debugging is %s\n",
			(heavyDebug = !heavyDebug) ? "on" : "off");
	    continue;
	    }
        if (debug)
            txprintf("You typed: [%s]\n", input);
        parse(input);
        }

    exit(0);
    }

// Describe how to run the program
[[noreturn]]
static void
usage(int code)
    {
    printf("Usage: %s [-d] [-f] [path]\n" \
           "Loads and runs a pre-compiled SMUGL game (see smuglcom)\n" \
           "Options:\n" \
           "  -d       Enable debugging output\n" \
           "  -f       Fork after initialising the game\n" \
           "  path     Specify the game path (default is pwd)\n",
           program);
    exit(code);
    }

// Process the arguments
static void
argue(int argc, char *argv[])
    {
    struct stat sbuf;
    int arg = 0;
    dir[0] = 0;
    // NOTE: This isn't real argv - argv[0] is the first ARGUMENT
    // the program was called with. Don't get confused ;-)
    if (argc >= 1)              // We have some arguments
        {
        if (strcmp(argv[0], "?") == 0)
            usage(0);       // Allow "smugl ?" for usage
        for ( ; arg < argc; arg++ )
            {
            if (argv[arg][0] == '-' || argv[arg][0] == '/')
                {
                switch (argv[arg][1])
                    {
                  case '?':     // Things that look like usage requests
                  case 'h':
                  case 'u':
                        usage(0);
                        /* NOTREACHED */

                  case 'f':     // fork after loading everything
                        fork_on_load = 1;
                        break;

                  case 'd':     // debug mode (incremental)
                        debug++;
                        break;

                  default:
                        usage(0);
                        /* NOTREACHED */
                    }
                }
            else
                {
                if (arg + 1 < argc)
                    {
                    error(LOG_ERR, "Don't understand arguments at: %s %s ...",
                          argv[arg], argv[arg + 1]);
                    usage(1);
                    /* NOTREACHED */
                    }
                strcpy(dir, argv[arg]);
                }
            }
        }
    if (dir[0] == 0)
	getcwd(dir, MAXPATHLEN);
    if (dir[strlen(dir) - 1] != '/')
        strcat(dir, "/");
    if (stat(dir, &sbuf) == -1 || !S_ISDIR(sbuf.st_mode))
        {
        error(LOG_ERR, "Can't access directory %s", dir);
        usage(1);
        }
    }
