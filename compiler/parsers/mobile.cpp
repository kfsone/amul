// mobiles parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

void
mobmis(char *field)
{
    GetLogger().errorf("Mobile: %s: Missing '%s' field", mob.id, field);
    skipblock();
}

int
badmobend()
{
    return -1;
}

const char *px;

// Fetch mobile message line
int
getmobmsg(char *s)
{
    const char *q;
    int   n;

loop:
    while (isCommentChar(*px)) {
        px = skipline(px);
    }
    if (*px == 0 || *px == 13 || *px == 10) {
        GetLogger().errorf("Mobile: %s: Unexpected end of mobile definition", mob.id);
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

void
mob_proc1()
{
    const char *  p, *s1, *s2;
    int32_t n;

    mobchars = 0;
    fopenw(Resources::Compiled::mobData());
    if (nextc(0) == -1)
        return;

    blkget(&moblen, &mobdat, 0L);
    p = mobdat;
    repspc(mobdat);

    do {
        while (*p != 0 && *p != '!')
            p = skipline(p);
        if (*p == 0)
            break;
        p = sgetl(p, block);
        mobchars++;
        s1 = getword(block + 1);
        strcpy(mob.id, Word);
        do {
            s1 = skipspc(s1);
            if (*s1 == 0 || *s1 == ';')
                break;
            if ((s2 = skiplead("dead=", s1)) != s1) {
                s1 = getword(s2);
                mob.dead = atoi(Word);
                continue;
            }
            if ((s2 = skiplead("dmove=", s1)) != s1) {
                s1 = getword(s2);
                mob.dmove = isroom(Word);
                if (mob.dmove == -1) {
                    GetLogger().errorf("Mobile: %s: Invalid dmove destination: '%s'", mob.id, Word);
                }
                continue;
            }
        } while (*s1 != 0 && *s1 != ';' && Word[0] != 0);

        p = sgetl(p, block);
        tidy(block);
        s1 = block;
        mob.dmove = -1;

        if ((s2 = skiplead("speed=", s1)) == s1) {
            mobmis("speed=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.speed = atoi(Word);
        if ((s2 = skiplead("travel=", s1)) == s1) {
            mobmis("travel=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.travel = atoi(Word);
        if ((s2 = skiplead("fight=", s1)) == s1) {
            mobmis("speed=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.fight = atoi(Word);
        if ((s2 = skiplead("act=", s1)) == s1) {
            mobmis("act=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.act = atoi(Word);
        if ((s2 = skiplead("wait=", s1)) == s1) {
            mobmis("wait=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.wait = atoi(Word);
        if (mob.travel + mob.fight + mob.act + mob.wait != 100) {
            GetLogger().errorf("Mobile: %s, Total of action ratios not equal to 100%", mob.id);
        }
        if ((s2 = skiplead("fear=", s1)) == s1) {
            mobmis("fear=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.fear = atoi(Word);
        if ((s2 = skiplead("attack=", s1)) == s1) {
            mobmis("attack=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.attack = atoi(Word);
        if ((s2 = skiplead("hitpower=", s1)) == s1) {
            mobmis("hitpower=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.hitpower = atoi(Word);

        if ((n = getmobmsg("arrive=")) == -1)
            continue;
        mob.arr = n;
        if ((n = getmobmsg("depart=")) == -1)
            continue;
        mob.dep = n;
        if ((n = getmobmsg("flee=")) == -1)
            continue;
        mob.flee = n;
        if ((n = getmobmsg("strike=")) == -1)
            continue;
        mob.hit = n;
        if ((n = getmobmsg("miss=")) == -1)
            continue;
        mob.miss = n;
        if ((n = getmobmsg("dies=")) == -1)
            continue;
        mob.death = n;
        p = px;

        fwrite(mob.id, sizeof(mob), 1, ofp1);
    } while (*p != 0);

    GetContext().terminateOnErrors();
    close_ofps();
    if (mobchars != 0) {
        if ((mobp = (struct _MOB_ENT *)OS::Allocate(sizeof(mob) * mobchars)) == NULL) {
            GetLogger().fatal("OUT OF MEMORY");
        }
        fopena(Resources::Compiled::mobData());
        fread((char *)mobp, sizeof(mob) * mobchars, 1, afp);
        close_ofps();
    }
}

// Pass 2: Indexes commands mobiles have access to

/*mob_proc2()
{*/
