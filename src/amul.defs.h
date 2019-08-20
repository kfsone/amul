#ifndef H_AMUL_DEFS_H
#define H_AMUL_DEFS_H
/*
 ****    AMUL.DEFS.H.....Adventure Compiler    ****
 ****                #defines!                 ****
 */

#include "parser.wtype.h"  ///TODO: Remove.
#include "typedefs.h"

enum {
    NRFLAGS = 16,
    NOFLAGS = 8,
    NOPARMS = 5,
    NSFLAGS = 7,
    NPUTS = 4,
    NPREP = 6,
    NSYNTS = 12,
};

enum : int {
    // Each client takes up a "slot" in an array of users, including the non-user
    // workers. These are then mapped to a 0-indexed bit array (player 5 = 1 << 4).
    // Original AMUL was on a 16-bit machine.
    MAXU = 14,  // Maximum player 'node's at once
    DEMON_SLOT = MAXU,
    NPC_SLOT = MAXU + 1,
    SYSNODES = 2,               // Daemon and NPC processors
    MAXNODE = MAXU + SYSNODES,  // Total nodes
};

enum Audibility {
    LOUD = 1,
    QUIET = 2,
};

enum BroadcastType {
    ACTION,  // involves motion so restricted to visibility and requires sight
    NOISE,   // something you hear so can be heard afar and requires hearing
    EVENT,   // ignores sight/hearing tests,
    TEXTS,   // hadn't been implemented as of the snapshot
};

// Get AMUL, AMAN and AMULCOM versions from relevant .H
#define PV "0.99d"       // Parser version
#define IDL 16           // Length of ID strings
#define RANKL 32         // Length of rank descs
#define NAMEL 20         // Length of names
#define ALWAYSEP "---"   // Always Endparse
#define INS (MAXU + 10)  // Start of insides!

// state of client slots
enum SlotState : uint8_t {
    OFFLINE,  // not in-use
    LOGGING,  // as in: logging in
    PLAYING,  // in-use
};

// Preposition flags
enum PrepositionType {
    PUT_IN,
    PUT_ON,
    PUT_BEHIND,
    PUT_UNDER,
};

// verb flags!
enum VerbFlag {
    VB_TRAVEL = 1,  // Verb can be used to move around the game
    VB_DREAM = 2,   // Verb can be executed while asleep
};

