/*
 * data.cpp -- This contains the bulk of the variable and constant
 * definitions for the SMUGL system.
 */

#include "actuals.hpp"
#include "defines.hpp"
#include "includes.hpp"
#include "typedefs.hpp"

char dir[130];      // spc for work dir path
char block[1024];   // 1k block of spare txt
char adname[65];    // Adventure name
char logname[130];  // Name of log file

const char *std_flag[] = {  // basic object std_flags
    "inplay", "scenery", "counter", "death",     "light",  "shines",   "flamable", "lit",
    "silent", "hidecre", "hideobj", "sanctuary", "nolook", "peaceful", "scaled",   nullptr
};

const char *rflag[] = {  // Room FLAGS
    "startloc", "randobjs", "small", "noexits", "anteroom", "nogo", nullptr
};

const char *rparam[] = {  // Room PARAMETERS
    "dark",
    "dmove=",
    nullptr
};

const char *obflags1[] = {  // Object flags
    "opens",
    "fire",
    "invis",
    "smell",
    nullptr
};

const char *obparms[] = {  // Object parameters
    "adj=", "start=", "holds=", "put=", "mobile=", "art=", nullptr
};

const char *obflags2[] = {  // Object state flags
    "open", "closed", "weapon", "opaque", "alive", nullptr
};

const char *syntax[NSYNTS] = {  // Syntax "types"
    "none", "any", "noun", "adj", "player", "room", "syn", "text", "verb", "class", "number"
};
// For speeds sake, we remember the size of each of the above
unsigned char syntl[NSYNTS] = { 4, 3, 4, 3, 6, 4, 3, 4, 4, 5, 6 };

const char *obputs[NPUTS] = { "in", "on", "behind", "under" };

const char *prep[NPREP] = { "in", "on", "behind", "under", "from", "with" };

const char *article[NART] = { "a", "an", "the", "some" };

char advfn[] = "Main.CMP";        // Game profile
char plyrfn[] = "PlayerData";     // User Details
char roomsfn[] = "Rooms.CMP";     // Room blocks
char ranksfn[] = "Ranks.CMP";     // Rank details
char ttfn[] = "TTEnt.CMP";        // T.T. Entries
char ttpfn[] = "TTPar.CMP";       // TT Paramters
char langfn[] = "Verbs.CMP";      // Verb blocks
char synsifn[] = "SynIdx.CMP";    // Syns Index
char objsfn[] = "Objects.CMP";    // Detail!!!
char statfn[] = "ObjStates.CMP";  // Obj. States
char umsgifn[] = "TextI.CMP";     // UMsgs Index
char umsgfn[] = "Text.CMP";       // Umsg text
char mobfn[] = "Mobs.CMP";        // Mobile data
char vocifn[] = "VocInd.CMP";     // Vocab Index
char vocfn[] = "Vocab.CMP";       // Vocab Index
char bobfn[] = "Bobs.CMP";        // Bob Index and Containers
char statsfn[] = "Stats.CMP";     // Stats

struct ACTUAL actual[NACTUALS] = {
    { "verb", IWORD + IVERB, WVERB },       { "adj", IWORD + IADJ1, WADJ },
    { "adj1", IWORD + IADJ1, WADJ },        { "noun", IWORD + INOUN1, WNOUN },
    { "noun1", IWORD + INOUN1, WNOUN },     { "player", IWORD + INOUN1, WPLAYER },
    { "player1", IWORD + INOUN1, WPLAYER }, { "text", IWORD + INOUN1, WTEXT },
    { "text1", IWORD + INOUN1, WTEXT },     { "room", IWORD + INOUN1, WROOM },
    { "room1", IWORD + INOUN1, WROOM },     { "number", IWORD + INOUN1, WNUMBER },
    { "number1", IWORD + INOUN1, WNUMBER }, { "adj2", IWORD + IADJ2, WADJ },
    { "noun2", IWORD + INOUN2, WNOUN },     { "player2", IWORD + INOUN2, WPLAYER },
    { "text2", IWORD + INOUN2, WTEXT },     { "room2", IWORD + INOUN2, WROOM },
    { "locate", MEPRM + LOCATE, WNOUN },    { "me", MEPRM + SELF, WPLAYER },
    { "here", MEPRM + HERE, WROOM },        { "myrank", MEPRM + RANK, WNUMBER },
    { "friend", MEPRM + FRIEND, WPLAYER },  { "helper", MEPRM + HELPER, WPLAYER },
    { "enemy", MEPRM + ENEMY, WPLAYER },    { "weapon", MEPRM + WEAPON, WNOUN },
    { "myscore", MEPRM + SCORE, WNUMBER },  { "mysctg", MEPRM + SCTG, WNUMBER },
    { "mystr", MEPRM + STR, WNUMBER },      { "lastroom", MEPRM + LASTROOM, WROOM },
    { "lastdir", MEPRM + LASTDIR, WVERB },  { "lastverb", MEPRM + LASTVB, WVERB }
};
