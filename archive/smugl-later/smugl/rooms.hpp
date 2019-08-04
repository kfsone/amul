#ifndef SMUGL_SMUGL_ROOMS_H
#define SMUGL_SMUGL_ROOMS_H 1

#include "players.hpp"  ///TODO: Eliminate by fixing arrive/depart/enter etc
#include "structs.hpp"

class Room final : public ROOM
{
  public:
    ~Room() override = default;
    bool describe() override;
    class TTEnt *Tabptr();
    void arrive(const char *how = me->arr);
    void depart(const char *how = me->dep);
    void enter(const char *how = me->arr);
    bool leave(vocid_t id);
    bool leave(class Verb *vb);
    void exits();
};

class RoomIdx final
{
  private:
    // Current room iteration or, for players, present location.
    int cur_no;

  public:
    RoomIdx() { cur_no = 0; }
    static Room *locate(const char *s);
    static Room *locate(vocid_t id);
    Room *first();
    Room *current();
    Room *next();
};

extern Room *cur_loc;
extern Room *last_room;

#endif  // SMUGL_SMUGL_ROOMS_H
