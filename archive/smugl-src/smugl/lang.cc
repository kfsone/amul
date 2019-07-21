// Definition of language (verb) classes and functions

static const char rcsid[] = "$Id: lang.cc,v 1.5 1999/06/11 14:26:45 oliver Exp $";

#define LANG_C 1

#include "smugl.hpp"
#include "structs.hpp"
#include "aliases.hpp"
#include "lang.hpp"
#include "parser.hpp"

//class VerbIdx VerbIdx;

int
Verb::describe(void)
    {
    txprintf("You want to describe %s.\n", word(id));
    return TRUE;
    }

//////////////////////////////////////// Verb Index functions

Verb *
VerbIdx::locate(char *s)        // Locate a verb by it's name
    {
    long w;
    w = is_word(s);
    if (w == -1)
        return NULL;
    return locate(w);
    }

Verb *
VerbIdx::locate(long id)        // Locate a verb by it's vocab id
    {
    class Verb *ptr;
    int i;
    for (i = 0, ptr = data->verbbase; i < data->verbs; i++, ptr++)
        {
        if (ptr->id == id)
            return ptr;
        }
    long alias = Alias::locate(id);
    if (alias != -1 && (alias = Alias::meaning(alias)) != -1)
        return locate(alias);
    return NULL;
    }

