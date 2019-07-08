#ifndef H_AMUL_DEFS_H
#define H_AMUL_DEFS_H 1
/*
              ****    AMUL.DEFS.H.....Adventure Compiler    ****
              ****                #defines!                 ****
									   */

#define	MAXU	14				/* Max users at 1 time		*/
#define	MAXNODE	(MAXU+2)			/* + 1 mobiles+1 daemons	*/

#define	YES	0
#define	NO	1

#define	LOUD	1
#define	QUIET	2

#define	ACTION	0
#define	NOISE	1
#define	EVENT	2
#define	TEXTS	3

	/* Get AMUL, AMAN and AMULCOM versions from relevant .H */
#define	PV		"0.99d"			/* Parser version	*/
#define	IDL		12			/* Length of ID strings	*/
#define	RANKL		32			/* Length of rank descs	*/
#define	NAMEL		20			/* Length of names	*/
#define	ALWAYSEP	"---"			/* Always Endparse	*/
#define	INS		( MAXU+10 )		/* Start of insides!	*/

			/*-	Modes	  -*/
#define	OFFLINE		0
#define	LOGGING		1
#define	PLAYING		2

#define	am_USER	0			/* This AMUL is for a user	*/
#define	am_DAEM 1			/* the daemon processor		*/
#define	am_MOBS	2			/* Am the mobiles!		*/

			/* IO Support types */
#define	CLIWINDOW	0	/* Def */	/* Use CLI window	*/
#define	CUSSCREEN	CLIWINDOW+1		/* Provide Custm Screen	*/
#define	SERIO		CUSSCREEN+1		/* Serial IO		*/
#define	LOGFILE		99			/* Output ONLY to log	*/

			/* Room bit-flags */
#define	DMOVE	 1		/* When players die, move rooms to...	*/
#define	STARTL	 2		/* Players can start from this room	*/
#define	RANDOB	 4		/* Random objects can start here..	*/
#define	DARK	 8		/* Room has no lighting			*/
#define	SMALL	 16		/* Only 1 player at a time		*/
#define	DEATH	 32		/* Players die after reading descrip	*/
#define	NOLOOK	 64		/* Cannot look into this room		*/
#define	SILENT	 128		/* Cannot hear outside noises		*/
#define	HIDE	 256		/* Players cannot be seen from outside	*/
#define	SANCTRY	 512		/* Score points for dropped objects	*/
#define	HIDEWY	 1024		/* Objects in here cannot be seen	*/
#define	PEACEFUL 2048		/* No fighting allowed here		*/
#define NOEXITS  4096           /* Can't list exits                     */

			/* Object flag bits */
#define	OF_OPENS	1	/* Object is openable			*/
#define	OF_SCENERY	2	/* Object is scenery			*/
#define	OF_COUNTER	4	/* Ignore me!				*/
#define	OF_FLAMABLE	8	/* Can we set fire to it?		*/
#define	OF_SHINES	16	/* Can it provide light?		*/
#define	OF_SHOWFIRE	32	/* Say 'The <noun> is on fire' when lit */
#define OF_INVIS	64	/* Object is invisible			*/
#define	OF_SMELL	128	/* Object has a smell not visual	*/
#define	OF_ZONKED	32768	/* Object was zonked!			*/

			/* Object parameter flag no.'s */
#define	OP_ADJ		1	/* ADJ=?	*/
#define	OP_START	2	/* START=?	*/
#define	OP_HOLDS	4	/* HOLDS=?	*/
#define	OP_PUT		8	/* PUT=?	*/
#define	OP_MOB		16	/* Mobile=	*/

			/* Object/state flags */
#define	SF_LIT		1	/* Object is lumious		*/
#define	SF_OPEN		2	/* Object is open		*/
#define	SF_CLOSED	4	/* Object is closed 		*/
#define	SF_WEAPON	8	/* Its a weapon			*/
#define	SF_OPAQUE	16	/* Can see inside object	*/
#define	SF_SCALED	32	/* Scale the value		*/
#define	SF_ALIVE	64	/* Mobile/Animated		*/

			/* 'put to' flags */
