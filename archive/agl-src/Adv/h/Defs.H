//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Defs.H		Standard AGL #defines
//
//	LMod: oliver 20/06/93	Added MTLIMIT for user time limiting
//	      oliver 19/06/93	Articles and EXTEND
//	      oliver 11/06/93	AMUL->AGL. MAXNODE->32
//

#ifndef	TRUE				// If we need TRUE/FALSE defined
#define	TRUE	1
#define	FALSE	0
#endif

#define	MAXNODE	32			// Total no. of nodes
#define	MAXU	30			// Maximum USER nodes

#define	YES	0
#define	NO	1

#define	LOUD	1
#define	QUIET	2

#define	ACTION	0
#define	NOISE	1
#define	EVENT	2
#define	TEXTS	3

#define	IDL		12		// Length of ID strings
#define	RANKL		32		// Length of rank descs
#define	NAMEL		20		// Length of names
#define	ALWAYSEP	"---"		// Always Endparse
#define	PGOT		5		// Start of 'owned' obs
#define	INS		( MAXU+10 )	// Start of insides!
#define	OWLIM		2048		// OutPut Buffer Size

			// Modes
#define	OFFLINE		0
#define	LOGGING		1
#define	PLAYING		2
#define	CHATTING	3

#define	am_USER	0			// This AGL is for a user
#define	am_DAEM 1			// the daemon processor
#define	am_MOBS	2			// Am the mobiles!

			// IO Support types
#define	CLIWINDOW	0	// Def	// Use CLI window
#define	CUSSCREEN	CLIWINDOW+1	// Provide Custm Screen
#define	SERIO		CUSSCREEN+1	// Serial IO
#define	DSERIO		SERIO+1		// Direct Serial IO
#define	LOGFILE		99		// Output ONLY to log

			// Room bit-flags
#define	DMOVE	 1		// When players die, move rooms to.
#define	STARTL	 2		// Players can start from this room
#define	RANDOB	 4		// Random objects can start here..
#define	DARK	 8		// Room has no lighting
#define	SMALL	 16		// Only 1 player at a time
#define	DEATH	 32		// Players die after reading descrip
#define	NOLOOK	 64		// Cannot look into this room
#define	SILENT	 128		// Cannot hear outside noises
#define	HIDE	 256		// Players cannot be seen from outside
#define	SANCTRY	 512		// Score points for dropped objects
#define	HIDEWY	 1024		// Objects in here cannot be seen
#define	PEACEFUL 2048		// No fighting allowed here
#define	NOEXITS	 4096		// Can't list exits
#define	ANTERM	 8192		// Ante Room (Pre-Start location)
#define	NOGORM	16384		// Can't random go to this room!

			// Object flag bits
#define	OF_OPENS	1	// Object is openable
#define	OF_SCENERY	2	// Object is scenery
#define	OF_COUNTER	4	// Ignore me!
#define	OF_FLAMABLE	8	// Can we set fire to it?
#define	OF_SHINES	16	// Can it provide light?
#define	OF_SHOWFIRE	32	// Say 'The <noun> is on fire' when lit
#define	OF_INVIS	64	// Object is invisible
#define	OF_SMELL	128	// Object has a smell not visual
#define	OF_ZONKED	32768	// Object was zonked!

			// Object parameter flag no.'s
#define	OP_ART		0x0001
#define	OP_ADJ		0x0002
#define	OP_START	0x0004
#define	OP_HOLDS	0x0008
#define	OP_PUT		0x0010
#define	OP_MOB		0x0020

			// Object/state flags
#define	SF_LIT		1	// Object is lumious
#define	SF_OPEN		2	// Object is open
#define	SF_CLOSED	4	// Object is closed
#define	SF_WEAPON	8	// Its a weapon
#define	SF_OPAQUE	16	// Can see inside object
#define	SF_SCALED	32	// Scale the value
#define	SF_ALIVE	64	// Mobile/Animated

			// 'put to' flags
#define	PUT_IN		0
#define	PUT_ON		1
#define	PUT_BEHIND	2
#define	PUT_UNDER	3

			// Articles (incase they're important)
#define	ART_A		0
#define	ART_AN		1
#define	ART_THE		2
#define	ART_SOME	3

			// verb flags!
