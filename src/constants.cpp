#include "amul.cons.h"
#include "amul.defs.h"
#include "amul.stct.h"
#include "amul.vmop.h"
#include "ptype.h"

// Remember to update amul.defs.h
const char *rflag[NRFLAGS] = { "light",    "dmove",   "startloc",  "randobjs",
                               "dark",     "small",   "death",     "nolook",
                               "silent",   "hide",    "sanctuary", "hideaway",
                               "peaceful", "noexits", "anteroom",  "nogo" };

const char *obflags1[NOFLAGS] = {  // Object flags
    "opens", "scenery", "counter", "flammable", "shines", "fire", "invis", "smell"
};

const char *obparms[NOPARMS] = {  // Object parameters
    "adj=",
    "start=",
    "holds=",
    "put=",
    "npc="
};

const char *obflags2[NSFLAGS] = {  // Object state flags
    "lit", "open", "closed", "weapon", "opaque", "scaled", "alive"
};

const char *obputs[NPUTS] = {  // Object 'put' destinations
    "In",
    "On",
    "Behind",
    "Under"
};

const char *prep[NPREP] = {  // Prepositions
    "in", "on", "behind", "under", "from", "with"
};

#define NO_ARGS                                                                                    \
    0, { 0, 0, 0 }

const VMOP conditions[NCONDS] = {
    // compiler conditionals
    { "&", NO_ARGS },
    { "-", NO_ARGS },
    { "else", NO_ARGS },
    { "always", NO_ARGS },
    { "light", NO_ARGS },
    { "ishere", 1, { PNOUN } },
    { "myrank", 1, { PNUM } },
    { "state", 2, { PNOUN, PNUM } },
    { "mysex", 1, { PSEX } },
    { "lastverb", 1, { PVERB } },
    { "lastdir", 1, { PVERB } },
    { "lastroom", 1, { PROOM } },
    { "asleep", NO_ARGS },
    { "sitting", NO_ARGS },
    { "lying", NO_ARGS },
    { "rand", 2, { PNUM, PNUM } },
    { "rdmode", 1, { -2 } },
    { "onlyuser", NO_ARGS },
    { "alone", NO_ARGS },
    { "inroom", 1, { PROOM } },
    { "opens", 1, { PNOUN } },
    { "gotnothing", NO_ARGS },
    { "carrying", 1, { PNOUN } },
    { "nearto", 1, { PNOUN } },
    { "hidden", NO_ARGS },
    { "cangive", 2, { PNOUN, PPLAYER } },
    { "infl", 2, { PPLAYER, -3 } },
    { "inflicted", 2, { PPLAYER, -3 } },
    { "sameroom", 1, { PPLAYER } },
    { "someonehas", 1, { PNOUN } },
    { "toprank", NO_ARGS },
    { "gota", 2, { PNOUN, PNUM } },
    { "active", 1, { PDAEMON } },
    { "timer", 2, { PDAEMON, PNUM } },
    { "burns", 1, { PNOUN } },
    { "container", 1, { PNOUN } },
    { "empty", 1, { PNOUN } },
    { "objsin", 2, { PNOUN, PNUM } },
    { "->", NO_ARGS },
    { "&>", NO_ARGS },
    { "helping", 1, { PPLAYER } },
    { "gothelp", NO_ARGS },
    { "givinghelp", NO_ARGS },
    { "else>", NO_ARGS },
    { "stat", 3, { -4, PPLAYER, PNUM } },
    { "objinv", 1, { PNOUN } },
    { "fighting", 1, { PPLAYER } },
    { "taskdone", 1, { PNUM } },
    { "cansee", 1, { PPLAYER } },
    { "visibleto", 1, { PPLAYER } },
    { "noun1", 1, { PNOUN } },
    { "noun2", 1, { PNOUN } },
    { "autoexits", NO_ARGS },
    { "debug", NO_ARGS },
    { "full", 2, { -4, PPLAYER } },
    { "time", 1, { PNUM } },
    { "dec", 1, { PNOUN } },
    { "inc", 1, { PNOUN } },
    { "lit", 1, { PNOUN } },
    { "fire", 1, { PNOUN } },
    { "health", 2, { PPLAYER, PNUM } },
    { "magic", 3, { PNUM, PNUM, PNUM } },
    { "spell", 2, { PPLAYER, PNUM } },
    { "in", 2, { PROOM, PNOUN } },
    { "exists", 1, { PNOUN } },
};

