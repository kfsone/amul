/*
              ****    AMUD.VARS.H.....Adventure Compiler    ****
              ****           Internal Variables!            ****
									   */

#ifndef	DEF
#define	DEF
#endif

	/* Integers */
DEF SHORT	rooms;			/* # rooms */
DEF char	ranks;			/* # ranks */
DEF long	ttents;			/* # TT entries */
DEF SHORT	verbs;			/* # verbs */
DEF SHORT	syns;			/* # synonyms */
DEF long	msgs;			/* # user msgs */
DEF SHORT	nouns;			/* # nouns */
DEF SHORT	mobs;			/* # mobiles */
DEF SHORT	adjs;			/* # adjectives */
DEF long	umsgs;			/* # user messages */
DEF long	smsgs;			/* # system msgs */
DEF SHORT	mobchars;		/* # mobile chars */
DEF char	invis;			/* rnk 4 invis 2C invis */
DEF char	invis2;			/* rnk 4 !invis 2C invis */
DEF char	minsgo;			/* Min rank 4 SuperGo */
DEF SHORT	rscale,tscale;		/* Rank/time scaleing */
#ifdef	COMPILER
long	obdes;				/* # obj descrips   */
#endif

	/* Misc. */
DEF FILE	*ifp,*ofp1,*ofp2,*ofp3,*afp;	/* In and out file ptrs  */

	/* Text */
DEF char	dir[60];		/* spc for work dir path */
DEF char	block[1024];		/* 1k block of spare txt */
DEF char	dmove[IDL+1];		/* Where to DMOVE to     */
DEF char	adname[42];		/* Adventure name	 */
DEF char	vername[40];		/* Version name etc	 */
#ifndef FRAME
DEF char	logname[41];		/* Name of log file */
#endif

	/* Longs */
DEF long	flen;			/* Length of file	 */

#ifdef	PORTS
DEF long	ormtablen;		/* Length of room table	 */
DEF long	ormtab;			/* Room table		 */
DEF long	desctlen;		/* Length of desc tab	 */
DEF long	statablen;		/* Len of states table	 */
DEF long	adtablen;		/* Length of table	 */
DEF long	stlen;			/* Length of slottab	 */
DEF long	vtlen;			/* Length of VT		 */
DEF long	vtplen;			/* Length of VT Parm Tab */
DEF long	ttlen;			/* Length of TT		 */
DEF long	ttplen;			/* Length of param tab	 */
DEF long	synlen;			/* Pointer to synonyms	 */
DEF long	synilen;		/* Len of syn index	 */
DEF long	umsgil;			/* User message indexes	 */
DEF long	umsgl;			/* User message length	 */
DEF long	*umsgip;		/* pointer to it!	 */
DEF char	*umsgp;			/* Actual messages	 */
DEF long	*ttpp;			/* Pointer to param tab	 */
DEF long	*vtpp;			/* ditto for VT		 */
DEF char	*adtab;			/* Adjective table	 */
DEF char	*desctab;		/* Object descrips	 */
DEF char	*synp;			/* Synonyms		 */
DEF SHORT	*synip;			/* Synonyms index	 */
DEF SHORT	*rctab;			/* Room count flags	 */

DEF struct	MsgPort	*port,*reply;
DEF struct	Aport *amud;
#endif

	/* Structures: */
DEF struct	_PLAYER *usr,*me,him,*you;
DEF struct	LS *lstat,*me2,*you2;
DEF struct	_ROOM_STRUCT *roomtab,*rmtab;	/* ptr->table, ptr->a room */
DEF struct	_VERB_STRUCT *vbtab,*vbptr;
DEF struct	_SLOTTAB *slottab,*stptr;
DEF struct	_VBTAB *vtp,*vtabp;
DEF struct	_RANK_STRUCT *rktab,*ranktab;
DEF struct	_NTAB_STRUCT *ntab,*ntabp;
DEF struct	_OBJ_STRUCT obj,*obtab,*objtab;
DEF struct	_OBJ_STATE *statab,*statep;
DEF struct	_TT_ENT tt,*ttp,*ttabp;
DEF struct	_MOB_ENT *mobp,*mobile;
DEF struct	_MOB_TAB *mtab,*mobtab;

#ifdef	COMPILER
	FILE	*ofp4,*umfp;
	struct	_ROOM_STRUCT	room;
	struct	_VERB_STRUCT	verb;
	struct	_SLOTTAB	vbslot;
	struct	_VBTAB		vt;
	struct	_RANK_STRUCT	rank;
	struct	_NTAB_STRUCT	nountab;
	struct	_OBJ_STATE	state;
	struct	_MOB_ENT	mob;

	struct	_OBJECT_DESCRIPTIONS {
		char	id[IDL+1];
		long	descrip;		/* ptr to descrp in file */
	} objdes,*obdesp;
#endif
