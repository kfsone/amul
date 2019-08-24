#ifndef H_AMUL_VARS_H
#define H_AMUL_VARS_H 1
/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

#include <h/amul.stct.h>

#ifdef PORTS
#    include <h/amigastubs.h>
long *     ttpp;  /* Pointer to param tab	 */
long *     vtpp;  /* ditto for VT		 */
char *     adtab; /* Adjective table	 */
char *     synp;  /* Synonyms		 */
short int *synip; /* Synonyms index	 */
short int *rctab; /* Room count flags	 */

struct MsgPort *port, *reply, *FindPort(), *CreatePort();
struct Task *   mytask, *FindTask();
struct Aport *  amul;
#endif

/* Structures: */
struct _PLAYER *    usr, *me, him, *you;
struct LS *         linestat, *me2, *you2;
struct _VERB_STRUCT g_verb, *vbtab, *vbptr;
struct _SLOTTAB     vbslot, *slottab, *stptr;
struct _VBTAB       vt, *vtp, *vtabp;
struct _RANK_STRUCT rank, *rktab, *ranktab;
struct _NTAB_STRUCT nountab, *ntab, *ntabp;
struct _OBJ_STRUCT  obj, *obtab, *objtab;
struct _OBJ_STATE   state, *statab, *statep;
struct _TT_ENT      tt, *ttp, *ttabp;
struct _MOB_ENT     mob, *mobp, *mobile;

#ifdef COMPILER
struct _OBJECT_DESCRIPTIONS {
    char id[IDL + 1];
    long descrip; /* ptr to descrp in file */
} objdes, *obdesp;
#endif

#endif
