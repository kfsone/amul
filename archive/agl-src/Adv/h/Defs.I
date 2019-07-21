**
**	Defs.I		Version 1, Revision 2
**
**	Based on Defs.H
**

	IFND	AGL_DEFS_I
AGL_DEFS_I	SET	1

; Various General Defines

MAXNODE	equ	32		; Max. nodes
MAXU	equ	MAXNODE-2	; Max users at one time

YES	equ	0
NO	equ	1

LOUD	equ	1
QUIET	equ	2

ACTION	equ	0
NOISE	equ	1
EVENT	equ	2
TEXTS	equ	3

IDL	equ	12		; Length of ID strings
RANKL	equ	32		; Length of rank descriptions
NAMEL	equ	20		; Length of names
INS	equ	MAXU+10		; Containers start here
OWLIM	equ	2048		; Output Buffer Limits

DED	equ	1		; Player killed
DEDDED	equ	2		; Player killed & erased

; Line States:

OFFLINE	equ	0
LOGGING	equ	1
PLAYING	equ	2
CHATTING	equ	3

; AGL Types

am_USER	equ	0		; User AGL
am_DAEM	equ	1		; Daemon Processor
am_MOBS	equ	2		; Mobile Processor

; IO Support Types

CLIWINDOW	equ	0		; Default - CLI Window
CUSSCREEN	equ	1		; Local/Custom screen
SERIO	equ	2		; Serial Port
DSERIO	equ	3		; Serial Port - No Carrier Checking
LOGFILE	equ	99		; Write to logfile

; Player Flags

PFINVIS	equ	$0001
PFGLOW	equ	$0002
PFBLIND	equ	$0004
PFDUMB	equ	$0008		; Can't speak
PFDEAF	equ	$0010
PFCRIP	equ	$0020		; Crippled
PFDYING	equ	$0040		; NOT USED YET - Player Dying!?!
PFLIMB	equ	$0080		; NOT USED YET - Player limping!
PFASLEEP	equ	$0100
PFSITTING	equ	$0200
PFLYING	equ	$0400		; Lying down
PFFIGHT	equ	$0800
PFATTACKER equ	$1000		; Player started the fight
PFMOVING	equ	$2000		; LOCK - Player In Transit
PFSINVIS	equ	$4000		; Super (Completely) Invisible

RDRC	equ	0		; Describe rooms in RC mode
RDVB	equ	1		; Verbose mode
RDBF	equ	2		; Brief mode

TYPEV	equ	0		; Brief
TYPEB	equ	1		; Verbose

; User Flags

ufANSI	equ	$0001		; ANSI mode
ufCRLF	equ	$0002		; Follow CR with LF
ufARDO	equ	$0004		; NOT USED YET - AutoRedo mode
ufNVER	equ	$0080		; New Version toggle

DLLEN	equ	80		; Def. line length
DSLEN	equ	24		; Def. screen length
DRCHAR	equ	'|'		; Def. refresh-key
DFLAGS	equ	ufCRLF		; Def. flags.

; Room Bit-Flags

DMOVE	equ	$0001		; Move objects when player dies
STARTL	equ	$0002		; Start Location
RANDOB	equ	$0004		; Random objects can start here
DARK	equ	$0008		; Room has no light source
SMALL	equ	$0010		; Only large enough for one
DEATH	equ	$0020		; Kills non-toprank players here
NOLOOK	equ	$0040		; Can't look into this room
SILENT	equ	$0080		; Can't hear noise from outside
HIDE	equ	$0100		; Can't see other players here
SANCTRY	equ	$0200		; Score points for dropped objects
HIDEWY	equ	$0400		; Cannot see objects here
PEACEFUL	equ	$0800		; No fighting
NOEXITS	equ	$1000		; Can't see exits
ANTERM	equ	$2000		; Pre-start location (AnteRoom)
NOGORM	equ	$4000		; Can't RandomGo to here.

; Object Flag Bits

