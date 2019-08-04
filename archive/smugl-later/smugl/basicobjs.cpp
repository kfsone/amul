// Basic Object manipulation code

#include <cassert>

#include "bobidx.hpp"
#include "smugl.hpp"

counter_t nbobs, ncontainers;
BASIC_OBJ **bobs;
CONTAINER *containers;

class BobIdx BobIdx;

//////////////////////////////////////////////////////////////////////
// Remove a container from it's environment

bool
from_container(container_t con)
{
    // We must:
    //  . Decrement the contents count and weight of the parent
    //  . Update prev and next
    //  . Change the parent's "content" pointer if it pointed at us
    CONTAINER *affected = containers + con;
    BASIC_OBJ *victim = bobs[affected->boSelf];
    BASIC_OBJ *parent = bobs[affected->boContainer];

    assert(con >= 0 && con <= ncontainers);

    // Start of by confirming we are inside another object
    if (affected->boContainer == -1)
        return false;

    parent->contents--;
    parent->contents_weight -= victim->contents_weight;

    // If prev is -1, then we are the head of the chain, update parent
    if (affected->conPrev == -1)
        parent->conTent = affected->conNext;
    else
        containers[affected->conPrev].conNext = affected->conNext;
    // If we're not the end of the chain, update the next...
    if (affected->conNext != -1)
        containers[affected->conNext].conPrev = affected->conPrev;

    // Finally, remove us properly
    affected->conPrev = -1;
    affected->conNext = -1;
    affected->boContainer = -1;

    return true;
}

//////////////////////////////////////////////////////////////////////
// Add a container into a new environment

bool
into_container(container_t con, basic_obj boNewloc)
{
    // We must:
    //  . Increment the contents count and weight of the parent
    //  . Append ourselves to the end of the chain
    //  . Change the parent's "content" pointer if it was empty
    CONTAINER *affected = containers + con;
    BASIC_OBJ *victim = bobs[affected->boSelf];
    BASIC_OBJ *parent;

    assert(boNewloc >= 0 && boNewloc <= nbobs);
    assert(con >= 0 && con <= ncontainers);

    // Start of by confirming we are inside another object
    if (affected->boContainer != -1 && !from_container(con))
        return false;

    parent = bobs[boNewloc];
    affected->boContainer = boNewloc;

    parent->contents++;
    parent->contents_weight += victim->contents_weight;

    // If the parent has no contents, then we become the head
    if (parent->conTent == -1) {
        parent->conTent = con;
        affected->conNext = affected->conPrev = -1;
    } else {
        container_t conTail = -1;
        CONTAINER *tail = containers + parent->conTent;

        // Iterate through the chain until we find the tail
        for (; tail->conNext != -1; tail = containers + conTail)
            conTail = tail->conNext;
        tail->conNext = con;
        affected->conPrev = conTail;
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// BASIC_OBJ functions

bool
BASIC_OBJ::is_in(basic_obj boContainer)
{
    if (locations >= 1) {
        CONTAINER *cur = (containers + conLocation);
        for (int i = 0; i < locations; i++, cur++) {
            if (cur->boContainer == boContainer)
                return true;
        }
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Base-class attempt to describe an object.

bool
BASIC_OBJ::describe()
{
    if (s_descrip == -1 && id == -1) {
        tx("<NULL OBJECT>");  // No description or short name
        return false;
    }
    if (s_descrip == -1)
        tx(word(id));  // Name only
    else
        tx(message(s_descrip));

    return true;
}

//////////////////////////////////////////////////////////////////////
// BobIdx functions
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// find(...)
// Look for a basic object by name, optionally type, and optionally
// starting from a given basic_obj
// Returns -1 on fail

basic_obj
BobIdx::find(vocid_t name, char type /*=WANY*/, basic_obj from /*=-1*/)
{
    BASIC_OBJ *cur;
    if (from >= nbobs - 1 || from < -1)
        return -1;
    for (cur = bobs[from + 1]; cur; cur = cur->next) {
        if (cur->id == name && (type == WANY || type == cur->type))
            return cur->bob;
    }
    return -1;
}
