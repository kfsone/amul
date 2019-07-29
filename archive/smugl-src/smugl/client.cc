// Client-specific routines, etc.

static const char rcsid[] = "$Id: client.cc,v 1.3 1997/05/22 02:21:20 oliver Exp $";

#include "smugl.hpp"
#include "client.hpp"
#include "misc.hpp"
#include "ipc.hpp"
#include "io.hpp"

char slot = -1;                 // Game slot assigned to us

// Initialise the client's environment
void
client_initialise(void)
    {
    // When we reach here, the manager will have a lock on 'sem_DATA'.
    // Until it's finished, we want to wait around, so use a dummy lock.
    sem_lock(sem_DATA);
    // The manager must have finished now
    sem_unlock(sem_DATA);

    // HACK: Because some telnet clients don't work right. We assume
    // the user is telnetting to us, and tell their telnet client
    // how to behave for this session. Icky, really.
    telnet_opt(252, 5);         // Won't status
    telnet_opt(251, 3);         // Will supress go ahead
    telnet_opt(251, 1);         // Will echo
    telnet_opt(254, 1);         // Don't echo
    // I.E. we tell the telnet client:
    //  I don't understand clever telnet stuff (status/go ahead)
    //  I'll echo all the text back, so don't do local echo.
    // Thank you

    txprintf("- Connection %ld successful - Current Session %d.\n",
             data->connections, data->connected);
    }

