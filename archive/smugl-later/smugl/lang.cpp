// Definition of language (verb) classes and functions

#include "smugl/lang.hpp"
#include "include/structs.hpp"
#include "smugl/aliases.hpp"
#include "smugl/parser.hpp"
#include "smugl/smugl.hpp"

// class VerbIdx VerbIdx;

bool
Verb::describe(void)
{
    txprintf("You want to describe %s.\n", word(id));
    return true;
}

//////////////////////////////////////// Verb Index functions

Verb*
VerbIdx::locate(char* s)
// Locate a verb by it's name
{
    vocid_t w = is_word(s);
    if (w == -1)
        return NULL;
    return locate(w);
}

Verb*
VerbIdx::locate(vocid_t id)
// Locate a verb by it's vocab id
{
    class Verb* ptr;
    int i;

    for (i = 0, ptr = data->verbbase; i < data->verbs; i++, ptr++) {
        if (ptr->id == id)
            return ptr;
    }
    long alias = Alias::locate(id);

    if (alias != -1 && (alias = Alias::meaning(alias)) != -1)
        return locate(alias);
    return NULL;
}
