#ifndef SMUGL_BOBIDX_H
#define SMUGL_BOBIDX_H 1

#include "defines.hpp"

class BobIdx
{
  public:
    // Locate an object of a given type
    static basic_obj find(vocid_t name, char type = WANY, basic_obj from = -1);
};

#endif
