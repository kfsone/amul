#pragma once
// This may look like C, but it's really -*- C++ -*-
// travel class definitions and function protos

class   TTEnt:public TT_ENT
{
  public:
	bool     describe(void);
};

class   TTIdx
{
  public:
	static class TTEnt *locate(char *s);
	static class TTEnt *locate(long id);
};

