// Definition of player classes and functions

#include "include/consts.hpp"
#include "smugl/smugl.hpp"

#include "smugl/ipc.hpp"
#include "smugl/ranks.hpp"
#include "smugl/rooms.hpp"

class Player *me;           // Pointer to our entry in data
class Player *userbase;     // Same as data->user :-(
class PlayerIdx PlayerIdx;  // Player-index functions
class Player *last_him;
class Player *last_her;

bool
Player::describe(void)
{  // Describe this player
    txprintf("%s is here. ", name());
    return true;
}

const char *
Player::name(void)
{
    if (_name[0])  // Does the player have a name?
        return _name;
    else
        return "(somebody)";  // Should probably trap this ;-)
}

// Remove a player from the game, "physically"
void
Player::disconnected(void)
{  // Remove evidence of a given player
    int i = number();

    remove_name();       // Make sure our name leaves, regardless
    sem_lock(sem_DATA);  // "Most interesting lock"
    if (data->pid[i]) {  // Only do this if we are 'connected'
        if (state >= PLAYING)
            announce(ALLBUT(i), EXITED);
        data->pid[i] = 0;  // Remove our process id
        data->user[i].init_bob();
        data->connected--;  // One less player
        _close(clifd[i][WRITEfd]);
        clifd[i][WRITEfd] = 0;  // Nuke the write fd
        clifd[i][READfd] = 0;   // Nuke the read fd
    }
    sem_unlock(sem_DATA);
}

// The next two functions deal with adding a players name into the
// vocab table.
void
Player::add_name(void)
{                       // Add name to the vocab table
    if (_name[0] == 0)  // Does the player have a name?
        return;
    sem_lock(sem_VOCAB);
    vc->index[vc->items + number()] = (long) (_name);
    sem_unlock(sem_VOCAB);
}

void
Player::remove_name(void)
{                       // Remove player's name from the vocab table
    if (_name[0] == 0)  // Does the player have a name?
        return;
    sem_lock(sem_VOCAB);
    vc->index[vc->items + number()] = -1;
    sem_unlock(sem_VOCAB);
}

// Initialise the 'bob' fields
void
Player::init_bob(basic_obj bobno /*=-1*/)
{
    id = -1;  // No ID at present
    if (bobno != -1)
        bob = bobno;         // Basic object number
    next = bobs[bobno + 1];  // Yeuch
    type = WPLAYER;
    std_flags = 0;  // Especially not bob_INPLAY
    weight = 0;
    max_weight = 0;
    contents_weight = 0;
    value = 0;
    damage = 0;
    strength = 0;
    contents = 0;   // We're not carrying anything
    conTent = -1;   // And we don't posses any containers
    locations = 0;  // Will need setting to '1' when connected
    containers[conLocation].boContainer = bobno;
    // But always retain 'location'
}

void
Player::set_rank(int rankno)
{
    rank = rankno;
    myRank = RankIdx::ptr(rank);
    strength = myRank->strength;
    stamina = myRank->stamina;
    dext = myRank->dext;
    me->dextadj = 0;
    wisdom = myRank->wisdom;
    experience = myRank->experience;
    magicpts = myRank->magicpts;
}

// Initialiase the structure of player to 'entering game' status
void
Player::reset(void)
{
    init_bob();
    remove_name();    // Make sure my name is not in the database
    *_name = 0;       // No name
    *passwd = 0;      // No password
    state = LOGGING;  // Flag that we're logging in
    plays = 0;        // Never played
    bitmask = 1 << g_slot;
    score = 0;
    tasks = 0;
    set_rank(0);
    tries = 0;
    sex = MALE;
    rdmode = RDRC;
    last_session = 0;
    llen = DLLEN;
    slen = DSLEN;
    flags = DFLAGS;

    me->rec = -1;

    me->arr = message(ARRIVED);  // Text when entering a room
    me->dep = message(LEFT);     // Text when leaving room
    *me->pre = 0;                // Pre-rank string
    *me->post = 0;               // Post-rank string

    last_him = NULL;  // We haven't talked about anyone yet
    last_her = NULL;
    last_room = NULL;  // Haven't been elsewhere yet
    cur_loc = NULL;
}

// Move the player from here to another room
bool
Player::go_to(basic_obj dest_rm, const char *dep_msg /*=NULL*/, const char *arr_msg /*=NULL*/)
{
    assert(dest_rm >= 0 && dest_rm < nbobs);

    class Room *dest = (Room *) bobs[dest_rm];

    if (dest == NULL || ((dest->flags & SMALL) && PlayerIdx::locate_in(dest_rm))) {
        tx(message(NOROOM), '\n');
        return false;
    }

    // Set up the arrive/depart messages

    sem_lock(sem_MOTION);
    // Move out of the old room
    if (dep_msg)
        cur_loc->depart(dep_msg);
    else
        cur_loc->depart();
    // Move to the new room
    // Don't use dest->enter, because we want to unlock
    // the MOTION semaphore before describing the room
    if (arr_msg)
        dest->arrive(arr_msg);
    else
        dest->arrive();
    sem_unlock(sem_MOTION);
    dest->describe();
    //            ipc_check();
    return true;
}

////////////////////////////// PlayerIdx functions

Player *PlayerIdx::locate(long id)  // Locate player by ID number
{
    int i;

    if ((i = id - vc->items) < 0 || i > MAXU)
        return NULL;
    return &data->user[i];
}

Player *PlayerIdx::locate(char *s)  // Locate player by name
{
    return locate(is_word(s));
}

// Iterate through players in a room
class Player *
PlayerIdx::locate_in(basic_obj cont, class Player *from /*=NULL*/, long want_id /*=-1*/)
{
    class Player *curnt = from;

    if (curnt)
        curnt++;
    else
        curnt = data->user;

    for (; curnt && curnt->type == WPLAYER; curnt = (Player *) curnt->next) {
        if ((want_id == -1 || want_id == curnt->id) && curnt->is_in(cont))
            return curnt;
    }
    return NULL;
}

// Iterate through players in a room, but exclude self
class Player *
PlayerIdx::locate_others_in(basic_obj in, class Player *from /*=NULL*/, long wantid /*=-1*/)
{
    Player *player = locate_in(in, from, wantid);

    if (player == me)
        return locate_in(in, player, wantid);
    return player;
}

// Return a mask for players in a given room
// Usually used for sending IPC messages to a given room
long
PlayerIdx::mask_in_room(basic_obj room)
{
    long mask = 0;

    if (room < 0 || room > nbobs || bobs[room]->type != WROOM)
        return 0;
    for (int i = 0; i < MAXU; i++) {
        if (data->user[i].is_in(room))
            mask += (1 << i);
    }
    return mask;
}

//// Other Stuff

// The manager uses this to switch identity
void
assume_identity(int id)
{
    me = &data->user[id];
    myRank = RankIdx::ptr(me->rank);
    basic_obj loc = me->Location();

    if (loc < 0 || loc == me->bob)
        cur_loc = NULL;
    else if ((cur_loc = (Room *) bobs[loc]) && cur_loc->type != WROOM)
        cur_loc = NULL;
}
