// This may look like C, but it's really -*- C++ -*-
// $Id: ranks.hpp,v 1.7 1997/05/22 02:21:28 oliver Exp $
// rank class definitions and function protos

class Rank : public RANKS
{
  public:
    int describe(Gender sex);  // Not really apropriate
    void detail(void);
    char *copy(char *dest, Gender sex = me->sex, int verbose = TRUE);
    inline long number(void) { return (int) (this - data->rankbase); };
};

class RankIdx
{
  public:
    static inline class Rank *ptr(long id) { return &data->rankbase[id]; };
    static inline class Rank *top_rank(void) { return ptr(data->ranks - 1); };
};

extern class Rank *myRank;
