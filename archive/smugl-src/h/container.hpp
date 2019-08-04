#ifndef SMUGL_H_CONTAINER_H
#define SMUGL_H_CONTAINER_H

#include "typedefs.hpp"

/* The container object: Indicates which objects are inside
** which other objects. Each object has an array of CONTAINERs
** describing all of the objects locations.
** All instances of an object are stored consecutively
*/
struct CONTAINER {
    basic_obj boSelf;       // The object being 'contained'
    basic_obj boContainer;  // The object 'self' is inside
    // The linked list lists contents of rooms; all locations of
    // an individual object are stored consecutively
    container_t conNext;  // } Neighbours in a
    container_t conPrev;  // } linked list
};

#endif  // SMUGL_H_CONTAINER_H
