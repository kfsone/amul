#ifndef OBJ_H
#define OBJ_H 1
#include "cl_basicobj.hpp"
////////////////////////////// OBJECT STRUCTURE

class OBJ : public BASIC_OBJ
{  // Object (temporary) definition
  public:
    ~OBJ() override = default;

    //// Object::FUNCTIONS
    bool describe() override { return false; };
    bool describe_verbose() override { return false; };
    int Write(FILE *) override;
    int Read(FILE *) override;

    //// Object::DATA
    char putto;           // Where things go
    char article;         // 'a'? 'an'?
    short nstates;        // No. of states
    short mobile;         // Mobile character
    class State *states;  // Ptr to states!
};

#endif  // OBJ_H
