#pragma once
// This may look like C, but it's really -*- C++ -*-
// language (verbs) class definitions and function protos

#include "smugl/rooms.hpp"

class   Verb:public VERB
{
  public:
	bool    describe(void);		// Not really apropriate
};

class   VerbIdx
{
  public:
	static class Verb* locate(char* s);
	static class Verb* locate(vocid_t id);
};

