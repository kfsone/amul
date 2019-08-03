// This may look like C, but it's really -*- C++ -*-
// $Id: cl_object.hpp,v 1.1 1999/06/08 15:36:45 oliver Exp $
//
////////////////////////////// OBJECT STRUCTURE
//

#ifndef OBJ_H
#define OBJ_H 1
#include "cl_basicobj.hpp"

class OBJ : public BASIC_OBJ
{  // Object (temporary) definition
  public:
    virtual ~OBJ() {}

    //// Object::FUNCTIONS
    virtual int describe(void) override { return 0; };
    virtual int describe_verbose(void) override { return 0; };
    virtual int Write(FILE *) override;
    virtual int Read(FILE *) override;

    //// Object::DATA
    char putto;           // Where things go
    char article;         // 'a'? 'an'?
    short nstates;        // No. of states
    short mobile;         // Mobile character
    class State *states;  // Ptr to states!
};

#endif /* OBJ_H */
