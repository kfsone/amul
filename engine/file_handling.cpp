#include "amulinc.h"

// What to do when ^C pressed

int
CXBRK()
{
    return 0;
}

void
memfail(const char *s)
{
    txs("** Unable to allocate memory for %s! **\n", s);
    quit();
}

void
givebackmemory()
{
    if (repbk != NULL)
        Amiga::DeletePort(repbk);
    if (replyPort != NULL)
        Amiga::DeletePort(replyPort);
    if (amul != NULL)
        OS::Free((char *)amul, (int32_t)sizeof(*amul));
    if (amanrep != NULL)
        Amiga::DeletePort(amanrep);
    if (amanp != NULL)
        OS::Free((char *)amanp, (int32_t)sizeof(*amanp));
    if (ob != NULL)
        OS::Free(ob, 5000);
    if (ow != NULL)
        OS::Free(ow, 3000);
    if (input != NULL)
        OS::Free(input, 400);
}

void
fopenr(const char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block,
                "rea");
        tx(spc);
        quit();
    }
}

void
fopena(const char *s)
{
    if (afp != NULL)
        fclose(afp);
    sprintf(block, "%s%s", dir, s);
    if ((afp = fopen(block, "rb+")) == NULL) {
        sprintf(spc, "\x07** Error: Can't open \"%s\" for %sding!\n\n", block,
                "appen");
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
    if (link > 0 && amul->from > -1)
        SendIt(MSG_DISCONNECT, 0, me->name);
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
    Amiga::PutMsg(amanPort, amanp);
    Amiga::WaitPort(amanrep);
    Amiga::GetMsg(amanrep);
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
    int l;
    sys(RETURN);
    Inp(block, 0);
    l = strlen(umsgp + *(umsgip + RETURN));
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
timeto(char *s, int32_t secs)
{
    int x, y;

    if (secs >= 3600)  // More than an hour
    {
        x = secs / 3600;        // Hours
        y = secs - (x * 3600);  // Minutes & seconds
        if (y < 60)             // Upto 1 minute?
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
    x = secs / 60;
    y = secs - (x * 60);
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
