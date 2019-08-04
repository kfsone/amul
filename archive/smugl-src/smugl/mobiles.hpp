// This may look like C, but it's really -*- C++ -*-
// $Id: mobiles.hpp,v 1.5 1997/05/22 02:21:26 oliver Exp $
// mobile class definitions and function protos

#ifndef MOBILES_H
#define MOBILES_H 1

#include "rooms.hpp"

class Mobile : public MOB_ENT
{
  public:
    int describe(void);                // Not really apropriate
    inline class Room *dmoveRm(void);  // What room should we dmove to?
};

class MobileIdx
{
  public:
    static class Mobile *locate(char *s);
    static class Mobile *locate(long id);
};

#endif  // MOBILES_H
