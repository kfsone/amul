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
#include <iostream>

#include "h/amul.gcfg.h"
#include "h/amul.type.h"

thread_local atomic_bool g_terminate { false };

void
temporary_main()
{
    // This is just a mock main function to explore msgport functionality.
    std::cout << "Press Enter to exit.\n";
    std::cout << GetRank(0).prompt << std::flush;

    char s[16];
    std::cin >> s; 

    g_terminate = true;
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
amul_main()
{
    clientTearUp();

    // entry point for the client
    temporary_main();

    clientTearDown();
}