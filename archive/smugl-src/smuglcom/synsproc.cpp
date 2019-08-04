/*
 * synsproc.cpp -- Synonym (alias) table processing.
 */

/*
 * This stuff is currently very basic, and synonyms are pretty much
 * treated as aliases. I'm not sure if I like the way this is done,
 * or if localisaing aliases (e.g. object aliases in objects.txt)
 * would be a good idea anyway.
 * AMUL also lacked support for object classing. If SMUGL does ever
 * obtain this, it won't be classing as seen in object oriented
 * languages. It's purely a way of linking a group of nouns together ;-)
 */

#include "errors.hpp"
#include "smuglcom.hpp"

// Process the synonyms table
void
syn_proc()
{
    char *p, *s;
    vocid_t real_word;
    vocid_t alias_word;

    if (nextc(0) == -1) {
        tx("<No Entries>");
        errabort();
        return;
    }
    fopena(synsifn);
    fseek(afp, 0, SEEK_END);

    data = cleanget();
    p = skipspc(data);

    do {
        p = skipline(s = p);
        s = getword(s);
        if (!*Word)
            continue;

        if ((real_word = is_word(Word)) == -1) {
            error("Invalid verb/noun: \"%s\".\n", Word);
            continue;
        }

        do {
            s = getword(s);
            if (!*Word)
                break;
            if ((alias_word = new_word(Word, FALSE)) == -1) {
                error("Invalid synonym, '%s'\n", Word);
                continue;
            }
            fwrite(&alias_word, sizeof(vocid_t), 1, afp);
            fwrite(&real_word, sizeof(vocid_t), 1, afp);
            syns++;
        } while (*s);
    } while (*p);
    errabort();
}
