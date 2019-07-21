#pragma once
// This may look like C, but it's really -*- C++ -*-
// #defines, enums and macros

#define NORMALISE(x) ((((unsigned long)(x)) + (sizeof(long) * 2)) & 0xfffffffc)
#define VOIDADD(p, o) (void *)((char *)p + (unsigned long)o)

#define	EOL	10                  // End of line character
#define	SPC	' '                 // Space character
#define TAB     9               // Tab character
#define	CMT     ';'             // Comment character

#define	MAXU    31              // Max USERS at 1 time
#define USER_VOCAB 8            // Extra vocab entries to add
#define	MAXNODE 32              // +1 (mobiles/daemons)
#define	VOCROWS 1499            // Nice prime number ;-)

// Size of the string indexes in lib/data.c -- YEUCH
#define	NSYNTS 	11              // Number of 'syntax' types
#define	NPUTS	4               // Number of 'put' locations
#define NPREP	6               // Number of prepositions
#define NART	4               // Number of articles

#define	YES	0
#define	NO	1

#define	LOUD	1
#define	QUIET	2

// Player connection status enums
enum player_state
    {
    OFFLINE, LOGGING, PLAYING, CHATTING
    };

#define	ACTION	0               // Event types
#define	NOISE	1
#define	EVENT	2
#define	TEXTS	3

#define	PV	"0.999b"        // Parser version
#define	RANKL	32              // Length of rank descs
#define	NAMEL	20              // Length of names
#define ADNAMEL 64              // Length of the game's name
#define	OBLIM	2048            // Output Buffer Size

// For 'actions': whether an action is a destination or an action
#define ACT_DO  0               // This is an action
#define	ACT_GO  1               // This is a destination (room)

// Basic Object std_flag bit fields
#define bob_INPLAY   0x0001     // Currently in play
#define bob_SCENERY  0x0002     // Not described in object content lists
#define bob_COUNTER  0x0004     // Not exposed to player-facing parser
#define bob_DEATH    0x0008     // Creature death for being in here
#define bob_LIGHT    0x0010     // Has internal light source
#define bob_SHINES   0x0020     // Can be externally luminous
#define bob_FLAMABLE 0x0040     // Can be set on fire
#define bob_LIT      0x0080     // SHINES/FLAMABLE currently active
#define bob_SILENT   0x0100     // Doesn't receive external noises
#define bob_HIDECRE  0x0200     // Can't see other creatures inside
#define bob_HIDEOBJ  0x0400     // Can't see non-creatures inside
#define bob_SANCTRY  0x0800     // Give score for putting objects here
#define bob_NOLOOK   0x1000     // Only see description from inside
#define bob_PEACEFUL 0x2000     // No fights allowed
#define bob_SCALED   0x4000     // Value subject to scaling

// Room bit-flags
#define	STARTL	     0x0001     // Can be started from
#define	RANDOB	     0x0002     // Can start random objects here
#define	SMALL	     0x0004     // Only holds 1 being
#define NOEXITS	     0x0008     // Exits are 'hidden'
#define	ANTERM	     0x0010     // Special Pre-Start start location
#define	NOGORM	     0x0020     // Can't random go to this room!

// Enum's for room parameters
enum
    {
    rp_dark,                    // Specify a room is dark
    rp_dmove                    // Dmove flag
    };

// Object flag bits
#define	OF_OPENS     0x0001     // Object is openable
#define	OF_SHOWFIRE  0x0002     // Say 'The <noun> is on fire' when lit
#define OF_INVIS     0x0004     // Object is invisible
#define	OF_SMELL     0x0008     // Object has a smell not visual

// Object parameter flag no.'s
#define	OP_ADJ       0x0001     // Adjective
#define	OP_START     0x0002     // Starting state
#define	OP_HOLDS     0x0004     // Maximum contents
#define	OP_PUT       0x0008     // Put "in", "on", "under" etc...
#define	OP_MOB       0x0010     // Mobile assignment
#define	OP_ART       0x0020     // Article assignment

// Object/state flags
// Note: Some of these should really be done by the user, by specifying
// object groups, may require an extension to object definition format
#define	SF_OPEN      0x0001     // Open
#define	SF_CLOSED    0x0002     // explicitly Closed
#define	SF_WEAPON    0x0004     // A weapon
#define	SF_OPAQUE    0x0008     // Can see inside object
#define	SF_ALIVE     0x0010     // Mobile/Animated
// Which std_flags a state can have
#define STATE_STD_FLAGS \
    (bob_DEATH | bob_LIT | bob_SILENT | bob_PEACEFUL | bob_SCALED)
