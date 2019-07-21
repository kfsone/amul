#ifndef H_AMUL_DEFS_H
#define H_AMUL_DEFS_H 1
/*
 ****    AMUL.DEFS.H.....Adventure Compiler    ****
 ****                #defines!                 ****
 */

enum {
    NRFLAGS = 17,
    NOFLAGS = 8,
    NOPARMS = 5,
    NSFLAGS = 7,
    NPUTS = 4,
    NPREP = 6,
    NSYNTS = 12,
};

#define MAXU 14             // Max users at 1 time
#define MAXNODE (MAXU + 2)  // + 1 mobiles+1 daemons

#define YES 0
#define NO 1

enum VolumeLevel {
    LOUD = 1,
    QUIET = 2,
};

#define ACTION 0
#define NOISE 1
#define EVENT 2
#define TEXTS 3

// Get AMUL, AMAN and AMULCOM versions from relevant .H
#define PV "0.99d"       // Parser version
#define IDL 12           // Length of ID strings
#define RANKL 32         // Length of rank descs
#define NAMEL 20         // Length of names
#define ALWAYSEP "---"   // Always Endparse
#define INS (MAXU + 10)  // Start of insides!

// Modes
#define OFFLINE 0
#define LOGGING 1
#define PLAYING 2

#define am_USER 0  // This AMUL is for a user
#define am_DAEM 1  // the daemon processor
#define am_MOBS 2  // Am the mobiles!

// IO Support types
#define CLIWINDOW 0              // Def */   /* Use CLI window
#define CUSSCREEN CLIWINDOW + 1  // Provide Custm Screen
#define SERIO CUSSCREEN + 1      // Serial IO
#define LOGFILE 99               // Output ONLY to log

// Room bit-flags
#define DMOVE 1        // When players die, move rooms to...
#define STARTL 2       // Players can start from this room
#define RANDOB 4       // Random objects can start here..
#define DARK 8         // Room has no lighting
#define SMALL 16       // Only 1 player at a time
#define DEATH 32       // Players die after reading descrip
#define NOLOOK 64      // Cannot look into this room
#define SILENT 128     // Cannot hear outside noises
#define HIDE 256       // Players cannot be seen from outside
#define SANCTRY 512    // Score points for dropped objects
#define HIDEWY 1024    // Objects in here cannot be seen
#define PEACEFUL 2048  // No fighting allowed here
#define NOEXITS 4096   // Can't list exits
#define ANTERM 8192    // Special Pre-Start start location
#define NOGORM 16834   // Can't random go to this room!

// Object flag bits
#define OF_OPENS 1       // Object is openable
#define OF_SCENERY 2     // Object is scenery
#define OF_COUNTER 4     // Ignore me!
#define OF_FLAMABLE 8    // Can we set fire to it?
#define OF_SHINES 16     // Can it provide light?
#define OF_SHOWFIRE 32   // Say 'The <noun> is on fire' when lit
#define OF_INVIS 64      // Object is invisible
#define OF_SMELL 128     // Object has a smell not visual
#define OF_ZONKED 32768  // Object was zonked!

// Object parameter flag no.'s
#define OP_ADJ 1    // ADJ=?
#define OP_START 2  // START=?
#define OP_HOLDS 4  // HOLDS=?
#define OP_PUT 8    // PUT=?
#define OP_MOB 16   // Mobile=

// Object/state flags
#define SF_LIT 1      // Object is lumious
#define SF_OPEN 2     // Object is open
#define SF_CLOSED 4   // Object is closed
#define SF_WEAPON 8   // Its a weapon
#define SF_OPAQUE 16  // Can see inside object
#define SF_SCALED 32  // Scale the value
#define SF_ALIVE 64   // Mobile/Animated

// 'put to' flags
#define PUT_IN 0      // put in
#define PUT_ON 1      // on...
#define PUT_BEHIND 2  // behind...
#define PUT_UNDER 3   // under!

// verb flags!
#define VB_TRAVEL 1  // Verb is travel verb
#define VB_DREAM 2   // Verb can be executed whilst sleeping

