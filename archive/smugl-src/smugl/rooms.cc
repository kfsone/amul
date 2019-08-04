// Definition of room classes and functions

static const char rcsid[] = "$Id: rooms.cc,v 1.15 1999/09/10 15:57:31 oliver Exp $";

#include <cassert>

#define ROOMS_C 1

#include "consts.hpp"
#include "data.hpp"
#include "ipc.hpp"
#include "lang.hpp"
#include "misc.hpp"
#include "objects.hpp"
#include "rooms.hpp"
#include "smugl.hpp"
#include "structs.hpp"
#include "travel.hpp"

Room *cur_loc;
// class RoomIdx RoomIdx;
Room *last_room;

Room *
RoomIdx::first()
{
    cur_no = 0;
    return data->roombase;
}

int
Room::describe()
{
    HEAVYDEBUG("Room::describe");
    int described = FALSE;
    char finish = 0;  // Character to add when we've finished
                      //    class Object *child;
                      //    class Player *guest;

    if (id == -1L) {
        tx("* Undefined room.\n", 0);
        syslog(LOG_NOTICE, "called Room::describe for undefined room");
        return FALSE;
    }
    // Describe a room
    if (s_descrip != -1) {
        txprintf("%.60s (%s)\n", message(s_descrip), word(id));
        described = TRUE;
    }
    if (l_descrip != -1 && me->rdmode != RDBF &&
        !(me->rdmode == RDRC && (visitor_bf & me->bitmask))) {
        tx(message(l_descrip), ' ');
        finish = '\n';
        described = TRUE;
        visitor_bf |= me->bitmask;
    }

    // Tell the player what objects they can see here.
    if (contents) {  // Can't see objects in hide-aways
        int entry;

        container_t conChild = conTent;
        for (entry = 0; entry < contents && conChild >= 0; entry++) {
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
        described = TRUE;
        txc(finish);
    }

    return described;
}

inline class TTEnt *
Room::Tabptr()
{  // Return tabptr as a TTEnt pointer
    return data->ttbase + tabptr;
}

// Room::leave()
// Try and leave a room.
// Returns NULL if there are no rules for the given exit,
// otherwise returns a TRUE value
int
Room::leave(vocid_t wordno)
{  // Called with a word ID
    HEAVYDEBUG("Room::leave");
    class Verb *vb = VerbIdx::locate(wordno);
    if (vb == nullptr)  // word must be a verb
        return FALSE;
    return leave(vb);
}

// Room::leave()
// Try and leave a room.
// Returns NULL if there are no rules for the given exit,
// otherwise returns a TRUE value
int
Room::leave(class Verb *vb)
{  // Called with a verb pointer
    HEAVYDEBUG("Room::leave");
    int i = 0;
    bool did_anything = false;
    class TTEnt *ttp;  // TT Entry pointer
    if (tabptr == -1)  // Anything to see?
    {
        tx(message(CANTGO), '\n');
        return TRUE;
    }

    // Iterate over the TTEntries...
    for (ttp = Tabptr(), i = 0; i < ttlines; i++, ttp++) {
        if (ttp->verb == vb->id) {
            did_anything = true;
            // XXX ** TEMPORARY **
            if (ttp->not_condition || ttp->condition != CALWAYS || ttp->action_type == ACT_DO) {
                tx("> There are instructions to exit this room, "
                   "but I can't handle them yet.\n");
                return TRUE;
            }
            Room *dest = (Room *) bobs[ttp->action];
            if ((dest->flags & SMALL) && PlayerIdx::locate_in(ttp->action)) {
                tx("Not enough.\n");
                return FALSE;
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
            return TRUE;
        }
    }

    return (did_anything) ? TRUE : FALSE;
}

// Leave a room
void
Room::depart(const char *how)
{  // Someone/thing is leaving this room
    HEAVYDEBUG("Room::depart");
    sem_lock(sem_MOTION);
    from_container(me->conLocation);  // Move to nowhere
    cur_loc = nullptr;
    if (bob != me->bob)           // If it's not inside itself
        announce_into(bob, how);  // Tell players back there
    sem_unlock(sem_MOTION);
}

// Arrive in another room. The player isn't notified that they've moved.
// We don't tell players in the previous room that they've left either
void
Room::arrive(const char *how)
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
Room::enter(const char *how)
{  // Enter a location properly
    HEAVYDEBUG("Room::enter");
    arrive(how);
    if (bob != me->bob)
        describe();
}

// Describe the exits in a room
void
Room::exits()
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

Room *
RoomIdx::current()
{
    if (cur_no >= data->rooms)
        return nullptr;
    return data->roombase + cur_no;
}

Room *
RoomIdx::next()
{
    if (++cur_no >= data->rooms) {
        cur_no = data->rooms;
        return nullptr;
    }
    return data->roombase + cur_no;
}

// Locate a room by it's name
Room *
RoomIdx::locate(const char *s)
{
    vocid_t id = is_word(s);
    // Is this a valid word?
    if (id == -1)
        return nullptr;
    return locate(id);
}

// Locate a room by it's vocab id
Room *
RoomIdx::locate(vocid_t id)
{
    Room *rm;
    long i;

    // Look for a room with this name
    for (rm = data->roombase, i = 0; i < data->rooms; i++, rm++) {
        if (rm->id == id)
            return rm;
    }

    // We didn't find a match, return a NULL pointer
    return nullptr;
}
