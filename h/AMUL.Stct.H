struct	_PLAYER			/* Player def struct */
{
	char	name[NAMEL+1];			/* Player's name	 */
	char	passwd[23];			/* Password or User ID	 */
	long	score;				/* Score to date...      */
	int	rdmode;				/* Users RD Mode	 */
   short int	strength;			/* How strong is he?	 */
   short int	stamina;			/* Stamina		 */
   short int	dext;				/* Dexterity		 */
   short int	wisdom;				/* Wisdom		 */
   short int	experience;			/* Experience		 */
   short int	magicpts;			/* Magic points		 */
   short int	rank;				/* How he rates!	 */
	long	plays;				/* Times played		 */
   	long	tasks;				/* Tasks completed	 */
   short int	tries;				/* Bad tries since last	 */
	char	class;				/* Player class		 */
	char	sex;				/* Players sex		 */
	char	flags;				/* Ansi, LF, Redo	 */
	char	llen;				/* Line length		 */
	char	slen;				/* Screen length	 */
	char	rchar;				/* Redo character	 */
};

struct	LS			/* IO Driver Line Status Info */
{
	char	*buf;				/* Pointer to output	 */
struct MsgPort	*rep;				/* My reply port	 */
	long	flags;				/* User's flags		 */
   short int	room;				/* My room		 */
	long	sctg;				/* SCore This Game	 */
unsigned short int rec;				/* My record no.	 */
   short int	numobj;				/* Objects carried	 */
	long	weight;				/* Weight carried	 */
   short int	strength;			/* How strong is he?	 */
   short int	stamina;			/* Stamina		 */
   short int	dext;				/* Dexterity		 */
   short int	dextadj;			/* Dexterity Adjustments */
   short int	wisdom;				/* Wisdom		 */
   short int	magicpts;			/* Magic points		 */
   short int	wield;				/* No. of weapon used	 */
   short int	unum;				/* My user number...	 */
	char	state;				/* State of line	 */
	char	IOlock;				/* Device in use?	 */
	char	following;			/* Who I am following	 */
	char	sup;				/* User's io type	 */
	char	light;				/* If player has a light */
	char	hadlight;			/* If I had a light	 */
	char	helping;			/* Player I am helping	 */
	char	helped;				/* Getting help from	 */
	char	followed;			/* Who is following me	 */
	char	fighting;			/* Who I am fighting	 */
	char	pre[80];			/* Pre-rank description	 */
	char	post[80];			/* Post-rank description */
	char	arr[80];			/* When player arrives	 */
	char	dep[80];			/* When player leaves	 */
};

struct Aport
{
	struct	Message msg;
	long	type,from,data;
	long	p1,p2,p3,p4;			/* Action parameters	  */
	char	*ptr;
};

struct	_ROOM_STRUCT		/* Room def struct */
{
	char	id[IDL+1];			/* Room I.D.		   */
   short int	flags;				/* Room FIXED flags        */
	long	desptr;				/* Ptr to des. in des file */
	long	tabptr;				/* Ptr to T.T. data 	   */
unsigned short int ttlines;			/* No. of TT lines	   */
};

struct	_VERB_STRUCT		/* Verb def struct */
{
	char	id[IDL+1];			/* The Verb itself       */
	char	flags;				/* Travel? etc...	 */
	char	sort[5];			/* Sort method... (yawn) */
	char	sort2[5];			/* Sort #2!		 */
   short int	ents;				/* No. of slot entries	 */
struct _SLOTTAB *ptr;				/* Pointer to slots tab	 */
};

struct	_SLOTTAB		/* Slot table def */
{
	char	wtype[5];			/* Type of word expected */
	long	slot[5];			/* List of slots	 */
	USHORT	ents;				/* No. of entries	 */
struct _VBTAB	*ptr;				/* Pointer to Verb Table */
};

struct	_VBTAB			/* Verb Table struct */
{
	long	condition;			/* Condition		 */
	long	action;				/* #>0=action, #<0=room  */
	long	*pptr;				/* Param ptr. -1 = none  */
};

struct	_RANK_STRUCT		/* Rank information */
{
	char	male[RANKL];			/* chars for male descrp */
	char	female[RANKL];			/* Women! Huh!           */
	long	score;				/* Score to date...      */
   short int	strength;			/* How strong is he?	 */
   short int	stamina;			/* Stamina		 */
   short int	dext;				/* Dexterity		 */
   short int	wisdom;				/* Wisdom		 */
   short int	experience;			/* Experience		 */
   short int	magicpts;			/* Magic points		 */
	long	maxweight;			/* Maximum weight	 */
   short int	numobj;				/* Max. objects carried	 */
   short int	minpksl;			/* Min. pts for killin	 */
   short int	tasks;				/* Tasks needed for level*/
	char	prompt[11];			/* Prompt 4 this rank	 */
};

