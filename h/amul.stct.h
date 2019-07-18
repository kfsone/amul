#ifndef H_AMUL_STCT_H
#define H_AMUL_STCT_H 1

#include <h/amul.defs.h>
#include <h/amul.type.h>

struct _PLAYER /* Player def struct */
{
    char    name[NAMEL + 1]; /* Player's name	 */
    char    passwd[23];      /* Password or User ID	 */
    int32_t score;           /* Score to date...      */
    int     rdmode;          /* Users RD Mode	 */
    int16_t strength;        /* How strong is he?	 */
    int16_t stamina;         /* Stamina		 */
    int16_t dext;            /* Dexterity		 */
    int16_t wisdom;          /* Wisdom		 */
    int16_t experience;      /* Experience		 */
    int16_t magicpts;        /* Magic points		 */
    int16_t rank;            /* How he rates!	 */
    int16_t plays;           /* Times played		 */
    int16_t tries;           /* Bad tries since last	 */
    int32_t tasks;           /* Tasks completed	 */
    char    archetype;       /* Player class		 */
    char    sex;             /* Players sex		 */
    uint8_t flags;           /* Ansi, LF, Redo	 */
    char    llen;            /* Line length		 */
    char    slen;            /* Screen length	 */
    char    rchar;           /* Redo character	 */
};

struct LS /* IO Driver Line Status Info */
{
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
    char            pre[80];   /* Pre-rank description	 */
    char            post[80];  /* Post-rank description */
    char            arr[80];   /* When player arrives	 */
    char            dep[80];   /* When player leaves	 */
};

struct _ROOM_STRUCT /* Room def struct */
{
    char       id[IDL + 1]; /* Room I.D.		   */
    char       dmove[IDL + 1];
    uint32_t   flags;   /* Room FIXED flags        */
    stringid_t descid;  /* Ptr to des. in des file */
    uint32_t   tabptr;  /* Ptr to T.T. data 	   */
    uint32_t   ttlines; /* No. of TT lines	   */
};

struct _VERB_STRUCT /* Verb def struct */
{
    char             id[IDL + 1]; /* The Verb itself       */
    uint8_t          flags;       /* Travel? etc...	 */ 
    union {
        struct Precedence {
            char sort[5];  /* Sort method... (yawn) */
            char sort2[5]; /* Sort #2!		 */
        };
        char precedences[10];
	};
    int16_t          ents;        /* No. of slot entries	 */
    struct _SLOTTAB *ptr;         /* Pointer to slots tab	 */
};

struct _SLOTTAB /* Slot table def */
{
    char           wtype[5]; /* Type of word expected */
    long           slot[5];  /* List of slots	 */
    uint16_t       ents;     /* No. of entries	 */
    struct _VBTAB *ptr;      /* Pointer to Verb Table */
};

struct _VBTAB /* Verb Table struct */
{
    int32_t  condition; /* Condition		 */
    int32_t  action;    /* #>0=action, #<0=room  */
    opparam_t *pptr;      /* Param ptr. -1 = none  */
};

struct _RANK_STRUCT /* Rank information */
{
    char    male[RANKL];   /* chars for male descrp */
    char    female[RANKL]; /* Women! Huh!           */
    long    score;         /* Score to date...      */
    int16_t strength;      /* How strong is he?	 */
    int16_t stamina;       /* Stamina		 */
    int16_t dext;          /* Dexterity		 */
    int16_t wisdom;        /* Wisdom		 */
    int16_t experience;    /* Experience		 */
    int16_t magicpts;      /* Magic points		 */
    int32_t maxweight;     /* Maximum weight	 */
    int16_t numobj;        /* Max. objects carried	 */
    int32_t minpksl;       /* Min. pts for killin	 */
    int32_t tasks;         /* Tasks needed for level*/
    char    prompt[11];    /* Prompt 4 this rank	 */
};

struct _NTAB_STRUCT /* Noun table structure */
{
    char                id[IDL + 1]; /* Object's NAME	 */
    int16_t             num_of;      /* Number of this type	 */
    struct _OBJ_STRUCT *first;       /* First in the list	 */
};

struct _OBJ_STRUCT /* Object (proper) definition */
{
    char               id[IDL + 1];                   /* Object's NAME	 */
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

struct _OBJ_STATE /* State description */
{
    uint32_t   weight;   /* In grammes		 */
    int32_t    value;    /* Whassit worth?	 */
    uint16_t   strength; /* How strong is it?	 */
    uint16_t   damage;   /* Amount of damage	 */
    stringid_t descrip;  /* ptr to descrp in file */
    uint16_t   flags;    /* State flags		 */
};

struct _TT_ENT /* TT Entry */
{
    verbid_t   verb;      /* Verb no.		 */
    int32_t    condition; /* Condition		 */
    int32_t    action;    /* #>0=action, #<0=room  */
    opparam_t *pptr;      /* Param ptr. -1 = none  */
};

struct _MOB  // Runtime definition of a monster
{
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

struct _MOB_ENT /* Mobile Character Entry */
{
    char        id[IDL + 1]; /* Name of mobile	*/
    struct _MOB mob;
};

struct _MOB_TAB /* Mobile table entry */
{
    uint16_t obj;    /* Object no.		*/
    uint16_t speed;  /* Secs/turn		*/
    uint16_t count;  /* Time till next	*/
    uint16_t pflags; /* Player style flags	*/
};

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

struct _BOBJ /* Basic object struct */
{
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

struct _ROOM /* Room object struct */
{
    uint16_t   flags;   /* Room FIXED flags        */
    stringid_t desptr;  /* Ptr to des. in des file */
    int32_t    tabptr;  /* Ptr to T.T. data 	   */
    uint16_t   ttlines; /* No. of TT lines	   */
};

#define CRE struct _BEING /* Creature */

struct _BEING /* Mobile/char struct */
{
    char *          buf;                     /* Where to put text	*/
    struct MsgPort *rep;                     /* Notify me here	*/
    uint32_t        flags;                   /* Flags		*/
    int32_t         sctg;                    /* Points scd this game	*/
    int16_t         strength, stamina, dext; /* Fators		*/
    int16_t         wisdom, magicpts, wield; /* factors		*/
    int16_t         dextadj;
    CRE *           helping;                          /* Who I'm helping	*/
    CRE *           nxthelp;                          /* Next person		*/
    CRE *           helped;                           /* 1st helping me	*/
    CRE *           following, *nxtfollow, *followed; /* followers		*/
    CRE *           snooping, *nxtsnoop, *snooper;    /* snoopers		*/
    CRE *           fighting, *attme;                 /* attme=attacking me	*/
    void *          info;                             /* A bit more		*/
};

struct _LS /* IO Driver Line Status Info */
{
    int16_t rec, unum;
    char    line, iosup; /* Which line am I?	*/
    char    IOlock;      /* Device in use?	*/
    char    pre[80];     /* Pre-rank description	*/
    char    post[80];    /* Post-rank description*/
    char    arr[80];     /* When player arrives	*/
    char    dep[80];     /* When player leaves	*/
};

struct _HUMAN /* Player def struct */
{
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

struct _OBJ /* Object definition */
{
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
