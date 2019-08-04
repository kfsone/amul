#pragma once
// This may look like C, but it's really -*- C++ -*-
// rooms class definitions and function protos

class Room : public ROOM
{
  public:
    virtual bool describe(void);
    inline class TTEnt *Tabptr(void);
    void arrive(const char *how = me->arr);
    void depart(const char *how = me->dep);
    void enter(const char *how = me->arr);
    bool leave(vocid_t id);
    bool leave(class Verb *vb);
    void exits(void);
};

class RoomIdx
{
  private:
    // Current room iteration or, for players, present location.
    static int cur_no;

  public:
    inline RoomIdx(void) { cur_no = 0; }
    static class Room *locate(const char *s);
    static class Room *locate(vocid_t id);
    static inline class Room *first(void)
    {
        cur_no = 0;
        return data->roombase;
    };
    static inline class Room *current(void)
    {
        if (cur_no >= data->rooms)
            return NULL;
        return data->roombase + cur_no;
    };
    static inline class Room *next(void)
    {
        if (++cur_no >= data->rooms) {
            cur_no = data->rooms;
            return NULL;
        }
        return data->roombase + cur_no;
    };
};

extern class Room *cur_loc;
extern class Room *last_room;