struct	_NTAB_STRUCT		/* Noun table structure */
{
	char	id[IDL+1];			/* Object's NAME	 */
   short int	num_of;				/* Number of this type	 */
struct _OBJ_STRUCT *first;			/* First in the list	 */
};

struct	_OBJ_STRUCT2		/* Object (temporary) definition */
{
	char	id[IDL+1];			/* Object's NAME	 */
   short int	idno;				/* Object's ID no	 */
   short int	nrooms;				/* No. of rooms its in	 */
   short int	adj;				/* Adjective. -1 = none	 */
   short int	inside;				/* No. objects inside	 */
   short int	flags;				/* Fixed flags		 */
	long	contains;			/* How much it will hold */
	char	nstates;			/* No. of states	 */
	char	putto;				/* Where things go	 */
	char	state;				/* Current state	 */
	char	mobile;				/* Mobile character	 */
struct	_OBJ_STATE *states;			/* Ptr to states!	 */
	long	*rmlist;			/* List of rooms	 */
};

struct	_OBJ_STRUCT		/* Object (proper) definition */
{
	char	id[IDL+1];			/* Object's NAME	 */
   short int	idno;				/* Object's ID no	 */
   short int	nrooms;				/* No. of rooms its in	 */
   short int	adj;				/* Adjective. -1 = none	 */
   short int	inside;				/* No. objects inside	 */
   short int	flags;				/* Fixed flags		 */
	long	contains;			/* How much it will hold */
	char	nstates;			/* No. of states	 */
	char	putto;				/* Where things go	 */
	char	state;				/* Current state	 */
	char	mobile;				/* Mobile character	 */
struct	_OBJ_STATE *states;			/* Ptr to states!	 */
	long	*rmlist;			/* List of rooms	 */
};

struct	_OBJ_STATE		/* State description */
{
unsigned long	weight;				/* In grammes		 */
	long	value;				/* Whassit worth?	 */
unsigned short int strength;			/* How strong is it?	 */
unsigned short int damage;			/* Amount of damage	 */
	long	descrip;			/* ptr to descrp in file */
   short int	flags;				/* State flags		 */
};

struct	_TT_ENT			/* TT Entry */
{
   short int	verb;				/* Verb no.		 */
	long	condition;			/* Condition		 */
	long	action;				/* #>0=action, #<0=room  */
	long	*pptr;				/* Param ptr. -1 = none  */
};

struct _MOB_ENT			/* Mobile Character Entry */
{
	char	id[IDL+1];			/* Name of mobile	*/
	char	dead;				/* State flags		*/
	char	speed,travel,fight,act,wait;	/* speed & %ages	*/
	char	fear,attack;			/* others		*/
	char	flags;				/* (padding really)	*/
	USHORT	dmove;
   short int	hitpower;			/* Hit power		*/
	long	arr,dep,flee,hit,miss,death;	/* Various UMsgs	*/
   short int	knows;				/* Number of &		*/
struct _VBTAB	*cmds;				/* & ptr to cmds	*/
};

struct _MOB_TAB			/* Mobile table entry */
{
	USHORT	obj;				/* Object no.		*/
	USHORT	speed;				/* Secs/turn		*/
	USHORT	count;				/* Time till next	*/
	USHORT	pflags;				/* Player style flags	*/
};

#define	boZONKED	0x0001			/* its out-of-play	*/
#define	boCREATURE	0x0002			/* its a creature	*/
#define	boGLOWING	0x0004			/* light-source		*/
#define	boINVIS		0x0008			/* its invisible	*/
#define	boSINVIS	0x0010			/* its super-invis	*/
#define	boMOVING	0x0020			/* Is in-transit	*/
#define	boDEAF		0x0040			/* Is deafened		*/
#define	boBLIND		0x0080			/* Is blind		*/
#define	boDUMB		0x0100			/* Dumbed		*/
#define	boCRIPPLED	0x0200			/* Crippled		*/
#define	boLOCKED	0x8000			/* CAN'T ACCESS		*/

#define	SBOB struct _BOBJ