const VMOP actions[NACTS] = {
    // actions
    { "goto", 1, { PROOM } },
    { "quit", NO_ARGS },
    { "save", NO_ARGS },
    { "score", 1, { -5 } },
    { "state", 2, { PNOUN, PNUM } },
    { "look", NO_ARGS },
    { "what", NO_ARGS },
    { "where", 1, { PNOUN } },
    { "who", 1, { -5 } },
    { "treatas", 1, { PVERB } },
    { "message", 1, { PUMSG } },
    { "skip", 1, { PNUM } },
    { "endparse", NO_ARGS },
    { "killme", NO_ARGS },
    { "finishparse", NO_ARGS },
    { "abortparse", NO_ARGS },
    { "failparse", NO_ARGS },
    { "wait", 1, { PNUM } },
    { "bleep", 1, { PNUM } },
    { "whereami", NO_ARGS },
    { "send", 2, { PNOUN, PROOM } },
    { "announce", 2, { -1, PUMSG } },
    { "get", 1, { PNOUN } },
    { "drop", 1, { PNOUN } },
    { "invent", NO_ARGS },
    { "reply", 1, { PUMSG } },
    { "changesex", 1, { PPLAYER } },
    { "sleep", NO_ARGS },
    { "wake", NO_ARGS },
    { "sit", NO_ARGS },
    { "stand", NO_ARGS },
    { "lie", NO_ARGS },
    { "rdmode", 1, { -2 } },
    { "reset", NO_ARGS },
    { "action", 2, { -1, PUMSG } },
    { "move", 1, { PROOM } },
    { "travel", NO_ARGS },
    { "announceto", 2, { PROOM, PUMSG } },
    { "actionto", 2, { PROOM, PUMSG } },
    { "announcefrom", 2, { PNOUN, PUMSG } },
    { "actionfrom", 2, { PNOUN, PUMSG } },
    { "tell", 2, { PPLAYER, PUMSG } },
    { "addval", 1, { PNOUN } },
    { "give", 2, { PNOUN, PPLAYER } },
    { "inflict", 2, { PPLAYER, -3 } },
    { "cure", 2, { PPLAYER, -3 } },
    { "summon", 1, { PPLAYER } },
    { "add", 3, { PNUM, -4, PPLAYER } },
    { "sub", 3, { PNUM, -4, PPLAYER } },
    { "checknear", 1, { PNOUN } },
    { "checkget", 1, { PNOUN } },
    { "destroy", 1, { PNOUN } },
    { "recover", 1, { PNOUN } },
    { "start", 2, { PDAEMON, PNUM } },
    { "cancel", 1, { PDAEMON } },
    { "begin", 1, { PDAEMON } },
    { "showtimer", 1, { PDAEMON } },
    { "objannounce", 2, { PNOUN, PUMSG } },
    { "objaction", 2, { PNOUN, PUMSG } },
    { "contents", 1, { PNOUN } },
    { "force", 2, { PPLAYER, PUMSG } },
    { "help", 1, { PPLAYER } },
    { "stophelp", NO_ARGS },
    { "fix", 2, { -4, PPLAYER } },
    { "objinvis", 1, { PNOUN } },
    { "objvis", 1, { PNOUN } },
    { "fight", 1, { PPLAYER } },
    { "flee", NO_ARGS },
    { "log", 1, { PUMSG } },
    { "combat", NO_ARGS },
    { "wield", 1, { PNOUN } },
    { "follow", 1, { PPLAYER } },
    { "lose", NO_ARGS },
    { "stopfollow", NO_ARGS },
    { "exits", NO_ARGS },
    { "settask", 1, { PNUM } },
    { "showtasks", NO_ARGS },
    { "syntax", 2, { PREAL, PREAL } },
    { "setpre", 2, { PPLAYER, PUMSG } },
    { "setpost", 2, { PPLAYER, PUMSG } },
    { "senddaemon", 3, { PPLAYER, PDAEMON, PNUM } },
    { "do", 1, { PVERB } },
    { "interact", 1, { PPLAYER } },
    { "autoexits", 1, { -6 } },
    { "setarr", 2, { PPLAYER, PUMSG } },
    { "setdep", 2, { PPLAYER, PUMSG } },
    { "respond", 1, { PUMSG } },
    { "error", 1, { PUMSG } },
    { "burn", 1, { PNOUN } },
    { "douse", 1, { PNOUN } },
    { "inc", 1, { PNOUN } },
    { "dec", 1, { PNOUN } },
    { "toprank", NO_ARGS },
    { "deduct", 2, { PPLAYER, PNUM } },
    { "damage", 2, { PNOUN, PNUM } },
    { "repair", 2, { PNOUN, PNUM } },
    { "gstart", 2, { PDAEMON, PNUM } },
};

const char *syntaxes[NSYNTS] = {  // syntaxes slot labels
    "none", "any", "noun", "adj", "prep", "player", "room", "syn", "text", "verb", "class", "number"
};

#if !defined(COMPILER)
char mannam[] = "AMUL Server Port";
char plyrfn[] = "Players Data"; /* User Details	*/
#endif