#define STATE_STD_FILTER ~STATE_STD_FLAGS

// 'put to' flags
// Allows the user to make objects other than "containers". Normally
// an object "connects" to another by being "in" it, but you can also
// place objects in other ways. E.g. behind, under, etc
enum
    {
    PUT_IN, PUT_ON, PUT_BEHIND, PUT_UNDER
    };

// Gender types
enum Gender { EITHER = -1, MALE, FEMALE };

// verb bit flags
#define	VB_TRAVEL	1	// motion verb, i.e. used to travel
#define	VB_DREAM	2	// can be done while asleep

// Actions
enum
    {
    AQUIT,                      // Quit
    ASAVE,                      // Save player details
    ASCORE,                     // Show player's status
    ASETSTAT,                   // Set object state
    ALOOK,                      // Look at current room
    AWHAT,                      // Listen room contents
    AWHERE,                     // List where objects are
    AWHO,                       // List who is playing
    ATREATAS,                   // Pretend player really typed 'verb'
    APRINT,                     // Display text
    ASKIP,                      // Skip next 'x' entries
    AENDPARSE,                  // Stop parsing
    AKILLME,                    // Kill the player
    AFINISHPARSE,
    AABORTPARSE,
    AFAILPARSE,                 // Endparse + fail
    AWAIT,                      // Wait 'n' seconds
    AHIT,                       // Single combat turn
    AWHEREAMI,                  // 'Where'
    ASEND,                      // Send object
    AANOUN,                     // Make announcement
    AGET,                       // Pick something up
    ADROP,                      // Drop something
    AINVENT,                    // List objects being carried
    ARANDGO,                    // Send player to random room
    ACHANGESEX,                 // Change players sex
    APROVOKE,                   // Force mobile to take a turn
    ABLAST,                     // "Blast" an object
    ASIT,                       // Make player "sit"
    ASTAND,                     // Make player "stand"
    ALIE,                       // Make player "lie down"
    ARDMODE,                    // Set room description mode
    ARESET,                     // Reset the game
    AACTION,                    // Like announce
    AMOVE,                      // Relocate player (quietly)
    ATRAVEL,                    // Process travel table instead
    AMSGIN,                     // Announce to a room
    AACTIN,                     // Action to a room
    AMSGFROM,                   // Announce via an object
    AACTFROM,                   // Action via an object
    ATELL,                      // Tell someone something
    APUT,                       // Put one object "in" another
    AGIVE,                      // Give an object to another user
    AINFLICT,                   // Cast a spell
    ACURE,                      // Remove spell
    ASUMMON,                    // Summon a player
    AADD,                       // Add stats to player
    ASUB,                       // Deduct stats from player
    ACHECKNEAR,                 // Is object 'near' player?
    ACHECKGET,                  // Can the player take an object?
    ADESTROY,                   // Destroy the object
    ARECOVER,                   // Recover a 'destroyed' object
    ASTART,                     // Start a daemon
    ACANCEL,                    // Cancel a daemon
    ABEGIN,                     // Force daemon to start
    ASHOWTIMER,                 // Displays counter for a given daemon
    AOBJAN,                     // Announce from an object
    AOBJACT,                    // Action from an object
    ACONTENTS,                  // Show object contents
    AFORCE,                     // Force a player to do something
    AHELP,                      // Help someone
    ASTOPHELP,                  // Cease helping someone
    AFIX,                       // Set player stat to exact value
    AOBJINVIS,                  // Make an object invisible
    AOBJSHOW,                   // Make an object visible
    AFIGHT,                     // Start a fight
    AFLEE,                      // Run away from a fight
    ALOG,                       // Write text to a log file
    ACOMBAT,                    // Combat action
    AWIELD,                     // Set current weapon
    AFOLLOW,                    // Pursue a player
    ALOSE,                      // Stop someone following you
    ASTOPFOLLOW,                // Cease following a player
    AEXITS,                     // List rooms exits
    ATASK,                      // Mark a 'task' as done
    ASHOWTASK,                  // Show tasks done so far
    ASYNTAX,                    // Specify syntax slot values
    ASETPRE,                    // Set pre-rank description
    ASETPOST,                   // Set post-rank description
    ASENDDAEMON,                // Attach a daemon to another player
    ADO,                        // Effectively "gosub"
    AINTERACT,                  // Set 'actor' - player we're working on
    AAUTOEXITS,                 // Enable auto-exits
    ASETARR,                    // Set 'player has arrived' string
    ASETDEP,                    // Set 'player has departed' string
    ARESPOND,                   // Print, endparse
    AERROR,                     // Give an error
    ABURN,                      // Ignite an object
    ADOUSE,                     // Douse an object
    AINC,                       // Increment object state
    ADEC,                       // Decrement object state
    ATOPRANK,                   // Make player "top" rank
    ADEDUCT,                    // Reduce players score by a %age
    ADAMAGE,                    // Damage an object
    AREPAIR,                    // Remove damage from an object
    AGSTART,                    // Start a global daemon
    AEXTEND,                    // Extend timer
// -- End of List --
    ACTIONS                     // Number of actions
    };

