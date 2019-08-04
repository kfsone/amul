// Loader functions for the various object types
// The purpose of this file is to provide a single loader for
// a game database.

#include "include/consts.hpp"
#include "include/fperror.hpp"
#include "include/structs.hpp"
#include "include/syslog.hpp"
#include "smugl/io.hpp"
#include "smugl/misc.hpp"
#include "smugl/smugl.hpp"

// Proto types and class definitions
#include "include/libprotos.hpp"
#include "smugl/aliases.hpp"
#include "smugl/lang.hpp"
#include "smugl/manager.hpp"
#include "smugl/mobiles.hpp"
#include "smugl/objects.hpp"
#include "smugl/ranks.hpp"
#include "smugl/rooms.hpp"
#include "smugl/travel.hpp"

// Read in the text messages and their indexes
static void*
read_in_umsgs(void* base)
{
    // First load in the index
    size_t size = read_file(umsgifn, base, true);
    data->msgbase = (long*) base;
    base = VOIDADD(base, size);
    size_t msgs = size / sizeof(long);  // Number of messages

    // Adjust the index pointers to point at the real text
    for (size_t i = 0; i < msgs; i++)
        data->msgbase[i] += (long) base;

    // Second load in the actual text
    size = read_file(umsgfn, base, true);

    return (void*) NORMALISE(VOIDADD(base, size));
}

// Read in the rank table
static void*
read_in_ranks(void* base)
{
    size_t size = read_file(ranksfn, base, true);
    data->rankbase = (class Rank*) base;
    data->ranks = size / sizeof(Rank);
    return (void*) NORMALISE(VOIDADD(base, size));
}

// Room objects
// We have to read the rooms in seperately so that we can
// process them into place
static void*
read_in_rooms(void* base)
{
    fileInfo* fi = locate_file(roomsfn, true);
    FILE* fp = fopen(fi->name, "rb");
    if (fp == NULL) {
        sysLog.Write(_FLT, "unable to read %s", roomsfn);
        /*ABORT*/
    }

    data->roombase = (class Room*) base;
    data->rooms = roomCount;

    // Work out how many start rooms there are...
    data->start_rooms = 0;

    Room* dest = data->roombase;
    data->anterm = NULL;

    for (counter_t i = 0; i < data->rooms; dest++, i++) {
        *dest = Room{};
        dest->Read(fp);

        if ((dest->flags & ANTERM) && data->anterm)
            sysLog.Write(_FLW, "redundant anteroom '%s' ignored", word(dest->id));
        else if (dest->flags & ANTERM)
            data->anterm = dest;
        if (dest->flags & STARTL)
            data->start_rooms++;
    }

    fclose(fp);
    return (void*) NORMALISE(dest);
}

// Mobile entities
static void*
read_in_mobiles(void* base)
{
    size_t size = read_file(mobfn, base, true);
    data->mobbase = (class Mobile*) base;
    data->mobiles = size / sizeof(Mobile);
    return (void*) NORMALISE((char*) base + size);
}

// Nouns (objects)
static void*
read_in_objects(void* base)
{
    // Read in the state descriptions
    size_t size = read_file(statfn, base, true);
    State* statep = static_cast<class State*>(base);
    base = VOIDADD(base, size);

    // Read in the main object sections
    fileInfo* fi = locate_file(objsfn, true);
    FILE* fp = fopen(fi->name, "rb");
    if (fp == NULL) {
        sysLog.Write(_FLT, "unable to read %s", objsfn);
        /*ABORT*/
    }
    data->objbase = (class Object*) base;
    data->objects = nounCount;

    Object* cur = static_cast<Object*>(base);
    for (counter_t i = 0; i < data->objects; i++, cur++) {
        *cur = Object{};
        cur->Read(fp);

        if (cur->nstates > 0) {
            cur->states = statep;
            statep += cur->nstates;
        }
    }

    return (void*) NORMALISE(cur);
}

// Read in the basic object index and containers
static void*
read_in_basic_objs(void* base)
{
    read_file(bobfn, base, true);
    // The first two longs of the file are the counters
    nbobs = ((container_t*) base)[0];
    ncontainers = ((container_t*) base)[1];
    // Next comes space for the basic object indexes
    // These will need initialising
    bobs = (BASIC_OBJ**) &(((container_t*) base)[2]);
    // Finally, the containers
    containers = (CONTAINER*) (bobs + nbobs);
    return (void*) NORMALISE(containers + ncontainers);
}

// This one gets a bit messy because the compiler leaves us some work to do
// in terms of updating pointers, etc.
void*
read_in_verbs(void* base)
{
    // Read in the base verb objects
    counter_t size = read_file(langfn, base, true);

    // The first 3 values are counters:
    counter_t* counters = static_cast<counter_t*>(base);
    data->verbs = *(counters++);
    counter_t slots = *(counters++);
    counter_t cmds = *(counters++);

    // Now we're at the base of the verb table
    class Verb* vbp = (data->verbbase = (class Verb*) (counters));
    struct SLOTTAB* slotp = (struct SLOTTAB*) (data->verbbase + data->verbs);
    struct VBTAB* cmdp = (struct VBTAB*) (slotp + slots);
    long* argp = (long*) (cmdp + cmds);

    // Change the offsets in the various objects to pointers
    for (counter_t i = 0; i < data->verbs; i++, vbp++) {
        if (vbp->ents <= 0) {
            vbp->ptr = NULL;
            continue;
        }
        vbp->ptr = slotp;
        // Now adjust the slot entries
        for (counter_t j = 0; j < vbp->ents; j++, slotp++) {
            if (slotp->ents <= 0) {
                slotp->ptr = NULL;
                continue;
            }
            slotp->ptr = cmdp;
            for (size_t k = 0; k < slotp->ents; k++, cmdp++) {
                int ncp = cond[cmdp->condition].argc;
                int nap = 0;

                if (cmdp->action_type == ACT_DO)
                    nap = action[cmdp->action].argc;
                if (ncp + nap == 0)
                    cmdp->pptr = NULL;
                else
                    cmdp->pptr = argp;
                argp += (ncp + nap);
            }
        }
    }
    return (void*) NORMALISE(VOIDADD(base, size));
}

