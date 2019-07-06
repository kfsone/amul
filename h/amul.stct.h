#pragma once

#include "h/amul.defs.h"

struct _PLAYER /* Player def struct */
{
	char	  name[NAMEL + 1]; /* Player's name	 */
	char	  passwd[23];	  /* Password or User ID	 */
	int32_t   score;		   /* Score to date...      */
	int		  rdmode;		   /* Users RD Mode	 */
	short int strength;		   /* How strong is he?	 */
	short int stamina;		   /* Stamina		 */
	short int dext;			   /* Dexterity		 */
	short int wisdom;		   /* Wisdom		 */
	short int experience;	  /* Experience		 */
	short int magicpts;		   /* Magic points		 */
	short int rank;			   /* How he rates!	 */
	int32_t   plays;		   /* Times played		 */
	int32_t   tasks;		   /* Tasks completed	 */
	short int tries;		   /* Bad tries since last	 */
	char	  archetype;	   /* Player archetype		 */
	char	  sex;			   /* Players sex		 */
	char	  flags;		   /* Ansi, LF, Redo	 */
	char	  llen;			   /* Line length		 */
	char	  slen;			   /* Screen length	 */
	char	  rchar;		   /* Redo character	 */
};

struct LS /* IO Driver Line Status Info */
{
	char *			   buf;		  /* Pointer to output	 */
	Amiga::MsgPort *   rep;		  /* My reply port	 */
	int32_t			   flags;	 /* User's flags		 */
	short int		   room;	  /* My room		 */
	int32_t			   sctg;	  /* SCore This Game	 */
	unsigned short int rec;		  /* My record no.	 */
	short int		   numobj;	/* Objects carried	 */
	int32_t			   weight;	/* Weight carried	 */
	short int		   strength;  /* How strong is he?	 */
	short int		   stamina;   /* Stamina		 */
	short int		   dext;	  /* Dexterity		 */
	short int		   dextadj;   /* Dexterity Adjustments */
	short int		   wisdom;	/* Wisdom		 */
	short int		   magicpts;  /* Magic points		 */
	short int		   wield;	 /* No. of weapon used	 */
	short int		   unum;	  /* My user number...	 */
	char			   state;	 /* State of line	 */
	char			   IOlock;	/* Device in use?	 */
	char			   following; /* Who I am following	 */
	char			   light;	 /* If player has a light */
	char			   hadlight;  /* If I had a light	 */
	char			   helping;   /* Player I am helping	 */
	char			   helped;	/* Getting help from	 */
	char			   followed;  /* Who is following me	 */
	char			   fighting;  /* Who I am fighting	 */
	char			   pre[80];   /* Pre-rank description	 */
	char			   post[80];  /* Post-rank description */
	char			   arr[80];   /* When player arrives	 */
	char			   dep[80];   /* When player leaves	 */
};

struct Aport : public Amiga::Message
{
	int32_t type, from, data;
	int32_t p1, p2, p3, p4; /* Action parameters	  */
	char *  ptr;
};

struct _ROOM_STRUCT /* Room def struct */
{
	char			   id[IDL + 1]; /* Room I.D.		   */
	short int		   flags;		/* Room FIXED flags        */
	int32_t			   desptr;		/* Ptr to des. in des file */
	int32_t			   tabptr;		/* Ptr to T.T. data 	   */
	unsigned short int ttlines;		/* No. of TT lines	   */
};

struct _VBTAB /* Verb Table struct */
{
	int32_t  condition; /* Condition		 */
	int32_t  action;	/* #>0=action, #<0=room  */
	int32_t *pptr;		/* Param ptr. -1 = none  */
};

struct _SLOTTAB /* Slot table def */
{
	char	 wtype[5]; /* Type of word expected */
	int32_t  slot[5];  /* List of slots	 */
	uint16_t ents;	 /* No. of entries	 */
	_VBTAB * ptr;	  /* Pointer to Verb Table */
};

struct _VERB_STRUCT /* Verb def struct */
{
	char	  id[IDL + 1]; /* The Verb itself       */
	char	  flags;	   /* Travel? etc...	 */
	char	  sort[5];	 /* Sort method... (yawn) */
	char	  sort2[5];	/* Sort #2!		 */
	short int ents;		   /* No. of slot entries	 */
	_SLOTTAB *ptr;		   /* Pointer to slots tab	 */
};

