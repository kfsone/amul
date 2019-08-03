// This may look like C, but it's really -*- C++ -*-
// $Id: travel.hpp,v 1.5 1997/05/22 02:21:33 oliver Exp $
// travel class definitions and function protos

#ifndef TRAVEL_H
#define TRAVEL_H 1

class TTEnt : public TT_ENT
{
  public:
    int describe(void);
};

class TTIdx
{
  public:
    static class TTEnt *locate(char *s);
    static class TTEnt *locate(long id);
};

#endif /* Travel_H */
