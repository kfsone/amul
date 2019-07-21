#pragma once
// This may look like C, but it's really -*- C++ -*-
//
////////////////////////////// ROOM STRUCTURE

#include "include/cl_basicobj.hpp"

class ROOM : public BASIC_OBJ
    {				// Room def struct
public:
    //// Room::FUNCTIONS
    virtual bool describe(void) { return 0; };
    virtual bool describe_verbose(void) { return 0; };
    virtual int Write(FILE *);
    virtual int Read(FILE *);

    //// Room::DATA
    u_long visitor_bf;          // Bit field - which users have visited?
    long tabptr;                // Travel Table data Offset (yeuch)
    short ttlines;              // No. of TT lines
    };

