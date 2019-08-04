#ifndef ROOMS_H
#define ROOMS_H 1

#include "players.hpp"  ///TODO: Eliminate by fixing arrive/depart/enter etc
#include "structs.hpp"

class Room : public ROOM
{
  public:
    virtual ~Room() {}
    virtual int describe(void) override;
    inline class TTEnt *Tabptr(void);
    void arrive(const char *how = me->arr);
    void depart(const char *how = me->dep);
    void enter(const char *how = me->arr);
    int leave(vocid_t id);
    int leave(class Verb *vb);
    void exits(void);
};

class RoomIdx
{
  private:
    // Current room iteration or, for players, present location.
    int cur_no;

  public:
    RoomIdx(void) { cur_no = 0; }
    static Room *locate(const char *s);
    static Room *locate(vocid_t id);
    Room *first();
    Room *current();
    Room *next();
};

extern Room *cur_loc;
extern Room *last_room;

#endif  // ROOMS_H