struct _BOBJ			/* Basic object struct */
{
	BYTE	type,state;			/* Type of 'presence'	*/
	char	*name,*class;			/* Name & classes	*/
	ULONG	holdsw,holdsi;			/* Max. contents	*/
	ULONG	flags;				/* Any fixed flags	*/
	ULONG	myweight,weight;		/* Item's weight & carried */
	USHORT	light;				/* Light-source count	*/
	USHORT	inv;				/* Inventory (count)	*/
	struct	_BOBJ *nxt,*pre,*in,*own;	/* Next, prev, 1st in, owner */
	APTR	*info;				/* Pointer to REAL data	*/
};

struct	_ROOM			/* Room object struct */
{
	USHORT	flags;				/* Room FIXED flags        */
	long	desptr;				/* Ptr to des. in des file */
	long	tabptr;				/* Ptr to T.T. data 	   */
	USHORT	ttlines;			/* No. of TT lines	   */
};

#define	CRE	struct _BEING	/* Creature */

struct	_BEING			/* Mobile/char struct */
{
	char	*buf;				/* Where to put text	*/
struct MsgPort	*rep;				/* Notify me here	*/
	ULONG	flags;				/* Flags		*/
	long	sctg;				/* Points scd this game	*/
	USHORT	strength,stamina,dext;		/* Fators		*/
	USHORT	wisdom,magicpts,wield;		/* factors		*/
	USHORT	dextadj;
	CRE	*helping;			/* Who I'm helping	*/
	CRE	*nxthelp;			/* Next person		*/
	CRE	*helped;			/* 1st helping me	*/
	CRE	*following,*nxtfollow,*followed;/* followers		*/
	CRE	*snooping,*nxtsnoop,*snooper;	/* snoopers		*/
	CRE	*fighting,*attme;		/* attme=attacking me	*/
	APTR	*info;				/* A bit more		*/
};
	
struct	_LS			/* IO Driver Line Status Info */
{
	USHORT	rec,unum;
	char	line,iosup;			/* Which line am I?	*/
	char	IOlock;				/* Device in use?	*/
	char	pre[80];			/* Pre-rank description	*/
	char	post[80];			/* Post-rank description*/
	char	arr[80];			/* When player arrives	*/
	char	dep[80];			/* When player leaves	*/
};

struct	_HUMAN			/* Player def struct */
{
	char	*name;				/* Player's name	 */
	char	passwd[9];			/* Password or User ID	 */
	long	score;				/* Score to date...      */
	USHORT	rdmode;				/* Users RD Mode	 */
	USHORT	strength;			/* How strong is he?	 */
	USHORT	stamina;			/* Stamina		 */
	USHORT	dext;				/* Dexterity		 */
	USHORT	wisdom;				/* Wisdom		 */
	USHORT	experience;			/* Experience		 */
	USHORT	magicpts;			/* Magic points		 */
	USHORT	rank;				/* How he rates!	 */
	long	plays;				/* Times played		 */
   	long	tasks;				/* Tasks completed	 */
	USHORT	tries;				/* Bad tries since last	 */
	char	class;				/* Player class		 */
	char	sex;				/* Players sex		 */
	char	flags;				/* Ansi, LF, Redo	 */
	char	llen;				/* Line length		 */
	char	slen;				/* Screen length	 */
	char	rchar;				/* Redo character	 */
};

struct	_OBJ			/* Object definition */
{
	USHORT	id;				/* My name		*/
	USHORT	nrooms;				/* No. of rooms its in	*/
	USHORT	adj;				/* It's adjective	*/
	USHORT	flags;				/* Various 'fixed' flags*/
	char	nstates;			/* No. of states	*/
	char	putto;				/* Where things go	*/
	char	state;				/* Current state	*/
	char	mobile;				/* Mobile character	*/
struct	_OBJ_STATE *states;			/* Ptr to states!	*/
};

struct _MOB			/* Mobile Character Entry */
{
	USHORT	dmove;				/* Move to when it dies	*/
	char	deadstate;			/* State flags		*/
	char	speed,travel,fight,act,wait;	/* speed & %ages	*/
	char	fear,attack;			/* others		*/
	USHORT	flags;				/* -- none yet --	*/
	USHORT	rank;				/* Rank equiv		*/
	long	arr,dep,flee,hit,miss,death;	/* Various UMsgs	*/
   short int	knows;				/* Number of &		*/
struct _VBTAB	*cmds;				/* & ptr to cmds	*/
};

