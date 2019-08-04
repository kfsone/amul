// Definition of language (verb) classes and functions

#define LANG_C 1

#include "lang.hpp"
#include "aliases.hpp"
#include "parser.hpp"
#include "smugl.hpp"
#include "structs.hpp"

// class VerbIdx VerbIdx;

int
Verb::describe()
{
    txprintf("You want to describe %s.\n", word(id));
    return TRUE;
}

//////////////////////////////////////// Verb Index functions

Verb *
VerbIdx::locate(char *s)
// Locate a verb by it's name
{
    long w;
    w = is_word(s);
    if (w == -1)
        return nullptr;
    return locate(w);
}

Verb *
VerbIdx::locate(long id)
// Locate a verb by it's vocab id
{
    class Verb *ptr;
    int i;
    for (i = 0, ptr = data->verbbase; i < data->verbs; i++, ptr++) {
        if (ptr->id == id)
            return ptr;
    }
    long alias = Alias::locate(id);
    if (alias != -1 && (alias = Alias::meaning(alias)) != -1)
        return locate(alias);
    return nullptr;
}