#define	VB_TRAVEL	1	// travel verb
#define	VB_DREAM	2	// can be done while asleep

			// Actions
#define	AQUIT		0	// Quit action
#define	ASAVE		1	// Save players details
#define	ASCORE		2	// Show players status
#define	ASETSTAT	3	// Set object state
#define	ALOOK		4	// Look at this room
#define	AWHAT		5	// List room inventory
#define	AWHERE		6	// List where objs are
#define	AWHO		7	// List who is playing
#define	ATREATAS	8	// Process as 'verb'
#define	APRINT		9	// Display text
#define	ASKIP		10	// Skip next x entries
#define	AENDPARSE	11	// Stop parsing
#define	AKILLME		12	// Kill player
#define	AFAILPARSE	13	// EndParse + Fail
#define	AFINISHPARSE	14
#define	AABORTPARSE	15
#define	AWAIT		16	// Waits for n seconds
#define	AHIT		17	// Single combat turn
#define	AWHEREAMI	18	// Function for 'where'
#define	ASEND		19	// Send object...
#define	AANOUN		20	// Make anouncement
#define	AGET		21	// Pick something up
#define	ADROP		22	// Drop it
#define	AINVENT		23	// List objects carried
#define	ARANDGO		24	// Send player to random room
#define	ACHANGESEX	25	// Change players sex
#define	APROVOKE	26	// Provoke mobile to take turn
#define	ABLAST		27	// "Blast" object
#define	ASIT		28	// Makes player Sit down
#define	ASTAND		29	// Makes player stand up
#define	ALIE		30	// Makes player lie down
#define	ARDMODE		31	// Set RD Mode
#define	ARESET		32	// Reset the game
#define	AACTION		33	// Like anounce
#define	AMOVE		34	// Move player quietly
#define	ATRAVEL		35	// Process tt instead
#define	AMSGIN		36	// Announce to a room
#define	AACTIN		37	// Action to a room
#define	AMSGFROM	38	// Annouce via object
#define	AACTFROM	39	// Action via object
#define	ATELL		40	// Tell someone summats
#define	APUT		41	// Put object into another
#define	AGIVE		42	// Give object to user
#define	AINFLICT	43	// Cast spell
#define	ACURE		44	// Remove spell
#define	ASUMMON		45	// Summon Player
#define	AADD		46	// Add stats to player
#define	ASUB		47	// Minus stats to player
#define	ACHECKNEAR	48	// 'near' processing
#define	ACHECKGET	49	// 'get' checking
#define	ADESTROY	50	// Destroy object
#define	ARECOVER	51	// Recover object
#define	ASTART		52	// Start a daemon
#define	ACANCEL		53	// Cancel a daemon
#define	ABEGIN		54	// Force daemon to start
#define	ASHOWTIMER	55	// Displays time left
#define	AOBJAN		56	// Announce from obj
#define	AOBJACT		57	// Action from object
#define	ACONTENTS	58	// Show obj contents
#define	AFORCE		59	// Force em!
#define	AHELP		60	// Help someone!
#define	ASTOPHELP	61	// Stop helping someone
#define	AFIX		62	// Fixes players stat
#define	AOBJINVIS	63	// Turns an obj. invis
#define	AOBJSHOW	64	// Displays an invis obj
#define	AFIGHT		65	// Start a fight
#define	AFLEE		66	// The chickens way out
#define	ALOG		67	// Text to the LOG file
#define	ACOMBAT		68	// Combat action etc.
#define	AWIELD		69	// Use a wpn in combat
#define	AFOLLOW		70	// Follow someone!
#define	ALOSE		71	// Lose your tail.
#define	ASTOPFOLLOW	72	// Stop following
#define	AEXITS		73	// Shows exits to a room
#define	ATASK		74	// Sets the tasks done
#define	ASHOWTASK	75	// Shows the tasks done
#define	ASYNTAX		76	// Set slots
#define	ASETPRE		77	// Set pre-rank desc
#define	ASETPOST	78	// Set post-rank desc
#define	ASENDDAEMON	79	// Send a daemon
#define	ADO		80	// GOSUB
#define	AINTERACT	81	// Set actor
#define	AAUTOEXITS	82	// Auto-exits
#define	ASETARR		83	// Set arrived string
#define	ASETDEP		84	// Set departed string
#define	ARESPOND	85	// Reply, endparse
#define	AERROR		86	// Give an error!
#define	ABURN		87	// Ignite an object
#define	ADOUSE		88	// Put it out again
#define	AINC		89	// Increment state
#define	ADEC		90	// Decrement
#define	ATOPRANK	91	// Make player toprank
#define	ADEDUCT		92	// Reduce score by %
#define	ADAMAGE		93	// Damage object
#define	AREPAIR		94	// Repair object
#define	AGSTART		95	// Start global daemon
#define	AEXTEND		96	// Extend game

			// Conditions
