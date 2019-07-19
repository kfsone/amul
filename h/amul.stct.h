#ifndef H_AMUL_STCT_H
#define H_AMUL_STCT_H 1

#include <h/amul.defs.h>
#include <h/amul.type.h>

// _PLAYER describes a human actor in the game world.
/// TODO: Share common properties with NPCs into structs.
struct _PLAYER {
    char     name[NAMEL + 1];
    char     passwd[23];  /// TODO: hash
    int32_t  score;       // Total points scored
    int      rdmode;      // Room Description mode (brief/verbose)
    int16_t  strength;
    int16_t  stamina;
    int16_t  dext;
    int16_t  wisdom;
    int16_t  experience;
    int16_t  magicpts;
    int16_t  rank;
    uint16_t plays;      // times played
    uint16_t tries;      // failed login attempts since last login
    uint32_t tasks;      // bitmask of tasks completed
    char     archetype;  // Player class: unused yet
    char     gender;     // gender
    uint8_t  flags;      // preferences, really, and unused
                         // client/terminal preferences
    char llen;           // preferred line length
    char slen;           // preferred screen length
    char rchar;          // 'redo' character (refresh)
};

// LS represents the "line status", reflecting modem/bbs handling terminology
struct LS {
    char *          buf;       /* Pointer to output	 */
    struct MsgPort *rep;       /* My reply port	 */
    uint32_t        flags;     /* User's flags		 */
    int16_t         room;      /* My room		 */
    uint32_t        sctg;      /* SCore This Game	 */
    uint32_t        rec;       /* My record no.	 */
    int16_t         numobj;    /* Objects carried	 */
    int32_t         weight;    /* Weight carried	 */
    int16_t         strength;  /* How strong is he?	 */
    int16_t         stamina;   /* Stamina		 */
    int16_t         dext;      /* Dexterity		 */
    int16_t         dextadj;   /* Dexterity Adjustments */
    int16_t         wisdom;    /* Wisdom		 */
    int16_t         magicpts;  /* Magic points		 */
    int16_t         wield;     /* No. of weapon used	 */
    int16_t         unum;      /* My user number...	 */
    char            state;     /* State of line	 */
    char            IOlock;    /* Device in use?	 */
    char            following; /* Who I am following	 */
    char            sup;       /* User's io type	 */
    char            light;     /* If player has a light */
    char            hadlight;  /* If I had a light	 */
    char            helping;   /* Player I am helping	 */
    char            helped;    /* Getting help from	 */
    char            followed;  /* Who is following me	 */
    char            fighting;  /* Who I am fighting	 */
                               /// TODO: normalize to stringids
    char pre[80];              /* Pre-rank description	 */
    char post[80];             /* Post-rank description */
    char arr[80];              /* When player arrives	 */
    char dep[80];              /* When player leaves	 */
};

// _ROOM_STRUCT represents a room, or location, in the game world.
struct _ROOM_STRUCT {
    char       id[IDL + 1];
    char       dmove[IDL + 1];  /// TODO: Rename CEMETERY
    uint32_t   flags;           // static flags
    stringid_t descid;
    uint32_t   tabptr;   // offset to data in travel table
    uint32_t   ttlines;  /// TODO: normalize 'count'
};

// _VBTAB represents an action (and optional condition) in the table of instructions
// associated with a verb + syntax block.
/// TODO: Allow multiple conditions, multiple actions.
/// TODO: Rename from 'vbtab' to 'VMOp', 'Instruction', something .. better
struct _VBTAB {
    int32_t condition;
    /// TODO: Replace negative actions with a GOTO action.
    int32_t action;   /* #>0=action, #<0=room  */
                      /// TODO: pptr[]
    opparam_t *pptr;  // Parameter list, or -1 for none.
};

// _SLOTTAB represents a syntax slot that expands to a series of instructions
struct _SLOTTAB {
    char           wtype[5]; /* Type of word expected */
    long           slot[5];  /* wtype specific values	 */
    uint16_t       ents;     /// TODO: normalize 'count'
    struct _VBTAB *ptr;      /// TODO: vbtab[]
};

