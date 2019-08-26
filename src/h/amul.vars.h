#ifndef AMUL_VARS_H
#define AMUL_VARS_H 1
/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

#include "h/amul.stct.h"

#ifdef PORTS
#include "h/amigastubs.h"
long *ttpp;       /* Pointer to param tab	 */
long *vtpp;       /* ditto for VT		 */
short int *rctab; /* Room count flags	 */

struct MsgPort *port, *reply, *FindPort(), *CreatePort();
struct Task *mytask, *FindTask();
struct Aport *amul;
#endif

/* Structures: */
struct _PLAYER *usr, *me, him, *you;
struct LS *linestat, *me2, *you2;
struct _VERB_STRUCT g_verb, *vbtab, *vbptr;
struct _SLOTTAB vbslot, *slottab, *stptr;
struct _VBTAB vt, *vtp, *vtabp;
struct _NTAB_STRUCT nountab, *ntab, *ntabp;
struct _TT_ENT tt, *ttp, *ttabp;
struct _MOB_ENT mob, *mobp, *mobile;

#endif
