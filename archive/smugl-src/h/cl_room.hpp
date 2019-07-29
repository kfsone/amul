// This may look like C, but it's really -*- C++ -*-
// $Id: cl_room.hpp,v 1.1 1999/06/08 15:36:45 oliver Exp $
//
////////////////////////////// ROOM STRUCTURE
//

#ifndef ROOM_H
#define ROOM_H 1

#include "cl_basicobj.hpp"

class ROOM : public BASIC_OBJ
    {				// Room def struct
public:
	virtual ~ROOM() {}
    //// Room::FUNCTIONS
    virtual int describe(void) override { return 0; };
    virtual int describe_verbose(void) override { return 0; };
    virtual int Write(FILE *) override;
    virtual int Read(FILE *) override;

    //// Room::DATA
    u_long visitor_bf;          // Bit field - which users have visited?
    long tabptr;                // Travel Table data Offset (yeuch)
    short ttlines;              // No. of TT lines
    };

#endif /* ROOM_H */
