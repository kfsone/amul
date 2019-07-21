//
//		## AMUD.Stct.H -- AMUD Structure Definitions ##
//

SDEF _PLAYER {			// Player
	char	name[NAMEL+1];		// Player's name
	char	passwd[41];		// Password or User ID
	long	score;			// Score to date...
	int	rdmode;			// Users RD Mode
	long	plays,tasks;
	long	last_session;		// Start of session of last game
	SHORT	strength;		// How strong is he?
	SHORT	stamina;		// Stamina
	SHORT	dext;			// Dexterity
	SHORT	wisdom;			// Wisdom
	SHORT	experience;		// Experience
	SHORT	magicpts;		// Magic points
	SHORT	rank;			// How he rates!
	SHORT	tries;			// Bad tries since last
	SHORT	credits;		// -- Not Used Yet! --
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
	SHORT	room;			// My room
	long	sctg;			// SCore This Game
	USHORT	rec;			// My record no.
	SHORT	numobj;			// Objects carried
	long	weight;			// Weight carried
	SHORT	dextadj;		// Dexterity Adjustments
	SHORT	wield;			// Current Weapon
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
	SHORT	flags;			// Room FIXED flags
	long	desptr;			// Ptr to des. in des file
	long	tabptr;			// Ptr to T.T. data
};

SDEF _VERB_STRUCT {		// Verb def struct
	char	id[IDL+1];		// The Verb itself
	char	flags;			// Travel? etc...
	char	sort[4];		// Sort method...(yawn)
	char	sort2[4];		// Sort #2!
	SHORT	ents;			// No. of slot entries
struct _SLOTTAB *ptr;			// Pointer to slots tab
};

SDEF	_SLOTTAB {		// Slot table def
	char	wtype[4];		// Word type expected
	long	slot[4];		// List of slots
	USHORT	ents;			// No. of entries
struct _VBTAB	*ptr;			// Points to Verb Table
};

SDEF _VBTAB {			// Verb Table struct
	SHORT	condition;		// Condition
	SHORT	action;			// #>0=action, #<0=room
	long	*pptr;			// Param ptr; -1=none
};

SDEF _RANK_STRUCT {		// Rank information
	char	male[RANKL];		// chars for male descrp
	char	female[RANKL];		// Women! Huh!
	long	score;			// Score to date
	SHORT	strength;		// How strong is he?
	SHORT	stamina;		// Stamina
	SHORT	dext;			// Dexterity
	SHORT	wisdom;			// Wisdom
	SHORT	experience;		// Experience
	SHORT	magicpts;		// Magic points
	long	maxweight;		// Maximum weight
	SHORT	numobj;			// Max. objects carried
	SHORT	minpksl;		// Min. pts for killin
	SHORT	tasks;			// Tasks needed for level
	char	prompt[12];		// Prompt 4 this rank
};

SDEF _NTAB_STRUCT {		// Noun table structure
	char	id[IDL+1];		// Object's NAME
unsigned char	num_of;			// Number of this type
struct _OBJ_STRUCT *first;		// First in the list
};

SDEF _OBJ_STRUCT2 {		// Object (temporary) definition
	char	id[IDL+1];		// Object's NAME
	char	nstates;		// No. of states
	char	putto;			// Where things go
	char	state;			// Current state
	SHORT	idno;			// Object's ID no
	SHORT	nrooms;			// No. of rooms its in
	SHORT	adj;			// Adjective. -1 = none
	SHORT	inside;			// No. objects inside
	SHORT	flags;			// Fixed flags
	SHORT	mobile;			// Mobile character
	long	contains;		// How much it will hold
struct	_OBJ_STATE *states;		// Ptr to states!
	long	*rmlist;		// List of rooms
};

SDEF _OBJ_STRUCT {		// Object (proper) definition
	char	id[IDL+1];		// Object's NAME
	char	nstates;		// No. of states
	char	putto;			// Where things go
	char	state;			// Current state
	SHORT	idno;			// Object's ID no
	SHORT	nrooms;			// No. of rooms its in
	SHORT	adj;			// Adjective. -1 = none
	SHORT	inside;			// No. objects inside
	SHORT	flags;			// Fixed flags
	SHORT	mobile;			// Mobile character
	long	contains;		// How much it will hold
struct	_OBJ_STATE *states;		// Ptr to states!
	long	*rmlist;		// List of rooms
};

SDEF _OBJ_STATE {		// State description
	ULONG	weight;			// In grammes
	long	value;			// Whassit worth?
	SHORT	strength;		// How strong is it?
	SHORT	damage;			// Amount of damage
	long	descrip;		// ptr to descrp in file
	SHORT	flags;			// State flags
};

SDEF _TT_ENT {			// TT Entry
	SHORT	verb;			// Verb no.
	SHORT	condition;		// Condition
	SHORT	action;			// #>0=action, #<0=room
	long	*pptr;			// Param ptr. -1 = none
};

SDEF _MOB_ENT {			// Mobile Character Entry
	char	id[IDL+1];		// Name of mobile
	char	dead;			// State flags
	char	speed,travel,fight,act,wait;	// speed & %ages
	char	fear,attack;		// others
	char	flags;			// (padding really)
	SHORT	dmove;
	SHORT	hitpower;		// Hit power
	long	arr,dep,flee,hit,miss,death;	// Various UMsgs
};

SDEF _MOB_TAB {			// Mobile table entry
	SHORT	obj;			// Object no.
	SHORT	speed;			// Secs/turn
	SHORT	count;			// Time till next
	USHORT	pflags;			// Player style flags
};

SDEF _DAEMON  {			// AMan Daemon structure
	_DAEMON	*nxt,*prv;	// Structured list info
	char	own,pad;	// Who own's the daemon - -1 = GLOBAL
	SHORT	count,num;	// Count and "verb" number
	long	val[2],typ[2];	// Noun1 and Noun2
};