enum ActionID {
    AGOTO_ROOM,  // Move player to a room
    AQUIT,       // Quit action
    ASAVE,       // Save player details
    ASCORE,      // Show player status
    ASETSTATE,   // Set object state
    ALOOK,       // Look at this room
    AWHAT,       // List room inventory
    AWHERE,      // List where objs are
    AWHO,        // List who is playing
    ATREATAS,    // Process as 'verb'
    AMESSAGE,    // Send message to me
    ASKIP,       // Skip next x entries
    AENDPARSE,   // Stop parsing
    AKILLME,     // Kill player
    AFAILPARSE,  // EndParse + Fail
    AFINISHPARSE,
    AABORTPARSE,
    AWAIT,        // Waits for n seconds
    ABLEEP,       // Prints '.' n times
    AWHEREAMI,    // Function for 'where'
    ASEND,        // Send object...
    AANOUN,       // Make anouncement
    AGET,         // Pick something up
    ADROP,        // Drop it
    AINVENT,      // List objects carried
    AREPLY,       // Same as message
    ACHANGESEX,   // Change players sex
    ASLEEP,       // Put player to sleep
    AWAKE,        // Wake player up
    ASIT,         // Makes player Sit down
    ASTAND,       // Makes player stand up
    ALIE,         // Makes player lie down
    ARDMODE,      // Set RD Mode
    ARESET,       // Reset the game
    AACTION,      // Like anounce
    AMOVE,        // Move player quietly
    ATRAVEL,      // Process tt instead
    AMSGIN,       // Announce to a room
    AACTIN,       // Action to a room
    AMSGFROM,     // Annouce via object
    AACTFROM,     // Action via object
    ATELL,        // Tell someone summats
    AADDVAL,      // Add object value
    AGIVE,        // Give object to user
    AINFLICT,     // Cast spell
    ACURE,        // Remove spell
    ASUMMON,      // Summon Player
    AADD,         // Add stats to player
    ASUB,         // Minus stats to player
    ACHECKNEAR,   // 'near' processing
    ACHECKGET,    // 'get' checking
    ADESTROY,     // Destroy object
    ARECOVER,     // Recover object
    ASTART,       // Start a daemon
    ACANCEL,      // Cancel a daemon
    ABEGIN,       // Force daemon to start
    ASHOWTIMER,   // Displays time left
    AOBJAN,       // Announce from obj
    AOBJACT,      // Action from object
    ACONTENTS,    // Show obj contents
    AFORCE,       // Force em!
    AHELP,        // Help someone!
    ASTOPHELP,    // Stop helping someone
    AFIX,         // Fixes players stat
    AOBJINVIS,    // Turns an obj. invis
    AOBJSHOW,     // Displays an invis obj
    AFIGHT,       // Start a fight
    AFLEE,        // The chickens way out
    ALOG,         // Text to the LOG file
    ACOMBAT,      // Combat action etc.
    AWIELD,       // Use a wpn in combat
    AFOLLOW,      // Follow someone!
    ALOSE,        // Lose your tail.
    ASTOPFOLLOW,  // Stop following
    AEXITS,       // Shows exits to a room
    ATASK,        // Sets the tasks done
    ASHOWTASK,    // Shows the tasks done
    ASYNTAX,      // Set slots
    ASETPRE,      // Set pre-rank desc
    ASETPOST,     // Set post-rank desc
    ASENDDAEMON,  // Send a daemon
    ADO,          // GOSUB
    AINTERACT,    // Set actor
    AAUTOEXITS,   // Auto-exits
    ASETARR,      // Set arrived string
    ASETDEP,      // Set departed string
    ARESPOND,     // Reply, endparse
    AERROR,       // Give an error!
    ABURN,        // Ignite an object
    ADOUSE,       // Put it out again
    AINC,         // Increment state
    ADEC,         // Decrement
    ATOPRANK,     // Make player toprank
    ADEDUCT,      // Reduce score by %
    ADAMAGE,      // Damage object
    AREPAIR,      // Repair object
    AGSTART,      // Start global daemon

    NACTS,
};

// Conditions
enum ConditionID {
    CAND,         // And then ....
    CSTAR,        // Same as always
    CELSE,        // If last was false
    CALWAYS,      // Always do this
    CLIGHT,       // If there is light...
    CISHERE,      // If obj is here
    CMYRANK,      // If my rank is...
    CSTATE,       // If state of obj
    CMYSEX,       // My sex is...
    CLASTVB,      // If last verb was..
    CLASTDIR,     // If last TRAVEL verb
    CLASTROOM,    // If last room was
    CASLEEP,      // Is player sleeping
    CSITTING,     // Is player sitting
    CLYING,       // Is player lying down
    CRAND,        // If rand(n1) eq n2
    CRDMODE,      // If rdmode is...
    CONLYUSER,    // If only player
    CALONE,       // If only person here
    CINROOM,      // If player in room
    COPENS,       // If object opens
    CGOTNOWT,     // If carrying nothing
    CCARRYING,    // Carrying object?
    CNEARTO,      // Is it here SOMEWHERE
    CHIDDEN,      // Can others see me?
    CCANGIVE,     // Can player manage it
    CINFL,        //
    CINFLICTED,   // Is played inflicted
    CSAMEROOM,    // Same room as player?
    CSOMEONEHAS,  // If obj is carried?
    CTOPRANK,     // If u'r the top rank
    CGOTA,        // Carrying obj in stat
    CACTIVE,      // Is daemon active?
    CTIMER,       // Check time left
    CBURNS,       // If object burns
    CCONTAINER,   // If its a container
    CEMPTY,       // If object is empty
    COBJSIN,      // Check # of contents
    CALTEP,       // Always .., endparse
    CANTEP,       // And    .., endparse
    CHELPING,     // Are we helping him?
    CGOTHELP,     // If we've got help
    CANYHELP,     // Helping ANYONE?
    CELTEP,       // Else   ..., endparse
    CSTAT,        // If attrib <> no.
    COBJINV,      // If object invisible
    CFIGHTING,    // Is player fighting?
    CTASKSET,     // Has task been done?
    CCANSEE,      // Can I see <player>
    CVISIBLETO,   // Am I visible to
    CNOUN1,       // Match noun1
    CNOUN2,       // Match noun2
    CAUTOEXITS,   // Auto exits on?
    CDEBUG,       // Debug mode on?
    CFULL,        // Stat at full?
    CTIME,        // Time remaining?
    CDEC,         // Decrement & test
    CINC,         // Increment & test
    CLIT,         // Is object lit?
    CFIRE,        // Is object flamable?
    CHEALTH,      // Is players health %?
    CMAGIC,       // Can magic be done?
    CSPELL,       // Can spell be done?
    CIN,          // IN <ROOM> <NOUN>
    CEXISTS,

