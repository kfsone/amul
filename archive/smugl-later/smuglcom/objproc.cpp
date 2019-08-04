// objproc.cpp -- process OBJECTS.txt

#include "smuglcom/smuglcom.hpp"

// Default the default object flags
#define DEFAULT_STD (bob_INPLAY)
#define FILTER_STD (bob_INPLAY)

#define INCOMPLETE "Incomplete state line"

static inline void
object(const char *s)
{  // Report invalid object phrase
    error("%s: Invalid %s, \"%s\"!\n", word(obj->id), s, Word);
}

/*** The following 'set_*' functions assume the value to be used
**** is currently in 'Word' */

static inline void
set_art(void)
{  // Set the article of an object
    int i;

    for (i = 0; i < NART; i++) {
        if (!strcmp(article[i], Word)) {
            obj->article = i;
            return;
        }
    }
    object("article type");
}

static inline void
set_start(void)
{  // Set start state of an object
    if (!isdigit(Word[0]))
        object("start state");
    obj->state = atoi(Word);
    if (obj->state < 0 || obj->state > 100)
        object("start state");
}

static inline void
set_holds(void)
{  // Set container capacity for an object
    if (!isdigit(Word[0]))
        object("holds= value");
    obj->max_weight = atoi(Word);
    if (obj->max_weight < 0 || obj->max_weight > 1000000)
        object("holds= value");
}

static inline void
set_put(void)
{  // Set container type for an object
    int i;

    for (i = 0; i < NPUTS; i++)
        if (!strcmp(obputs[i], Word)) {
            obj->putto = i;
            return;
        }
    object("put= flag");
}

static inline void set_mob(void)  // Set mobile character for an object
{
    int i;
    vocid_t wordno;

    /* Mobile ID's are prefixed with a bang ('!'); but you don't specify
     * them like that - so we may need to insert the bang */
    if (Word[0] == '!')
        wordno = is_word(Word);
    else {
        char *p = (char *) malloc(strlen(Word) + 1);

        *p = '!';
        strcpy(p + 1, Word);
        wordno = is_word(p);
        free(p);
    }
    if (wordno != -1) {
        for (i = 0; i < mobchars; i++)
            if ((mobp + i)->id == wordno) {
                obj->mobile = i;
                return;
            }
    }
    object("mobile= flag");
}

static inline void
statinv(const char *s)
{  // Report invalid state line
    error("%s state %ld: %s!\n", word(obj->id), obj->nstates + 1, s);
}

static inline void
state_proc(const char *s)
{  // Process a state-line of an object
    int flag;
    char *p;

    state.std_flags = 0;
    state.weight = state.value = state.flags = 0;
    state.descrip = -1;
    if (!strncmp(s, "none", 4))
        goto write;

    strcpy(g_block, s);

    // Get the weight of the object
    p = skiplead("weight=", skipspc(g_block));
    p = getword(p);
    if (!*p) {
        statinv(INCOMPLETE);
        return;
    }
    if (!isdigit(Word[0]) && Word[0] != '-') {
        statinv("bad weight= value");
        return;
    }
    state.weight = atoi(Word);

    // Get the value of it
    p = skipspc(p);
    p = skiplead("value=", p);
    p = getword(p);
    if (!*p) {
        statinv(INCOMPLETE);
        return;
    }
    if (!isdigit(Word[0]) && Word[0] != '-') {
        statinv("bad value= value");
        return;
    }
    state.value = atoi(Word);

    // Get the strength of it (hit points)
    p = skipspc(p);
    p = skiplead("str=", p);
    p = getword(p);
    if (!*p) {
        statinv(INCOMPLETE);
        return;
    }
    if (!isdigit(Word[0]) && Word[0] != '-') {
        statinv("bad str= value");
        return;
    }
    state.strength = atoi(Word);

    // Get the damage it does as a weapon
    p = getword(skiplead("dam=", skipspc(p)));
    if (!*p) {
        statinv(INCOMPLETE);
        return;
    }
    if (!isdigit(Word[0]) && Word[0] != '-') {
        statinv("bad dam= value");
        return;
    }
    state.damage = atoi(Word);

    // Description
    p = skiplead("desc=", skipspc(p));
    if (!*p) {
        statinv(INCOMPLETE);
        return;
    }
    if (*p == '\"' || *p == '\'') {
        char *quote = p++;

        while (*p && *p != *quote)
            p++;  // Find end of quote/string
        quote++;  // Now forget that character
        if (*(p - 1) == '{' || *(p - 1) == '\\')
            p--;  // Allow for continuation characters
        else
            *(p++) = ' ';  // Otherwise remove this character
        while (*(p - 1) == SPC)
            p--;  // Remove trailing spaces
        state.descrip = add_msg(NULL);
        fwrite(quote, (size_t)(p - quote), 1, msgfp);
        fputc(0, msgfp);  // Add end of string
        strcpy(g_block, skipspc(p));
    } else {  // It's a message ID, we hope
        p = getword(p);
        state.descrip = ismsgid(Word);
    }
    if (state.descrip == -1) {
        sprintf(temp, "bad desc= ID (%s)", Word);
        statinv(temp);
        return;
    }
    while (*p) {
        p = getword(p);
        if (!*Word)
            break;
        if ((flag = isoflag2(Word)) == -1) {
            flag = handle_std_flag(Word, state.std_flags, (flag_t) STATE_STD_FILTER);
            if (flag < 0) {
                sprintf(g_block, "bad state flag '%s'", Word);
                statinv(g_block);
                return;
            } else if (flag > 0) {
                sprintf(g_block, "inapropriate state flag '%s'", Word);
                statinv(g_block);
                return;
            }
        } else
            state.flags = (state.flags | (1 << flag));
    }
write:
    fwrite(&state, sizeof(state), 1, ofp2);
    obj->nstates++;
    return;
}

