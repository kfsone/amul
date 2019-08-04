#ifndef SMUGL_H_CL_ROOM_H
#define SMUGL_H_CL_ROOM_H 1

////////////////////////////// ROOM STRUCTURE

#include "cl_basicobj.hpp"

class ROOM : public BASIC_OBJ
{  // Room def struct
  public:
    ~ROOM() override = default;
    //// Room::FUNCTIONS
    bool describe() override { return false; };
    bool describe_verbose() override { return false; };
    int Write(FILE *) override;
    int Read(FILE *) override;

    //// Room::DATA
    flag_t visitor_bf;  // Bit field - which users have visited?
    long tabptr;        // Travel Table data Offset
    uint16_t ttlines;   // No. of TT lines
};

#endif  // SMUGL_H_CL_ROOM_H
