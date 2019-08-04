// Definition of alias (synonym) class and functions

#define ALIASESS_C 1

#include "aliases.hpp"
#include "smugl.hpp"
#include "structs.hpp"

vocid_t
Alias::meaning(vocid_t num)
{
    return data->aliasbase[num].means;
}

vocid_t
Alias::locate(string *alias)
// locate an alias by it's name
{
    vocid_t id = is_word(alias);
    return (id == -1) ? -1 : locate(id);
}

vocid_t
Alias::locate(vocid_t id)
// Locate an alias by it's vocab id
{
    long i;
    class Alias *ptr;
    for (i = 0, ptr = data->aliasbase; i < data->aliases; i++, ptr++) {
        if (ptr->given == id)
            return i;
    }
    return -1;
}
