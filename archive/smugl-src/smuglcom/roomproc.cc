// Rooms Table processor
static const char rcsid[] = "$Id: roomproc.cc,v 1.11 1999/06/08 15:36:54 oliver Exp $";

#include <cassert>
#include <cctype>
#include <cstring>

#include "fileio.hpp"
#include "smuglcom.hpp"

#define ROOMDSC_GROW_RATE 4096  // Rate to grow description buffer
size_t rdalloc = 0;             // Text buffer memory allocated

/* To enable us to track DMOVEs which aren't resolvable first pass,
 * we'll track each unresolved DMOVE and then make a second pass */
struct Dmove {
    struct Dmove *next; /* Next DMOVE object */
    basic_obj room;     /* The room the dmove is FROM */
    char *to;           /* Name of room to dmove to */
};
struct Dmove *first = NULL, *dmv;

// Define the default, initial std_flags for a room
#define DEFAULT_STD (bob_INPLAY | bob_SCENERY | bob_LIGHT | bob_SHINES | bob_LIT)
// Specify what flags aren't allowed for a room using handle_std_flags
#define FILTER_STD (bob_INPLAY | bob_SCENERY | bob_COUNTER | bob_FLAMABLE | bob_SCALED)

int
is_room_flag(const char *s)
{  // Check if 's' is a room flag
    if (*s == 0)
        return -1;
    for (int flag = 0; rflag[flag]; flag++)
        if (strcmp(s, rflag[flag]) == 0)
            return flag;
    return -1;
}

int
is_room_param(char *&s)
{  // Check if 's' is a room paramter
    char *p;
    for (int param = 0; rparam[param]; param++) {
        // Skiplead only works if we wholly match argument 1
        // within argument 2. It's roughly equivalent to doing
        //  if (strncmp(s, rparam[param], strlen(rparam[param])))
        // which we don't particularly want to do
        if ((p = skiplead(rparam[param], s)) != s) {
            s = p;  // Forward the pointer
            return param;
        }
    }
    return -1;
}