void
objs_proc(void)
{  // Process the objects file
    char *p, *s;

    nouns = 0;

    fopenw(objsfn);
    fopenw(statfn);

    if (nextc(0) == -1) {
        tx("<No Entries>");
        errabort();
        return;
    }  // Nothing to process
    p = cleanget();
    p = skipspc(p);

    do {
        p = skipline(s = p);
        s = skipspc(s);
        if (!*s)
            continue;

        s = getword(skiplead("noun=", s));
        if (!Word[0])
            continue;

        obj = (OBJ *) grow(NULL, sizeof(OBJ), "New Object");
        if (!obtab)
            obtab = obj;

        obj->clear();
        obj->id = new_word(Word, false);
        add_basic_obj(obj, WNOUN, DEFAULT_STD);

        obj->mobile = -1;
        obj->putto = obj->article = 0;

        // Get the object flags
        do {
            long flag;

            s = getword(s);
            if (!*Word)
                continue;
            if ((flag = isoflag1(Word)) != -1)
                obj->flags = (obj->flags | (1 << flag));
            else {
                if ((flag = isoparm()) == -1) {
                    flag = handle_std_flag(Word, obj->std_flags, FILTER_STD);
                    if (flag < 0)
                        error("%s: Invalid parameter '%s'\n", word(obj->id), Word);
                    else if (flag > 0)
                        error("%s: Inapropriate flag '%s'\n", word(obj->id), Word);
                }
                switch (1 << flag) {
                    case OP_ART:
                        set_art();
                        break;
                    case OP_ADJ:
                        obj->adj = new_word(Word, false);
                        break;
                    case OP_START:
                        set_start();
                        break;
                    case OP_HOLDS:
                        set_holds();
                        break;
                    case OP_PUT:
                        set_put();
                        break;
                    case OP_MOB:
                        set_mob();
                        mobs++;
                        break;
                    default:
                        error("%s: Invalid parameter '%s'\n", word(obj->id), obparms[flag]);
                }
            }
        } while (*Word);

        // Get the next line of data
        do {
            p = skipline(s = p);
            if (!*p)
                quit("%s: Unexpected end of file!\n", word(obj->id));
            s = skipspc(s);
        } while (!*s);

        // It should contain the room list
        do {
            s = getword(s);
            if (!*Word)
                // Dummy word
                break;
            basic_obj loc = is_container(Word);

            if (loc == -1)
            // Invalid word or basic obj
            {
                if (is_word(Word) != -1)
                    error("%s: Invalid location: '%s'\n", word(obj->id), Word);
                else
                    error("%s: Bad start location: '%s' not a container\n", word(obj->id), Word);
                continue;
            }
            add_container(obj->bob, loc);
        } while (*Word);

        if (obj->locations == 0) {
            error("%s: No locations!\n", word(obj->id));
            p = skipdata(p);
            continue;
        }

        obj->nstates = 0;
        do {
            p = skipline(s = p);
            if (!*s)
                break;
            s = skipspc(s);
            if (*s)
                state_proc(s);
        } while (*p != 0);
        if (!obj->nstates)
            state_proc("none");
        else if (obj->nstates > 120)
            object("number of states");
        nouns++;
    } while (*p);
    if (!err) {
        OBJ temp;

        for (obj = obtab; obj; obj = (OBJ *) obj->next) {
            temp = *obj;
            temp.Write(ofp1);
        }
    }
    errabort();  // Abort if an error
}

int isoflag1(const char *s)  // Is it a FIXED object flag?
{
    int i;

    for (i = 0; obflags1[i]; i++)
        if (!strcmp(obflags1[i], s))
            return i;
    return -1;
}

int isoparm(void)  // Is it an object parameter?
{
    int i;
    char *p;

    for (i = 0; obparms[i]; i++) {
        if ((p = skiplead(obparms[i], Word)) != Word) {
            memmove(Word, p, strlen(p) + 1);
            return i;
        }
    }
    return -1;
}

int isoflag2(const char *s)  // Is it a state flag?
{
    int i;

    for (i = 0; obflags2[i]; i++)
        if (!strcmp(obflags2[i], s))
            return i;
    return -1;
}
