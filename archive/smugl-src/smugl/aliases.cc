// Definition of alias (synonym) class and functions

static const char rcsid[] = "$Id: aliases.cc,v 1.4 1997/05/22 02:21:17 oliver Exp $";

#define ALIASESS_C 1

#include "aliases.hpp"
#include "smugl.hpp"
#include "structs.hpp"

long Alias::locate(const char *alias)  // locate an alias by it's name
{
    vocid_t id = is_word(alias);
    return (id == -1) ? -1 : locate(id);
}

long Alias::locate(vocid_t id)  // Locate an alias by it's vocab id
{
    long i;
    class Alias *ptr;
    for (i = 0, ptr = data->aliasbase; i < data->aliases; i++, ptr++) {
        if (ptr->given == id)
            return i;
    }
    return -1;
}
