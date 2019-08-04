#ifndef SMUGL_SMUGL_LANG_H
#define SMUGL_SMUGL_LANG_H

#include "rooms.hpp"

class Verb : public VERB
{
  public:
    bool describe();  // Not really apropriate
};

class VerbIdx
{
  public:
    static Verb *locate(char *s);
    static Verb *locate(vocid_t id);
};

#endif  // SMUGL_SMUGL_LANG_H
