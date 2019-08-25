
/* Daemon processing bizness! */

// Start a private (player owned) demon
dpstart(int d, int c) { dstart(d, c, MDSTART); }

// Start a global demon
dgstart(int d, int c) { dstart(d, c, MGDSTART); }

// Start a demon
dstart(int d, int c, int t)
{
    if (c == 0) {
        long lv, ld, lr, v, a1, n1, pp, a2, n2;
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
        SendIt(t, d, (char *)c); /* Inform AMAN... */
    }
}

// For a demon to execute
dbegin(int d)
{
}

dshow(int d)
{
    SendIt(MCHECKD, d, NULL);
    if (Ad == -1) {
        tx("eventually");
        return;
    }
    timeto(block, Ap1);
    tx(block);
}

// Dispatch a demon to the manager
dsend(int p, int d, int c)
{
    if (p == Af)
        dpstart(d, c);
    sendex(p, ASTART, d, c, 0, 0); /* Tell THEM to start the daemon */
}
