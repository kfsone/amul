// This may look like C, but it's really -*- C++ -*-
// $Id: cl_basicobj.hpp,v 1.1 1999/06/08 15:36:45 oliver Exp $
//
////////////////////////////// BASIC OBJECT STRUCTURE
//
// "AMUL" treated all objects seperately, all of them having to be
// handled completely differently. In SMUGL I intend to allow common
// operations across all object types, but still provide the control
// over specifics (e.g. you can still say "this must be a noun")

#ifndef BASIC_OBJ_H
#define BASIC_OBJ_H 1

class BASIC_OBJ
    {
public:
	virtual ~BASIC_OBJ() {}

    //// Basic_Obj::FUNCTIONS
    virtual int describe(void) = 0;
    virtual int describe_verbose(void) = 0;
    int is_in(basic_obj item);  // Tests if one object is here
    virtual int Write(FILE *);	// Write out this object
    virtual int Read(FILE *);	// Load this object from disk
    void clear(void);           // Used by SMUGLCOM

    //// Basic_Obj::DATA
    vocid_t id;                 // Vocab table entry
    vocid_t adj;                // Adjective
    basic_obj bob;              // Because we're stored non-linearly
    BASIC_OBJ *next;            // forward linked list
    short type;                 // Type of object (player, noun, ...)
    short state;                // Current state
    flag_t std_flags;           // Common object flags
    flag_t flags;               // Type specific flags
    long weight;                // How much it weighs (on it's own)
    long max_weight;            // Maximum contents (grammes)
    long contents_weight;       // Grammes contained
    long value;                 // What's this worth?
    long damage;                // How much damage we've taken
    long strength;              // How much we can take
    counter_t locations;        // How many locations it's in
    counter_t contents;         // How many objects we contain
    container_t conLocation;    // Where it's at
    container_t conTent;        // What it contains
    msgno_t s_descrip;          // Short description
    msgno_t l_descrip;          // Extended description
    basic_obj dmove;            // Where to move objects on death;
                                // -1 for drop in parent

	template<typename T>
	T* getNext(T*) noexcept {
		for (auto *cur = next; cur; cur = cur->next) {
			auto *ptr = dynamic_cast<T*>(cur);
			if (ptr)
				return ptr;
		}
		return nullptr;
	}
   };

#define WRITE(field) \
    fwrite((void *)(&field), sizeof(field), 1, file)

#define READ(field) \
    fread((void *)(&field), sizeof(field), 1, file)

#endif /* BASIC_OBJ_H */