#define	PUT_IN		0	/* put in	*/
#define	PUT_ON		1	/* on...	*/
#define	PUT_BEHIND	2	/* behind...	*/
#define	PUT_UNDER	3	/* under!	*/

			/* verb flags! */
#define	VB_TRAVEL	1	/* Verb is travel verb			*/
#define	VB_DREAM	2	/* Verb can be executed whilst sleeping	*/

#define	AQUIT		0		/* Quit action		*/
#define	ASAVE		1		/* Save players details	*/
#define	ASCORE		2		/* Show players status	*/
#define	ASETSTAT	3		/* Set object state	*/
#define	ALOOK		4		/* Look at this room	*/
#define	AWHAT		5		/* List room inventory	*/
#define	AWHERE		6		/* List where objs are	*/
#define	AWHO		7		/* List who is playing	*/
#define	ATREATAS	8		/* Process as 'verb'	*/
#define	AMESSAGE	9		/* Send message to me	*/
#define	ASKIP		10		/* Skip next x entries	*/
#define	AENDPARSE	11		/* Stop parsing		*/
#define	AKILLME		12		/* Kill player		*/
#define	AFAILPARSE	13		/* EndParse + Fail	*/
#define	AFINISHPARSE	14
#define	AABORTPARSE	15
#define	AWAIT		16		/* Waits for n seconds	*/
#define	ABLEEP		17		/* Prints '.' n times	*/
#define	AWHEREAMI	18		/* Function for 'where'	*/
#define	ASEND		19		/* Send object...	*/
#define	AANOUN		20		/* Make anouncement	*/
#define	AGET		21		/* Pick something up	*/
#define	ADROP		22		/* Drop it		*/
#define	AINVENT		23		/* List objects carried	*/
#define	AREPLY		24		/* Same as message	*/
#define	ACHANGESEX	25		/* Change players sex   */
#define	ASLEEP		ACHANGESEX+1	/* Put player to sleep  */
#define	AWAKE		ASLEEP+1	/* Wake player up       */
#define	ASIT		AWAKE+1		/* Makes player Sit down*/
#define	ASTAND		ASIT+1		/* Makes player stand up*/
#define	ALIE		ASTAND+1	/* Makes player lie down*/
#define	ARDMODE		ALIE+1		/* Set RD Mode		*/
#define	ARESET		ARDMODE+1	/* Reset the game	*/
#define	AACTION		ARESET+1	/* Like anounce		*/
#define	AMOVE		AACTION+1	/* Move player quietly	*/
#define	ATRAVEL		AMOVE+1		/* Process tt instead	*/
#define	AMSGIN		ATRAVEL+1	/* Announce to a room	*/
#define	AACTIN		AMSGIN+1	/* Action to a room	*/
#define	AMSGFROM	AACTIN+1	/* Annouce via object	*/
#define	AACTFROM	AMSGFROM+1	/* Action via object	*/
#define	ATELL		AACTFROM+1	/* Tell someone summats	*/
#define	AADDVAL		ATELL+1		/* Add object value	*/
#define	AGIVE		AADDVAL+1	/* Give object to user	*/
#define	AINFLICT	AGIVE+1		/* Cast spell		*/
#define	ACURE		AINFLICT+1	/* Remove spell		*/
#define	ASUMMON		ACURE+1		/* Summon Player	*/
#define	AADD		ASUMMON+1	/* Add stats to player	*/
#define	ASUB		AADD+1		/* Minus stats to player*/
#define	ACHECKNEAR	ASUB+1		/* 'near' processing	*/
#define	ACHECKGET	ACHECKNEAR+1	/* 'get' checking	*/
#define	ADESTROY	ACHECKGET+1	/* Destroy object	*/
#define	ARECOVER	ADESTROY+1	/* Recover object	*/
#define	ASTART		ARECOVER+1	/* Start a daemon	*/
#define	ACANCEL		ASTART+1	/* Cancel a daemon	*/
#define	ABEGIN		ACANCEL+1	/* Force daemon to start*/
#define	ASHOWTIMER	ABEGIN+1	/* Displays time left	*/
#define	AOBJAN		ASHOWTIMER+1	/* Announce from obj	*/
#define	AOBJACT		AOBJAN+1	/* Action from object	*/
#define	ACONTENTS	AOBJACT+1	/* Show obj contents	*/
#define	AFORCE		ACONTENTS+1	/* Force em!		*/
#define	AHELP		AFORCE+1	/* Help someone!	*/
#define	ASTOPHELP	AHELP+1		/* Stop helping someone */
#define	AFIX		ASTOPHELP+1	/* Fixes players stat	*/
#define AOBJINVIS	AFIX+1		/* Turns an obj. invis	*/
#define AOBJSHOW	AOBJINVIS+1	/* Displays an invis obj*/
#define AFIGHT		AOBJSHOW+1	/* Start a fight	*/
#define AFLEE		AFIGHT+1	/* The chickens way out */
#define ALOG		AFLEE+1		/* Text to the LOG file */
#define ACOMBAT		ALOG+1		/* Combat action etc.	*/
#define AWIELD		ACOMBAT+1	/* Use a wpn in combat	*/
#define AFOLLOW		AWIELD+1	/* Follow someone!	*/
#define ALOSE		AFOLLOW+1	/* Lose your tail.	*/
#define ASTOPFOLLOW	ALOSE+1		/* Stop following	*/
#define AEXITS		ASTOPFOLLOW+1	/* Shows exits to a room*/
#define ATASK		AEXITS+1	/* Sets the tasks done  */
#define ASHOWTASK	ATASK+1		/* Shows the tasks done */
#define	ASYNTAX		ASHOWTASK+1	/* Set slots		*/
#define	ASETPRE		ASYNTAX+1	/* Set pre-rank desc	*/
#define	ASETPOST	ASETPRE+1	/* Set post-rank desc	*/
#define	ASENDDAEMON	ASETPOST+1	/* Send a daemon	*/
#define	ADO		ASENDDAEMON+1	/* GOSUB		*/
#define	AINTERACT	ADO+1		/* Set actor		*/
#define	AAUTOEXITS	AINTERACT+1	/* Auto-exits		*/
#define	ASETARR		AAUTOEXITS+1	/* Set arrived string	*/
#define	ASETDEP		ASETARR+1	/* Set departed string	*/
#define	ARESPOND	ASETDEP+1	/* Reply, endparse	*/
#define	AERROR		ARESPOND+1	/* Give an error!	*/
#define	ABURN		AERROR+1	/* Ignite an object	*/
#define	ADOUSE		ABURN+1		/* Put it out again	*/
#define	AINC		ADOUSE+1	/* Increment state	*/
#define	ADEC		AINC+1		/* Decrement		*/
#define	ATOPRANK	ADEC+1		/* Make player toprank	*/
#define	ADEDUCT		ATOPRANK+1	/* Reduce score by %	*/
#define	ADAMAGE		ADEDUCT+1	/* Damage object	*/
#define	AREPAIR		ADAMAGE+1	/* Repair object	*/
#define	AGSTART		AREPAIR+1	/* Start global daemon	*/

			/* Conditions */
