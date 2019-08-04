// Definition of room classes and functions

#include "include/consts.hpp"
#include "include/structs.hpp"
#include "include/syslog.hpp"
#include "smugl/smugl.hpp"

#include "smugl/data.hpp"
#include "smugl/ipc.hpp"
#include "smugl/lang.hpp"
#include "smugl/misc.hpp"
#include "smugl/objects.hpp"
#include "smugl/rooms.hpp"
#include "smugl/travel.hpp"

class Room *cur_loc;

// class RoomIdx RoomIdx;
class Room *last_room;

int RoomIdx::cur_no;

bool
Room::describe(void)
{
    HEAVYDEBUG("Room::describe");
    bool described = false;
    char finish = 0;  // Character to add when we've finished

    //    class Object *child;
    //    class Player *guest;

    if (id == -1L) {
        tx("* Undefined room.\n", 0);
        sysLog.Write(_FLW, "called Room::describe for undefined room");
        return false;
    }
    // Describe a room
    if (s_descrip != -1) {
        txprintf("%.60s (%s)\n", message(s_descrip), word(id));
        described = true;
    }
    if (l_descrip != -1 && me->rdmode != RDBF &&
        !(me->rdmode == RDRC && (visitor_bf & me->bitmask))) {
        tx(message(l_descrip), ' ');
        finish = '\n';
        described = true;
        visitor_bf |= me->bitmask;
    }

    // Tell the player what objects they can see here.
    if (contents) {  // Can't see objects in hide-aways
        container_t conChild = conTent;

        for (int entry = 0; entry < contents && conChild >= 0; entry++) {
            assert(conChild >= 0 && conChild <= ncontainers);
            assert(containers[conChild].boSelf >= 0 && containers[conChild].boSelf <= nbobs);
            BASIC_OBJ *child = bobs[containers[conChild].boSelf];

            if (child->bob == me->bob)
                continue;

            assert(child->type >= PNOUN && child->type <= PMAX_TYPE);

            if (!(child->type == PNOUN && (std_flags & bob_HIDEOBJ)) &&
                !(child->type == PPLAYER && (std_flags & bob_HIDECRE)) &&
                !(child->type == PMOBILE && (std_flags & bob_HIDECRE))) {
                if (child->describe())
                    finish = '\n';
            }
            conChild = containers[conChild].conNext;
        }
    }

    if (finish) {
        described = true;
        txc(finish);
    }

    return described;
}

inline class TTEnt *
Room::Tabptr(void)
{  // Return tabptr as a TTEnt pointer
    return data->ttbase + tabptr;
}

// Room::leave()
// Try and leave a room.
// Returns NULL if there are no rules for the given exit,
// otherwise returns a true value
bool
Room::leave(vocid_t wordno)
{  // Called with a word ID
    HEAVYDEBUG("Room::leave");
    class Verb *vb = VerbIdx::locate(wordno);

    if (vb == NULL)  // word must be a verb
        return false;
    return leave(vb);
}

// Room::leave()
// Try and leave a room.
// Returns NULL if there are no rules for the given exit,
// otherwise returns a true value
bool
Room::leave(class Verb *vb)
{  // Called with a verb pointer
    HEAVYDEBUG("Room::leave");
    int i = 0;
    bool did_anything = false;
    class TTEnt *ttp;  // TT Entry pointer

    if (tabptr == -1)  // Anything to see?
    {
        tx(message(CANTGO), '\n');
        return true;
    }

    // Iterate over the TTEntries...
    for (ttp = Tabptr(), i = 0; i < ttlines; i++, ttp++) {
        if (ttp->verb == vb->id) {
            bool did_anything = true;  // We've seen something to do
            // XXX ** TEMPORARY **
            if (ttp->not_condition || ttp->condition != CALWAYS || ttp->action_type == ACT_DO) {
                tx("> There are instructions to exit this room, "
                   "but I can't handle them yet.\n");
                return true;
            }

            class Room *dest = (Room *) bobs[ttp->action];
            if (dest == NULL || ((dest->flags & SMALL) && PlayerIdx::locate_in(ttp->action))) {
                tx("Not enough.\n");
                return false;
            }

            sem_lock(sem_MOTION);
            // Move out of the old room
            depart();
            // Move to the new room
            // Don't use dest->enter, because we want to unlock
            // the MOTION semaphore before describing the room
            dest->arrive();
            sem_unlock(sem_MOTION);
            dest->describe();
            //            ipc_check();
            return did_anything;
        }
    }

    return false;
}

// Leave a room
void
Room::depart(const char *how /*=me->dep*/)
{  // Someone/thing is leaving this room
    HEAVYDEBUG("Room::depart");
    sem_lock(sem_MOTION);
    from_container(me->conLocation);  // Move to nowhere
    cur_loc = NULL;
    if (bob != me->bob)           // If it's not inside itself
        announce_into(bob, how);  // Tell players back there
    sem_unlock(sem_MOTION);
}

// Arrive in another room. The player isn't notified that they've moved.
// We don't tell players in the previous room that they've left either
void
Room::arrive(const char *how /*=me->arr*/)
{  // Player is arriving in this room
    HEAVYDEBUG("Room::arrive");
    sem_lock(sem_MOTION);
    if (bob != me->bob)
        announce_into(bob, how);
    into_container(me->conLocation, bob);
    cur_loc = this;
    sem_unlock(sem_MOTION);
}

// Arrive in a new room, and describe it.
void
Room::enter(const char *how /*=me->arr*/)
{  // Enter a location properly
    HEAVYDEBUG("Room::enter");
    arrive(how);
    if (bob != me->bob)
        describe();
}

// Describe the exits in a room
void
Room::exits(void)
{
    HEAVYDEBUG("Room::exits");
    class TTEnt *ttp = cur_loc->Tabptr();

    for (int i = 0; i < cur_loc->ttlines; i++, ttp++) {
        txprintf("  %d:[%s %c%s %s %p]\n",
                 i,
                 word(ttp->verb),
                 (ttp->not_condition) ? '!' : ' ',
                 cond[ttp->condition].name,
                 (ttp->action_type != ACT_DO) ? word((data->roombase + ttp->action)->id)
                                              : action[ttp->action].name,
                 ttp->pptr);
    }
}

//////////////////////// RoomIdx functions

// Locate a room by it's name
class Room *
RoomIdx::locate(const char *s)
{
    vocid_t id = is_word(s);

    // Is this a valid word?
    if (id == -1)
        return NULL;
    return locate(id);
}

// Locate a room by it's vocab id
class Room *
RoomIdx::locate(vocid_t id)
{
    class Room *rm;
    long i;

    // Look for a room with this name
    for (rm = data->roombase, i = 0; i < data->rooms; i++, rm++) {
        if (rm->id == id)
            return rm;
    }

    // We didn't find a match, return a NULL pointer
    return NULL;
}