struct _RANK_STRUCT /* Rank information */
{
	char	  male[RANKL];   /* chars for male descrp */
	char	  female[RANKL]; /* Women! Huh!           */
	int32_t   score;		 /* Score to date...      */
	short int strength;		 /* How strong is he?	 */
	short int stamina;		 /* Stamina		 */
	short int dext;			 /* Dexterity		 */
	short int wisdom;		 /* Wisdom		 */
	short int experience;	/* Experience		 */
	short int magicpts;		 /* Magic points		 */
	int32_t   maxweight;	 /* Maximum weight	 */
	short int numobj;		 /* Max. objects carried	 */
	short int minpksl;		 /* Min. pts for killin	 */
	short int tasks;		 /* Tasks needed for level*/
	char	  prompt[11];	/* Prompt 4 this rank	 */
};

struct _NTAB_STRUCT /* Noun table structure */
{
	char				id[IDL + 1]; /* Object's NAME	 */
	short int			num_of;		 /* Number of this type	 */
	struct _OBJ_STRUCT *first;		 /* First in the list	 */
};

struct _OBJ_STRUCT2 /* Object (temporary) definition */
{
	char			   id[IDL + 1]; /* Object's NAME	 */
	short int		   idno;		/* Object's ID no	 */
	short int		   nrooms;		/* No. of rooms its in	 */
	short int		   adj;			/* Adjective. -1 = none	 */
	short int		   inside;		/* No. objects inside	 */
	short int		   flags;		/* Fixed flags		 */
	int32_t			   contains;	/* How much it will hold */
	char			   nstates;		/* No. of states	 */
	char			   putto;		/* Where things go	 */
	char			   state;		/* Current state	 */
	char			   mobile;		/* Mobile character	 */
	struct _OBJ_STATE *states;		/* Ptr to states!	 */
	int32_t *		   rmlist;		/* List of rooms	 */
};

struct _OBJ_STRUCT /* Object (proper) definition */
{
	char			   id[IDL + 1]; /* Object's NAME	 */
	short int		   idno;		/* Object's ID no	 */
	short int		   nrooms;		/* No. of rooms its in	 */
	short int		   adj;			/* Adjective. -1 = none	 */
	short int		   inside;		/* No. objects inside	 */
	short int		   flags;		/* Fixed flags		 */
	int32_t			   contains;	/* How much it will hold */
	char			   nstates;		/* No. of states	 */
	char			   putto;		/* Where things go	 */
	char			   state;		/* Current state	 */
	char			   mobile;		/* Mobile character	 */
	struct _OBJ_STATE *states;		/* Ptr to states!	 */
	int32_t *		   rmlist;		/* List of rooms	 */
};

struct _OBJ_STATE /* State description */
{
	uint32_t		   weight;   /* In grammes		 */
	int32_t			   value;	/* Whassit worth?	 */
	unsigned short int strength; /* How strong is it?	 */
	unsigned short int damage;   /* Amount of damage	 */
	int32_t			   descrip;  /* ptr to descrp in file */
	short int		   flags;	/* State flags		 */
};

struct _TT_ENT /* TT Entry */
{
	short int verb;		 /* Verb no.		 */
	int32_t   condition; /* Condition		 */
	int32_t   action;	/* #>0=action, #<0=room  */
	int32_t * pptr;		 /* Param ptr. -1 = none  */
};

struct _MOB_ENT /* Mobile Character Entry */
{
	char		   id[IDL + 1];						/* Name of mobile	*/
	char		   dead;							/* State flags		*/
	char		   speed, travel, fight, act, wait; /* speed & %ages	*/
	char		   fear, attack;					/* others		*/
	char		   flags;							/* (padding really)	*/
	uint16_t	   dmove;
	short int	  hitpower;						 /* Hit power		*/
	int32_t		   arr, dep, flee, hit, miss, death; /* Various UMsgs	*/
	short int	  knows;							 /* Number of &		*/
	struct _VBTAB *cmds;							 /* & ptr to cmds	*/
};

struct _MOB_TAB /* Mobile table entry */
{
	uint16_t obj;	/* Object no.		*/
	uint16_t speed;  /* Secs/turn		*/
	uint16_t count;  /* Time till next	*/
	uint16_t pflags; /* Player style flags	*/
};

#define boZONKED 0x0001   /* its out-of-play	*/
#define boCREATURE 0x0002 /* its a creature	*/
#define boGLOWING 0x0004  /* light-source		*/
#define boINVIS 0x0008	/* its invisible	*/
#define boSINVIS 0x0010   /* its super-invis	*/
#define boMOVING 0x0020   /* Is in-transit	*/
#define boDEAF 0x0040	 /* Is deafened		*/
#define boBLIND 0x0080	/* Is blind		*/
#define boDUMB 0x0100	 /* Dumbed		*/
#define boCRIPPLED 0x0200 /* Crippled		*/
#define boLOCKED 0x8000   /* CAN'T ACCESS		*/

#define SBOB struct _BOBJ

