#ifndef H_AMUL_VARS_H
#define H_AMUL_VARS_H 1
/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

#include <h/amul.stct.h>
#include <h/lang_struct.h>
#include <h/room_struct.h>
#include <h/obj_struct.h>

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
struct _RANK_STRUCT rank, *rktab, *ranktab;
struct _OBJ_STRUCT  obj, *obtab, *objtab;
struct _OBJ_STATE   state, *statab, *statep;
struct _MOB_ENT     mob, *mobp, *mobile;

#endif
