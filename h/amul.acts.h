#ifndef H_AMUL_ACTS_H
#define H_AMUL_ACTS_H 1
/*

    Actual definitions
    ------------------


 These are pretty important... The 'actuals' allow you the end user to refer
variable elements of the adventure... Such as 'noun1', 'adj2', 'room of object
the box', 'current state of door', 'no. of player carrying' etc...

*/

#include <h/amul.defs.h>

/* Actual flags */

#define IWORD 0x00100000 /* one of users words	*/
#define MEPRM 0x00200000 /* user parameter	*/
#define OBVAL 0x00400000 /* Object weight	*/
#define OBDAM 0x00800000 /* Object damage	*/
#define OBWHT 0x01000000 /* Object weight	*/
#define RAND0 0x02000000 /* Range 0 to <num>	*/
#define RAND1 0x04000000 /* <num/2> to <num*1.5>	*/
#define OBLOC 0x08000000 /* Location of obj	*/
#define PRANK 0x10000000 /* Rank of player	*/

/* 'i' words */
#define IVERB 0
#define IADJ1 1
#define INOUN1 2
#define IPREP 3
#define IADJ2 4
#define INOUN2 5

/* meprms */
#define LOCATE 0    /* Value of locate	*/
#define SELF 1      /* My user no.		*/
#define HERE 2      /* My room		*/
#define RANK 3      /* # of my rank		*/
#define FRIEND 4    /* Who I am helping	*/
#define HELPER 5    /* People helping me	*/
#define ENEMY 6     /* Who I'm fighting	*/
#define WEAPON 7    /* My current weapon	*/
#define SCORE 8     /* My score		*/
#define SCTG 9      /* My score this game	*/
#define STR 10      /* My strength		*/
#define LASTROOM 11 /* Last room I was at!	*/
#define LASTDIR 12  /* Last direction I did	*/
#define LASTVB 13   /* Last verb I did!	*/

/* Numbers */
#define LESS 0x40000000 /* <Beyond max lim>	*/
#define MORE 0x80000000 /* <Beyond max lim>	*/
#define NONE                                                                                       \
    {                                                                                              \
        0, 0, 0                                                                                    \
    }

#ifdef COMPILER
#    define NACTUALS 33 /* No. of actual names	*/
struct ACTUAL           /* Structure for 'actual' data */
{
    char *    name;  /* Name of actual	*/
    int       value; /* The effective no.	*/
    short int wtype; /* Wtype of word	*/
};

struct ACTUAL actual[NACTUALS] = {
        {"verb", IWORD + IVERB, WVERB},        {"adj", IWORD + IADJ1, WADJ},
        {"adj1", IWORD + IADJ1, WADJ},         {"noun", IWORD + INOUN1, WNOUN},
        {"noun1", IWORD + INOUN1, WNOUN},      {"player", IWORD + INOUN1, WPLAYER},
        {"player1", IWORD + INOUN1, WPLAYER},  {"text", IWORD + INOUN1, WTEXT},
        {"text1", IWORD + INOUN1, WTEXT},      {"room", IWORD + INOUN1, WROOM},
        {"room1", IWORD + INOUN1, WROOM},      {"number", IWORD + INOUN1, WNUMBER},
        {"number1", IWORD + INOUN1, WNUMBER},  {"prep", IWORD + IPREP, WPREP},
        {"adj2", IWORD + IADJ2, WADJ},         {"noun2", IWORD + INOUN2, WNOUN},
        {"player2", IWORD + INOUN2, WPLAYER},  {"text2", IWORD + INOUN2, WTEXT},
        {"room2", IWORD + INOUN2, WROOM},      {"locate", MEPRM + LOCATE, WNOUN},
        {"me", MEPRM + SELF, WPLAYER},         {"here", MEPRM + HERE, WROOM},
        {"myrank", MEPRM + RANK, WNUMBER},     {"friend", MEPRM + FRIEND, WPLAYER},
        {"helper", MEPRM + HELPER, WPLAYER},   {"enemy", MEPRM + ENEMY, WPLAYER},
        {"weapon", MEPRM + WEAPON, WNOUN},     {"myscore", MEPRM + SCORE, WNUMBER},
        {"mysctg", MEPRM + SCTG, WNUMBER},     {"mystr", MEPRM + STR, WNUMBER},
        {"lastroom", MEPRM + LASTROOM, WROOM}, {"lastdir", MEPRM + LASTDIR, WVERB},
        {"lastverb", MEPRM + LASTVB, WVERB},
};
#endif

#endif
