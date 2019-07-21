#pragma once
/*
** "Actuals"
** Why didn't I call these variables or indirections. An "actual" is a
** value outside the normal legal range (by setting the high bit) which
** indicates "this isn't a real value". Using the stuff defined below,
** the "actual" function can return what it was that you *actually*
** mean't. That's where the name came from. I was only 18 when I started
** writing this, and my grasp of Inglish wasn't so good.
** Some would argue it still isn't. ;-)
*/ 

/* "Actual" flags (to indicate what you actually mean) */
#define	IWORD 0x00400000	// one of users words
#define	MEPRM 0x00800000	// user parameter
#define	OBVAL 0x01000000	// Object weight
#define	OBDAM 0x02000000	// Object damage
#define	OBWHT 0x04000000	// Object weight
#define RAND0 0x08000000	// Range 0 to <num>
#define	RAND1 0x10000000	// <num/2> to <num*1.5>
#define	OBLOC 0x20000000	// Location of obj
#define	PRANK 0x40000000        // Rank of player

/* 'i' words (words relative to the players context, i.e. my verb, etc) */
#define	IVERB    0
#define	IADJ1    1
#define	INOUN1   2
#define	IADJ2    3
#define	INOUN2   4

/* meprms (player-context parameters, such as 'my location') */
#define	LOCATE   0              // Value of locate
#define	SELF     1              // My user no.
#define	HERE     2              // My room
#define	RANK     3              // # of my rank
#define	FRIEND   4              // Who I am helping
#define	HELPER   5              // People helping me
#define	ENEMY    6              // Who Im fighting
#define	WEAPON   7              // My current weapon
#define	SCORE    8              // My score
#define	SCTG     9              // My score this game
#define	STR      10             // My strength
#define	LASTROOM 11             // Last room I was at!
#define	LASTDIR  12             // Last direction I did
#define	LASTVB   13             // Last verb I did!

/* Numbers */
#define LESS     0x40000000     // <Beyond max lim>
#define MORE     0x80000000     // <Beyond max lim>

#define NACTUALS 32             // No. of actual names
struct ACTUAL
{				// Structure for actual data
    string *name;               // Name of actual
    int value;                  // The effective no.
    short wtype;                // Wtype of word
};

extern struct ACTUAL actual[];
