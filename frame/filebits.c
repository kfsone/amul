/* What to do when ^C pressed */ 
int
CXBRK()
{
	return 0;
}

void
memfail(char *s)
{
    txs("** Unable to allocate memory for %s! **\n", s);
    quit();
}

void
givebackmemory()
{
    if (repbk != NULL)
        DeletePort(repbk);
    if (reply != NULL)
        DeletePort(reply);
    ReleaseMem(&amul);
    if (amanrep != NULL)
        DeletePort(amanrep);
    ReleaseMem(&amanp);
    ReleaseMem(&ob);
    ReleaseMem(&ow);
    ReleaseMem(&input);
    if (serio != NULL)
        DeleteExtIO((struct IORequest *)serio);
    if (wserio != NULL)
        DeleteExtIO((struct IORequest *)wserio);
}

void
fopenr(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block, "rea");
        tx(spc);
        quit();
    }
}

void
fopena(char *s)
{
    if (afp != NULL)
        fclose(afp);
    sprintf(block, "%s%s", dir, s);
    if ((afp = fopen(block, "rb+")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block, "appen");
        tx(spc);
        quit();
    }
}

void
close_ofps()
{
    if (ofp1 != NULL)
        fclose(ofp1);
    if (ofp2 != NULL)
        fclose(ofp2);
    if (ofp3 != NULL)
        fclose(ofp3);
    ofp1 = ofp2 = ofp3 = NULL;
}

void
kquit(char *s)
{
    sprintf(block, "@me just dropped carrier.\n");
    action(ow, AOTHERS);
    quit();
}

void
quit()
{
    txs("\n%s exiting.\n\n", vername);
    scrend(); /* custom screen */
    if (link > 0 && amul->from > -1)
        SendIt(MDISCNCT, 0, me->name);
    if (ifp != NULL)
        fclose(ifp);
    if (afp != NULL)
        fclose(afp);
    close_ofps();
    givebackmemory();
    exit(0);
}

void
SendIt(int t, int d, char *p)
{
    if (link == 0)
        return;
    AMt = t;
    AMd = d;
    AMp = p;
    PutMsg(port, (struct Message *)amanp);
    WaitPort(amanrep);
    GetMsg(amanrep);
    Af = AMf;
    Ad = AMd;
    Ap = AMp;
    At = AMt;
    Ap1 = Apx1;
    Ap2 = Apx2;
    Ap3 = Apx3;
    Ap4 = Apx4;
}

void
pressret()
{
    sys(RETURN);
    Inp(block, 0);
    int l = strlen(umsgp + *(umsgip + RETURN));
    while (l > 0) {
        txc(8);
        txc(32);
        txc(8);
        l--;
    }
}

void
sys(int n)
{
	tx(umsgp + *(umsgip + n));
}

void
crsys(int n)
{
    txc('\n');
    tx(umsgp + *(umsgip + n));
}

void
timeto(char *s, long secs)
{
    if (secs >= 3600) /* More than an hour */
    {
        int x = secs / 3600;       /* Hours */
        int y = secs - (x * 3600); /* Minutes & seconds */
        if (y < 60)            /* Upto 1 minute? */
        {
            sprintf(s, "%ld %s, %ld %s", x, (x > 1) ? "hours" : "hour", y,
                    (y > 1) ? "seconds" : "second");
            return;
        }
        y = y / 60;
        sprintf(s, "%ld %s and %ld %s", x, (x > 1) ? "hours" : "hour", y,
                (y > 1) ? "minutes" : "minute");
        return;
    }
    int x = secs / 60;
    int y = secs - (x * 60);
    if (x == 0 && y == 0)
        strcpy(s, "now");
    if (x != 0 && y == 0)
        sprintf(s, "%ld %s", x, (x > 1) ? "minutes" : "minute");
    if (x == 0 && y != 0)
        sprintf(s, "%ld %s", y, (y > 1) ? "seconds" : "second");
    if (x != 0 && y != 0)
        sprintf(s, "%ld %s and %ld %s", x, (x > 1) ? "minutes" : "minute", y,
                (y > 1) ? "seconds" : "second");
}
