// mobiles parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

void
mobmis(const char *field)
{
    GetLogger().errorf("Mobile: %s: Missing '%s' field", mob.id, field);
    skipblock();
}

int
badmobend()
{
    return -1;
}

// Fetch mobile message line
int
getmobmsg(const char *s, const char **p)
{
    const char *q;
    int         n;

    const char *&px = *p;
loop:
    while (isCommentChar(*px)) {
        px = skipline(px);
    }
    if (*px == 0 || *px == 13 || *px == 10) {
        GetLogger().errorf(
                "Mobile: %s: Unexpected end of mobile definition", mob.id);
        return -1;
    }
    px = skipspc(px);
    if (*px == 0 || *px == 13 || *px == 10 || isCommentChar(*px))
        goto loop;

    if ((q = skiplead(s, px)) == px) {
        mobmis(s);
        return -1;
    }
    if (toupper(*q) == 'N') {
        px = skipline(px);
        return -2;
    }
    n = ttumsgchk(q);
    px = skipline(px);
    if (n == -1) {
        GetLogger().errorf("Mobile: %s, malformed '%s' line", mob.id, s);
    }
    return n;
}

// Pass 1: Indexes mobile names

Buffer mobBuffer{};

void
mob_proc1()
{
    const char *p, *cur;
    int32_t     n;

    mobchars = 0;
    fopenw(Resources::Compiled::mobData());
    if (nextc(false) == -1)
        return;

    mobBuffer.open(0L);
    mobdat = static_cast<char *>(mobBuffer.m_data);
    p = mobdat;
    repspc(mobdat);

    do {
        while (*p != 0 && *p != '!')
            p = skipline(p);
        if (*p == 0)
            break;
        p = extractLine(p, block);
        mobchars++;
        cur = getword(block + 1);
        strcpy(mob.id, Word);
        do {
            cur = skipspc(cur);
            if (isLineBreak(*cur))
                break;
            if (skiplead("dead=", &cur)) {
                cur = getword(cur);
                mob.dead = atoi(Word);
                continue;
            }
            if (skiplead("dmove=", &cur)) {
                cur = getword(cur);
                mob.dmove = isroom(Word);
                if (mob.dmove == -1) {
                    GetLogger().errorf(
                            "Mobile: %s: Invalid dmove destination: '%s'",
                            mob.id, Word);
                }
                continue;
            }
        } while (!isLineBreak(*cur) && Word[0] != 0);

        p = extractLine(p, block);
        tidy(block);
        cur = skipspc(block);
        mob.dmove = -1;

        if (!skiplead("speed=", &cur)) {
            mobmis("speed=");
            continue;
        }
        cur = getword(cur);
        mob.speed = atoi(Word);

        if (!skiplead("travel=", &cur)) {
            mobmis("travel=");
            continue;
        }
        cur = getword(cur);
        mob.travel = atoi(Word);

        if (!skiplead("fight=", &cur)) {
            mobmis("speed=");
            continue;
        }
        cur = getword(cur);
        mob.fight = atoi(Word);

        if (!skiplead("act=", &cur)) {
            mobmis("act=");
            continue;
        }
        cur = getword(cur);
        mob.act = atoi(Word);

        if (!skiplead("wait=", &cur)) {
            mobmis("wait=");
            continue;
        }
        cur = getword(cur);
        mob.wait = atoi(Word);

        if (mob.travel + mob.fight + mob.act + mob.wait != 100) {
            GetLogger().errorf(
                    "Mobile: %s, Total of action ratios not equal to 100%",
                    mob.id);
        }
        if (!skiplead("fear=", &cur)) {
            mobmis("fear=");
            continue;
        }
        cur = getword(cur);
        mob.fear = atoi(Word);

        if (!skiplead("attack=", &cur)) {
            mobmis("attack=");
            continue;
        }
        cur = getword(cur);
        mob.attack = atoi(Word);

        if (!skiplead("hitpower=", &cur)) {
            mobmis("hitpower=");
            continue;
        }
        cur = getword(cur);
        mob.hitpower = atoi(Word);

        if ((n = getmobmsg("arrive=", &p)) == -1)
            continue;
        mob.arr = n;
        if ((n = getmobmsg("depart=", &p)) == -1)
            continue;
        mob.dep = n;
        if ((n = getmobmsg("flee=", &p)) == -1)
            continue;
        mob.flee = n;
        if ((n = getmobmsg("strike=", &p)) == -1)
            continue;
        mob.hit = n;
        if ((n = getmobmsg("miss=", &p)) == -1)
            continue;
        mob.miss = n;
        if ((n = getmobmsg("dies=", &p)) == -1)
            continue;
        mob.death = n;

        fwritesafe(mob, ofp1);
    } while (*p != 0);

    GetContext().terminateOnErrors();
    close_ofps();
    if (mobchars != 0) {
        if ((mobp = (struct _MOB_ENT *)OS::Allocate(sizeof(mob) * mobchars)) ==
            NULL) {
            GetLogger().fatal("OUT OF MEMORY");
        }
        fopena(Resources::Compiled::mobData());
        fread((char *)mobp, sizeof(mob), mobchars, afp);
        close_ofps();
    }
}

// Pass 2: Indexes commands mobiles have access to

/*mob_proc2()
{*/