OF_OPENS	  equ	$0001		; Openable
OF_SCENERY  equ	$0002		; Scenery (can't move it)
OF_COUNTER  equ	$0004		; Used as a variable - ignore me!
OF_FLAMBLE  equ	$0008		; Can be FIREd
OF_SHINES	  equ	$0010		; Light source
OF_SHOWFIRE equ	$0020		; Describe as 'On fire' when lit.
OF_INVIS	  equ	$0040		; Invisible
OF_SMELL	  equ	$0080		; Not a visible object - a smell!
OF_ZONKED	  equ	$8000		; Out of play

; Object Parameter Flag No.s

OP_ART	equ	$01		; art=
OP_ADJ	equ	$02		; adj=
OP_START	equ	$04		; start=
OP_HOLDS	equ	$08		; holds=
OP_PUT	equ	$10		; put=
OP_MOB	equ	$20		; mobile=

; Object State Flags

SF_LIT	equ	$01		; On fire / luminous
SF_OPEN	equ	$02		; Open	} There is good reason
SF_CLOSED	equ	$04		; Closed	} for using two flags!
SF_WEAPON	equ	$08		; Can be used to fight
SF_OPAQUE	equ	$10		; Can see inside
SF_SCALED	equ	$20		; Always scale value
SF_ALIVE	equ	$40		; Mobile is active

; 'Put To' Flags

PUT_IN	equ	0
PUT_ON	equ	1
PUT_BEHIND equ	2
PUT_UNDER	equ	3

; Verb Flags

VB_TRAVEL	equ	1		; Travel verb
VB_DREAM	equ	2		; Can be executed whilst asleep

; Spell Flags

SGLOW	equ	1
SINVIS	equ	2
SBLIND	equ	3
SCRIPPLE	equ	4
SDEAF	equ	5
SDUMB	equ	6
SSLEEP	equ	7
SSINVIS	equ	8

; Play Statistics Field No.s

STSCORE	equ	1
STSTR	equ	2
STSTAM	equ	3
STDEX	equ	4
STWIS	equ	5
STEXP	equ	6
STMAGIC	equ	7
STSCTG	equ	8		; Points scored this game
STINFIGHT	equ	1024		; Flag - in a fight

; Action/Announce Types

AGLOBAL	equ	0		; Perhaps this should be bit flags
AEVERY1	equ	1		; allowing combinations?
AOUTSIDE	equ	2
AHERE	equ	3
AOTHERS	equ	4
AALL	equ	5
ACANSEE	equ	6
ANOTESEE	equ	7

; AMan Message Types (For AP_TYPE field)

				; Direction. <=Write, >=Read
MTIMER	equ	 0		;  >  1s timer event from timer task
MKILL	equ	 1		; <  Shutdown
MCNCT	equ	 2		; <  Connection request
MDISCNCT	equ	 3		; <  Disconnection
MDATAREQ	equ	 4		; <  Request for data
MLOGGED	equ	 5		; <  User Logged In
MMESSAGE	equ	 6		; <> Text Message
MCLOSEING	equ	 7		;  > AMan is shutting down
MRESET	equ	 8		;  > Reset starting
MLOCK	equ	 9		; <  Request lock
MUNLOCK	equ	10		; <  Unlock line
MSUMMONED	equ	11		;  > You have been summoned
MDIE	equ	12		; <> You have been killed
MBUSY	equ	13		; <  IO Busy
MFREE	equ	14		; <  IO Finished
MEXECUTE	equ	15		; <> Execute this here command
MDSTART	equ	16		; <  Start Daemon
MDCANCEL	equ	17		; <  Cancel daemon
MDAEMON	equ	18		; <> Execute daemon
MCHECKD	equ	19		; <  Check & return daemon status
MFORCE	equ	20		; <> Do it, bud.
MMADEWIZ	equ	21		; <  Tell AMan I reached TopRank
MLOG	equ	22		; <  Write to Log
MRWARN	equ	23		;  > Reset Warning
MEXTEND	equ	24		; <  Request game extension
MGDSTART	equ	25		; <  Start Global Daemon
MSWAP	equ	26		; <  Next Game Is...
MMOBILE	equ	27		;  > Process Mobile
MMOBFRZ	equ	28		; <  Freeze Mobiles
MMOBTHAW	equ	29		; <  UnFreeze Mobiles
MPROVOKE	equ	30		; <  Provoke mobile (execute now)
MKILLED	equ	31		;  > You've killed me!
MTLIMIT	equ	32		; <  Set max. user time limit

	ENDC	; AGL_DEFS_I

