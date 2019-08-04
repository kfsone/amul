#ifndef BASICOBJS_H
#define BASICOBJS_H 1

// This may look like C, but it's really -*- C++ -*-
// $Id: basicobjs.hpp,v 1.3 1999/05/25 13:08:12 oliver Exp $
// Stuff for 'basicobjs.C'

#include "typedefs.hpp"

extern counter_t nbobs;               // Number of basic objects
extern counter_t ncontainers;         // Number of containers
extern struct BASIC_OBJ **bobs;       // Basic object index
extern struct CONTAINER *containers;  // Container table

extern int from_container(container_t);             // Remove an objects presence from a container
extern int into_container(container_t, basic_obj);  // Add objects presence to a container

#endif  // BASICOBJS_H
