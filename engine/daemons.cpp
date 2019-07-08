// Daemon processing functions

#include "amulinc.h"

int32_t minop;  // Mobiles in operation

// Daemon processing host
void
Daem_Proc()
{
    int32_t lastt;
    int     i, ded, next;

    // Setup Mobile bits & pieces
    // Whats the highest travel verb number?
    for (i = 0; i < verbs; i++)
        if (!(vbptr->flags & VB_TRAVEL))
            lastt = i;

    do {
        Amiga::Wait(-1);
        iocheck();
    } while (1 == 1);
}

// Special Processor core
void
Special_Proc()
{
    int i;

    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    wtype[0] = wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = TC_NONE;
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = last_him = last_her = it = -1;
    switch (MyFlag)  // What type of processor ?
    {
    case ROLE_DAEMON:  // Execute the boot-up daemon
        if ((i = isverb("\"boot")) != -1)
            lang_proc(i, 0);
        Daem_Proc();  // Daemon Processor
    case ROLE_NPCS: printf("-- Mobile processor loaded\n");
    default: printf("-- Unsupported special processor requested\n");
    }
    quit();  // Don't go anywhere
}

// Begin PRIVATE daemon
void
dpstart(int d, int c)
{
    dstart(d, c, MSG_DAEMON_START);
}

// Begin GLOBAL daemon
void
dgstart(int d, int c)
{
    dstart(d, c, MSG_GDAEMON_START);
}

void
dstart(int d, int c, int t)
{
    if (c == 0) {
        int32_t lv, ld, lr, v, a1, n1, pp, a2, n2;
        v = iverb;
        lv = lverb;
        ld = ldir;
        lr = lroom;
        a1 = iadj1;
        a2 = iadj2;
        n1 = inoun1;
        n2 = inoun2;
        pp = iprep;
        lang_proc(d, 0);
        iverb = v;
        lverb = lv;
        ldir = ld;
        lroom = lr;
        iadj1 = a1;
        iadj2 = a2;
        inoun1 = n1;
        inoun2 = n2;
        iprep = pp;
    } else {
        Apx1 = inoun1;
        Apx2 = inoun2;
        Apx3 = wtype[2];
        Apx4 = wtype[5];
        SendIt(t, d, (char *)c);  // Inform AMAN...
    }
}

// Force daemon to happen
void
dbegin(int d)
{
}

void
dshow(int d)
{
    SendIt(MSG_DAEMON_STATUS, d, NULL);
    if (Ad == -1) {
        tx("eventually");
        return;
    }
    timeto(block, Ap1);
    tx(block);
}

// Send a daemon
void
dsend(int p, int d, int c)
{
    if (p == Af)
        dpstart(d, c);
    sendex(p, ASTART, d, c, 0, 0);  // Tell THEM to start the daemon
}
