// Basic Object routines
// Mostly to do with table management (AGAIN)

#include "fileio.hpp"
#include "libprotos.hpp"
#include "smuglcom.hpp"

#include <cstring>

// Container variables
constexpr size_t CONTAINER_GROW_RATE = 1024;  // Containers to allocate at-a-time
CONTAINER *containers;                        // The container table
counter_t ncontainers;                        // Number of containers
counter_t ncontainers_allocd;                 // Number allocated for

constexpr size_t BOB_GROW_RATE = 256;  // Bob indexes to allocate at-a-time
BASIC_OBJ **bobs;                      // The bob index table
counter_t nbobs;                       // Number of containers
counter_t nbobs_allocd;                // Number allocated for

// BASIC_OBJ::clear()
// Nuke the fields in a bob
void
BASIC_OBJ::clear()
{
    id = -1;
    adj = -1;
    bob = -1;
    next = nullptr;
    type = 0;
    state = 0;
    std_flags = 0;
    flags = 0;
    weight = 0;
    max_weight = 0;
    contents_weight = 0;
    value = 0;
    damage = 0;
    strength = 0;
    locations = 0;
    contents = 0;
    conLocation = -1;
    conTent = -1;
    s_descrip = l_descrip = -1;
    dmove = -1;
}

// add_container(for_which, into_what)
// Adds a container entry for an object
container_t
add_container(basic_obj boSelf, basic_obj boContainer)
{
    CONTAINER *cont;
    BASIC_OBJ *self_bob = bobs[boSelf];
    BASIC_OBJ *cont_bob = bobs[boContainer];

    // Make sure we've got enough memory allocated
    if (ncontainers >= ncontainers_allocd) {
        ncontainers_allocd += CONTAINER_GROW_RATE;
        const size_t new_size = ncontainers_allocd * sizeof(CONTAINER);
        containers =
                static_cast<CONTAINER *>(grow(containers, new_size, "Allocating container memory"));
    }

    cont = containers + ncontainers;
    cont->boSelf = boSelf;
    cont->boContainer = boContainer;

    // Add another 'presence' (location) of self
    if (self_bob->locations == 0)
        self_bob->conLocation = ncontainers;
    self_bob->locations++;
    cont->conNext = -1;  // } Assume we're on our own
    cont->conPrev = -1;  // } We'll double check below

    // If the container isn't the same object, add ourselves
    // to the contents of the object
    if (boContainer != boSelf) {
        // Now add ourself to the contents of 'container', and
        // maintain the linked list of it's contents
        if (cont_bob->contents == 0)
            cont_bob->conTent = ncontainers;  // We -are- the contents ;-)
        else {
            // Always insert at the tail of the list; find the tail
            container_t conLast = cont_bob->conTent;
            while (containers[conLast].conNext != -1)
                conLast = containers[conLast].conNext;
            containers[conLast].conNext = ncontainers;
            cont->conPrev = conLast;
        }
        cont_bob->contents++;  // It contains one more item
        cont_bob->contents_weight += self_bob->weight;
    }

    return ncontainers++;
}

// Determine if 'item' is inside 'container'
bool
is_inside(basic_obj boItem, basic_obj boContainer)
{
    container_t con;
    for (con = bobs[boContainer]->conTent; con != -1; con = containers[con].conNext) {
        if (containers[con].boSelf == boItem)
            return true;
    }
    return false;
}

// add_basic_obj(id)
// Adds a new object to the basic-object index
// Only thing it doesn't initialise is 'id' which it assumes you've
// already set
basic_obj
add_basic_obj(BASIC_OBJ *ptr, char type, flag_t flags)
{
    // Make sure we've got enough memory allocated
    if (nbobs >= nbobs_allocd) {
        nbobs_allocd += BOB_GROW_RATE;
        const size_t new_size = nbobs_allocd * sizeof(ptr);
        bobs = static_cast<BASIC_OBJ **>(
                grow(bobs, new_size, "Allocating basic object index memory"));
    }

    // Add us to the chain if neccesary
    if (nbobs > 0)
        bobs[nbobs - 1]->next = ptr;

    // Initialise all the various parameters
    bobs[nbobs] = ptr;
    ptr->bob = nbobs;
    ptr->type = type;
    ptr->next = nullptr;
    ptr->std_flags = flags;

    nbobs++;
    printf("Added %s basic obj#%d:", word(ptr->id), nbobs);
    printf("\n");

    return nbobs;
}

// Write the basic object set to disk
void
save_basic_objs()
{
    int fd;

    // Add space for the player objects to the 'bob' list
    PLAYER temp_player;
    temp_player.id = -1;
    for (int i = 0; i < MAXU; i++) {
        basic_obj newBob = add_basic_obj(&temp_player, WPLAYER, 0);
        add_container(newBob, newBob);
    }

    const int permissions = 0664;
    fd = open(datafile(bobfn), O_WRONLY | O_CREAT | O_TRUNC, permissions);
    if (fd == -1)
        Err("write", datafile(bobfn));
    // Write the indexes first
    write(fd, &nbobs, sizeof(counter_t));
    write(fd, &ncontainers, sizeof(counter_t));
    // Now we write out both the bobs and the containers. We write the
    // bob index out (which contains pointers, and so is non-transferable)
    // purely to make it easy to tell how big the bob index needs to be.
    // Otherwise it requires calculation.
    write(fd, bobs, nbobs * sizeof(*bobs));
    write(fd, containers, ncontainers * sizeof(*containers));
    close(fd);
}

// Locate a basic obj by name
basic_obj
is_bob(const char *name, char type /*=-1*/)
{
    vocid_t id = is_word(name);
    if (id == -1)
        return -1;
    BASIC_OBJ **bob = bobs;
    for (basic_obj i = 0; i < nbobs; i++, bob++)
        if ((*bob)->id == id && (type == -1 || (*bob)->type == type))
            return i;
    // Otherwise...
    return -1;
}

// Locate a basic object that can be a container, by name
basic_obj
is_container(const char *name)
{
    vocid_t id = is_word(name);
    if (id == -1)
        return -1;
    BASIC_OBJ **bob = bobs;
    for (basic_obj i = 0; i < nbobs; i++, bob++)
        if ((*bob)->id == id && (*bob)->max_weight != 0)
            return (*bob)->bob;
    // Otherwise...
    return -1;
}

// Process a flag, assuming it's a 'std_flag'.
// If it is, set the bit in 'flags', otherwise
// return:
//  <0 == unknown flag
//  >0 == not allowed by filter
//   0 == OK
int
handle_std_flag(const char *phrase, flag_t &flags, flag_t filter)
{
    // phrase = the word suspected to be a std_flag
    // flags = current bit-flags
    // filter = flags not allowed for this situation
    if (!*phrase)
        return 0;
    filter |= bob_INPLAY;  // You can't specify this
    int i, bit;
    for (i = 0, bit = 1; std_flag[i]; i++, bit = bit << 1) {
        if (strcmp(phrase, std_flag[i]) != 0)
            continue;  // Doesn't match
        if ((filter & bit) != 0)
            // Not allowed by filter
            return 1;
        flags |= bit;  // Add to flags
        return 0;
    }
    return -1;
}