#define	CAND		0	// And then...
#define	CSTAR		1	// Same as always
#define	CELSE		2	// If last was false
#define	CALWAYS		3	// Always do this
#define	CLIGHT		4	// If there is light...
#define	CISHERE		5	// If obj is here
#define	CMYRANK		6	// If my rank is...
#define	CSTATE		7	// If state of obj
#define	CMYSEX		8	// My sex is...
#define	CLASTVB		9	// If last verb was..
#define	CLASTDIR	10	// If last TRAVEL verb
#define	CLASTROOM	11	// If last room was
#define	CASLEEP		12	// Is player sleeping
#define	CSITTING	13	// Is player sitting
#define	CLYING		14	// Is player lying down
#define	CRAND		15	// If rand(n1) eq n2
#define	CRDMODE		16	// If rdmode is...
#define	CONLYUSER	17	// If only player
#define	CALONE		18	// If only person here
#define	CINROOM		19	// If player in room
#define	COPENS		20	// If object opens
#define	CGOTNOWT	21	// If carrying nothing
#define	CCARRYING	22	// Carrying object?
#define	CNEARTO		23	// Is it here SOMEWHERE
#define	CHIDDEN		24	// Can others see me?
#define	CCANGIVE	25	// Can player manage it
#define	CINFL		26	// Same as below
#define	CINFLICTED	27	// Is played inflicted
#define	CSAMEROOM	28	// Same room as player?
#define	CSOMEONEHAS	29	// If obj is carried?
#define	CTOPRANK	30	// If u'r the top rank
#define	CGOTA		31	// Carrying obj in stat
#define	CACTIVE		32	// Is daemon active?
#define	CTIMER		33	// Check time left
#define	CBURNS		34	// If object burns
#define	CCONTAINER	35	// If its a container
#define	CEMPTY		36	// If object is empty
#define	COBJSIN		37	// Check # of contents
#define	CALTEP		38	// Always .., endparse
#define	CANTEP		39	// And    .., endparse
#define	CHELPING	40	// Are we helping him?
#define	CGOTHELP	41	// If we've got help
#define	CANYHELP	42	// Helping ANYONE?
#define	CELTEP		43	// Else   ..., endparse
#define	CSTAT		44	// If attrib <> no.
#define	COBJINV		45	// If object invisible
#define	CFIGHTING	46	// Is player fighting?
#define	CTASKSET	47	// Has task been done?
#define	CCANSEE		48	// Can I see <player>
#define	CVISIBLETO	49	// Am I visible to
#define	CNOUN1		50	// Match noun1
#define	CNOUN2		51	// Match noun2
#define	CAUTOEXITS	52	// Auto exits on?
#define	CDEBUG		53	// Debug mode on?
#define	CFULL		54	// Stat at full?
#define	CTIME		55	// Time remaining?
#define	CDEC		56	// Decrement & test
#define	CINC		57	// Increment & test
#define	CLIT		58	// Is object lit?
#define	CFIRE		59	// Is object flamable?
#define	CHEALTH		60	// Is players health %?
#define	CMAGIC		61	// Can magic be done?
#define	CSPELL		62	// Can spell be done?
#define	CIN		63	// IN <ROOM> <NOUN>
#define	CEXISTS		64	// Does object exist?
#define	CWILLGO		65	// Will it go in?

			// Paramter types
#define	PREAL		-70	// Noun or slot label
#define	PNOUN		1	// Must be a noun
#define	PADJ		2	// Must be an adj
#define	PPLAYER		3	// Must be a player
#define	PROOM		4	// Must be a room
#define	PUMSG		6	// Must be text
#define	PVERB		7	// Must be a verb
#define	PCLASS		8	// Must be a class
#define	PNUM		9	// Must be a number
#define	PRFLAG		11	// Must be a room flag
#define	POFLAG		12	// Must be an obj flag
#define	PSFLAG		13	// Must be a stat flag
#define	PSEX		14	// Must be a gender
#define	PDAEMON		15	// Must be a daemon ID
#define	PMOBILE		16	// Must be a mobile

