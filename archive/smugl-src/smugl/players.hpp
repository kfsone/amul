// This may look like C, but it's really -*- C++ -*-
// $Id: players.hpp,v 1.12 1999/06/08 15:36:50 oliver Exp $
// player class definitions and function protos

#ifndef PLAYERS_H
#define PLAYERS_H 1

extern class Player *userbase;

class Player : public PLAYER
    {
public:
    virtual int describe(void);

    const char *name(void);     // Return the player's name
    inline int number(void) { return (int) (this - userbase); };
    void disconnected(void);    // Post-mortem
    void reset(void);           // Reset player values
    unsigned long bitmask;
    void add_name(void);        // Add name to the vocab database
    void remove_name(void);     // Remove name from the vocab database
    void init_bob(basic_obj bobno=-1);
    void set_rank(int to);
    inline basic_obj Location(void) const { return containers[conLocation].boContainer; };
    int go_to(basic_obj dest_rm, const char *dep_msg=NULL, const char *arr_msg=NULL);
    };

class PlayerIdx
    {
public:
    static class Player *locate(char *s);
    static class Player *locate(long id);
    static class Player *locate_in(long room, class Player *first=NULL, long id=-1);
    static class Player *locate_others_in(long room, class Player *first=NULL, long id=-1);
    static long mask_in_room(long room);
    };

extern void assume_identity(int id);

extern class Player *me;
extern class Player *last_him;
extern class Player *last_her;

#endif /* PLAYERS_H */