enum ActionID {
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

// Parameter types
#define PREAL -70   // Noun or slot label
#define PNOUN 1     // Must be a noun
#define PADJ 2      // Must be an adj
#define PPLAYER 4   // Must be a player
#define PROOM 5     // Must be a room
#define PUMSG 7     // Must be text
#define PVERB 8     // Must be a verb
#define PCLASS 9    // Must be a class
#define PNUM 10     // Must be a number
#define PRFLAG 11   // Must be a room flag
#define POFLAG 12   // Must be an obj flag
#define PSFLAG 13   // Must be a stat flag
#define PSEX 14     // Must be a gender
#define PDAEMON 15  // Must be a daemon ID

// Really the built-in magic should be limited to
// things that have effects you can't implement with
// the game itself, and allow users to define their own
// player and object flags to manipulate.
enum SpellID {
    SGLOW = 1,  // Spell #1
    SINVIS,     // Spell #2
    SBLIND,     // Spell #3
    SCRIPPLE,   // Spell #4
    SDEAF,      // Spell #5
    SDUMB,      // Player cant speak
    SSLEEP,     // Puts a player to bedie byes
    SSINVIS,    // Super Invisible
};

enum StatID {
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
#define MaxLines 10
#define MKILL 1       // Close down
#define MCNCT 2       // Connection
#define MDISCNCT 3    // Disconnect
#define MDATAREQ 4    // Gets ptrs!
#define MLOGGED 5     // Logged in!
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
#define MDAEMON 18    // We have lift off!
#define MCHECKD 19    // Get daemon status
#define MFORCE 20     // Do it, buddo!
#define MMADEWIZ 21   // Reached top rank!
#define MLOG 22       // Write log entry
#define MRWARN 23     // Reset Warning
#define MEXTEND 24    // Extend game
#define MGDSTART 25   // Start global daemon

#define WNONE -1    // None!
#define WANY 0      // Anything!
#define WNOUN 1     // Word is a noun
#define WADJ 2      // Word is an adjective
#define WPREP 3     // Its a prep
#define WPLAYER 4   // Its a player
#define WROOM 5     // Its a room ID
#define WSYN 6      // Its a synonym
#define WTEXT 7     // Its text
#define WVERB 8     // Its a verb!
#define WCLASS 9    // Class name
#define WNUMBER 10  // A number

// -- Player flags --
#define PFINVIS 0x00001     // Player invisible
#define PFGLOW 0x00002      // Player glowing
#define PFBLIND 0x00004     // Blind
#define PFDUMB 0x00008      // Can't speak
#define PFDEAF 0x00010      // Player's deaf
#define PFCRIP 0x00020      // Can't move
#define PFDYING 0x00040     // Player is dying
#define PFLIMP 0x00080      // Limping
#define PFASLEEP 0x00100    // Sleeping
#define PFSITTING 0x00200   // Sitting down
#define PFLYING 0x00400     // Lying Down
#define PFFIGHT 0x00800     // Fighting
#define PFATTACKER 0x01000  // If you started the fight
#define PFMOVING 0x02000    // If you are 'in transit'
#define PFSINVIS 0x04000    // Player is Super Invis

#define RDRC 0  // RC Mode
#define RDVB 1  // Verbose mode
#define RDBF 2  // Brief mode

#define TYPEV 0  // Brief mode
#define TYPEB 1  // Verbose mode

#define At amul->type
#define Am amul->msg
#define Af amul->from
#define Ad amul->data
#define Ap amul->ptr
#define Ap1 amul->p1
#define Ap2 amul->p2
#define Ap3 amul->p3
#define Ap4 amul->p4

#define AMt amanp->type
#define AMm amanp->msg
#define AMf amanp->from
#define AMd amanp->data
#define AMp amanp->ptr
#define Apx1 amanp->p1
#define Apx2 amanp->p2
#define Apx3 amanp->p3
#define Apx4 amanp->p4

#define IAt intam->type
#define IAm intam->msg
#define IAf intam->from
#define IAd intam->data
#define IAp intam->ptr

// -- User Flags --

#define ufANSI 0x001  // ANSI bit
#define ufCRLF 0x002  // Add LineFeeds
#define ufARDO 0x004  // Auto Redo

#define DLLEN 80       // Default line length
#define DSLEN 24       // Default screen length
#define DRCHAR '|'     // Default redo-char
#define DFLAGS ufCRLF  // Default = cr/lf ON

// -- Useful defines --

#define isOINVIS(x) ((obtab + x)->flags & OF_INVIS)
#define isPINVIS(x) ((linestat + x)->flags & PFINVIS)
#define IamINVIS (me2->flags & PFINVIS)
#define IamSINVIS (me2->flags & PFSINVIS)
#define pROOM(x) ((linestat + x)->room)
#define pRANK(x) ((usr + x)->rank)
#define myRANK me->rank
#define mySCORE me->score
#define myROOM me2->room
#define LightHere lit(me2->room)
#define unfreeze                                                                                   \
    Permit();                                                                                      \
    return

#define CP1 actual(*(tt.pptr))
#define CP2 actual(*(tt.pptr + 1))
#define CP3 actual(*(tt.pptr + 2))
#define CP4 actual(*(tt.pptr + 3))
#define TP1 actual(*(tt.pptr + ncop[tt.condition]))
#define TP2 actual(*(tt.pptr + ncop[tt.condition] + 1))
#define TP3 actual(*(tt.pptr + ncop[tt.condition] + 2))
#define TP4 actual(*(tt.pptr + ncop[tt.condition] + 3))
#define AP1 (char *)actptr(*(tt.pptr + ncop[tt.condition]))
#define AP2 (char *)actptr(*(tt.pptr + ncop[tt.condition] + 1))
#define AP3 (char *)actptr(*(tt.pptr + ncop[tt.condition] + 2))
#define AP4 (char *)actptr(*(tt.pptr + ncop[tt.condition] + 3))
#define STATE (objtab->states + (long)objtab->state)
#define State(i) ((obtab + i)->states + (long)(obtab + i)->state)
#define ItsState (it->states + (long)it->state)
#define xLIGHT(x) (linestat + x)->light
#define xHADLIGHT(x) (linestat + x)->hadlight

#define acp (char *)actptr

#endif
