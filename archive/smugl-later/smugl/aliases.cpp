// Definition of alias (synonym) class and functions

#include "smugl/aliases.hpp"
#include "include/structs.hpp"
#include "smugl/smugl.hpp"

long
Alias::locate(const char* alias)
// locate an alias by it's name
{
    vocid_t id = is_word(alias);

    return (id == -1) ? -1 : locate(id);
}

long
Alias::locate(vocid_t id)
// Locate an alias by it's vocab id
{
    long i;
    Alias* ptr;

    for (i = 0, ptr = data->aliasbase; i < data->aliases; i++, ptr++) {
        if (ptr->given == id)
            return i;
    }
    return -1;
}
