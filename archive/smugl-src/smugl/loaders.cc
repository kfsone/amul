// Loader functions for the various object types
// The purpose of this file is to provide a single loader for
// a game database.
// I've broken this down into functions, but then I've made them
// static inline, so that the end result is one big function.

#define LOADERS_C 1

#include <cassert>
#include <cerrno>
#include <cstring>
#include <new>

#include "consts.hpp"
#include "io.hpp"
#include "loaders.hpp"
#include "misc.hpp"
#include "smugl.hpp"
#include "structs.hpp"

// Proto types and class definitions
#include "aliases.hpp"
#include "lang.hpp"
#include "libprotos.hpp"
#include "manager.hpp"
#include "mobiles.hpp"
#include "objects.hpp"
#include "ranks.hpp"
#include "rooms.hpp"
#include "travel.hpp"

// Read in the text messages and their indexes
static void *
read_in_umsgs(void *base)
{
    long msgs;

    // First load in the index
    size_t size = read_file(umsgifn, base, TRUE);
    data->msgbase = (long *) base;
    base = void_add(base, size);
    msgs = size / sizeof(long);  // Number of messages

    // Adjust the index pointers to point at the real text
    for (long i = 0; i < msgs; i++)
        data->msgbase[i] += (long) base;

    // Second load in the actual text
    size = read_file(umsgfn, base, TRUE);

    return ptr_align(void_add(base, size));
}

// Read in the rank table
static void *
read_in_ranks(void *base)
{
    size_t size = read_file(ranksfn, base, TRUE);
    data->rankbase = (class Rank *) base;
    data->ranks = size / sizeof(Rank);
    return ptr_align(void_add(base, size));
}

// Room objects
// We have to read the rooms in seperately so that we can
// process them into place
static void *
read_in_rooms(void *base)
{
    long i;

    fileInfo *fi = locate_file(roomsfn, TRUE);
    FILE *fp = fopen(fi->name, "rb");
    if (fp == nullptr) {
        error(LOG_ERR, "unable to read %s", roomsfn);
        exit(1);
    }

    data->roombase = (class Room *) base;
    data->rooms = roomCount;

    // Work out how many start rooms there are...
    data->start_rooms = 0;

    Room *roomcur = data->roombase;
    data->anterm = nullptr;

    for (i = 0; i < data->rooms; roomcur++, i++) {
        Room *dest = new (roomcur) Room;
        dest->Read(fp);

        if ((dest->flags & ANTERM) && data->anterm)
            error(LOG_NOTICE, "redundant anteroom '%s' ignored", word(dest->id));
        else if (dest->flags & ANTERM)
            data->anterm = dest;
        if (dest->flags & STARTL)
            data->start_rooms++;
    }

    fclose(fp);
    return ptr_align(roomcur);
}

// Mobile entities
static void *
read_in_mobiles(void *base)
{
    size_t size = read_file(mobfn, base, TRUE);
    data->mobbase = (class Mobile *) base;
    data->mobiles = size / sizeof(Mobile);
    return ptr_align(void_add(base, size));
}

// Nouns (objects)
static void *
read_in_objects(void *base)
{
    FILE *fp;
    State *statep;
    Object *cur;
    fileInfo *fi;
    int i;

    // Read in the state descriptions
    size_t size = read_file(statfn, base, TRUE);
    statep = (class State *) base;
    base = void_add(base, size);

    // Read in the main object sections
    fi = locate_file(objsfn, TRUE);
    fp = fopen(fi->name, "rb");
    if (fp == nullptr) {
        error(LOG_ERR, "unable to read %s", objsfn);
        exit(1);
    }
    data->objbase = (class Object *) base;
    data->objects = nounCount;

    cur = (Object *) base;
    for (i = 0; i < data->objects; i++, cur++) {
        new (cur) Object;
        cur->Read(fp);

        if (cur->nstates > 0) {
            cur->states = statep;
            statep += cur->nstates;
        }
    }

    return ptr_align(cur);
}

