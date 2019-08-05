
/*

                     AMULInc.H - AMUL.C & AMUL2.C Includes
             -------------------------------------

  SPECIAL NOTICE!

  Anything you alter here REALLY ought to be added/altered in AMUL2.C
                                        */

#ifndef FRAME
#define FRAME 1
#define PORTS 1
#endif

#include <h/amul.defs.h>
#include <h/amul.msgs.h>
#include <setjmp.h>

#define dtx(x)                                                                                     \
if (debug != 0)                                                                                    \
tx(x)
extern short int debug; /* Is debug mode on?	 */

extern struct _PLAYER *    usr, *me, him, *you;
extern struct LS *         linestat, *me2, *you2;
extern struct _ROOM_STRUCT room, *roomtab, *rmtab; /* ptr->table, ptr->a room */
extern struct _VERB_STRUCT verb, *vbtab, *vbptr;
extern struct _SLOTTAB     vbslot, *slottab, *stptr;
extern struct VMCnA        vt, *vtp, *vtabp;
extern struct _RANK_STRUCT rank, *rktab, *ranktab;
extern struct _OBJ_STRUCT  obj, *obtab, *objtab;
extern struct _NTAB_STRUCT nountab, *ntab, *ntabp;
extern struct _OBJ_STATE   state, *statab, *statep;

extern struct Screen *  OpenScreen(), *sC;
extern struct Window *  OpenWindow(), *wG;
extern struct IOExtSer *serio, *wserio, *CreateExtIO();
extern void             DeletePort();
extern struct Task *    mytask, *FindTask();
extern struct ViewPort  vP;
extern struct RastPort *rpG;
extern struct MsgPort * ReadRep, *WriteRep, *repbk, *FindPort(), *CreatePort();
extern struct IOStdReq  ReadIo, WriteIo;

/* get the powerwindows file */
#ifdef AMUL1
#include <h/amul.scrn.h>

void end(), sendmessage(), parget();
#endif

/* Frame specific variables */
extern char            serop, MyFlag;                         /* SerPort open? What am I? */
extern char *          input;                                 /* 400 bytes, 5 lines */
extern char            str[], spc[], mxx[], mxy[];            /* Output string */
extern char            wtil[];                                /* Window title */
extern char            iosup;                                 /* What kind of IO support */
extern char            inc, forced, failed, died, addcr, fol; /* For parsers use */
extern char            actor, last_him, last_her;             /* People we know of */
extern char            autoexits, needcr;                     /* Autoexits on/off? */
extern long            iverb, overb, iadj1, inoun1, iprep, iadj2, inoun2, lverb, ldir, lroom;
extern long            wtype[6], word;                   /* Type of word... */
extern short int       donev, skip;                      /* No. of vb's/TT's done */
extern char            link, exeunt, more;               /* If we've linked yet */
extern long            ml, donet, it;                    /* Maximum lines */
extern struct Aport *  ap, *amanp, *intam;               /* The message pointers */
extern struct MsgPort *amanrep;                          /* AMAN reply port */
extern char *          ob, *gp, *ow, *lastres, *lastcrt; /* Pointer to output buffers etc */
extern short int       rset, rclr, ip, csyn;             /* Masks for Room Counter */

/* Text */
extern struct MsgPort * port, *reply;
extern struct Aport *   amul;
extern struct IOExtSer *serio, *wserio;
extern struct MsgPort * ReadRep, *WriteRep, *repbk;
extern struct IOStdReq  ReadIo, WriteIo;
extern struct MsgPort * MyPort, *replyport; /* For paragon */

#ifdef AMUL1
#include <ADV:Frame/Cond.C> /* Parser & VT processor	 */
#include <ADV:Frame/GetID.C>
#include <ADV:Frame/IOBits.C>
#include <ADV:Frame/Proc.C> /* Parser processing routines	 */
#include <ADV:Frame/Reset.C>
#include <ADV:Frame/ScrnIO.C> /* Screen driver bits		 */
#endif