struct _BOBJ /* Basic object struct */
{
	uint8_t		  type, state;			/* Type of 'presence'	*/
	char *		  name, *archetype;		/* Name & archetypees	*/
	uint32_t	  holdsw, holdsi;		/* Max. contents	*/
	uint32_t	  flags;				/* Any fixed flags	*/
	uint32_t	  myweight, weight;		/* Item's weight & carried */
	uint16_t	  light;				/* Light-source count	*/
	uint16_t	  inv;					/* Inventory (count)	*/
	struct _BOBJ *nxt, *pre, *in, *own; /* Next, prev, 1st in, owner */
	void *		  info;					/* Pointer to REAL data	*/
};

struct _ROOM /* Room object struct */
{
	uint16_t flags;   /* Room FIXED flags        */
	int32_t  desptr;  /* Ptr to des. in des file */
	int32_t  tabptr;  /* Ptr to T.T. data 	   */
	uint16_t ttlines; /* No. of TT lines	   */
};

#define CRE struct _BEING /* Creature */

struct _BEING /* Mobile/char struct */
{
	char *			buf;					 /* Where to put text	*/
	Amiga::MsgPort *rep;					 /* Notify me here	*/
	uint32_t		flags;					 /* Flags		*/
	int32_t			sctg;					 /* Points scd this game	*/
	uint16_t		strength, stamina, dext; /* Fators		*/
	uint16_t		wisdom, magicpts, wield; /* factors		*/
	uint16_t		dextadj;
	CRE *			helping;						  /* Who I'm helping	*/
	CRE *			nxthelp;						  /* Next person		*/
	CRE *			helped;							  /* 1st helping me	*/
	CRE *			following, *nxtfollow, *followed; /* followers		*/
	CRE *			snooping, *nxtsnoop, *snooper;	/* snoopers		*/
	CRE *			fighting, *attme;				  /* attme=attacking me	*/
	void *			info;							  /* A bit more		*/
};

struct _LS /* IO Driver Line Status Info */
{
	uint16_t rec, unum;
	char	 line;	 /* Which line am I?	*/
	char	 IOlock;   /* Device in use?	*/
	char	 pre[80];  /* Pre-rank description	*/
	char	 post[80]; /* Post-rank description*/
	char	 arr[80];  /* When player arrives	*/
	char	 dep[80];  /* When player leaves	*/
};

struct _HUMAN /* Player def struct */
{
	char *   name;		 /* Player's name	 */
	char	 passwd[9];  /* Password or User ID	 */
	int32_t  score;		 /* Score to date...      */
	uint16_t rdmode;	 /* Users RD Mode	 */
	uint16_t strength;   /* How strong is he?	 */
	uint16_t stamina;	/* Stamina		 */
	uint16_t dext;		 /* Dexterity		 */
	uint16_t wisdom;	 /* Wisdom		 */
	uint16_t experience; /* Experience		 */
	uint16_t magicpts;   /* Magic points		 */
	uint16_t rank;		 /* How he rates!	 */
	int32_t  plays;		 /* Times played		 */
	int32_t  tasks;		 /* Tasks completed	 */
	uint16_t tries;		 /* Bad tries since last	 */
	char	 archetype;  /* Player archetype		 */
	char	 sex;		 /* Players sex		 */
	char	 flags;		 /* Ansi, LF, Redo	 */
	char	 llen;		 /* Line length		 */
	char	 slen;		 /* Screen length	 */
	char	 rchar;		 /* Redo character	 */
};

struct _OBJ /* Object definition */
{
	uint16_t		   id;		/* My name		*/
	uint16_t		   nrooms;  /* No. of rooms its in	*/
	uint16_t		   adj;		/* It's adjective	*/
	uint16_t		   flags;   /* Various 'fixed' flags*/
	char			   nstates; /* No. of states	*/
	char			   putto;   /* Where things go	*/
	char			   state;   /* Current state	*/
	char			   mobile;  /* Mobile character	*/
	struct _OBJ_STATE *states;  /* Ptr to states!	*/
};

struct _MOB /* Mobile Character Entry */
{
	uint16_t	   dmove;							 /* Move to when it dies	*/
	char		   deadstate;						 /* State flags		*/
	char		   speed, travel, fight, act, wait;  /* speed & %ages	*/
	char		   fear, attack;					 /* others		*/
	uint16_t	   flags;							 /* -- none yet --	*/
	uint16_t	   rank;							 /* Rank equiv		*/
	int32_t		   arr, dep, flee, hit, miss, death; /* Various UMsgs	*/
	short int	  knows;							 /* Number of &		*/
	struct _VBTAB *cmds;							 /* & ptr to cmds	*/
};