void
room_proc(void)
{ /* Process the rooms file */
    ROOM *rmp;
    char *bufmem;
    char *p;
    vocid_t rid;

    rdalloc = ROOMDSC_GROW_RATE;
    bufmem = (char *) grow(NULL, rdalloc + 2, "Room description buffer");

    rooms = 0;
    nextc(1); /* Skip to first character */

    for (rmp = NULL; rmp == NULL || nextc(0) == 0; rooms++) {
        // Create a new object
        rmp = new ROOM;
        if (!roomtab)
            roomtab = rmp;

        ////////////////////////////////////////////////////////
        // First line of the room table is the room ID and flags

        /* allow for line-extension */
        get_line(ifp, block, 1000);
        p = skiplead("room=", block);
        p = getword(p);  // Extract the room id

        /* Add the room name to vocab; mustn't already be in use */
        rmp->clear();
        rmp->id = new_word(Word, TRUE);
        if ((rid = rmp->id) == -1)
            error("Duplicate Room name: %s\n", Word);

        // Initialise the flags and add to basic object table
        add_basic_obj(rmp, WROOM, DEFAULT_STD);
        // Each room has 1 presence, although it should never be used
        add_container(rmp->bob, rmp->bob);
        rmp->flags = 0;
        rmp->tabptr = -1L;
        rmp->visitor_bf = 0;   // Can't have been here ;-)
        rmp->max_weight = -1;  // There is no limit

        while (*p) {
            int flag;
            p = getword(p);
            char *ptr = Word;  // For is_room_param
            if ((flag = is_room_flag(ptr)) != -1)
                rmp->flags = (rmp->flags | (1 << flag));
            else
                switch (is_room_param(ptr)) {
                    case rp_dark:  // Dark room
                        // Disable all luminence flags, so the room is
                        // pitch black inside and out.
                        rmp->std_flags &= ~(bob_LIGHT | bob_SHINES | bob_LIT);
                        break;

                    case rp_dmove:  // Inventory moves elsewhere if you die
                        basic_obj dmove;
                        dmove = is_container(ptr);
                        if (dmove == -1)  // Don't know this room yet
                        {
                            dmv = (Dmove *) grow(NULL, sizeof(struct Dmove), "Tracking DMOVEs");
                            dmv->room = rmp->bob;
                            dmv->to = strdup(ptr);
                            dmv->next = first; /* forward-only Linked list */
                            first = dmv;
                        } else
                            rmp->dmove = dmove;
                        break;

                    default:
                        int code;

                        code = handle_std_flag(Word, rmp->std_flags, FILTER_STD);
                        if (code < 0)
                            error("%s: Invalid room flag/parameter '%s'.\n", word(rmp->id), Word);
                        else if (code > 0)
                            error("%s: Flag '%s' not appropriate for room.\n", word(rmp->id), Word);
                        break;
                }
        }

        fgets(bufmem, rdalloc, ifp); /* Get short desc */
        if (bufmem[0] != '\n' && bufmem[0] != '\r') {
            char *q = NULL;
            offset_t mem_off = 0;

            int len = strlen(bufmem);
            rmp->s_descrip = add_msg(NULL);
            bufmem[len - 1] = 0;
            fwrite(bufmem, len, 1, msgfp);

            while (fgets(bufmem + mem_off, rdalloc - mem_off, ifp)) {
                char *base = bufmem + mem_off;
                /* Was it a blank line? */
                if (*base == '\n' || *base == 0) {
                    *base = 0;
                    break;
                }
                /* Remove one leading tab */
                if (*base == '\t')
                    memmove(base, base + 1, strlen(base));
                /* Fetch more lines as neccesary */
                while ((q = strrchr(base, '\n')) == NULL) {
                    /* Grow some more memory */
                    mem_off += strlen(base);
                    if ((size_t)(mem_off + ROOMDSC_GROW_RATE) >= rdalloc) {
                        rdalloc += ROOMDSC_GROW_RATE;
                        bufmem = (char *) grow(bufmem, rdalloc + 2, "Extending RDesc Line");
                    }
                    base = bufmem + mem_off;
                    /* Read some more text */
                    if (fgets(base, rdalloc - mem_off, ifp) == NULL)
                        break;
                }
                if (q == NULL)
                    q = base + strlen(base);
                /* Remove the carriage return */
                *q = ' ';
                *(q + 1) = 0;
                /* Trim any extra trailing white spaces */
                while (isspace(*(--q)))
                    *(q + 1) = 0;
                /* Grow the memory buffer */
                mem_off += strlen(base);
                if ((size_t)(mem_off + ROOMDSC_GROW_RATE) >= rdalloc) {
                    rdalloc += ROOMDSC_GROW_RATE;
                    bufmem = (char *) grow(bufmem, rdalloc + 2, "Extra Memory for Room Descrip");
                }
            }

            if (bufmem && *bufmem != 0 && *bufmem != '\n') {
                rmp->l_descrip = add_msg(NULL);
                fwrite(bufmem, (size_t)(mem_off + 1), 1, msgfp);
            }
        }
    }
    if (bufmem)
        free(bufmem);

    // We don't do any post processing, instead finish_rooms gets
    // called later
    errabort();
}

// After we've finished processing all the objects, we'll be able
// to tell for definite whether any of the dmove's are invalid; we
// can't tell before because it's quite legal to have a dmove
// sending all of your objects to the inside of another object.
// Example: room which contains a swamp, if you die in the room,
// all your objects end up in the swamp.
void
finish_rooms(void)
{
    /* Process the dmove tables */
    /* The DMOVE flag specifies that when a player dies in a given room,
     * their inventory should be relocated to a different room (e.g. if
     * you drown underwater, you might want the players inventory to be
     * washed ashore). Because the user isn't forced to specify these in
     * order, we need to resolve any DMOVEs that weren't already
     */
    while ((dmv = first)) {
        first = dmv->next;
        ROOM *rmp = dynamic_cast<ROOM *>(bobs[dmv->room]);
        assert(rmp);
        basic_obj dest = is_container(dmv->to);
        if (dest == -1 || bobs[dest]->max_weight == 0)
            error("%s: Invalid dmove location, '%s'\n", word(rmp->id), dmv->to);
        else
            rmp->dmove = dest;
        free(dmv->to);
        free(dmv); /* Release this memory */
    }

    errabort();

    /* Write copy of stuff to disk */
    fopenw(roomsfn);
    for (ROOM *rmp = roomtab; rmp && rmp->type == WROOM; rmp = rmp->getNext(rmp)) {
        rmp->Write(ofp1);
    }
    close_ofps();
}