#define	CAND		0		/* And then ....	*/
#define	CSTAR		1		/* Same as always	*/
#define	CELSE		2		/* If last was false	*/
#define	CALWAYS		3		/* Always do this 	*/
#define	CLIGHT		4		/* If there is light...	*/
#define	CISHERE		5		/* If obj is here	*/
#define	CMYRANK		6		/* If my rank is...	*/
#define	CSTATE		7		/* If state of obj	*/
#define	CMYSEX		8		/* My sex is...		*/
#define	CLASTVB		9		/* If last verb was..	*/
#define	CLASTDIR	10		/* If last TRAVEL verb	*/
#define	CLASTROOM	11		/* If last room was	*/
#define	CASLEEP		12		/* Is player sleeping	*/
#define	CSITTING	13		/* Is player sitting	*/
#define	CLYING		14		/* Is player lying down	*/
#define	CRAND		15		/* If rand(n1) eq n2	*/
#define	CRDMODE		16		/* If rdmode is...	*/
#define	CONLYUSER	17		/* If only player	*/
#define	CALONE		18		/* If only person here	*/
#define	CINROOM		19		/* If player in room	*/
#define	COPENS		20		/* If object opens	*/
#define	CGOTNOWT	21		/* If carrying nothing	*/
#define	CCARRYING	22		/* Carrying object?	*/
#define	CNEARTO		23		/* Is it here SOMEWHERE	*/
#define	CHIDDEN		24		/* Can others see me?	*/
#define	CCANGIVE	25		/* Can player manage it	*/
#define	CINFL		26		/* 			*/
#define	CINFLICTED	27		/* Is played inflicted	*/
#define	CSAMEROOM	28		/* Same room as player? */
#define	CSOMEONEHAS	29		/* If obj is carried?	*/
#define	CTOPRANK	30		/* If u'r the top rank	*/
#define	CGOTA		31		/* Carrying obj in stat */
#define	CACTIVE		32		/* Is daemon active?	*/
#define	CTIMER		33		/* Check time left	*/
#define	CBURNS		34		/* If object burns	*/
#define	CCONTAINER	35		/* If its a container	*/
#define	CEMPTY		CCONTAINER+1	/* If object is empty	*/
#define	COBJSIN		CEMPTY+1	/* Check # of contents	*/
#define	CALTEP		COBJSIN+1	/* Always .., endparse	*/
#define	CANTEP		CALTEP+1	/* And    .., endparse	*/
#define	CHELPING	CANTEP+1	/* Are we helping him?	*/
#define	CGOTHELP	CHELPING+1	/* If we've got help	*/
#define	CANYHELP	CGOTHELP+1	/* Helping ANYONE?	*/
#define	CELTEP		CANYHELP+1	/* Else   ..., endparse	*/
#define	CSTAT		CELTEP+1	/* If attrib <> no.	*/
#define	COBJINV		CSTAT+1		/* If object invisible	*/
#define CFIGHTING	COBJINV+1	/* Is player fighting?	*/
#define CTASKSET	CFIGHTING+1	/* Has task been done?	*/
#define	CCANSEE		CTASKSET+1	/* Can I see <player>	*/
#define	CVISIBLETO	CCANSEE+1	/* Am I visible to	*/
#define	CNOUN1		CVISIBLETO+1	/* Match noun1		*/
#define	CNOUN2		CNOUN1+1	/* Match noun2		*/
#define	CAUTOEXITS	CNOUN2+1	/* Auto exits on?	*/
#define	CDEBUG		CAUTOEXITS+1	/* Debug mode on?	*/
#define	CFULL		CDEBUG+1	/* Stat at full?	*/
#define	CTIME		CFULL+1		/* Time remaining?	*/
#define	CDEC		CTIME+1		/* Decrement & test	*/
#define	CINC		CDEC+1		/* Increment & test	*/
#define	CLIT		CINC+1		/* Is object lit?	*/
#define	CFIRE		CLIT+1		/* Is object flamable?	*/
#define	CHEALTH		CFIRE+1		/* Is players health %?	*/
#define	CMAGIC		CHEALTH+1	/* Can magic be done?	*/
#define	CSPELL		CMAGIC+1	/* Can spell be done?	*/
#define	CIN		CSPELL+1	/* IN <ROOM> <NOUN>	*/

			/* Paramter types */
