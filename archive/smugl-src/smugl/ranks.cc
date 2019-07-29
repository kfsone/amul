// Definition of rank classes and functions

static const char rcsid[] = "$Id: ranks.cc,v 1.7 1999/06/08 15:36:50 oliver Exp $";

#define	RANKS_C	1

#include "smugl.hpp"
#include "data.hpp"

#include "ranks.hpp"

//class RankIdx RankIdx;
class Rank *myRank;

int
Rank::describe(Gender sex)
    {
    if (sex == FEMALE)
        tx(female);
    else tx(male);
    return TRUE;
    }

void
Rank::detail(void)
    {
    txprintf("Rank#%d\n male=%s\n female=%s\n strength=%d\n stamina=%d",
             number(), male, female, strength, stamina);
    txprintf(" dext=%d\n wisdom=%d\n experience=%d\n magicpts=%d\n",
             dext, wisdom, experience, magicpts);
    txprintf(" numobj=%d\n tasks=%d\n score=%ld\n maxweight=%ld\n",
             numobj, tasks, score, maxweight);
    txprintf(" minpksl=%ld\n prompt=%s\n",
             minpksl, (prompt < 0) ? "(none)" : message(prompt));
//    long minpksl;		// Min. pts for killin
    }

char *
Rank::copy(char *dest, Gender sex, int verbose)
    {
    if (verbose && me->pre[0])
        {
        dest = strcopy(dest, me->pre);
        *(dest++) = ' ';
        }
    dest = strcopy(dest, (sex == MALE) ? male : female);
    if (verbose && me->post[0])
        {
        *(dest++) = ' ';
        strcopy(dest, me->post);
        }
    return dest;
    }
