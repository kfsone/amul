// Definition of rank classes and functions
#include "data.hpp"
#include "smugl.hpp"

#include "ranks.hpp"

// class RankIdx RankIdx;
class Rank *myRank;

long
Rank::number()
{
    return (this - data->rankbase);
}

bool
Rank::describe(Gender sex)
{
    if (sex == FEMALE)
        tx(female);
    else
        tx(male);
    return true;
}

void
Rank::detail()
{
    txprintf("Rank#%d\n male=%s\n female=%s\n strength=%d\n stamina=%d",
             number(),
             male,
             female,
             strength,
             stamina);
    txprintf(" dext=%d\n wisdom=%d\n experience=%d\n magicpts=%d\n",
             dext,
             wisdom,
             experience,
             magicpts);
    txprintf(
            " numobj=%d\n tasks=%d\n score=%ld\n maxweight=%ld\n", numobj, tasks, score, maxweight);
    txprintf(" minpksl=%ld\n prompt=%s\n", minpksl, (prompt < 0) ? "(none)" : message(prompt));
    //    long minpksl;		// Min. pts for killin
}

char *
Rank::copy(char *dest, Gender sex /*=me->sex*/, bool verbose /*=true*/)
{
    if (verbose && me->pre[0]) {
        dest = strcopy(dest, me->pre);
        *(dest++) = ' ';
    }
    dest = strcopy(dest, (sex == MALE) ? male : female);
    if (verbose && me->post[0]) {
        *(dest++) = ' ';
        strcopy(dest, me->post);
    }
    return dest;
}

Rank *
RankIdx::ptr(long id)
{
    return &data->rankbase[id];
}

Rank *
RankIdx::top_rank()
{
    return ptr(data->ranks - 1);
}