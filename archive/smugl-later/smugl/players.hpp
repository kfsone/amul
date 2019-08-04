#ifndef SMUGL_PLAYERS_H
#define SMUGL_PLAYERS_H 1
// player class definitions and function protos

#include "cl_player.hpp"
#include "container.hpp"

extern class Player *userbase;

class Player : public PLAYER
{
  public:
    Player() {}
    ~Player() override = default;

    bool describe() override;

    const char *name();  // Return the player's name
    inline int number() { return (int) (this - userbase); };
    void disconnected();  // Post-mortem
    void reset();         // Reset player values
    unsigned long bitmask;
    void add_name();     // Add name to the vocab database
    void remove_name();  // Remove name from the vocab database
    void init_bob(basic_obj bobno = -1);
    void set_rank(int to);
    basic_obj Location() const;
    bool go_to(basic_obj dest_rm, const char *dep_msg = nullptr, const char *arr_msg = nullptr);
};

class PlayerIdx
{
  public:
    static class Player *locate(char *s);
    static class Player *locate(basic_obj id);
    static class Player *locate_in(basic_obj room, class Player *first = nullptr, long id = -1);
    static class Player *
    locate_others_in(basic_obj room, class Player *first = nullptr, long id = -1);
    static long mask_in_room(basic_obj room);
};

extern void assume_identity(int id);

extern class Player *me;
extern class Player *last_him;
extern class Player *last_her;

#endif  // SMUGL_PLAYERS_H
