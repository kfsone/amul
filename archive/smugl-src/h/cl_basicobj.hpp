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

#include <cstdio>  // For FILE*

class BASIC_OBJ
{
  public:
    virtual ~BASIC_OBJ() {}

    //// Basic_Obj::FUNCTIONS
    virtual int describe(void) = 0;
    virtual int describe_verbose(void) = 0;
    int is_in(basic_obj item);  // Tests if one object is here
    virtual int Write(FILE *);  // Write out this object
    virtual int Read(FILE *);   // Load this object from disk
    void clear(void);           // Used by SMUGLCOM

    //// Basic_Obj::DATA
    vocid_t id{ -1 };              // Vocab table entry
    vocid_t adj{ -1 };             // Adjective
    basic_obj bob{ -1 };           // Because we're stored non-linearly
    BASIC_OBJ *next{ nullptr };    // forward linked list
    short type{ 0 };               // Type of object (player, noun, ...)
    short state{ 0 };              // Current state
    flag_t std_flags{ 0 };         // Common object flags
    flag_t flags{ 0 };             // Type specific flags
    long weight{ 0 };              // How much it weighs (on it's own)
    long max_weight{ 0 };          // Maximum contents (grammes)
    long contents_weight{ 0 };     // Grammes contained
    long value{ 0 };               // What's this worth?
    long damage{ 0 };              // How much damage we've taken
    long strength{ 0 };            // How much we can take
    counter_t locations{ 0 };      // How many locations it's in
    counter_t contents{ 0 };       // How many objects we contain
    container_t conLocation{ 0 };  // Where it's at
    container_t conTent{ 0 };      // What it contains
    msgno_t s_descrip{ 0 };        // Short description
    msgno_t l_descrip{ 0 };        // Extended description
    basic_obj dmove{ 0 };          // Where to move objects on death;
                                   // -1 for drop in parent

    template<typename T>
    T *getNext(T *) noexcept
    {
        for (auto *cur = next; cur; cur = cur->next) {
            auto *ptr = dynamic_cast<T *>(cur);
            if (ptr)
                return ptr;
        }
        return nullptr;
    }
};

#define WRITE(field) fwrite((void *) (&field), sizeof(field), 1, file)

#define READ(field) fread((void *) (&field), sizeof(field), 1, file)

#endif /* BASIC_OBJ_H */
