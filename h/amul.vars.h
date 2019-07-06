#pragma once

/*
 ****    AMUL.VARS.H.....Adventure Compiler    ****
 ****           Internal Variables!            ****
 */

// Integers
extern int32_t rooms;           // No. of rooms
extern int32_t ranks;           // No. of ranks
extern int32_t ttents;          // No. of TT entries
extern int32_t verbs;           // No. of verbs
extern int32_t syns;            // No. of synonyms
extern int32_t msgs;            // No. of user def. msgs
extern int32_t nouns;           // No. of nouns
extern int32_t mobs;            // No. of mobiles
extern int32_t adjs;            // No. of adjectives
extern int32_t umsgs;           // No. of user messages
extern int32_t smsgs;           // No. of system msgs
extern int32_t mobchars;        // No. of mobile chars
extern int     invis;           // rnk 4 invis 2C invis
extern int     invis2;          // rnk 4 !invis 2C invis
extern int     minsgo;          // Minimum rank 4 SupaGo
extern int     rscale, tscale;  // Rank and time scaleing
#ifdef COMPILER
extern int32_t obdes;  // No. of obj descrips
#endif

// Misc.
extern FILE *ifp, *ofp1, *ofp2, *ofp3, *afp;  // In and out file ptrs
#ifdef COMPILER
extern FILE *ofp4;  // For lang. table
#endif

// Text
extern char dir[60];         // spc for work dir path
extern char block[1024];     // 1k block of spare txt
extern char temp[256];       // temp store
extern char dmove[IDL + 1];  // Where to RF_CEMETERY to
extern char adname[41];      // Adventure name
extern char vername[41];     // Version name etc

// Longs
extern int32_t flen;  // Length of file

extern int32_t    ormtablen;  // Length of room table
extern int32_t    ormtab;     // Room table
extern int32_t    desctlen;   // Length of desc tab
extern int32_t    statablen;  // Len of states table
extern int32_t    adtablen;   // Length of table
extern int32_t    stlen;      // Length of slottab
extern int32_t    vtlen;      // Length of VT
extern int32_t    vtplen;     // Length of VT Parm Tab
extern int32_t    ttlen;      // Length of TT
extern int32_t    ttplen;     // Length of param tab
extern int32_t    synlen;     // Pointer to synonyms
extern int32_t    synilen;    // Len of syn index
extern int32_t    umsgil;     // User message indexes
extern int32_t    umsgl;      // User message length
extern int32_t *  umsgip;     // pointer to it!
extern char *     umsgp;      // Actual messages
extern int32_t *  ttpp;       // Pointer to param tab
extern int32_t *  vtpp;       // ditto for VT
extern char *     adtab;      // Adjective table
extern char *     desctab;    // Object descrips
extern char *     synp;       // Synonyms
extern short int *synip;      // Synonyms index
extern short int *rctab;      // Room count flags

extern Amiga::MsgPort *amanPort;
extern Aport *         amul;

// Structures:
extern struct _PLAYER *    usr, *me, him, *you;
extern struct LS *         lstat, *me2, *you2;
extern struct _ROOM_STRUCT room, *roomtab, *rmtab;  // ptr->table, ptr->a room
extern struct _VERB_STRUCT verb, *vbtab, *vbptr;
extern struct _SLOTTAB     vbslot, *slottab, *stptr;
extern struct _VBTAB       vt, *vtp, *vtabp;
extern struct _RANK_STRUCT rank, *rktab, *ranktab;
extern struct _NTAB_STRUCT nountab, *ntab, *ntabp;
extern struct _OBJ_STRUCT  obj, *obtab, *objtab;
extern struct _OBJ_STATE   state, *statab, *statep;
extern struct _TT_ENT      tt, *ttp, *ttabp;
extern struct _MOB_ENT     mob, *mobp, *mobile;

struct _OBJECT_DESCRIPTIONS {
    char    id[IDL + 1];
    int32_t descrip;  // ptr to descrp in file
};

extern _OBJECT_DESCRIPTIONS objdes, *obdesp;