// _VERB_STRUCT describes the verbs that operate as runtime commands from
// players. Each verb has to describe the order in which it prefers to
// resolve conflicting matches for noun tokens: for example, if there is
// a "note" on the ground and a "note" in the player's inventory, then
// the "get" and "drop" verbs will want to try those in opposite orders.
//
// History: 'CHAE' is 'Carried, Here, Another (player's inventory) and
// Elsewhere (not in the room)'. This is really precedence or ordering,
//
struct _VERB_STRUCT {
    char    id[IDL + 1];
    uint8_t flags;
    union {
        char precedence[2][5];
        char precedences[10];
    };
    int16_t          ents;  /// TODO: normalize 'count'
    struct _SLOTTAB *ptr;   /// TODO: variants[]
};

// Player ranks
struct _RANK_STRUCT {
    char     male[RANKL];    // Male title
    char     female[RANKL];  // Female title
    char     prompt[11];
    int32_t  score;     // Score required to attain
    int16_t  strength;  // Combat; does not affect carry capacity
    int16_t  stamina;
    int16_t  dext;
    int16_t  wisdom;
    int16_t  experience;
    int16_t  magicpts;
    int32_t  maxweight;
    int16_t  numobj;
    int32_t  minpksl;  // Base points for killing someone of this rank
    uint32_t tasks;    // Bitmask of required tasks to attain rank
};

// "Noun" is the name of an object so that it can be used as *a noun*.
struct _NTAB_STRUCT {
    char                id[IDL + 1];  /// TODO: Dictionary id
    uint32_t            numInstances;
    struct _OBJ_STRUCT *first;
};

// Object: Item or npc in the game world
struct _OBJ_STRUCT {
    char               id[IDL + 1];                   /// TODO: noun id
    int16_t            idno;                          /* Object's ID no	 */
    adjid_t            adj;                           /* Adjective. -1 = none	 */
    int16_t            inside;                        /* No. objects inside	 */
    int16_t            flags;                         /* Fixed flags		 */
    int32_t            contains;                      /* How much it will hold */
    int8_t             nstates;                       /* No. of states	 */
    int8_t             putto;                         /* Where things go	 */
    int8_t             state;                         /* Current state	 */
    int8_t             mobile;                        /* Mobile character	 */
    struct _OBJ_STATE *states;                        /* Ptr to states!	 */
    int16_t            nrooms;                        /* No. of rooms its in	 */
    roomid_t *         rmlist; /* List of rooms	 */  /// TODO: rmlist[]
};

// Object: State specific properties
struct _OBJ_STATE {
    uint32_t   weight;    // In grammes
    int32_t    value;     // Base points for dropping in a swamp
    uint16_t   strength;  // } Unclear: May be health of the item,
    uint16_t   damage;    // } damage it does or damage done to it
    stringid_t description;
    uint16_t   flags;
};

// Travel table entry, which you will note is a verb table entry with a verb.
/// TODO: See above.
struct _TT_ENT {
    verbid_t   verb;
    int32_t    condition;
    int32_t    action; /* #>0=action, #<0=room  */
    opparam_t *pptr;   /* Param ptr. -1 = none  */
};

// _MOB is the "live" representation of a runtime NPC
struct _MOB {
    int16_t        dmove;                           /* Move to when it dies	*/
    char           deadstate;                       /* State flags		*/
    char           speed, travel, fight, act, wait; /* speed & %ages	*/
    char           fear, attack;                    /* others		*/
    int8_t         flags;                           /* -- none yet --	*/
    int16_t        hitpower;
    uint16_t       rank;                             /* Rank equiv		*/
    stringid_t     arr, dep, flee, hit, miss, death; /* Various UMsgs	*/
    int16_t        knows;                            /* Number of &		*/
    struct _VBTAB *cmds;                             /* & ptr to cmds	*/
};

// _MOB_ENT is the compile-time 'class' definition of an NPC
struct _MOB_ENT {
    char        id[IDL + 1]; /* Name of mobile	*/
    struct _MOB mob;
};

// _MOB_TAB is a component of runtime association of a mob with its controller.
struct _MOB_TAB {
    uint16_t obj;    /* Object no.		*/
    uint16_t speed;  /* Secs/turn		*/
    uint16_t count;  /* Time till next	*/
    uint16_t pflags; /* Player style flags	*/
};

