#ifndef H_AMUL_VARS_H
#define H_AMUL_VARS_H 1
/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

#ifdef PORTS
long       ormtablen; /* Length of room table	 */
long       ormtab;    /* Room table		 */
long       desctlen;  /* Length of desc tab	 */
long       statablen; /* Len of states table	 */
long       adtablen;  /* Length of table	 */
long       stlen;     /* Length of slottab	 */
long       vtlen;     /* Length of VT		 */
long       vtplen;    /* Length of VT Parm Tab */
long       ttlen;     /* Length of TT		 */
long       ttplen;    /* Length of param tab	 */
long       synlen;    /* Pointer to synonyms	 */
long       synilen;   /* Len of syn index	 */
long       umsgil;    /* User message indexes	 */
long       umsgl;     /* User message length	 */
long *     umsgip;    /* pointer to it!	 */
char *     umsgp;     /* Actual messages	 */
long *     ttpp;      /* Pointer to param tab	 */
long *     vtpp;      /* ditto for VT		 */
char *     adtab;     /* Adjective table	 */
char *     desctab;   /* Object descrips	 */
char *     synp;      /* Synonyms		 */
short int *synip;     /* Synonyms index	 */
short int *rctab;     /* Room count flags	 */

struct MsgPort *port, *reply, *FindPort(), *CreatePort();
struct Task *   mytask, *FindTask();
struct Aport *  amul;
#endif

/* Structures: */
struct _PLAYER *    usr, *me, him, *you;
struct LS *         linestat, *me2, *you2;
struct _ROOM_STRUCT room, *roomtab, *rmtab; /* ptr->table, ptr->a room */
struct _VERB_STRUCT verb, *vbtab, *vbptr;
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
