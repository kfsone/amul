#ifndef SMUGL_SMUGL_OBJECTS_H
#define SMUGL_SMUGL_OBJECTS_H

#include "cl_object.hpp"
#include "structs.hpp"

class Object : public OBJ
{
  public:
    bool describe() override;
};

class State : public OBJ_STATE
{
  public:
    bool describe();
};

class ObjectIdx
{
  public:
    static Object *locate(char *s);
    static Object *locate(long id);
    static Object *locate_in(basic_obj in, class Object *first = NULL, long id = -1);
};

#endif  // SMUGL_SMUGL_OBJECTS_H