    NCONDS
};

// Anouncement types
enum AnnounceType {
    AGLOBAL,
    AEVERY1,
    AOUTSIDE,
    AHERE,
    AOTHERS,
    AALL,
    ACANSEE,
    ANOTSEE,

    MAX_ANNOUNCE_TYPE,
};

// -- Message Types --
#define MKILL 1       // Close down
#define MDATAREQ 4    // Gets ptrs!
#define MMESSAGE 6    // Sent a msg
#define MCLOSEING 7   // Closeing..
#define MRESET 8      // Reset
#define MLOCK 9       // Line Lock
#define MUNLOCK 10    // Unlock it
#define MSUMMONED 11  // COME HERE!
#define MDIE 12       // Ciao!!
#define MBUSY 13      // I'm busy
#define MFREE 14      // I'm free!
#define MEXECUTE 15   // Execute a command!
#define MDSTART 16    // Daemon start
#define MDCANCEL 17   // Cancel a daemon
#define MCHECKD 19    // Get daemon status
#define MFORCE 20     // Do it, buddo!
#define MMADEWIZ 21   // Reached top rank!
#define MLOG 22       // Write log entry
#define MRWARN 23     // Reset Warning
#define MEXTEND 24    // Extend game
#define MGDSTART 25   // Start global daemon
#define MPING 26      // Ping packet

enum RoomDescMode {
    RDRC,  // Room count mode (only describe on first visit)
    RDVB,  // Verbose (always describe)
    RDBF,  // Brief mode (only show short description)
};

enum Verbosity {
    TYPEV,  // Verbose
    TYPEB,  // Brief
};

enum UserFlag {
    ufANSI = 0x01,  // Enable ANSI
    ufCRLF = 0x02,  // Add linefeeds
    ufARDO = 0x04,  // Auto redo (experimental readline type behavior)
};

// Default user settings
enum {
    DLLEN = 80,       // Default line length
    DSLEN = 24,       // Default screen length
    DRCHAR = '|',     // Default redo chararacter,
    DFLAGS = ufCRLF,  // Default = cr/lf ON
};

// -- Useful defines --

#define isOINVIS(objId) (GetObject(objId).flags & OF_INVIS)
#define isPINVIS(slotId) (GetAvatar(slotId).flags & PFINVIS)
#define isPSINVIS(slotId) (GetAvatar(slotId).flags & PFSINVIS)
#define IamINVIS (t_avatar->flags & PFINVIS)
#define IamSINVIS (t_avatar->flags & PFSINVIS)
#define pROOM(slotId) (GetAvatar(slotId).room)
#define pRANK(slotId) (GetCharacter(slotId).rank)
#define myRANK t_character->rank
#define mySCORE t_character->score
#define myROOM t_avatar->room

#define ConditionArg(n) condition.m_args[n]
#define CA1 ConditionArg(0)
#define CA2 ConditionArg(1)
#define CA3 ConditionArg(2)
#define CA4 ConditionArg(3)

#define VerbActionParam(n) GetConcreteValue(tt.pptr[ncop[tt.condition] + (n) -1])
#define AP1 VerbActionParam(1)
#define AP2 VerbActionParam(2)
#define AP3 VerbActionParam(3)
#define AP4 VerbActionParam(4)
#define xLIGHT(slot) (GetAvatar(slot)).light)
#define xHADLIGHT(slot) (GetAvatar(slot).hadlight)

#endif