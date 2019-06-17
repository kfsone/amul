/*

  AMULInc.H - AMUL.C & AMUL2.C Includes
	-------------------------------------

  SPECIAL NOTICE!

  Anything you alter here REALLY ought to be added/altered in AMUL2.C
*/

#include "h/AMUL.Defs.H"		/* Defines in one nice file	*/
#include "h/AMUL.Incs.H"		/* Include files tidily stored.	*/
#include "h/AMUL.Lnks.H"		/* (external) Linkage symbols	*/
#include "h/AMUL.Msgs.H"		/* System Message Flags		*/

#define	dtx(x)	if(debug!=0) tx(x)
extern short int	debug;				/* Is debug mode on?	 */

extern struct	_PLAYER *usr,*me,him,*you;
extern struct	LS *lstat,*me2,*you2;
extern struct	_ROOM_STRUCT  room,*roomtab,*rmtab;	/* ptr->table, ptr->a room */
extern struct	_VERB_STRUCT verb,*vbtab,*vbptr;
extern struct	_SLOTTAB vbslot,*slottab,*stptr;
extern struct	_VBTAB vt,*vtp,*vtabp;
extern struct	_RANK_STRUCT rank,*rktab,*ranktab;
extern struct	_OBJ_STRUCT obj,*obtab,*objtab;
extern struct	_NTAB_STRUCT nountab,*ntab,*ntabp;
extern struct	_OBJ_STATE state,*statab,*statep;
extern struct	_TT_ENT tt,*ttp,*ttabp;

extern struct Screen *OpenScreen(),*sC;
extern struct Window *OpenWindow(),*wG;
extern struct IOExtSer *serio,*wserio,*CreateExtIO();
extern void   DeletePort();
extern struct Task   *mytask,*FindTask();
extern struct ViewPort vP;
extern struct RastPort *rpG;
extern struct MsgPort *ReadRep,*WriteRep,*repbk,*FindPort(),*CreatePort();
extern struct IOStdReq ReadIo, WriteIo;

/* Frame specific variables */
extern char	serop,MyFlag;		/* SerPort open? What am I? */
extern char	*input;			/* 400 bytes, 5 lines */
extern char	str[],spc[],mxx[],mxy[];/* Output string */
extern char	wtil[];			/* Window title */
extern char	inc,forced,failed,died,addcr,fol;/* For parsers use */
extern char	actor,last_him,last_her;	/* People we know of */
extern char	autoexits,needcr;		/* Autoexits on/off? */
extern long	iverb,overb,iadj1,inoun1,iprep,iadj2,inoun2,lverb,ldir,lroom;
extern long	wtype[6],word;		/* Type of word... */
extern short int donev,skip;		/* No. of vb's/TT's done */
extern char	link,exeunt,more;	/* If we've linked yet */
extern long ml,donet,it;		/* Maximum lines */
extern struct Aport *ap,*amanp,*intam;	/* The message pointers */
extern struct MsgPort *amanrep;	/* AMAN reply port */
extern char	*ob,*gp,*ow,*lastres,*lastcrt;/* Pointer to output buffers etc */
extern short int rset, rclr, ip, csyn;	/* Masks for Room Counter */

	/* Integers */
extern	long	rooms;				/* No. of rooms          */
extern	long	ranks;				/* No. of ranks          */
extern	long	ttents;				/* No. of TT entries     */
extern	long	verbs;				/* No. of verbs		 */
extern	long	syns;				/* No. of synonyms  	 */
extern	long	msgs;				/* No. of user def. msgs */
extern	long	nouns;				/* No. of nouns		 */
extern	long	mobs;				/* No. of mobiles	 */
extern	long	adjs;				/* No. of adjectives	 */
extern	long	umsgs;				/* No. of user messages	 */
extern	long	smsgs;				/* No. of system msgs	 */
extern	long	mobchars;			/* Number of mobile chars*/
extern	int	invis;				/* Rank for invis to see each other.*/
extern	int	invis2;				/* Rank for vis to see invis */
extern	int	minsgo;				/* Minimum supergo rank	 */
extern	int	rscale;				/* Rank scaleing	 */
extern	int	tscale;				/* Time scaleing	 */
extern	long	mins;				/* Game length		 */
extern	unsigned short int *rescnt;		/* Reset counter	 */

	/* Text */
extern	char	useful;				/* FLAG: Anything yet??? */
extern	char	dir[];				/* spc for work dir path */
extern	char	block[];			/* 1k block of spare txt */
extern	char	temp[];				/* temp store		 */
extern	char	dmove[];			/* Where to RF_CEMETERY to     */
extern	char	adname[];			/* Adventure name	 */
extern	char	vername[];			/* Version name		 */

extern	FILE *fp,*afp,*ofp1,*ofp2,*ofp3,*ifp;
extern  long	ormtablen;			/* Length of room table	 */
extern	long	ormtab;				/* Room table		 */
extern	long	desctlen;			/* Length of desc tab	 */
extern	long	statablen;			/* Len of states table	 */
extern	long	adtablen;			/* Length of table	 */
extern	long	stlen;				/* Length of slottab	 */
extern	long	vtlen;				/* Length of VT		 */
extern	long	vtplen;				/* Length of VT Parm Tab */
extern	long	ttlen;				/* Length of TT		 */
extern	long	ttplen;				/* Length of param tab	 */
extern	long	synlen;				/* Pointer to synonyms	 */
extern	long	synilen;			/* Len of syn index	 */
extern	long	umsgil;				/* User message indexes	 */
extern	long	umsgl;				/* User message length	 */
extern	long	*umsgip;			/* pointer to it!	 */
extern	char	*umsgp;				/* Actual messages	 */
extern	long	*ttpp;				/* Pointer to param tab	 */
extern	long	*vtpp;				/* ditto for VT		 */
extern	char	*adtab;				/* Adjective table	 */
extern	char	*desctab;			/* Object descrips	 */
extern	char	*synp;				/* Synonyms		 */
extern	short int *synip;			/* Synonyms index	 */
extern	short int *rctab;			/* Room count flags	 */

extern	struct	MsgPort	*port,*reply;
extern	struct Aport *amul;
extern	struct	IOExtSer *serio,*wserio;
extern	struct MsgPort *ReadRep,*WriteRep,*repbk;
extern	struct IOStdReq ReadIo, WriteIo;
extern  struct MsgPort *MyPort,*replyport;	/* For paragon */
