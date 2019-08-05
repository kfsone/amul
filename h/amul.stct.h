#ifndef H_AMUL_STCT_H
#define H_AMUL_STCT_H 1

#include <vector>

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
    char     archetype;  // Player class: unused yet
    char     gender;     // gender
    uint32_t tasks;      // bitmask of tasks completed
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

#include <h/lang_struct.h>

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
    std::vector<VMCnA> cmds;
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

#endif
