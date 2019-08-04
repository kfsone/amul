// Object and Object-State class/functions

#define OBJECTS_C 1

#include "consts.hpp"
#include "smugl.hpp"
#include "structs.hpp"

#include "data.hpp"
#include "objects.hpp"
#include "rooms.hpp"

// class ObjectIdx ObjectIdx;

int
Object::describe()
{
    if (state < 0 || !states || state >= nstates)
        return FALSE;  // Not in play or no states
    return states[state].describe();
}

// State Functions

// Describe a given state of an object
int
State::describe()
{
    if (descrip != -1 && descrip != -2) {
        tx(message(descrip), ' ');
        return TRUE;
    }
    return FALSE;
}

///////////////////////////////// Object Index Functions

class Object *ObjectIdx::locate(char *s)  // Locate an object by it's name
{
    long w = is_word(s);
    if (w == -1)  // We don't know that word
        return nullptr;
    return locate(w);
}

class Object *ObjectIdx::locate(long id)  // Locate an object by it's vocab id
{
    class Object *ptr;
    long i;
    // Search through the objects for something with this id
    for (ptr = data->objbase, i = 0; i < data->objects; i++, ptr++) {
        if (ptr->id == id)
            return ptr;
    }
    // No match
    return nullptr;
}

// Locate an object, or all objects, in a given room
//  Using 'from' allows you to do backtracking, also using want_id of -1
//  allows you to iterate through all objects in a given room
// By properly arranging linked lists, this function shouldn't be
// neccesary; you should be able to say:
//  for (curnt = room->child; curnt; curnt = curnt->next_here) ...
class Object *
ObjectIdx::locate_in(basic_obj in, class Object *from, long want_id)
{
    class Object *curnt = from;

    if (curnt)
        curnt++;
    else
        curnt = data->objbase;

    for (; curnt && curnt->type == WNOUN; curnt = (Object *) curnt->next) {
        if ((want_id == -1 || want_id == curnt->id) && curnt->is_in(in))
            return curnt;
    }
    return nullptr;
}
