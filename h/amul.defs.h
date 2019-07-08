#pragma once

// defines and enums
/// TODO: Move enums to their own file

enum {
    // 'node' being an instance of the AMUL engine
    NUM_USER_NODES = 14,
    NUM_MOBILE_NODES = 1,
    NUM_DAEMON_NODES = 1,

    MAXNODE = NUM_USER_NODES + NUM_MOBILE_NODES + NUM_DAEMON_NODES,
    MAXU = NUM_USER_NODES,
};

// Old hard-coded value replacement. Conversion in-progress.
enum { OBJ_DESC_NONE = -2 };

enum {
    NPUTS = 4,
    NPREP = 6,
    NRFLAGS = 15,
    NRNULL = 1,
    NOFLAGS = 8,
    NOPARMS = 5,
    NSFLAGS = 7,
    NCONDS = 64,
    NACTS = 96,
    NSYNTS = 12,
    NACTUALS = 33,
};

enum Truth { NO, YES };

enum PlaceholderClass {
    PC_ACTION,
    PC_NOISE,
    PC_EVENT,
    PC_TEXTS,
};

// Get AMUL, AMAN and AMULCOM versions from relevant .H
#define PV "0.99d"       // Parser version
#define IDL 16           // Length of ID strings
#define RANKL 32         // Length of rank descs
#define NAMEL 32         // Length of names
#define ALWAYSEP "---"   // Always Endparse
#define INS (MAXU + 10)  // Start of insides

// Modes
enum UserState {
    US_OFFLINE,
    US_LOGGING_IN,
    US_CONNECTED,
};

enum Role {
    ROLE_PLAYER,  // Service players
    ROLE_DAEMON,  // Service daemons
    ROLE_NPCS,    // Service NPCs
};

// Room bit-flags
enum RoomFlag {
    RF_CEMETERY = 0x00000001,      // Dead players are moved here (dmove)
    RF_PLAYER_START = 0x00000002,  // Players can start from this room
    RF_OBJECT_START = 0x00000004,  // Random objects can start here..
    RF_DARK = 0x00000008,          // Room has no lighting
    RF_TINY = 0x00000010,          // Only 1 player at a time
    RF_LETHAL = 0x00000020,        // Players die after reading description
    RF_OPAQUE = 0x00000040,        // Cannot look into this room
    RF_SILENT = 0x00000080,        // Cannot hear outside noises
    RF_HIDE_PLAYERS = 0x00000100,  // Players cannot be seen from outside
    RF_SINKHOLE = 0x00000200,      // Score points for dropped objects
    RF_HIDE_OBJECTS = 0x00000400,  // Objects in here cannot be seen
    RF_SANCTUARY = 0x00000800,     // No fighting allowed here
    RF_HIDE_EXITS = 0x00001000,    // Can't list exits
};

// Object flag bits
enum ObjectFlag {
    OF_OPENS = 0x00000001,     // Object is openable
    OF_SCENERY = 0x00000002,   // Object is scenery
    OF_COUNTER = 0x00000004,   // Ignore me!
    OF_FLAMABLE = 0x00000008,  // Can we set fire to it?
    OF_SHINES = 0x00000010,    // Can it provide light?
    OF_SHOWFIRE = 0x00000020,  // Say 'The <noun> is on fire' when lit
    OF_INVIS = 0x00000040,     // Object is invisible
    OF_SMELL = 0x00000080,     // Object has a smell not visual
    OF_ZONKED = 0x00000100,    // Object was zonked!
};

// Object parameter flag no.'s
enum ObjectParameter {
    OP_ADJ = 0x00000001,    // ADJ=?
    OP_START = 0x00000002,  // START=?
    OP_HOLDS = 0x00000004,  // HOLDS=?
    OP_PUT = 0x00000008,    // PUT=?
    OP_MOB = 0x00000010,    // Mobile=
};

// Object/state flags
enum ObjectState {
    SF_LIT = 0x00000001,     // Object is lumious
    SF_OPEN = 0x00000002,    // Object is open
    SF_CLOSED = 0x00000004,  // Object is closed
    SF_WEAPON = 0x00000008,  // Its a weapon
    SF_OPAQUE = 0x00000010,  // Can see inside object
    SF_SCALED = 0x00000020,  // Scale the value
    SF_ALIVE = 0x00000040,   // Mobile/Animated
};