// Read in the basic object index and containers
static void *
read_in_basic_objs(void *base)
{
    read_file(bobfn, base, TRUE);
    // The first two longs of the file are the counters
    nbobs = ((container_t *) base)[0];
    ncontainers = ((container_t *) base)[1];
    // Next comes space for the basic object indexes
    // These will need initialising
    bobs = (BASIC_OBJ **) &(((container_t *) base)[2]);
    // Finally, the containers
    containers = (CONTAINER *) (bobs + nbobs);
    return ptr_align(containers + ncontainers);
}

// This one gets a bit messy because the compiler leaves us some work to do
// in terms of updating pointers, etc.
static void *
read_in_verbs(void *base)
{
    int i;
    counter_t slots, cmds;
    counter_t *counters;
    class Verb *vbp;
    struct SLOTTAB *slotp;
    struct VBTAB *cmdp;
    long *argp;

    // Read in the base verb objects
    size_t size = read_file(langfn, base, TRUE);

    // The first 3 values are counters:
    counters = (counter_t *) base;
    data->verbs = *(counters++);
    slots = *(counters++);
    cmds = *(counters++);

    // Now we're at the base of the verb table
    vbp = (data->verbbase = (class Verb *) counters);
    slotp = (struct SLOTTAB *) (data->verbbase + data->verbs);
    cmdp = (struct VBTAB *) (slotp + slots);
    argp = (long *) (cmdp + cmds);

    // Change the offsets in the various objects to pointers
    for (i = 0; i < data->verbs; i++, vbp++) {
        int j;
        if (vbp->ents <= 0) {
            vbp->ptr = nullptr;
            continue;
        }
        vbp->ptr = slotp;
        // Now adjust the slot entries
        for (j = 0; j < vbp->ents; j++, slotp++) {
            if (slotp->ents <= 0) {
                slotp->ptr = nullptr;
                continue;
            }
            slotp->ptr = cmdp;
            for (int k = 0; k < slotp->ents; k++, cmdp++) {
                int ncp = cond[cmdp->condition].argc;
                int nap = 0;

                if (cmdp->action_type == ACT_DO)
                    nap = action[cmdp->action].argc;
                if (ncp + nap == 0)
                    cmdp->pptr = nullptr;
                else
                    cmdp->pptr = argp;
                argp += (ncp + nap);
            }
        }
    }
    return ptr_align(void_add(base, size));
}

// Read in the travel table
static inline void *
read_in_travel(void *base)
{
    int i;
    class TTEnt *ttp;
    long *argp;

    // First read in the TT_ENT data
    size_t size = read_file(ttfn, base, TRUE);
    data->ttbase = (class TTEnt *) base;
    data->ttents = size / sizeof(struct TT_ENT);
    base = void_add(base, size);

    // Now read in the condition/action parameters
    size = read_file(ttpfn, base, TRUE);
    argp = (long *) base;

    // Now fix pointers
    for (i = 0, ttp = data->ttbase; i < data->ttents; i++, ttp++) {
        long fn;

        fn = (long) ttp->pptr;
        ttp->pptr = argp;
        if (fn == -2)
            continue;
        argp += cond[ttp->condition].argc;

        // And those for the condition
        if (ttp->action_type == ACT_DO)
            argp += action[ttp->action].argc;
    }

    return ptr_align(void_add(base, size));
}

// Read in the aliases (synonyms) file
static void *
read_in_aliases(void *base)
{
    size_t size = read_file(synsifn, base, TRUE);
    data->aliasbase = (class Alias *) base;
    data->aliases = size / sizeof(struct ALIAS);
    return void_add(base, size);
}

// Tell the user what stage we're at
static void
mention(const char *s)
{
    // Print everything on one line - no carriage return. As a result,
    // we need to fflush to cause it to be written right away
    printf("%s:", s);
    fflush(stdout);
}

