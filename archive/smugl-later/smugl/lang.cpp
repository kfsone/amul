// Definition of language (verb) classes and functions

#define LANG_C 1

#include "lang.hpp"
#include "aliases.hpp"
#include "parser.hpp"
#include "smugl.hpp"
#include "structs.hpp"

// class VerbIdx VerbIdx;

bool
Verb::describe()
{
    txprintf("You want to describe %s.\n", word(id));
    return true;
}

//////////////////////////////////////// Verb Index functions

Verb *
VerbIdx::locate(char *s)
// Locate a verb by it's name
{
    vocid_t w = is_word(s);
    if (w == -1)
        return nullptr;
    return locate(w);
}

Verb *
VerbIdx::locate(vocid_t id)
// Locate a verb by it's vocab id
{
    Verb *ptr = data->verbbase;
    for (int i = 0; i < data->verbs; i++, ptr++) {
        if (ptr->id == id)
            return ptr;
    }
    vocid_t alias = Alias::locate(id);

    if (alias != -1 && (alias = Alias::meaning(alias)) != -1)
        return locate(alias);
    return nullptr;
}
