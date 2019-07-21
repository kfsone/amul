#pragma once
// This may look like C, but it's really -*- C++ -*-
// Stuff for 'basicobjs.C'

// Class BASIC_OBJ is already defined, thanks.

extern counter_t nbobs;			// Number of basic objects
extern counter_t ncontainers;	// Number of containers
extern BASIC_OBJ **bobs;		// Basic object index
extern CONTAINER *containers;	// Container table

extern bool from_container(container_t);	// Remove an objects presence from a container
extern bool into_container(container_t, basic_obj);	// Add objects presence to a container

class   BobIdx
{
  public:
	// Locate an object of a given type
	static basic_obj find(vocid_t name, char type = WANY, basic_obj from = -1);
};