#define	WNONE		-1	// None!
#define	WANY		0	// Anything!
#define	WNOUN		1	// Word is a noun
#define	WADJ		2	// Word is an adjective
#define	WPLAYER		3	// Its a player
#define	WROOM		4	// Its a room ID
#define	WSYN		5	// Its a synonym
#define	WTEXT		6	// Its text
#define	WVERB		7	// Its a verb!
#define	WCLASS		8	// Class name
#define	WNUMBER		9	// A number

#define	SGLOW		1
#define	SINVIS		2
#define	SBLIND		3
#define	SCRIPPLE	4
#define	SDEAF		5
#define	SDUMB		6
#define	SSLEEP		7
#define	SSINVIS		8


#define	STSCORE		1
#define	STSTR		2
#define	STSTAM		3
#define	STDEX		4
#define	STWIS		5
#define	STEXP		6
#define	STMAGIC		7
#define	STSCTG		8
#define	STINFIGHT	1024	// Flag - in a fight

			// Anouncement types
#define	AGLOBAL		0
#define	AEVERY1		1
#define	AOUTSIDE	2
#define	AHERE		3
#define	AOTHERS		4
#define	AALL		5
#define	ACANSEE		6
#define	ANOTSEE		7

			// Message Types
#define	MTIMER		0	// 1s timer event
#define	MKILL		1	// Close down
#define	MCNCT		2	// Connection
#define	MDISCNCT	3	// Disconnect
#define	MDATAREQ	4	// Gets ptrs!
#define	MLOGGED		5	// Logged in!
#define	MMESSAGE	6	// Sent a msg
#define	MCLOSEING	7	// Closeing..
#define	MRESET		8	// Reset
#define	MLOCK		9	// User Lock
#define	MUNLOCK		10	// Unlock it
#define	MSUMMONED	11	// COME HERE!
#define	MDIE		12	// Ciao!!
#define	MBUSY		13	// I'm busy
#define	MFREE		14	// I'm free!
#define	MEXECUTE	15	// Execute a command!
#define	MDSTART		16	// Daemon start
#define	MDCANCEL	17	// Cancel a daemon
#define	MDAEMON		18	// We have lift off!
#define	MCHECKD		19	// Get daemon status
#define	MFORCE		20	// Do it, buddo!
#define	MMADEWIZ	21	// Reached top rank!
#define	MLOG		22	// Write log entry
#define	MRWARN		23	// Reset Warning
#define	MEXTEND		24	// Extend game
#define	MGDSTART	25	// Start global daemon
#define	MSWAP		26	// Signal 'swap next game'
#define	MMOBILE		27	// Process mobile
#define	MMOBFRZ		28	// Freeze mobiles
#define	MMOBTHAW	29	// Thaw mobiles
#define	MPROVOKE	30	// Provoke Mobile
#define	MKILLED		31	// You've killed me
#define	MTLIMIT		32	// Set user time limit

// BIOD M types

#define	MPRINT		80	// Print Text String / Reply
#define	MINPUT		81	// Allow input
#define	MHANGUP		82	// Disconnect

			// Player flags
#define	PFINVIS		0x00001
#define	PFGLOW		0x00002
#define	PFBLIND		0x00004
#define	PFDUMB		0x00008
#define	PFDEAF		0x00010
#define	PFCRIP		0x00020	// Can't move
#define	PFDYING		0x00040	// NOT USED - dying
#define	PFLIMP		0x00080	// NOT USED - Limping
#define	PFASLEEP	0x00100
#define	PFSITTING	0x00200
#define	PFLYING		0x00400	// Lying Down
#define	PFFIGHT		0x00800
#define	PFATTACKER	0x01000	// started the fight
#define	PFMOVING	0x02000	// 'in transit'
#define	PFSINVIS	0x04000
#define	PFWORKING	0x08000	// Using Player Data

#define	RDRC		0	// RC Mode
#define	RDVB		1	// Verbose mode
#define	RDBF		2	// Brief mode