#define	PREAL		-70		/* Noun or slot label	*/
#define	PNOUN		1		/* Must be a noun	*/
#define	PADJ		2		/* Must be an adj	*/
#define	PPLAYER		4		/* Must be a player	*/
#define	PROOM		5		/* Must be a room	*/
#define	PUMSG		7		/* Must be text		*/
#define	PVERB		8		/* Must be a verb	*/
#define	PCLASS		9		/* Must be a class	*/
#define	PNUM		10		/* Must be a number	*/
#define	PRFLAG		11		/* Must be a room flag	*/
#define	POFLAG		12		/* Must be an obj flag	*/
#define	PSFLAG		13		/* Must be a stat flag	*/
#define PSEX		14		/* Must be a gender	*/
#define	PDAEMON		15		/* Must be a daemon ID	*/

#define	SGLOW		1		/* Spell #1			*/
#define	SINVIS		2		/* Spell #2			*/
#define	SBLIND		3		/* Spell #3			*/
#define	SCRIPPLE	4		/* Spell #4			*/
#define	SDEAF		5		/* Spell #5			*/
#define	SDUMB		6		/* Player cant speak		*/
#define SSLEEP		7		/* Puts a player to bedie byes	*/
#define SSINVIS		8		/* Super Invisible		*/


#define STSCORE		1		/* Score 		*/
#define STSTR		2		/* Strength		*/
#define STSTAM		3		/* Stamina		*/
#define STDEX		4		/* Dexterity		*/
#define STWIS		5		/* Wisdom		*/
#define STEXP		6		/* Experience		*/
#define STMAGIC		7		/* Magic points		*/
#define	STSCTG		8		/* Score This Game	*/

			/* Anouncement types */
