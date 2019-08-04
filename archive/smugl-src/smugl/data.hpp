// This may look like C, but it's really -*- C++ -*-
// $Id: data.hpp,v 1.8 1999/06/08 15:36:50 oliver Exp $
// Most important; the data structure is the heart of the shared memory
// blocks. All of the base information will go in here. It should be
// read only, but it isn't [for laziness sake ;-)].

#ifndef DATA_H
#define DATA_H 1

#include "basicobjs.hpp"
#include "cl_vocab.hpp"
#include "players.hpp"

struct DATA {
    int semid;        // Semaphore ID for locking
    uint16_t errors;  // Global error count
    uint16_t wflags;  // World flags

    // Basic game information (from advfn)
    char name[ADNAMEL + 1];     // The game's name
    char logfile[ADNAMEL + 1];  // Name of the output logfile
    long compiled;              // Time stamp of last compile
    long game_start;            // Time stamp of last game start
    long time;                  // Time left in game
    int see_invis;              // Rank where invis can see each other
    int all_see_invis;          // Rank where anyone can see invis's
    int minsgo;                 // Rank where players can use sgo
    int rscale;                 // Rank scaling factor on object values
    int tscale;                 // Time scaling factor on object values
    int port;                   // Port number to run on
    char lastres[ADNAMEL + 1];  // Last reset in ascii
    char lastcrt[ADNAMEL + 1];  // Last compile in ascii

    // Pointer table
    void *shmbase;  // Base of shared memory
    long *msgbase;  // Base of text messages
    class Room *roombase;
    class Rank *rankbase;
    class Mobile *mobbase;
    class Object *objbase;
    class Verb *verbbase;
    class TTEnt *ttbase;
    class Alias *aliasbase;

    // Item counts
    counter_t rooms;  // Number of rooms in game
    counter_t ranks;
    counter_t mobiles;
    counter_t objects;
    counter_t verbs;
    counter_t ttents;
    counter_t aliases;

    // The vocab table data
    struct VOCAB VC;

    // Useful information
    counter_t start_rooms;  // Number of start rooms
    class Room *anterm;     // If there's an ante-room

    // Multi-user status
    counter_t connections;  // Connections this game has seen
    int connected;          // Number of people connected
    int pid[MAXU];          // Process ID's
    Player user[MAXU];      // Player details
};

extern struct DATA *data;
#endif /* DATA_H */
