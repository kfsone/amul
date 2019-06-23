/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

/* Integers */
int32_t rooms;			/* No. of rooms          */
int32_t ranks;			/* No. of ranks          */
int32_t ttents;			/* No. of TT entries     */
int32_t verbs;			/* No. of verbs		 */
int32_t syns;			/* No. of synonyms  	 */
int32_t msgs;			/* No. of user def. msgs */
int32_t nouns;			/* No. of nouns		 */
int32_t mobs;			/* No. of mobiles	 */
int32_t adjs;			/* No. of adjectives	 */
int32_t umsgs;			/* No. of user messages	 */
int32_t smsgs;			/* No. of system msgs	 */
int32_t mobchars;		/* No. of mobile chars	 */
int		invis;			/* rnk 4 invis 2C invis	 */
int		invis2;			/* rnk 4 !invis 2C invis */
int		minsgo;			/* Minimum rank 4 SupaGo */
int		rscale, tscale; /* Rank and time scaleing */
#ifdef COMPILER
int32_t obdes; /* No. of obj descrips   */
#endif

/* Misc. */
FILE *ifp, *ofp1, *ofp2, *ofp3, *afp; /* In and out file ptrs  */
#ifdef COMPILER
FILE *ofp4; /* For lang. table	 */
#endif

/* Text */
char dir[60];		 /* spc for work dir path */
char block[1024];	/* 1k block of spare txt */
char temp[256];		 /* temp store		 */
char dmove[IDL + 1]; /* Where to RF_CEMETERY to     */
char adname[42];	 /* Adventure name	 */
char vername[40];	/* Version name etc	 */

/* Longs */
int32_t flen; /* Length of file	 */

int32_t	ormtablen; /* Length of room table	 */
int32_t	ormtab;	/* Room table		 */
int32_t	desctlen;  /* Length of desc tab	 */
int32_t	statablen; /* Len of states table	 */
int32_t	adtablen;  /* Length of table	 */
int32_t	stlen;	 /* Length of slottab	 */
int32_t	vtlen;	 /* Length of VT		 */
int32_t	vtplen;	/* Length of VT Parm Tab */
int32_t	ttlen;	 /* Length of TT		 */
int32_t	ttplen;	/* Length of param tab	 */
int32_t	synlen;	/* Pointer to synonyms	 */
int32_t	synilen;   /* Len of syn index	 */
int32_t	umsgil;	/* User message indexes	 */
int32_t	umsgl;	 /* User message length	 */
int32_t *  umsgip;	/* pointer to it!	 */
char *	 umsgp;	 /* Actual messages	 */
int32_t *  ttpp;	  /* Pointer to param tab	 */
int32_t *  vtpp;	  /* ditto for VT		 */
char *	 adtab;	 /* Adjective table	 */
char *	 desctab;   /* Object descrips	 */
char *	 synp;	  /* Synonyms		 */
short int *synip;	 /* Synonyms index	 */
short int *rctab;	 /* Room count flags	 */

Amiga::MsgPort *amanPort;
Amiga::Task *   mytask;
Aport *			amul;

/* Structures: */
struct _PLAYER *	usr, *me, him, *you;
struct LS *			lstat, *me2, *you2;
struct _ROOM_STRUCT room, *roomtab, *rmtab; /* ptr->table, ptr->a room */
struct _VERB_STRUCT verb, *vbtab, *vbptr;
struct _SLOTTAB		vbslot, *slottab, *stptr;
struct _VBTAB		vt, *vtp, *vtabp;
struct _RANK_STRUCT rank, *rktab, *ranktab;
struct _NTAB_STRUCT nountab, *ntab, *ntabp;
struct _OBJ_STRUCT  obj, *obtab, *objtab;
struct _OBJ_STATE   state, *statab, *statep;
struct _TT_ENT		tt, *ttp, *ttabp;
struct _MOB_ENT		mob, *mobp, *mobile;

#ifdef COMPILER
struct _OBJECT_DESCRIPTIONS
{
	char	id[IDL + 1];
	int32_t descrip; /* ptr to descrp in file */
} objdes, *obdesp;
#endif