#define	AGLOBAL		0
#define AEVERY1		1
#define AOUTSIDE	2
#define	AHERE		3
#define	AOTHERS		4
#define AALL		5

/* -- Message Types -- */
#define MaxLines	10
#define	MKILL		1		/* Close down */
#define	MCNCT		2		/* Connection */
#define	MDISCNCT	3		/* Disconnect */
#define	MDATAREQ	4		/* Gets ptrs! */
#define MLOGGED		5		/* Logged in! */
#define	MMESSAGE	6		/* Sent a msg */
#define	MCLOSEING	7		/* Closeing.. */
#define MRESET		8		/* Reset      */
#define MLOCK		9		/* Line Lock  */
#define MUNLOCK		10		/* Unlock it  */
#define	MSUMMONED	11		/* COME HERE! */
#define	MDIE		12		/* Ciao!!     */
#define	MBUSY		13		/* I'm busy   */
#define	MFREE		14		/* I'm free!  */
#define	MEXECUTE	15		/* Execute a command! */
#define	MDSTART		16		/* Daemon start */
#define	MDCANCEL	17		/* Cancel a daemon */
#define	MDAEMON		18		/* We have lift off! */
#define	MCHECKD		19		/* Get daemon status */
#define	MFORCE		20		/* Do it, buddo! */
#define	MMADEWIZ	21		/* Reached top rank! */
#define	MLOG		22		/* Write log entry */
#define	MRWARN		23		/* Reset Warning */
#define	MEXTEND		24		/* Extend game */
#define	MGDSTART	25		/* Start global daemon */

#define	WNONE		-1		/* None! */
#define	WANY		0		/* Anything! */
#define	WNOUN		1		/* Word is a noun */
#define	WADJ		2		/* Word is an adjective */
#define	WPREP		3		/* Its a prep */
#define	WPLAYER		4		/* Its a player */
#define	WROOM		5		/* Its a room ID */
#define	WSYN		6		/* Its a synonym */
#define	WTEXT		7		/* Its text */
#define	WVERB		8		/* Its a verb! */
#define WCLASS		9		/* Class name */
#define WNUMBER		10		/* A number */

	/* -- Player flags -- */
