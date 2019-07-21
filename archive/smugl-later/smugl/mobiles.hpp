#pragma once
// This may look like C, but it's really -*- C++ -*-
// mobile class definitions and function protos

class   Mobile: public MOB_ENT
{
  public:
	bool     describe(void);		// Not really apropriate
	inline class Room *dmoveRm(void);	// What room should we dmove to?
};

class   MobileIdx
{
  public:
	static class Mobile *locate(char *s);
	static class Mobile *locate(long id);
};

