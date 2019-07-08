/*

    Actual definitions
    ------------------


These are pretty important... The 'actuals' allow the end user to refer to
variable elements of the adventure... Such as 'noun1', 'adj2', 'room of object
the box', 'current state of door', 'no. of player carrying' etc...

*/

// Actual flags

#define IWORD 0x00100000  // one of users words
#define MEPRM 0x00200000  // user parameter
#define OBVAL 0x00400000  // Object weight
#define OBDAM 0x00800000  // Object damage
#define OBWHT 0x01000000  // Object weight
#define RAND0 0x02000000  // Range 0 to <num>
#define RAND1 0x04000000  // <num/2> to <num*1.5>
#define OBLOC 0x08000000  // Location of obj
#define PRANK 0x10000000  // Rank of player

// 'i' words
#define IVERB 0
#define IADJ1 1
#define INOUN1 2
#define IPREP 3
#define IADJ2 4
#define INOUN2 5

// meprms
#define LOCATE 0     // Value of locate
#define SELF 1       // My user no.
#define HERE 2       // My room
#define RANK 3       // # of my rank
#define FRIEND 4     // Who I am helping
#define HELPER 5     // People helping me
#define ENEMY 6      // Who I'm fighting
#define WEAPON 7     // My current weapon
#define SCORE 8      // My score
#define SCTG 9       // My score this game
#define STR 10       // My strength
#define LASTROOM 11  // Last room I was at!
#define LASTDIR 12   // Last direction I did
#define LASTVB 13    // Last verb I did!

// Numbers
#define LESS 0x40000000  // <Beyond max lim>
#define MORE 0x80000000  // <Beyond max lim>

struct ACTUAL  // Structure for actual data
{
    const char* name;   // Name of actual
    int         value;  // The effective no.
    short int   wtype;  // Wtype of word
};

extern const ACTUAL actual[NACTUALS];