#define	TYPEV		0	// Brief mode
#define	TYPEB		1	// Verbose mode

#define	At	agl->type
#define	Am	agl->msg
#define	Af	agl->from
#define	Ad	agl->data
#define	Ap	agl->ptr
#define	Ap1	agl->p1
#define	Ap2	agl->p2
#define	Ap3	agl->p3
#define	Ap4	agl->p4

#define	AMt	amanp->type
#define	AMm	amanp->msg
#define	AMf	amanp->from
#define	AMd	amanp->data
#define	AMp	amanp->ptr
#define	Apx1	amanp->p1
#define	Apx2	amanp->p2
#define	Apx3	amanp->p3
#define	Apx4	amanp->p4

#define	IAt	intam->type
#define	IAm	intam->msg
#define	IAf	intam->from
#define	IAd	intam->data
#define	IAp	intam->ptr

				// User Flags

#define	ufANSI	0x001		// ANSI bit
#define	ufCRLF	0x002		// Add LineFeed
#define	ufNVER	0x080		// New Version

#define	DLLEN	80		// Default line length
#define	DSLEN	24		// Default screen length
#define	DFLAGS	ufCRLF		// Default = cr/lf ON & auto-redo on

				// Useful defines

#define	isOINVIS(x)	((obtab+x)->flags & OF_INVIS)
#define	isPINVIS(x)	((lstat+x)->flags & PFINVIS)
#define	isPSINVIS(x)	((lstat+x)->flags & PFSINVIS)
#define	IamINVIS	(me2->flags & PFINVIS)
#define	IamSINVIS	(me2->flags & PFSINVIS)
#define	pNAME(x)	(usr+x)->name
#define	pROOM(x)	((lstat+x)->room)
#define	pRANK(x)	((usr+x)->rank)
#define	pLIGHT(x)	(lstat+x)->light
#define	pFLAGS(x)	((lstat+x)->flags)
#define	pFLAG(x,y)	(pFLAGS(x) & y)
#define	pHADLIGHT(x)	(lstat+x)->hadlight
#define	myRANK		me->rank
#define	mySCORE		me->score
#define	myROOM		me2->room
#define	myNAME		me->name
#define	myLIGHT		me2->light
#define	myHLIGHT	me2->hadlight
#define	hisRANK		you->rank
#define	hisSCORE	you->score
#define	hisROOM		you2->room
#define	hisNAME		you->name
#define	hisLIGHT	you2->light
#define	hisHLIGHT	you2->hadlight
#define	LightHere	lit(myROOM)
#define	unfreeze	Permit(); return

#include <adv:h/Msgs.H>		// System message defines
#include <adv:h/Acts.H>

#define	acp	(char *)actptr

#define	CP1	actual(*(tt.pptr))
#define	CP2	actual(*(tt.pptr+1))
#define	CP3	actual(*(tt.pptr+2))
#define	CP4	actual(*(tt.pptr+3))
#define	TP1	actual(*(tt.pptr+ncop[tt.condition]))
#define	TP2	actual(*(tt.pptr+ncop[tt.condition]+1))
#define	TP3	actual(*(tt.pptr+ncop[tt.condition]+2))
#define	TP4	actual(*(tt.pptr+ncop[tt.condition]+3))
#define	AP1	acp(*(tt.pptr+ncop[tt.condition]))
#define	AP2	acp(*(tt.pptr+ncop[tt.condition]+1))
#define	AP3	acp(*(tt.pptr+ncop[tt.condition]+2))
#define	AP4	acp(*(tt.pptr+ncop[tt.condition]+3))
#define	tP1	actual(*(tpt))
#define	tP2	actual(*(tpt+1))
#define	tP3	actual(*(tpt+2))
#define	tP4	actual(*(tpt+3))
#define	aP1	acp(*(tpt))
#define	aP2	acp(*(tpt+1))
#define	aP3	acp(*(tpt+2))
#define	aP4	acp(*(tpt+3))
#define	STATE	(objtab->states+(long)objtab->state)
#define	State(i) ((obtab+i)->states+(long)(obtab+i)->state)
#define	ItsState(it) (it->states+(long)it->state)
#define	STDMEM	(MEMF_PUBLIC+MEMF_CLEAR)
#define	SDEF	typedef struct

#define	DED	1
#define	DEDDED	2