/// TODO: enumize
#define boZONKED 0x0001   /* its out-of-play	*/
#define boCREATURE 0x0002 /* its a creature	*/
#define boGLOWING 0x0004  /* light-source		*/
#define boINVIS 0x0008    /* its invisible	*/
#define boSINVIS 0x0010   /* its super-invis	*/
#define boMOVING 0x0020   /* Is in-transit	*/
#define boDEAF 0x0040     /* Is deafened		*/
#define boBLIND 0x0080    /* Is blind		*/
#define boDUMB 0x0100     /* Dumbed		*/
#define boCRIPPLED 0x0200 /* Crippled		*/
#define boLOCKED 0x8000   /* CAN'T ACCESS		*/

#define SBOB struct _BOBJ

// _BOBJ was a work-in-progress when I took this snapshot, I was effectively
// starting to create a class hierarchy for tangible things in the game world,
// and probably not doing a great job of it.
struct _BOBJ {
    int8_t        type, state; /* Type of 'presence'	*/
    char *        name;
    const char *  archetype;
    uint32_t      holdsw, holdsi;       /* Max. contents	*/
    uint32_t      flags;                /* Any fixed flags	*/
    uint32_t      myweight, weight;     /* Item's weight & carried */
    uint16_t      light;                /* Light-source count	*/
    uint16_t      inv;                  /* Inventory (count)	*/
    struct _BOBJ *nxt, *pre, *in, *own; /* Next, prev, 1st in, owner */
    void *        info;                 /* Pointer to REAL data	*/
};

// _ROOM represents a room's extension of BOBJS
struct _ROOM {
    uint16_t   flags;   /* Room static flags        */
    stringid_t desptr;  /* Ptr to des. in des file */
    int32_t    tabptr;  /* Ptr to T.T. data 	   */
    uint16_t   ttlines; /* No. of TT lines	   */
};

#define CRE struct _BEING /* Creature */

// _BEING represents a player/mob
struct _BEING {
    char *          buf;   /* Where to put text	*/
    struct MsgPort *rep;   /* Notify me here	*/
    uint32_t        flags; /* Flags		*/
    int32_t         sctg;  /* Points scd this game	*/
    int16_t         strength, stamina, dext;
    int16_t         wisdom, magicpts, wield;
    int16_t         dextadj;
    CRE *           helping;                          /* Who I'm helping	*/
    CRE *           nxthelp;                          /* Next person		*/
    CRE *           helped;                           /* 1st helping me	*/
    CRE *           following, *nxtfollow, *followed; /* followers		*/
    CRE *           snooping, *nxtsnoop, *snooper;    /* snoopers		*/
    CRE *           fighting, *attme;                 /* attme=attacking me	*/
    void *          info;                             /* A bit more		*/
};

// Line-status
struct _LS {
    int16_t rec, unum;
    char    line, iosup; /* Which line am I?	*/
    char    IOlock;      /* Device in use?	*/
    char    pre[80];     /* Pre-rank description	*/
    char    post[80];    /* Post-rank description*/
    char    arr[80];     /* When player arrives	*/
    char    dep[80];     /* When player leaves	*/
};

// Player
struct _HUMAN {
    char *  name;       /* Player's name	 */
    char    passwd[9];  /* Password or User ID	 */
    long    score;      /* Score to date...      */
    int16_t rdmode;     /* Users RD Mode	 */
    int16_t strength;   /* How strong is he?	 */
    int16_t stamina;    /* Stamina		 */
    int16_t dext;       /* Dexterity		 */
    int16_t wisdom;     /* Wisdom		 */
    int16_t experience; /* Experience		 */
    int16_t magicpts;   /* Magic points		 */
    int16_t rank;       /* How he rates!	 */
    long    plays;      /* Times played		 */
    long    tasks;      /* Tasks completed	 */
    int16_t tries;      /* Bad tries since last	 */
    char    archetype;  /* Player class		 */
    char    sex;        /* Players sex		 */
    char    flags;      /* Ansi, LF, Redo	 */
    char    llen;       /* Line length		 */
    char    slen;       /* Screen length	 */
    char    rchar;      /* Redo character	 */
};

// Inanimate object bobj info
struct _OBJ {
    uint16_t           id;      /* My name		*/
    int16_t            nrooms;  /* No. of rooms its in	*/
    int16_t            adj;     /* It's adjective	*/
    uint16_t           flags;   /* Various 'fixed' flags*/
    char               nstates; /* No. of states	*/
    char               putto;   /* Where things go	*/
    char               state;   /* Current state	*/
    char               mobile;  /* Mobile character	*/
    struct _OBJ_STATE *states;  /* Ptr to states!	*/
};

#endif
