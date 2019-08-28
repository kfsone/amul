/* Daemon processing bizness! */

// Start a private (player owned) demon
void
dpstart(int d, int seconds)
{
	dstart(d, seconds, MDSTART);
}

// Start a global demon
void
dgstart(int d, int seconds)
{
	dstart(d, seconds, MGDSTART);
}

// Start a demon
void
dstart(int d, int seconds, int type)
{
	// Immediate?
    if (seconds == 0) {
        long v = iverb;
        long lv = lverb;
        long ld = ldir;
        long lr = lroom;
        long a1 = iadj1;
        long a2 = iadj2;
        long n1 = inoun1;
        long n2 = inoun2;
        long pp = iprep;
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
        amanp->p1 = inoun1;
        amanp->p2 = inoun2;
        amanp->p3 = wtype[2];
        amanp->p4 = wtype[5];
        SendIt(t, d, (char *)c); /* Inform AMAN... */
    }
}

// Force a demon to execute
void
dbegin(int d)
{
}

void
dshow(int d)
{
    SendIt(MCHECKD, d, NULL);
    if (amul->data == -1) {
        tx("eventually");
        return;
    }
    timeto(block, amul->p1);
    tx(block);
}

// Dispatch a demon to the manager
void
dsend(int p, int d, int c)
{
    if (p == amul->from)
        dpstart(d, c);
    sendex(p, ASTART, d, c, 0, 0); /* Tell THEM to start the daemon */
}