#define	PFINVIS		0x00001		/* Player invisible */
#define PFGLOW		0x00002		/* Player glowing */
#define PFBLIND		0x00004		/* Blind */
#define	PFDUMB		0x00008		/* Can't speak */
#define PFDEAF		0x00010		/* Player's deaf */
#define	PFCRIP		0x00020		/* Can't move */
#define	PFDYING		0x00040		/* Player is dying */
#define PFLIMP		0x00080		/* Limping */
#define PFASLEEP	0x00100		/* Sleeping */
#define PFSITTING	0x00200		/* Sitting down */
#define PFLYING		0x00400		/* Lying Down */
#define PFFIGHT		0x00800		/* Fighting */
#define PFATTACKER	0x01000		/* If you started the fight */
#define	PFMOVING	0x02000		/* If you are 'in transit' */
#define	PFSINVIS	0x04000		/* Player is Super Invis */

#define RDRC		0		/* RC Mode */
#define RDVB		1		/* Verbose mode */
#define RDBF		2		/* Brief mode */

#define TYPEV		0		/* Brief mode */
#define TYPEB		1		/* Verbose mode */

#define	At	amul->type
#define	Am	amul->msg
#define	Af	amul->from
#define	Ad	amul->data
#define	Ap	amul->ptr
#define	Ap1	amul->p1
#define	Ap2	amul->p2
#define	Ap3	amul->p3
#define	Ap4	amul->p4

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

	/* -- User Flags -- */

#define	ufANSI	0x001				/* ANSI bit		*/
#define	ufCRLF	0x002				/* Add LineFeeds	*/
#define	ufARDO	0x004				/* Auto Redo		*/

#define	DLLEN	80			/* Default line length */
#define	DSLEN	24			/* Default screen length */
#define	DRCHAR	'|'			/* Default redo-char */
#define	DFLAGS	ufCRLF			/* Default = cr/lf ON */

	/* -- Useful defines -- */

#define	isOINVIS(x)	((obtab+x)->flags & OF_INVIS)
#define	isPINVIS(x)	((lstat+x)->flags & PFINVIS)
#define	IamINVIS	(me2->flags & PFINVIS)
#define	IamSINVIS	(me2->flags & PFSINVIS)
#define	pROOM(x)	((lstat+x)->room)
#define	pRANK(x)	((usr+x)->rank)
#define	myRANK		me->rank
#define	mySCORE		me->score
#define	myROOM		me2->room
#define	LightHere	lit(me2->room)
#define	unfreeze	Permit(); return

#include "h/amul.msgs.h"		/* System message defines */
#include "h/amul.acts.h"

#define	CP1	actual(*(tt.pptr))
#define	CP2	actual(*(tt.pptr+1))
#define	CP3	actual(*(tt.pptr+2))
#define	CP4	actual(*(tt.pptr+3))
#define	TP1	actual(*(tt.pptr+ncop[tt.condition]))
#define	TP2	actual(*(tt.pptr+ncop[tt.condition]+1))
#define	TP3	actual(*(tt.pptr+ncop[tt.condition]+2))
#define	TP4	actual(*(tt.pptr+ncop[tt.condition]+3))
#define	AP1	(char *)actptr(*(tt.pptr+ncop[tt.condition]))
#define	AP2	(char *)actptr(*(tt.pptr+ncop[tt.condition]+1))
#define	AP3	(char *)actptr(*(tt.pptr+ncop[tt.condition]+2))
#define	AP4	(char *)actptr(*(tt.pptr+ncop[tt.condition]+3))
#define	STATE	(objtab->states+(long)objtab->state)
#define	State(i) ((obtab+i)->states+(long)(obtab+i)->state)
#define ItsState (it->states+(long)it->state)
#define	xLIGHT(x)	(lstat+x)->light
#define xHADLIGHT(x)	(lstat+x)->hadlight

#define	acp	(char *)actptr

#endif
