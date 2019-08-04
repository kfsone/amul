/*
 * mobproc.cpp -- process MOBILES.txt
 *
 */

#include <cstring>

#include "errors.hpp"
#include "fileio.hpp"
#include "fperror.hpp"
#include "smuglcom.hpp"

static char *mobpget(const char *s, char *p, msgno_t *n);
static char *getmobmsg(char *p, const char *s, msgno_t *n);

static inline void
mobmis(const char *s)
{  // Report missing mobile fields
    error("%s: Missing %s field.\n", mob.id, s);
}

/* For the mobile-message index table:
** Nasty Hack; for each mobile message label, we specify the label
** and a pointer to it's accompanying position with the 'mob' variable.
** This isn't very nice */
struct MOBMSGS {
    const char *msg;  // The label text
    msgno_t *into;    // The pointer within 'mob' for the value
} * mmsgd;

// The mobile-message index table
struct MOBMSGS mmsgdata[] = { { "arrive=", &mob.arr }, { "depart=", &mob.dep },
                              { "flee=", &mob.flee },  { "strike=", &mob.hit },
                              { "miss=", &mob.miss },  { "dies=", &mob.death },
                              { nullptr, nullptr } };

void
mob_proc1()
{
    char *p, *s;
    msgno_t n;

    mobchars = 0;
    fopenw(mobfn);
    if (nextc(0) == -1) {
        tx("<No Entries>");
        errabort();
        return;
    }

    mobdat = p = cleanget();

    do {
        do
            p = skipline(s = p);
        while (*s != '!' && *s);
        if (!*s)
            continue;
        mobchars++;
        s = getword(s);
        mob.id = new_word(Word, true);
        mob.dmove = -1;
        mob.dead = 1;
        do {
            if (!*s)
                break;
            if (strncmp(s, "dead=", 5) == 0) {
                s = getword(s + 5);
                mob.dead = atoi(Word);
                continue;
            }
            if (strncmp(s, "dmove=", 6) == 0) {
                s = getword(s + 6);
                mob.dmove = is_container(Word);
                if (mob.dmove == -1)
                    error("%s: invalid DMove '%s'.\n", word(mob.id), Word);
                continue;
            }
        } while (*s);

        do {
            p = skipline(s = p);
            if (!*s) {
                error("%s: Unexpected end of mobile!\n", mob.id);
                continue;
            }
        } while (!*(s = skipspc(s)));

        if (!(s = mobpget("speed=", s, &n)))
            goto end;
        mob.speed = n;
        if (!(s = mobpget("travel=", s, &n)))
            goto end;
        mob.travel = n;
        if (!(s = mobpget("fight=", s, &n)))
            goto end;
        mob.fight = n;
        if (!(s = mobpget("act=", s, &n)))
            goto end;
        mob.act = n;
        if (!(s = mobpget("wait=", s, &n)))
            goto end;
        mob.wait = n;
        if (mob.travel + mob.fight + mob.act + mob.wait != 100) {
            warne("%s: Travel+Fight+Act+Wait don't add to 100%! Please check!\n", mob.id);
        }

        if (!(s = mobpget("fear=", s, &n)))
            goto end;
        mob.fear = n;
        if (!(s = mobpget("attack=", s, &n)))
            goto end;
        mob.attack = n;
        if (!(s = mobpget("hitpower=", s, &n)))
            goto end;
        mob.hitpower = n;

        for (mmsgd = &mmsgdata[0]; mmsgd->msg; mmsgd++)
            p = getmobmsg(p, mmsgd->msg, mmsgd->into);

        fwrite((char *) &mob, sizeof(mob), 1, ofp1);
    end:
        if (!s)
            p = skipdata(p);
    } while (*p);

    errabort();  // Abort if an error
    if (mobchars) {
        mobp = (struct MOB_ENT *) grow(nullptr, sizeof(*mobp) * mobchars, "Reading Mobile Table");
        fopena(mobfn);
        static const size_t bytes = sizeof(mob) * mobchars;
        if (fread((char *) mobp, bytes, 1, afp) < bytes)
            throw Smugl::FPReadError(mobfn, errno, afp);
        close_ofps();
    }
}

static char *
mobpget(const char *s, char *p, msgno_t *n)
{  // Get a mobile's percentage value
    p = getword(skiplead(s, p));
    if (!*Word) {
        mobmis(s);
        return nullptr;
    }
    *n = atoi(Word);
    return p;
}

static char *
getmobmsg(char *p, const char *s, msgno_t *n)
{  // Fetch a mobile message line
    char *q = nullptr;
    *n = -1;

    // We have to allow that there might be some 'empty' lines
    // XXX: Do we really, I thought clean up, etc, sorted all that?
    do {
        p = skipline(q = p);
        if (!*q) {
            mobmis(s);
            return p;
        }
        q = skipspc(q);
    } while (!*q);

    q = skiplead(s, q);
    if (*q == '\'' || *q == '\"') {
        strcpy(g_block, q);
        char *copy = g_block + 1;
        while (*copy && *copy != g_block[0])
            copy++;
        *(copy++) = 0;
    }
    if ((*n = ttumsgchk(q)) == -1)
        error("%s: Bad text on '%s' line!\n", mob.id, s);
    return p;
}