// Conditions
enum
    {
    CAND,                       // And then...
    CSTAR,                      // Same as always
    CELSE,                      // If last was false
    CALWAYS,                    // Always do this
    CLIGHT,                     // If there is light...
    CISHERE,                    // If obj is here
    CMYRANK,                    // If my rank is...
    CSTATE,                     // If state of obj
    CMYSEX,                     // My sex is...
    CLASTVB,                    // If last verb was..
    CLASTDIR,                   // If last TRAVEL verb
    CLASTROOM,                  // If last room was
    CASLEEP,                    // Is player sleeping
    CSITTING,                   // Is player sitting
    CLYING,                     // Is player lying down
    CRAND,                      // If rand(n1) eq n2
    CRDMODE,                    // If rdmode is...
    CONLYUSER,                  // If only player
    CALONE,                     // If only person here
    CINROOM,                    // If player in room
    COPENS,                     // If object opens
    CGOTNOWT,                   // If carrying nothing
    CCARRYING,                  // Carrying object?
    CNEARTO,                    // Is it here SOMEWHERE
    CHIDDEN,                    // Can others see me?
    CCANGIVE,                   // Can player manage it
    CINFL,                      // Same as below
    CINFLICTED,                 // Is played inflicted
    CSAMEROOM,                  // Same room as player?
    CSOMEONEHAS,                // If obj is carried?
    CTOPRANK,                   // If u'r the top rank
    CGOTA,                      // Carrying obj in stat
    CACTIVE,                    // Is daemon active?
    CTIMER,                     // Check time left
    CBURNS,                     // If object burns
    CCONTAINER,                 // If its a container
    CEMPTY,                     // If object is empty
    COBJSIN,                    // Check # of contents
    CALTEP,                     // Always .., endparse
    CANTEP,                     // And    .., endparse
    CHELPING,                   // Are we helping him?
    CGOTHELP,                   // If we've got help
    CANYHELP,                   // Helping ANYONE?
    CELTEP,                     // Else   ..., endparse
    CSTAT,                      // If attrib <> no.
    COBJINV,                    // If object invisible
    CFIGHTING,                  // Is player fighting?
    CTASKSET,                   // Has task been done?
    CCANSEE,                    // Can I see <player>
    CVISIBLETO,                 // Am I visible to
    CNOUN1,                     // Match noun1
    CNOUN2,                     // Match noun2
    CAUTOEXITS,                 // Auto exits on?
    CDEBUG,                     // Debug mode on?
    CFULL,                      // Stat at full?
    CTIME,                      // Time remaining?
    CDEC,                       // Decrement & test
    CINC,                       // Increment & test
    CLIT,                       // Is object lit?
    CFIRE,                      // Is object flamable?
    CHEALTH,                    // Is players health %?
    CMAGIC,                     // Can magic be done?
    CSPELL,                     // Can spell be done?
    CIN,                        // IN <ROOM> <NOUN>
    CEXISTS,                    // Does object exist?
    CWILLGO,                    // Will it go in?
/* -- End of List -- */
    CONDITIONS
    };
 

