#ifndef OBJECTS_H
#define OBJECTS_H 1

#include "cl_object.hpp"
#include "structs.hpp"

class Object : public OBJ
{
  public:
    int describe(void);
};

class State : public OBJ_STATE
{
  public:
    int describe(void);
};

class ObjectIdx
{
  public:
    static class Object *locate(char *s);
    static class Object *locate(long id);
    static class Object *locate_in(basic_obj in, class Object *first = NULL, long id = -1);
};

#endif
