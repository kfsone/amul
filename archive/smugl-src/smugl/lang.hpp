// This may look like C, but it's really -*- C++ -*-
// $Id: lang.hpp,v 1.5 1997/05/22 02:21:24 oliver Exp $
// language (verbs) class definitions and function protos

#ifndef LANG_H
#define LANG_H 1

#include "rooms.hpp"

class Verb : public VERB
    {
public:
    int describe(void);             // Not really apropriate
    };

class VerbIdx
    {
public:
    static class Verb *locate(char *s);
    static class Verb *locate(long id);
    };

#endif /* LANG_H */