// Paramter types
#define PREAL -70               // Noun or slot label
enum
    {
    PNOUN = 1,                  // Must be a noun
    PADJ,                       // Must be an adjective
    PPLAYER,                    // Must be a player
    PROOM,                      // Must be a room
    PSYN,                       // Must be a synonym (redundant)
    PUMSG,                      // Must be text
    PVERB,                      // Must be a verb
    PCLASS,                     // Must be a class
    PNUM,                       // Must be a number
    PRFLAG,                     // Must be a room flag
    POFLAG,                     // Must be an object flag
    PSFLAG,                     // Must be a state flag
    PSEX,                       // Must be a gender
    PDAEMON,                    // Must be a daemon id
    PMOBILE,                    // Must be a mobile
    PMAX_TYPE
    };

// Word types
enum
    {
    WNONE = -1,                 // None!
    WANY,                       // Anything!
    WNOUN,                      // Word is a noun
    WADJ,                       // Word is an adjective
    WPLAYER,                    // Its a player
    WROOM,                      // Its a room ID
    WSYN,                       // Its a synonym
    WTEXT,                      // Its text
    WVERB,                      // Its a verb!
    WCLASS,                     // Class name
    WNUMBER                     // A number
    };                          

// Spells
enum
    {
    SGLOW = 1,
    SINVIS, SBLIND, SCRIPPLE, SDEAF, SDUMB, SSLEEP, SSINVIS
    };


#define STSCORE		1
#define STSTR		2
#define STSTAM		3
#define STDEX		4
#define STWIS		5
#define STEXP		6
#define STMAGIC		7
#define	STSCTG		8
#define	STINFIGHT	1024        // Flag - in a fight

// Anouncement types
enum
    {
    AGLOBAL, AEVERY1, AOUTSIDE, AHERE, AOTHERS, AALL, ACANSEE, ANOTSEE
    };

// Message Types
enum
    {
    MKILL,                      // Close down
    MCNCT,                      // Connection
    MDISCNCT,                   // Disconnect
    MDATAREQ,                   // Gets ptrs!
    MLOGGED,                    // Logged in!
    MMESSAGE,                   // Sent a msg
    MCLOSEING,                  // Closeing..
    MRESET,                     // Reset
    MLOCK,                      // User Lock
    MUNLOCK,                    // Unlock it
    MSUMMONED,                  // COME HERE!
    MDIE,                       // Ciao!!
    MBUSY,                      // I'm busy
    MFREE,                      // I'm free!
    MEXECUTE,                   // Execute a command!
    MDSTART,                    // Daemon start
    MDCANCEL,                   // Cancel a daemon
    MDAEMON,                    // We have lift off!
    MCHECKD,                    // Get daemon status
    MFORCE,                     // Do it, buddo!
    MMADEWIZ,                   // Reached top rank!
    MLOG,                       // Write log entry
    MRWARN,                     // Reset Warning
    MEXTEND,                    // Extend game
    MGDSTART,                   // Start global daemon
    MSWAP,                      // Signal 'swap next game'
    MMOBILE,                    // Process mobile
    MMOBFRZ,                    // Freeze mobiles
    MMOBTHAW,                   // Thaw mobiles
    MPROVOKE,                   // Provoke Mobile
    MKILLED,                    // You've killed me
    MRMLOCK,                    // Lock room
    MRMFREE,                    // Free room lock
/* -- End of List -- */
    NMSGS
    };

