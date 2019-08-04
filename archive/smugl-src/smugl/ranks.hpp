#ifndef SMUGL_SMUGL_RANKS_H
#define SMUGL_SMUGL_RANKS_H

#include "players.hpp"
#include "structs.hpp"

class Rank : public RANKS
{
  public:
    int describe(Gender sex);  // Not really apropriate
    void detail(void);
    char *copy(char *dest, Gender sex = me->sex, int verbose = TRUE);
    long number();
};

class RankIdx
{
  public:
    static Rank *ptr(long id);
    static Rank *top_rank();
};

extern class Rank *myRank;

#endif
