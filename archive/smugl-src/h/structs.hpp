// This may look like C, but it's really -*- C++ -*-
// $Id: structs.hpp,v 1.17 1999/06/11 14:26:08 oliver Exp $
// Structure definitions

#ifndef STRUCTS_H
#define STRUCTS_H 1

#include "defines.hpp"
#include "typedefs.hpp"

#include "cl_basicobj.hpp"
#include "cl_object.hpp"
#include "cl_player.hpp"
#include "cl_room.hpp"
#include "container.hpp"

struct OBJ_STATE {     // State description
    long weight;       // In grammes
    long value;        // Whassit worth?
    long strength;     // How strong is it?
    long damage;       // How much damage can it inflict?
    msgno_t descrip;   // ptr to descrp in file
    flag_t std_flags;  // Which std_flags we affect
    flag_t flags;      // State flags
};

// MOB_ENT defines a mobile character, not an actual mobile.
// There can be multiple objects with the same mobile
// character attached to them. The object is actually the
// mobile.
struct MOB_ENT {                           // Mobile Character Entry
    vocid_t id;                            // Name of mobile (vocab entry)
    char speed, travel, fight, act, wait;  // speed & %ages
    char fear, attack;                     // others
    char dead;                             // State flags
    flag_t flags;                          // Mobile state flags
    basic_obj dmove;                       // Where to move on death (?)
    short hitpower;                        // Hit power
    short strength;                        // How strong is he?
    short stamina;                         // Stamina
    short dext;                            // Dexterity
    short wisdom;                          // Wisdom
    short experience;                      // Experience
    short magicpts;                        // Magic points
    msgno_t arr;                           // 'has arrived' message
    msgno_t dep;                           // 'has departed' message
    msgno_t flee;                          // 'has fled' message
    msgno_t hit;                           // 'has hit you' message
    msgno_t miss;                          // 'has missed you' message
    msgno_t death;                         // 'has died' message
};

struct VERB {    // Verb def struct
    vocid_t id;  // The Verb itself
    union {
        char precedence[2][4];
        char precedences[8];
    };
    short ents;           // No. of slot entries
    flag_t flags;         // Travel? etc...
    struct SLOTTAB *ptr;  // Pointer to slots tab
};

struct SLOTTAB {        // Slot table def
    char wtype[2];      // Word type expected
    long slot[2];       // List of slots
    adjid_t adj[2];     // List of adjectives
    uint16_t ents;      // No. of entries
    struct VBTAB *ptr;  // Points to Verb Table
};

struct VBTAB {           // Verb Table struct
    bool not_condition;  // Condition negator
    long condition;      // Condition
    bool action_type;    // TRUE = action, FALSE = room
    long action;         // action or room
    long *pptr;          // Param ptr; -1=none
};

struct TT_ENT : public VBTAB {  // TT Entry
    long verb;                  // Verb no. (not vocab id)
};

struct RANKS {           // Rank information
    char male[RANKL];    // chars for male descrp
    char female[RANKL];  // Women! Huh!
    long strength;       // How strong is he?
    long stamina;        // Stamina
    long dext;           // Dexterity
    long wisdom;         // Wisdom
    long experience;     // Experience
    long magicpts;       // Magic points
    long numobj;         // Max. objects carried
    long tasks;          // Tasks needed for level
    long score;          // Score to date
    long maxweight;      // Maximum weight
    long minpksl;        // Min. pts for killin
    msgno_t prompt;      // Prompt 4 this rank
};

struct MOB_TAB {       // Mobile table entry
    basic_obj obj;     // Object no.
    short speed;       // Secs/turn
    short count;       // Time till next
    flag_t std_flags;  // Standard flags
    flag_t flags;      // Player style flags
};

/* ARGS is used by the SMUGLcom condition/action table */
struct ARGS {          // action/condition: name and arguments, etc
    const char *name;  // name of the thing
    u_char argc;       // number of arguments it takes
    arg_t argv[3];     // list of arguments it accepts
};

struct ALIAS  // For synonyms
{
    vocid_t given;  // The given name
    vocid_t means;  // The real word
};

#ifdef AMAN
struct DAEMON {  // AMan Daemon Timer
    struct DAEMON *nxt, *prv;
    short own, count, num;
    long val[2], typ[2];
};
#endif

#endif /* STRUCTS_H */
