//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: h/Stct.H		Structure Definitions
//
//	LMod: oliver 20/06/93	Added time limit variables (s_used,s_left etc)
//	      oliver 19/06/93	Added VERB to mobile character
//				& article/initstate to objects
//	      oliver 11/06/93	AMUL->AGL
//

SDEF _PLAYER {			// Player
	char	name[NAMEL+1];		// Player's name
	char	passwd[41];		// Password or User ID
	long	score;			// Score to date...
	int	rdmode;			// Users RD Mode
	long	plays,tasks;
	long	last_session;		// Start of session of last game
	short	strength;		// How strong is he?
	short	stamina;		// Stamina
	short	dext;			// Dexterity
	short	wisdom;			// Wisdom
	short	experience;		// Experience
	short	magicpts;		// Magic points
	short	rank;			// How he rates!
	short	tries;			// Bad tries since last
	short	minutes;		// Maximum time allowed online
	char	class;			// Player class
	char	sex;			// Players sex
	char	flags,spareC;		// Flags
	char	llen,slen;		// Screen Length/Width
	char	spare[16];		// -- Spare Space!! --
};

SDEF LS {			// Line Status
	char	*buf;			// Text buffer
struct MsgPort	*rep;			// My reply port
	long	flags;			// User's flags
	short	room;			// My room
	short	m_left,m_used;		// Minutes left on-line/used
	long	sctg;			// SCore This Game
unsigned short	rec;			// My record no.
	short	numobj;			// Objects carried
	long	weight;			// Weight carried
	short	dextadj;		// Dexterity Adjustments
	short	wield;			// Current Weapon
	char	state;			// State of line
	char	IOlock;			// Device in use?
	char	xpos;			// User's io type
	char	light;			// If player has a light
	char	hadlight;		// If I had a light
	char	helping;		// Player I am helping
	char	helped;			// Getting help from
	char	following;		// Who I am following
	char	followed;		// Who is following me
	char	fighting;
	char	pre[80];		// Pre-rank description
	char	post[80];		// Post-rank description
	char	*arr,*dep;		// Arrive/Depart strings
};

SDEF Aport {
	struct	Message msg;
	long	type,from,data,p1,p2,p3,p4;
	char	*ptr;
};

SDEF _ROOM_STRUCT {		// Room def struct
	char	id[IDL+1];		// Room I.D.
unsigned char	ttlines;		// No. of TT lines
	short	flags;			// Room FIXED flags
	long	desptr;			// Ptr to des. in des file
	long	tabptr;			// Ptr to T.T. data
};

SDEF _VERB_STRUCT {		// Verb def struct
	char	id[IDL+1];		// The Verb itself
	char	flags;			// Travel? etc...
	char	sort[4];		// Sort method...(yawn)
	char	sort2[4];		// Sort #2!
	short	ents;			// No. of slot entries
struct _SLOTTAB *ptr;			// Pointer to slots tab
};

SDEF	_SLOTTAB {		// Slot table def
	char	wtype[4];		// Word type expected
	long	slot[4];		// List of slots
unsigned short	ents;			// No. of entries
struct _VBTAB	*ptr;			// Points to Verb Table
};

SDEF _VBTAB {			// Verb Table struct
	short	condition;		// Condition
	short	action;			// #>0=action, #<0=room
	long	*pptr;			// Param ptr; -1=none
};

SDEF _RANK_STRUCT {		// Rank information
	char	male[RANKL];		// chars for male descrp
	char	female[RANKL];		// Women! Huh!
	long	score;			// Score to date
	short	strength;		// How strong is he?
	short	stamina;		// Stamina
	short	dext;			// Dexterity
	short	wisdom;			// Wisdom
	short	experience;		// Experience
	short	magicpts;		// Magic points
	long	maxweight;		// Maximum weight
	short	numobj;			// Max. objects carried
	short	minpksl;		// Min. pts for killin
	short	tasks;			// Tasks needed for level
	char	prompt[12];		// Prompt 4 this rank
};

SDEF _NTAB_STRUCT {		// Noun table structure
	char	id[IDL+1];		// Object's NAME
unsigned char	num_of;			// Number of this type
struct _OBJ_STRUCT *first;		// First in the list
};

SDEF _OBJ_STRUCT2 {		// Object (temporary) definition
	char	id[IDL+1];		// Object's NAME
	char	state;			// Current state
	short	putto	:	5;	// Where things go
	short	article :	3;	// Is it an in on under?
	short	spare	:	8;	// Not used - yet
	char	nstates;		// No. of states
	char	initstate;		// Initial state
	short	idno;			// Object's ID no
	short	nrooms;			// No. of rooms its in
	short	adj;			// Adjective. -1 = none
	short	inside;			// No. objects inside
	short	flags;			// Fixed flags
	short	mobile;			// Mobile character
	long	contains;		// How much it will hold
	long	winside;		// How much weight inside it?
struct	_OBJ_STATE *states;		// Ptr to states!
	long	*rmlist;		// List of rooms
};

SDEF _OBJ_STRUCT {		// Object (proper) definition
	char	id[IDL+1];		// Object's NAME
	char	state;			// Current state
	short	putto	:	5;	// Where things go
	short	article :	3;	// Is it an in on under?
	short	spare	:	8;	// Not used - yet
	char	nstates;		// No. of states
	char	initstate;		// Initial state
	short	idno;			// Object's ID no
	short	nrooms;			// No. of rooms its in
	short	adj;			// Adjective. -1 = none
	short	inside;			// No. objects inside
	short	flags;			// Fixed flags
	short	mobile;			// Mobile character
	long	contains;		// How much it will hold
	long	winside;		// How much weight inside it?
struct	_OBJ_STATE *states;		// Ptr to states!
	long	*rmlist;		// List of rooms
};

SDEF _OBJ_STATE {		// State description
unsigned long	weight;			// In grammes
	long	value;			// Whassit worth?
	short	strength;		// How strong is it?
	short	damage;			// Amount of damage
	long	descrip;		// ptr to descrp in file
	short	flags;			// State flags
};

SDEF _TT_ENT {			// TT Entry
	short	verb;			// Verb no.
	short	condition;		// Condition
	short	action;			// #>0=action, #<0=room
	long	*pptr;			// Param ptr. -1 = none
};

SDEF _MOB_ENT {			// Mobile Character Entry
	char	id[IDL+1];		// Name of mobile
	char	dead;			// State flags
	char	speed,travel,fight,act,wait;	// speed & %ages
	char	fear,attack;		// others
	char	flags;			// (padding really)
	short	dmove;
	short	hitpower;		// Hit power
	short	verb;			// Verb No.
	long	arr,dep,flee,hit,miss,death;	// Various UMsgs
};

SDEF _MOB_TAB {			// Mobile table entry
	short	obj;			// Object no.
	short	speed;			// Secs/turn
	short	count;			// Time till next
unsigned short	pflags;			// Player style flags
};

#ifdef	AMAN
SDEF _DAEMON {			// AMan Daemon Timer
	struct _DAEMON	*nxt,*prv;
	short		own,count,num;
	long		val[2],typ[2];
};
#endif

