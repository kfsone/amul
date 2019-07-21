// smuglcom.hpp -- Main smuglcom include file (used by most other files)
// $Id: smuglcom.hpp,v 1.10 1997/05/22 02:21:41 oliver Exp $

#ifndef COMPILER
#define	COMPILER	1

#include "includes.hpp"
#include "structs.hpp"
#include "variables.hpp"
#include "consts.hpp"
#include "protos.hpp"
#include "vocab.hpp"

#include <time.h>

#ifndef	SMUGLCOM /* Allow re-use of declarations in smuglcom.h */
# define DEC extern
#else
# define DEC
#endif

DEC long FPos;			/* Used during TT/Lang writes */
DEC char Word[64];		/* For internal use only <grin> */
DEC char temp[180];		/* temporary string */
DEC long err;			/* Error count */
DEC short proc;			/* What we are processing */
DEC time_t compiled;		/* Bits for time etc */
DEC char *data;			/* Pointer to data buffer */
DEC char *data2;		/* Secondary buffer area */
DEC char *syntab;		/* Synonym table, re-read */
DEC long mins;			/* Length of data & gametime */
DEC long obmem;			/* Size of Objects.TXT */
DEC long vbmem;			/* Size of Lang.Txt */
DEC long wizstr;		/* Wizards strength */
DEC char *mobdat;		/* Mobile data */
DEC char inc_hash_stats;        /* Command line switch */
DEC counter_t cur_room;         /* "Current room" (for travel table) */

DEC char warn, needcr, exi;

extern char *sgetl(char *, char *);
extern char *skipline(char *);

extern BASIC_OBJ **bobs;
extern CONTAINER *containers;

    /* Counters */
DEC counter_t rooms;            /* # rooms */
DEC counter_t ranks;            /* # ranks */
DEC counter_t ttents;		/* # TT entries */
DEC counter_t verbs;		/* # verbs */
DEC counter_t syns;             /* # synonyms */
DEC counter_t nouns;		/* # nouns */
DEC counter_t mobs;             /* # mobiles */
DEC counter_t msgs;             /* # text messages */
DEC counter_t mobchars;		/* # mobile chars */
DEC counter_t obdes;            /* # obj descrips */

extern char adname[];		/* Adventure name */
extern char logname[];		/* Name of log file */

DEC char invis;			/* rnk 4 invis 2C invis */
DEC char invis2;		/* rnk 4 !invis 2C invis */
DEC char minsgo;		/* Min rank 4 SuperGo */
DEC short rscale, tscale;	/* Rank/time scaleing */
DEC int port;                   // The game port

DEC long *umsgip;		/* pointer to it!        */
DEC char *umsgp;		/* Actual messages       */

	/* Misc. */
DEC FILE *ifp, *ofp1, *ofp2, *ofp3, *afp;	/* In and out file ptrs  */

	/* Longs */
DEC long flen;			/* Length of file        */

DEC FILE *ofp5;
DEC FILE *msgfp;

DEC FILE *ofp4;
DEC ROOM room;
DEC struct VERB verb;
DEC struct SLOTTAB vbslot;
DEC struct VBTAB vt;
DEC struct RANKS rank;
DEC struct OBJ_STATE state;
DEC struct MOB_ENT mob;
DEC ROOM *roomtab;	/* ptr->table, ptr->a room */
DEC struct VERB *vbtab, *vbptr;
DEC struct SLOTTAB *slottab, *stptr;
DEC struct RANKS *ranktab;
DEC struct NTAB *ntabp;
DEC OBJ *obj, *obtab;
DEC struct OBJ_STATE *statab, *statep;
DEC struct TT_ENT tt, *ttp, *ttabp;
DEC struct MOB_ENT *mobp, *mobile;
DEC struct MOB_TAB *mtab, *mobtab;

enum
{				/* Text file numbers */
    TF_SYSMSG,
    TF_SYSTEM,
    TF_UMSG,
    TF_ROOMS,
    TF_MOBILES,
    TF_OBDESCS,
    TF_OBJECTS,
    TF_LANG,
    TF_TRAVEL,
    TF_SYNS,
    TXTFILES
};

#ifdef SMUGLCOM
const char *txtfile[TXTFILES] =
{
    "sysmsg", "system", "umsg", "rooms", "mobiles", "obdescs",
    "objects", "lang", "travel", "syns"
};
#else
extern const char *txtfile[];
#endif

#endif /* COMPILER */
