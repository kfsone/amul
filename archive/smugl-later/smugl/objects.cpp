// Object and Object-State class/functions

#include "include/consts.hpp"
#include "include/structs.hpp"
#include "smugl/smugl.hpp"

#include "smugl/data.hpp"
#include "smugl/objects.hpp"
#include "smugl/rooms.hpp"

// class ObjectIdx ObjectIdx;

bool
Object::describe(void)
{
    if (state < 0 || !states || state >= nstates)
        return false;  // Not in play or no states
    return states[state].describe();
}

// State Functions

// Describe a given state of an object
bool
State::describe(void)
{
    if (descrip != -1 && descrip != -2) {
        tx(message(descrip), ' ');
        return true;
    }
    return false;
}

///////////////////////////////// Object Index Functions

Object *
ObjectIdx::locate(char *s)
// Locate an object by it's name
{
    long w = is_word(s);

    if (w == -1)
        // We don't know that word
        return NULL;
    return locate(w);
}

Object *
ObjectIdx::locate(long id)
// Locate an object by it's vocab id
{
    Object *ptr;
    long i;

    // Search through the objects for something with this id
    for (ptr = data->objbase, i = 0; i < data->objects; i++, ptr++) {
        if (ptr->id == id)
            return ptr;
    }
    // No match
    return NULL;
}

// Locate an object, or all objects, in a given room
//  Using 'from' allows you to do backtracking, also using want_id of -1
//  allows you to iterate through all objects in a given room
// By properly arranging linked lists, this function shouldn't be
// neccesary; you should be able to say:
//  for (curnt = room->child; curnt; curnt = curnt->next_here) ...
Object *
ObjectIdx::locate_in(basic_obj in, Object *from /*=NULL*/, long want_id /*=-1*/)
{
    Object *curnt = from;

    if (curnt)
        curnt++;
    else
        curnt = data->objbase;

    for (; curnt && curnt->type == WNOUN; curnt = (Object *) curnt->next) {
        if ((want_id == -1 || want_id == curnt->id) && curnt->is_in(in))
            return curnt;
    }
    return NULL;
}
