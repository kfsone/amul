#ifndef SMUGL_H_AMUL_H
#define SMUGL_H_AMUL_H

// defines for the main program

#define DED 1
#define DEDDED 2

// Messageing shortcuts
#define At amul->type
#define Am amul->msg
#define Af amul->from
#define Ad amul->data
#define Ap amul->ptr
#define Ap1 amul->p1
#define Ap2 amul->p2
#define Ap3 amul->p3
#define Ap4 amul->p4

#define AMt amanp->type
#define AMm amanp->msg
#define AMf amanp->from
#define AMd amanp->data
#define AMp amanp->ptr
#define Apx1 amanp->p1
#define Apx2 amanp->p2
#define Apx3 amanp->p3
#define Apx4 amanp->p4

#define IAt intam->type
#define IAm intam->msg
#define IAf intam->from
#define IAd intam->data
#define IAp intam->ptr

// User Flags
#define ufANSI 0x001  // ANSI bit
#define ufCRLF 0x002  // Add LineFeed
#define ufNVER 0x080  // New Version

#define DLLEN 80       // Default line length
#define DSLEN 24       // Default screen length
#define DFLAGS ufCRLF  // Default = cr/lf ON & auto-redo on

// ENUMS
// Modes - Depicts a line/players current "login" status
enum { OFFLINE, LOGGING, PLAYING, CHATTING };

// Client modes
enum {
    am_USER,  // User client
    am_DAEM,  // Daemon processing client
    am_MOBS,  // Mobiles client
};

// Macros
#define isOINVIS(x) ((obtab + x)->flags & OF_INVIS)
#define isPINVIS(x) ((lstat + x)->flags & PFINVIS)
#define isPSINVIS(x) ((lstat + x)->flags & PFSINVIS)
#define IamINVIS (me2->flags & PFINVIS)
#define IamSINVIS (me2->flags & PFSINVIS)
#define pNAME(x) (usr + x)->name
#define pROOM(x) ((lstat + x)->room)
#define pRANK(x) ((usr + x)->rank)
#define pLIGHT(x) (lstat + x)->light
#define pFLAGS(x) ((lstat + x)->flags)
#define pFLAG(x, y) (pFLAGS(x) & y)
#define pHADLIGHT(x) (lstat + x)->hadlight
#define myRANK me->rank
#define mySCORE me->score
#define myROOM me2->room
#define myNAME me->name
#define myLIGHT me2->light
#define myHLIGHT me2->hadlight
#define hisRANK you->rank
#define hisSCORE you->score
#define hisROOM you2->room
#define hisNAME you->name
#define hisLIGHT you2->light
#define hisHLIGHT you2->hadlight
#define LightHere lit(myROOM)
#define unfreeze                                                                                   \
    Permit();                                                                                      \
    return

#define acp (char *) actptr

#define CP1 actual(*(tt.pptr))
#define CP2 actual(*(tt.pptr + 1))
#define CP3 actual(*(tt.pptr + 2))
#define CP4 actual(*(tt.pptr + 3))
#define TP1 actual(*(tt.pptr + ncop[tt.condition]))
#define TP2 actual(*(tt.pptr + ncop[tt.condition] + 1))
#define TP3 actual(*(tt.pptr + ncop[tt.condition] + 2))
#define TP4 actual(*(tt.pptr + ncop[tt.condition] + 3))
#define AP1 acp(*(tt.pptr + ncop[tt.condition]))
#define AP2 acp(*(tt.pptr + ncop[tt.condition] + 1))
#define AP3 acp(*(tt.pptr + ncop[tt.condition] + 2))
#define AP4 acp(*(tt.pptr + ncop[tt.condition] + 3))
#define tP1 actual(*(tpt))
#define tP2 actual(*(tpt + 1))
#define tP3 actual(*(tpt + 2))
#define tP4 actual(*(tpt + 3))
#define aP1 acp(*(tpt))
#define aP2 acp(*(tpt + 1))
#define aP3 acp(*(tpt + 2))
#define aP4 acp(*(tpt + 3))
#define STATE (objtab->states + (long) objtab->state)
#define State(i) ((obtab + i)->states + (long) (obtab + i)->state)
#define ItsState(it) (it->states + (long) it->state)

#endif