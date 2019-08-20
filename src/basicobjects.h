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
    int8_t type, state; /* Type of 'presence'	*/
    char *name;
    const char *archetype;
    uint32_t holdsw, holdsi;            /* Max. contents	*/
    uint32_t flags;                     /* Any fixed flags	*/
    uint32_t myweight, weight;          /* Item's weight & carried */
    uint16_t light;                     /* Light-source count	*/
    uint16_t inv;                       /* Inventory (count)	*/
    struct _BOBJ *nxt, *pre, *in, *own; /* Next, prev, 1st in, owner */
    void *info;                         /* Pointer to REAL data	*/
};

// _ROOM represents a room's extension of BOBJS
struct _ROOM {
    uint16_t flags;    /* Room static flags        */
    stringid_t desptr; /* Ptr to des. in des file */
    int32_t tabptr;    /* Ptr to T.T. data 	   */
    uint16_t ttlines;  /* No. of TT lines	   */
};

#define CRE struct _BEING /* Creature */

// _BEING represents a player/npc
struct _BEING {
    char *m_outputBuffer;        /* Where to put text	*/
    struct MsgPort *m_replyPort; /* Notify me here	*/
    uint32_t flags;              /* Flags		*/
    int32_t sctg;                /* Points scd this game	*/
    int16_t strength, stamina, dext;
    int16_t wisdom, magicpts, wield;
    int16_t dextadj;
    CRE *helping;                          /* Who I'm helping	*/
    CRE *nxthelp;                          /* Next person		*/
    CRE *helped;                           /* 1st helping me	*/
    CRE *following, *nxtfollow, *followed; /* followers		*/
    CRE *snooping, *nxtsnoop, *snooper;    /* snoopers		*/
    CRE *fighting, *attme;                 /* attme=attacking me	*/
    void *info;                            /* A bit more		*/
};

// Line-status
struct _LS {
    int16_t rec, unum;
    char line, iosup; /* Which line am I?	*/
    slotid_t IOlock;  /* Device in use?	*/
    char pre[80];     /* Pre-rank description	*/
    char post[80];    /* Post-rank description*/
    char arr[80];     /* When player arrives	*/
    char dep[80];     /* When player leaves	*/
};

// Player
struct _HUMAN {
    char *name;         /* Player's name	 */
    char passwd[9];     /* Password or User ID	 */
    long score;         /* Score to date...      */
    int16_t rdmode;     /* Users RD Mode	 */
    int16_t strength;   /* How strong is he?	 */
    int16_t stamina;    /* Stamina		 */
    int16_t dext;       /* Dexterity		 */
    int16_t wisdom;     /* Wisdom		 */
    int16_t experience; /* Experience		 */
    int16_t magicpts;   /* Magic points		 */
    int16_t rank;       /* How he rates!	 */
    long plays;         /* Times played		 */
    long tasks;         /* Tasks completed	 */
    int16_t tries;      /* Bad tries since last	 */
    char archetype;     /* Player class		 */
    char sex;           /* Players sex		 */
    char flags;         /* Ansi, LF, Redo	 */
    char llen;          /* Line length		 */
    char slen;          /* Screen length	 */
    char rchar;         /* Redo character	 */
};

// Inanimate object bobj info
struct _OBJ {
    uint16_t id;               /* My name		*/
    int16_t nrooms;            /* No. of rooms its in	*/
    int16_t adj;               /* It's adjective	*/
    uint16_t flags;            /* Various 'fixed' flags*/
    char nstates;              /* No. of states	*/
    char putto;                /* Where things go	*/
    char state;                /* Current state	*/
    char npc;               /* Mobile character	*/
    struct ObjState *states; /* Ptr to states!	*/
};
