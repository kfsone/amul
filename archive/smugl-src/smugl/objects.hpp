// This may look like C, but it's really -*- C++ -*-
// $Id: objects.hpp,v 1.7 1997/05/22 02:21:27 oliver Exp $
// object and object-state class definitions and function protos

#ifndef OBJECTS_H
#define OBJECTS_H 1

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
