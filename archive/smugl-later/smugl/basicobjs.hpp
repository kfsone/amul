#ifndef BASICOBJS_H
#define BASICOBJS_H 1

// Stuff for 'basicobjs.C'

#include "typedefs.hpp"

extern counter_t nbobs;               // Number of basic objects
extern counter_t ncontainers;         // Number of containers
extern struct BASIC_OBJ **bobs;       // Basic object index
extern struct CONTAINER *containers;  // Container table

extern bool from_container(container_t);             // Remove an objects presence from a container
extern bool into_container(container_t, basic_obj);  // Add objects presence to a container

#endif  // BASICOBJS_H
