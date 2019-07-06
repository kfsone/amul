/*

  AMULInc.H - AMUL.C & AMUL2.C Includes
	-------------------------------------

  SPECIAL NOTICE!

  Anything you alter here REALLY ought to be added/altered in AMUL2.C
*/

#include "h/amigastubs.h"
#include "h/amul.cons.h"
#include "h/amul.defs.h"
#include "h/amul.incs.h"
#include "h/amul.msgs.h"
#include "h/os.h"

#define dtx(x)                                                                                     \
	if (debug != 0)                                                                                \
	tx(x)
extern short int debug; /* Is debug mode on?	 */

extern struct _PLAYER *	usr, *me, him, *you;
extern struct LS *		   lstat, *me2, *you2;
extern struct _ROOM_STRUCT room, *roomtab, *rmtab; /* ptr->table, ptr->a room */
extern struct _VERB_STRUCT verb, *vbtab, *vbptr;
extern struct _SLOTTAB	 vbslot, *slottab, *stptr;
extern struct _VBTAB	   vt, *vtp, *vtabp;
extern struct _RANK_STRUCT rank, *rktab, *ranktab;
extern struct _OBJ_STRUCT  obj, *obtab, *objtab;
extern struct _NTAB_STRUCT nountab, *ntab, *ntabp;
extern struct _OBJ_STATE   state, *statab, *statep;
extern struct _TT_ENT	  tt, *ttp, *ttabp;

extern struct Screen *  OpenScreen(), *sC;
extern struct Window *  OpenWindow(), *wG;
extern struct IOExtSer *serio, *wserio, *CreateExtIO();
extern Amiga::Task *	mytask;
extern struct ViewPort  vP;
extern struct RastPort *rpG;
extern Amiga::MsgPort * ReadRep, *WriteRep, *repbk;
extern struct IOStdReq  ReadIo, WriteIo;

/* Frame specific variables */
extern char			   serop, MyFlag;						  /* SerPort open? What am I? */
extern char *		   input;								  /* 400 bytes, 5 lines */
extern char			   str[], spc[], mxx[], mxy[];			  /* Output string */
extern char			   wtil[];								  /* Window title */
extern char			   inc, forced, failed, died, addcr, fol; /* For parsers use */
extern char			   actor, last_him, last_her;			  /* People we know of */
extern char			   autoexits, needcr;					  /* Autoexits on/off? */
extern int32_t		   iverb, overb, iadj1, inoun1, iprep, iadj2, inoun2, lverb, ldir, lroom;
extern int32_t		   wtype[6], word;					 /* Type of word... */
extern short int	   donev, skip;						 /* No. of vb's/TT's done */
extern char			   link, exeunt, more;				 /* If we've linked yet */
extern int32_t		   ml, donet, it;					 /* Maximum lines */
extern Aport *		   ap, *amanp, *intam;				 /* The message pointers */
extern Amiga::MsgPort *amanrep;							 /* AMAN reply amanPort */
extern char *		   ob, *gp, *ow, *lastres, *lastcrt; /* Pointer to output buffers etc */
extern short int	   rset, rclr, ip, csyn;			 /* Masks for Room Counter */

/* Integers */
extern int32_t			   rooms;	/* No. of rooms          */
extern int32_t			   ranks;	/* No. of ranks          */
extern int32_t			   ttents;   /* No. of TT entries     */
extern int32_t			   verbs;	/* No. of verbs		 */
extern int32_t			   syns;	 /* No. of synonyms  	 */
extern int32_t			   msgs;	 /* No. of user def. msgs */
extern int32_t			   nouns;	/* No. of nouns		 */
extern int32_t			   mobs;	 /* No. of mobiles	 */
extern int32_t			   adjs;	 /* No. of adjectives	 */
extern int32_t			   umsgs;	/* No. of user messages	 */
extern int32_t			   smsgs;	/* No. of system msgs	 */
extern int32_t			   mobchars; /* Number of mobile chars*/
extern int				   invis;	/* Rank for invis to see each other.*/
extern int				   invis2;   /* Rank for vis to see invis */
extern int				   minsgo;   /* Minimum supergo rank	 */
extern int				   rscale;   /* Rank scaleing	 */
extern int				   tscale;   /* Time scaleing	 */
extern int32_t			   mins;	 /* Game length		 */
extern unsigned short int *rescnt;   /* Reset counter	 */

/* Text */
extern char useful;	/* FLAG: Anything yet??? */
extern char dir[];	 /* spc for work dir path */
extern char block[];   /* 1k block of spare txt */
extern char temp[];	/* temp store		 */
extern char dmove[];   /* Where to RF_CEMETERY to     */
extern char adname[];  /* Adventure name	 */
extern char vername[]; /* Version name		 */

extern FILE *	 fp, *afp, *ofp1, *ofp2, *ofp3, *ifp;
extern int32_t	ormtablen; /* Length of room table	 */
extern int32_t	ormtab;	/* Room table		 */
extern int32_t	desctlen;  /* Length of desc tab	 */
extern int32_t	statablen; /* Len of states table	 */
extern int32_t	adtablen;  /* Length of table	 */
extern int32_t	stlen;	 /* Length of slottab	 */
extern int32_t	vtlen;	 /* Length of VT		 */
extern int32_t	vtplen;	/* Length of VT Parm Tab */
extern int32_t	ttlen;	 /* Length of TT		 */
extern int32_t	ttplen;	/* Length of param tab	 */
extern int32_t	synlen;	/* Pointer to synonyms	 */
extern int32_t	synilen;   /* Len of syn index	 */
extern int32_t	umsgil;	/* User message indexes	 */
extern int32_t	umsgl;	 /* User message length	 */
extern int32_t *  umsgip;	/* pointer to it!	 */
extern char *	 umsgp;	 /* Actual messages	 */
extern int32_t *  ttpp;		 /* Pointer to param tab	 */
extern int32_t *  vtpp;		 /* ditto for VT		 */
extern char *	 adtab;	 /* Adjective table	 */
extern char *	 desctab;   /* Object descrips	 */
extern char *	 synp;		 /* Synonyms		 */
extern short int *synip;	 /* Synonyms index	 */
extern short int *rctab;	 /* Room count flags	 */

extern Amiga::MsgPort * amanPort, *replyPort;
extern Aport *			amul;
extern struct IOExtSer *serio, *wserio;
extern Amiga::MsgPort * ReadRep, *WriteRep, *repbk;
extern struct IOStdReq  ReadIo, WriteIo;
extern Amiga::MsgPort * MyPort, *replyport; /* For paragon */

bool match(const char *lhs, const char *rhs);

#include "amul2.h"
#include "cond.h"
#include "daemons.h"
#include "file_handling.h"
#include "getid.h"
#include "iobits.h"
#include "parser.h"
#include "proc.h"