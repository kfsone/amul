// Definition of room classes and functions

static const char rcsid[] = "$Id: travel.cc,v 1.4 1997/05/22 02:21:32 oliver Exp $";

#define TRAVEL_C 1

#include "smugl.hpp"
#include "structs.hpp"

#include "data.hpp"
#include "rooms.hpp"
#include "travel.hpp"

class TTIdx TTIdx;

int
TTEnt::describe()
{
    tx("Why are you calling TTEnt::describe? Duffer.\n");
    return FALSE;
}

//////////////////////// TTIdx functions

// Locate a Travel Entry by it's rooms name
class TTEnt *
TTIdx::locate(char *s)
{
    long id = is_word(s);
    // Is this a valid word?
    if (id == -1)
        return nullptr;
    return locate(id);
}

// Locate a Travel Entry by it's rooms vocab id
class TTEnt *
TTIdx::locate(long id)
{
    class Room *ptr;
    long i;

    // Look for a room with this name
    for (ptr = data->roombase, i = 0; i < data->rooms; i++, ptr++) {
        if (ptr->id == id)
            return (class TTEnt *) ptr->tabptr;
    }

    // We didn't find a match, return a NULL pointer
    return nullptr;
}
