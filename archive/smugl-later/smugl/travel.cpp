// Definition of room classes and functions

#include "include/structs.hpp"
#include "smugl/smugl.hpp"

#include "smugl/data.hpp"
#include "smugl/rooms.hpp"
#include "smugl/travel.hpp"

class TTIdx TTIdx;

bool
TTEnt::describe(void)
{
    tx("Why are you calling TTEnt::describe? Duffer.\n");
    return false;
}

//////////////////////// TTIdx functions

// Locate a Travel Entry by it's rooms name
class TTEnt *
TTIdx::locate(char *s)
{
    long id = is_word(s);

    // Is this a valid word?
    if (id == -1)
        return NULL;
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
    return NULL;
}
