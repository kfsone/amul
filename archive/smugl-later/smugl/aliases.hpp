#ifndef SMUGL_SMUGL_ALIASES_H
#define SMUGL_SMUGL_ALIASES_H

#include "structs.hpp"
#include "typedefs.hpp"

class Alias : public ALIAS
{
  public:
    static vocid_t locate(string *s);
    static vocid_t locate(vocid_t id);
    static vocid_t meaning(vocid_t num);
};

#endif
