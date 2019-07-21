#pragma once
// This may look like C, but it's really -*- C++ -*-
//
////////////////////////////// OBJECT STRUCTURE

#include "include/cl_basicobj.hpp"

class OBJ : public BASIC_OBJ
    {				// Object (temporary) definition
public:
    //// Object::FUNCTIONS
    virtual bool describe(void) { return 0; };
    virtual bool describe_verbose(void) { return 0; };
    virtual int Write(FILE *);
    virtual int Read(FILE *);

    //// Object::DATA
    char putto;                 // Where things go
    char article;               // 'a'? 'an'?
    short nstates;              // No. of states
    short mobile;               // Mobile character
    class State *states;        // Ptr to states!
    };

