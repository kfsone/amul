#include "h/amul.incs.h"
#include "h/amul.vars.h"

int32_t rooms;           // No. of rooms
int32_t ranks;           // No. of ranks
int32_t ttents;          // No. of TT entries
int32_t verbs;           // No. of verbs
int32_t syns;            // No. of synonyms
int32_t msgs;            // No. of user def. msgs
int32_t nouns;           // No. of nouns
int32_t mobs;            // No. of mobiles
int32_t adjs;            // No. of adjectives
int32_t umsgs;           // No. of user messages
int32_t mobchars;        // No. of mobile chars
int     invis;           // rnk 4 invis 2C invis
int     invis2;          // rnk 4 !invis 2C invis
int     minsgo;          // Minimum rank 4 SupaGo
int     rscale, tscale;  // Rank and time scaleing
int32_t obdes;           // No. of obj descrips

// Misc.
FILE *ifp, *ofp1, *ofp2, *ofp3, *afp;  // In and out file ptrs
FILE *ofp4;                            // For lang. table

// Text
char dir[60];         // spc for work dir path
char block[1024];     // 1k block of spare txt
char dmove[IDL + 1];  // Where to RF_CEMETERY to
char adname[41];      // Adventure name
char vername[41];     // Version name etc

// Longs
int32_t flen;  // Length of file

int32_t    ormtablen;  // Length of room table
int32_t    ormtab;     // Room table
int32_t    desctlen;   // Length of desc tab
int32_t    statablen;  // Len of states table
int32_t    adtablen;   // Length of table
int32_t    stlen;      // Length of slottab
int32_t    vtlen;      // Length of VT
int32_t    vtplen;     // Length of VT Parm Tab
int32_t    ttlen;      // Length of TT
int32_t    ttplen;     // Length of param tab
int32_t    synlen;     // Pointer to synonyms
int32_t    synilen;    // Len of syn index
int32_t    umsgil;     // User message indexes
int32_t    umsgl;      // User message length
int32_t *  umsgip;     // pointer to it!
char *     umsgp;      // Actual messages
int32_t *  ttpp;       // Pointer to param tab
int32_t *  vtpp;       // ditto for VT
char *     adtab;      // Adjective table
char *     desctab;    // Object descrips
char *     synp;       // Synonyms
short int *synip;      // Synonyms index
short int *rctab;      // Room count flags

Amiga::MsgPort *amanPort;
Aport *         amul;

// Structures:
_PLAYER *    usr, *me, him, *you;
LS *         lstat, *me2, *you2;
_ROOM_STRUCT room, *roomtab, *rmtab;  // ptr->table, ptr->a room
_VERB_STRUCT verb, *vbtab, *vbptr;
_SLOTTAB     vbslot, *slottab, *stptr;
_VBTAB       vt, *vtp, *vtabp;
_RANK_STRUCT rank, *rktab, *ranktab;
_NTAB_STRUCT nountab, *ntab, *ntabp;
_OBJ_STRUCT  obj, *obtab, *objtab;
_OBJ_STATE   state, *statab, *statep;
_TT_ENT      tt, *ttp, *ttabp;
_MOB_ENT     mob, *mobp, *mobile;

_OBJECT_DESCRIPTIONS objdes, *obdesp;