enum                            // System message numbers
    {
    RESETTING,                  // Reset in progress
    NOSLOTS,                    // No free slots
    RETURN,                     // '[Press RETURN] '
    WHATNAME,                   // 'Personna name: '
    LENWRONG,                   // Must be between 3-20 chars long
    NAME_USED,                  // When system already uses name
    LOGINASME,                  // Message->user already logged in
    ALREADYIN,                  // User is already logged in
    CREATE,                     // Create new user?
    WHATGENDER,                 // (M)ale or (F)emale?
    GENDINVALID,                // Invalid, only M or F valid
    ENTERPASSWD,                // Enter password:
    PASLENWRONG,                // Invalid length!
    FAILEDLOGIN,                // Login Failed - Bye bye
    KILLEDPL,                   // You have killed @pl
    YOUBEGIN,                   // You begin as...
    TRIESOUT,                   // Out of tries (passwd failed)
    FAILEDTRIES,                // No. of failed attempts
    WELCOMEBAK,                 // Welcome back!
    ANSION,                     // ANSI Enabled!
    COMMENCED,                  // Message to others when you begin
    HELP,                       // Available commands
    BYEPASSWD,                  // Byebye, passwd is...
    EXITED,                     // I've exited
    DIED,                       // You have died
    HEDIED,                     // @me just died
    SPELLFAIL,                  // Spell failed
    ISPLAYING,                  // is online...
    MADERANK,                   // When you achieve a rank
    SAVED,                      // When saving score
    REALLYQUIT,                 // Really quit (y/N):
    NOWTOODARK,                 // It is now too dark to see.
    NOWLIGHT,                   // It is now light enough to see.
    TOODARK,                    // It is too dark to see.
    TOOMAKE,                    // Too dark to make anything out
    NOWTSPECIAL,                // You can see nothing special.
    ISHERE,                     // %s the %q is here...
    RESETSTART,                 // Message when reset starts...
    INVALIDVERB,                // Try using a verb...
    CANTGO,                     // Can't go that way...
    CANTDO,                     // Cant do that
    SGO,                        // Kaapppoowwww!
    SGOVANISH,                  // ->others when you SGOleave
    SGOAPPEAR,                  // ->others when you SGOarrive
    NONOUN,                     // Missing noun!
    WORDMIX,                    // When your words are mixed up
    ALMOST,                     // Almost understood
    BEENSUMND,                  // You have been summoned
    SUMVANISH,                  // @me vanishes...
    SUMARRIVE,                  // @me arrives out of nowhere
    WOKEN,                      // @me has just woken up.
    IWOKEN,                     // You have just woken up.
    CANTSUMN,                   // %s is already here!
    LEFT,                       // @me has just left!
    ARRIVED,                    // @me has just arrived.
    NOROOM,                     // There is no room in there for you.
    NOROOMIN,                   // @me just tried to enter, but there wasn't enuff room!
    CHANGESEX,                  // You have magically become @gn!
    ATTACK,                     // You strike @pl with your bare hands!
    DEFEND,                     // You manage to block @me's strike.
    WATTACK,                    // You strike @pl with your trusty @o1!
    WDEFEND,                    // You wield your @o2 and block @me's strike.
    AMHIT,                      // @me strikes you with his bare hands.
    BLOCK,                      // @pl manages to block your strike.
    WHIT,                       // @me strikes you with his @o1!
    WBLOCK,                     // @pl wields a @o2 and blocks your attack!
    MISSED,                     // You attack @pl but miss!
    HEMISSED,                   // @me attacks you but misses!
    TOPRANK,                    // Well done! You have obtained the highest rank possible!
    NOTASK,                     // Sorry you haven't completed adequate tasks for this level.
    YOURBLIND,                  // You can't see anything, you're blind.
    WHO_HIDE,                   // You can't be sure if there are other people nearby...
    CERTDEATH,                  // Exit description of Death room
    NOFIGHT,                    // Sorry no fighting allowed here.
    LOWLEVEL,                   // Sorry you're not high enough to cast that spell.
    NOMAGIC,                    // Sorry you have not enough magic points for that spell.
    BROKEN,                     // broken and crumbles away
    NSMSGS                      // Total number of messages
    };

// Player flags
#define	PFINVIS		0x00001
#define PFGLOW		0x00002
#define PFBLIND		0x00004
#define	PFDUMB		0x00008
#define PFDEAF		0x00010
#define	PFCRIP		0x00020     // Can't move
#define	PFDYING		0x00040     // NOT USED - dying
#define PFLIMP		0x00080     // NOT USED - Limping
#define PFASLEEP	0x00100     
#define PFSITTING	0x00200     
#define PFLYING		0x00400     // Lying Down
#define PFFIGHT		0x00800     
#define PFATTACKER	0x01000     // started the fight
#define	PFMOVING	0x02000     // 'in transit'
#define	PFSINVIS	0x04000     
#define	PFWORKING	0x08000     // Using Player Data

// Room Description Modes
enum RDMode
    {
    RDRC, RDVB, RDBF            // RoomCount, VerBose, BrieF
    };

// Description modes
enum DMode
    {
    TYPEV, TYPEB
    };

#define NONE 0, 0, 0

// Shared memory and IPC stuff
#define	MSG_SZ           1024   // Size of text in DATA struct
#define	MAX_SHM_SEGS	 2      // Maximum shared memory segments

