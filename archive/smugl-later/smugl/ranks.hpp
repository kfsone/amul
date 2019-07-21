#pragma once
// This may look like C, but it's really -*- C++ -*-
// rank class definitions and function protos

class   Rank:public RANKS
{
  public:
	bool     describe(Gender sex);	// Not really apropriate
	void    detail(void);
	char   *copy(char *dest, Gender sex = me->sex, bool verbose = true);
	inline long number(void)
	{
		return (int) (this - data->rankbase);
	};
};

class   RankIdx
{
  public:
	static inline class Rank *ptr(long id)
	{
		return &data->rankbase[id];
	};
	static inline class Rank *top_rank(void)
	{
		return ptr(data->ranks - 1);
	};
};

extern class Rank *myRank;
