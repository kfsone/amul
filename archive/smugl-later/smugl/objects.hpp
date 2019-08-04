#pragma once

// This may look like C, but it's really -*- C++ -*-
// object and object-state class definitions and function protos

class Object : public OBJ
{
  public:
    bool describe(void);
};

class State : public OBJ_STATE
{
  public:
    bool describe(void);
};

class ObjectIdx
{
  public:
    static class Object *locate(char *s);
    static class Object *locate(long id);
    static class Object *locate_in(basic_obj in, class Object *first = NULL, long id = -1);
};