// How things are placed relative to an object
enum Placement {
    PUT_IN,      // put in
    PUT_ON,      // on...
    PUT_BEHIND,  // behind...
    PUT_UNDER,   // under!
};

// verb flags!
enum VerbFlag {
    VB_TRAVEL = 0x00000001,  // Verb is travel verb
    VB_DREAM = 0x00000002,   // Verb can be executed whilst sleeping
};

enum Action {
    AQUIT,       // Quit action
    ASAVE,       // Save players details
    ASCORE,      // Show players status
    ASETSTAT,    // Set object state
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
    AFLEE,        // The chicken's way out
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
};

// Conditions
enum Condition {
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
};

// Condition/Action Paramter types
enum Parameter {
    CAP_REAL = -70,      // Noun or slot label
    CAP_NOUN = 1,        // Must be a noun
    CAP_ADJ = 2,         // Must be an adj
    CAP_PLAYER = 4,      // Must be a player
    CAP_ROOM = 5,        // Must be a room
    CAP_UMSG = 7,        // Must be text
    CAP_VERB = 8,        // Must be a verb
    CAP_ARCHETYPE = 9,   // Must be a class
    CAP_NUM = 10,        // Must be a number
    CAP_ROOM_FLAG = 11,  // Must be a room flag
    CAP_OBJ_FLAG = 12,   // Must be an obj flag
    CAP_STAT_FLAG = 13,  // Must be a stat flag
    CAP_GENDER = 14,     // Must be a gender
    CAP_DAEMON_ID = 15,  // Must be a daemon ID
};

// Spells
enum Spell {
    SPELL_GLOW = 1,
    SPELL_INVISIBLE,
    SPELL_BLIND,
    SPELL_CRIPPLE,
    SPELL_DEAFEN,
    SPELL_MUTE,
    SPELL_SLEEP,
    SPELL_SUPER_INVIS,
};

enum Stat {
    STSCORE = 1,  // Score
    STSTR,        // Strength
    STSTAM,       // Stamina
    STDEX,        // Dexterity
    STWIS,        // Wisdom
    STEXP,        // Experience
    STMAGIC,      // Magic points
    STSCTG,       // Score This Game
};

// Anouncement types
enum AnnouncementType {
    AGLOBAL,
    AEVERY1,
    AOUTSIDE,
    AHERE,
    AOTHERS,
    AALL,
};

// -- Message Types --
enum Message {
    MSG_INVALID,
    MSG_KILL,           // Close down
    MSG_CONNECT,        // Connection
    MSG_DISCONNECT,     // Disconnect
    MSG_DATA_REQUEST,   // Gets ptrs!
    MSG_LOGGED_IN,      // Logged in!
    MSG_MESSAGE,        // Sent a msg
    MSG_CLOSEING,       // Closeing..
    MSG_RESET,          // Reset
    MSG_LOCK,           // Line Lock
    MSG_UNLOCK,         // Unlock it
    MSG_SUMMONED,       // COME HERE!
    MSG_DIE,            // Ciao!!
    MSG_BUSY,           // I'm busy
    MSG_FREE,           // I'm free!
    MSG_EXECUTE,        // Execute a command!
    MSG_DAEMON_START,   // Daemon start
    MSG_DAEMON_CANCEL,  // Cancel a daemon
    MSG_DAEMON,         // We have lift off!
    MSG_DAEMON_STATUS,  // Get daemon status
    MSG_FORCE,          // Do it, buddo!
    MSG_MADE_ADMIN,     // Reached top rank!
    MSG_LOG,            // Write log entry
    MSG_RESET_WARNING,  // Reset Warning
    MSG_EXTENDED,       // Extend game
    MSG_GDAEMON_START,  // Start global daemon
    MAX_MESSAGE,
};

enum TokenClass {  // I originally called these 'wtypes' for "word types"
    TC_NONE,       // None!
    TC_ANY,        // Anything!
    TC_NOUN,       // Word is a noun
    TC_ADJ,        // Word is an adjective
    TC_PREP,       // Its a prep
    TC_PLAYER,     // Its a player
    TC_ROOM,       // Its a room ID
    TC_SYN,        // Its a synonym
    TC_TEXT,       // Its text
    TC_VERB,       // Its a verb!
    TC_CLASS,      // Class name
    WC_NUMBER,     // A number
};