// Read in the travel table
static void*
read_in_travel(void* base)
{
    int i;
    class TTEnt* ttp;
    long* argp;

    // First read in the TT_ENT data
    size_t size = read_file(ttfn, base, true);
    data->ttbase = (class TTEnt*) base;
    data->ttents = size / sizeof(struct TT_ENT);
    base = VOIDADD(base, size);

    // Now read in the condition/action parameters
    size = read_file(ttpfn, base, true);
    argp = (long*) base;

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

    return (void*) NORMALISE(VOIDADD(base, size));
}

// Read in the aliases (synonyms) file
void*
read_in_aliases(void* base)
{
    const size_t size = read_file(synsifn, base, true);
    data->aliasbase = (class Alias*) base;
    data->aliases = size / sizeof(struct ALIAS);
    return VOIDADD(base, size);
}

// Tell the user what stage we're at
static void
mention(const char* s)
{
    // Print everything on one line - no carriage return. As a result,
    // we need to fflush to cause it to be written right away
    printf("%s:", s);
    fflush(stdout);
}

// Read the primary game file
static void
read_in_advfn(void)
{
    FILE* fp = fopen(datafile(advfn), "r");
    if (fp == NULL) {
        sysLog.Write(_FLT, "can't open file %s: %s", advfn, strerror(errno));
        /*ABORT*/
    }

    // Read game and logfile names
    if (fread(data->name, ADNAMEL + 1, 1, fp) < ADNAMEL + 1)
        throw Smugl::FPReadError(datafile(advfn), errno, fp);
    if (fread(data->logfile, ADNAMEL + 1, 1, fp) < ADNAMEL + 1)
        throw Smugl::FPReadError(datafile(advfn), errno, fp);
    if (fscanf(fp,
               "%ld %ld %d %d %d %d %d %d\n",
               &data->compiled,
               &data->time,
               &data->see_invis,
               &data->all_see_invis,
               &data->minsgo,
               &data->rscale,
               &data->tscale,
               &data->port) < 8)
        throw Smugl::FPReadError(datafile(advfn), errno, fp);

    fclose(fp);
}

void
load_database(void* const membase)
// Load all the files in the database
{
    container_t conPlayer;
    Player* ppCur;
    int i;

    vc = &data->VC;  // Set the vocab table base

    mention("Info");
    read_in_advfn();
    mention("Text");
    void* ptr = read_in_umsgs(membase);
    mention("Vocab");
    ptr = read_in_vocab(ptr);
    mention("Ranks");
    ptr = read_in_ranks(ptr);
    mention("Rooms");
    ptr = read_in_rooms(ptr);
    mention("Mobiles");
    ptr = read_in_mobiles(ptr);
    mention("Objects");
    ptr = read_in_objects(ptr);
    ptr = read_in_basic_objs(ptr);
    mention("Lang");
    ptr = read_in_verbs(ptr);
    mention("Travel");
    ptr = read_in_travel(ptr);
    mention("Syns");
    ptr = read_in_aliases(ptr);
    printf("LOADED. [%zu bytes]\n",
           static_cast<const char*>(ptr) - static_cast<const char*>(membase));

    // Clear some of the fields in data

    // Initialise the basic object index
    int bobno = 0;

    // Index rooms
    for (i = 0; i < data->rooms; i++, bobno++)
        bobs[bobno] = (BASIC_OBJ*) (data->roombase + i);

    // Index objects
    for (i = 0; i < data->objects; i++, bobno++)
        bobs[bobno] = (BASIC_OBJ*) (data->objbase + i);

    // Index players
    // conPlayer is which container will describe the location
    // of this player
    conPlayer = ncontainers - MAXU;

    for (i = 0, ppCur = data->user; i < MAXU; i++, ppCur++) {
        *ppCur = Player{};
        ppCur->state = OFFLINE;
        ppCur->id = -1;
        ppCur->_name[0] = 0;
        ppCur->std_flags = 0;  // Especially not 'bob_INPLAY'
        ppCur->flags = 0;

        bobs[bobno] = (BASIC_OBJ*) (data->user + i);
        data->user[i].conLocation = conPlayer;
        data->user[i].init_bob(bobno);

        bobno++;
        conPlayer++;
    }

    // Now we need to set all the 'next' values to make the
    // basic object chain work properly
    bobs[--bobno]->next = NULL;
    for (; bobno-- > 0;)
        bobs[bobno]->next = bobs[bobno + 1];

    if (g_debug > 1) {
        printf("\nScanning Contents Tree\n");
        for (i = 0; i < data->rooms; i++) {
            Room* room = data->roombase + i;
            container_t conChild;

            printf("+ Room: RoomNo=%4d, ID=%4ld, Name=%s, Contains %ld\n",
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

                printf("| + Item=%s Bob=%ld, bob->bob=%ld, container=%ld\n",
                       word(bobs[boSelf]->id),
                       boSelf,
                       bobs[boSelf]->bob,
                       boParent);
            }
        }

        printf("\nScanning Object Tree\n");
        for (i = 0; i < data->objects; i++) {
            Object* obj = data->objbase + i;

            printf("+ Object: ObjNo=%4d, ID=%4ld, Name=%s, Contains %ld, Locations=%ld\n",
                   i,
                   obj->id,
                   word(obj->id),
                   obj->contents,
                   obj->locations);
        }
    }
}