// Read the primary game file
static void
read_in_advfn()
{
    FILE *fp = fopen(datafile(advfn), "r");
    if (fp == nullptr) {
        error(LOG_ERR, "can't open file %s: %s", advfn, strerror(errno));
        exit(1);
    }
    // Read game and logfile names
    fread(data->name, ADNAMEL + 1, 1, fp);
    fread(data->logfile, ADNAMEL + 1, 1, fp);
    fscanf(fp,
           "%ld %ld %d %d %d %d %d %d\n",
           &data->compiled,
           &data->time,
           &data->see_invis,
           &data->all_see_invis,
           &data->minsgo,
           &data->rscale,
           &data->tscale,
           &data->port);
    fclose(fp);
}

void load_database(void *membase)  // Load all the files in the database
{
    container_t conPlayer;
    int i;

    vc = &data->VC;  // Set the vocab table base

    mention("Info");
    read_in_advfn();
    mention("Text");
    membase = read_in_umsgs(membase);
    mention("Vocab");
    membase = read_in_vocab(membase);
    mention("Ranks");
    membase = read_in_ranks(membase);
    mention("Rooms");
    membase = read_in_rooms(membase);
    mention("Mobiles");
    membase = read_in_mobiles(membase);
    mention("Objects");
    membase = read_in_objects(membase);
    membase = read_in_basic_objs(membase);
    mention("Lang");
    membase = read_in_verbs(membase);
    mention("Travel");
    membase = read_in_travel(membase);
    mention("Syns");
    membase = read_in_aliases(membase);
    printf("LOADED.\n");

    // Clear some of the fields in data

    // Initialise the basic object index
    int bobno = 0;
    // Index rooms
    for (i = 0; i < data->rooms; i++, bobno++)
        bobs[bobno] = (BASIC_OBJ *) (data->roombase + i);

    // Index objects
    for (i = 0; i < data->objects; i++, bobno++)
        bobs[bobno] = (BASIC_OBJ *) (data->objbase + i);

    // Index players
    // conPlayer is which container will describe the location
    // of this player
    conPlayer = ncontainers - MAXU;

    for (i = 0; i < MAXU; i++) {
        Player *cur = &data->user[i];
        new (cur) Player;
        bobs[bobno] = cur;
        cur->conLocation = conPlayer;
        cur->init_bob(bobno);
        bobno++;
        conPlayer++;
    }

    // Now we need to set all the 'next' values to make the
    // basic object chain work properly
    bobs[--bobno]->next = nullptr;
    for (; bobno-- > 0;)
        bobs[bobno]->next = bobs[bobno + 1];

    if (debug > 1) {
        printf("\nScanning Contents Tree\n");
        for (i = 0; i < data->rooms; i++) {
            Room *room = data->roombase + i;
            container_t conChild;

            printf("+ Room: RoomNo=%4d, ID=%4d, Name=%s, Contains %d\n",
                   i,
                   room->id,
                   word(room->id),
                   room->contents);
            assert((room->contents <= 0) || (room->conTent >= 0 && room->conTent <= ncontainers));

            for (conChild = room->conTent; conChild >= 0 && conChild <= ncontainers;
                 conChild = containers[conChild].conNext) {
                basic_obj boSelf, boParent;

                boSelf = containers[conChild].boSelf;
                boParent = containers[conChild].boContainer;

                printf("| + Item=%s Bob=%d, bob->bob=%d, container=%d\n",
                       word(bobs[boSelf]->id),
                       boSelf,
                       bobs[boSelf]->bob,
                       boParent);
            }
        }

        printf("\nScanning Object Tree\n");
        for (i = 0; i < data->objects; i++) {
            Object *obj = data->objbase + i;

            printf("+ Object: ObjNo=%4d, ID=%4d, Name=%s, Contains %d, Locations=%d\n",
                   i,
                   obj->id,
                   word(obj->id),
                   obj->contents,
                   obj->locations);
        }
    }
}