// -- Player flags --
enum PlayerFlag {
    PFINVIS = 0x00000001,            // Player invisible
    PFGLOW = 0x00000002,             // Player glowing
    PFBLIND = 0x00000004,            // Blind
    PFDUMB = 0x00000008,             // Can't speak
    PFDEAF = 0x00000010,             // Player's deaf
    PFCRIP = 0x00000020,             // Can't move
    PFDYING = 0x00000040,            // Player is dying
    PFLIMP = 0x00000080,             // Limping
    PFASLEEP = 0x00000100,           // Sleeping
    PFSITTING = 0x00000200,          // Sitting down
    PFLYING = 0x00000400,            // Lying Down
    PFFIGHT = 0x00000800,            // Fighting
    PFATTACKER = 0x00001000,         // If you started the fight
    PFMOVING = 0x00002000,           // If you are 'in transit'
    PFSPELL_INVISIBLE = 0x00004000,  // Player is Super Invis
};

enum RoomDescMode {
    RD_VERBOSE_ONCE,  // Only describe on first visit
    RD_VERBOSE,       // Always describe rooms
    RD_TERSE,         // Never describe rooms
};

enum Verbosity {
    VERBOSE,  // Verbose mode
    TERSE,    // Terse mode
};

#define At amul->type
#define Am amul
#define Af amul->from
#define Ad amul->data
#define Ap amul->ptr
#define Ap1 amul->p1
#define Ap2 amul->p2
#define Ap3 amul->p3
#define Ap4 amul->p4

#define AMt amanp->type
#define AMm amanp
#define AMf amanp->from
#define AMd amanp->data
#define AMp amanp->ptr
#define Apx1 amanp->p1
#define Apx2 amanp->p2
#define Apx3 amanp->p3
#define Apx4 amanp->p4

#define IAt intam->type
#define IAm intam
#define IAf intam->from
#define IAd intam->data
#define IAp intam->ptr

// -- User Flags --
enum UserFlag {
    UF_ANSI = 0x0001,  // Enable ANSI output
    UF_CRLF = 0x0002,  // Add linefeeds
};

enum UserDefault {
    UD_LINE_LENGTH = 80,
    UD_SCREEN_LINES = 24,
    UD_REDO_CHAR = '|',
    UD_FLAGS = UF_CRLF,  // Default user flags
};

// Helper macros and wannabe member functions
#define isOINVIS(x) ((obtab + x)->flags & OF_INVIS)
#define isPINVIS(x) ((lstat + x)->flags & PFINVIS)
#define IamINVIS (me2->flags & PFINVIS)
#define IamSINVISIBLE (me2->flags & PFSPELL_INVISIBLE)
#define pROOM(x) ((lstat + x)->room)
#define pRANK(x) ((usr + x)->rank)
#define myRANK me->rank
#define mySCORE me->score
#define myROOM me2->room
#define LightHere lit(me2->room)
#define unfreeze                                                                                   \
    Permit();                                                                                      \
    return

#include "h/amul.acts.h"
#include "h/amul.msgs.h"  // System message defines

#define CP1 actual(*(tt.pptr))
#define CP2 actual(*(tt.pptr + 1))
#define CP3 actual(*(tt.pptr + 2))
#define CP4 actual(*(tt.pptr + 3))
#define TP1 actual(*(tt.pptr + conditions[tt.condition].parameterCount))
#define TP2 actual(*(tt.pptr + conditions[tt.condition].parameterCount + 1))
#define TP3 actual(*(tt.pptr + conditions[tt.condition].parameterCount + 2))
#define TP4 actual(*(tt.pptr + conditions[tt.condition].parameterCount + 3))
#define AP1 (char *)actptr(*(tt.pptr + conditions[tt.condition].parameterCount))
#define AP2 (char *)actptr(*(tt.pptr + conditions[tt.condition].parameterCount + 1))
#define AP3 (char *)actptr(*(tt.pptr + conditions[tt.condition].parameterCount + 2))
#define AP4 (char *)actptr(*(tt.pptr + conditions[tt.condition].parameterCount + 3))
#define STATE (objtab->states + (int32_t)objtab->state)
#define State(i) ((obtab + i)->states + (int32_t)(obtab + i)->state)
#define ItsState (it->states + (int32_t)it->state)
#define xLIGHT(x) (lstat + x)->light
#define xHADLIGHT(x) (lstat + x)->hadlight

#define acp (char *)actptr
