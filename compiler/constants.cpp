#include "amulcom.includes.h"

// remember to update amul.defs.h

const char *rflag[NRFLAGS] = {"light", "dmove", "startloc", "randobjs", "dark", "small", "death", "nolook", "silent",
        "hide", "sanctuary", "hideaway", "peaceful", "noexits"};

const char *obflags1[NOFLAGS] = {"opens", "scenery", "counter", "flammable", "shines", "fire", "invis", "smell"};

const char *obparms[NOPARMS] = {"adj=", "start=", "holds=", "put=", "mobile="};

const char *obflags2[NSFLAGS] = {"lit", "open", "closed", "weapon", "opaque", "scaled", "alive"};

const char *conds[NCONDS] = {"&", "-", "else", "always", "light", "ishere", "myrank", "state", "mysex", "lastverb",
        "lastdir", "lastroom", "asleep", "sitting", "lying", "rand", "rdmode", "onlyuser", "alone", "inroom", "opens",
        "gotnothing", "carrying", "nearto", "hidden", "cangive", "infl", "inflicted", "sameroom", "someonehas",
        "toprank", "gota", "active", "timer", "burns", "container", "empty", "objsin", "->", "&>", "helping", "gothelp",
        "givinghelp", "else>", "stat", "objinv", "fighting", "taskdone", "cansee", "visibleto", "noun1", "noun2",
        "autoexits", "debug", "full", "time", "dec", "inc", "lit", "fire", "health", "magic", "spell", "in"};

// Types of parameters for each condition
const char tcop[NCONDS][3] = {NONE, NONE, NONE, NONE, NONE, CAP_NOUN, 0, 0, CAP_NUM, 0, 0, CAP_NOUN, CAP_NUM, 0,
        CAP_GENDER, 0, 0, CAP_VERB, 0, 0, CAP_VERB, 0, 0, CAP_ROOM, 0, 0, NONE, NONE, NONE, CAP_NUM, CAP_NUM, 0, -2, 0,
        0, NONE, NONE, CAP_ROOM, 0, 0, CAP_NOUN, 0, 0, NONE, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, NONE, CAP_NOUN, CAP_PLAYER,
        0, CAP_PLAYER, -3, 0, CAP_PLAYER, -3, 0, CAP_PLAYER, 0, 0, CAP_NOUN, 0, 0, NONE, CAP_NOUN, CAP_NUM, 0,
        CAP_DAEMON_ID, 0, 0, CAP_DAEMON_ID, CAP_NUM, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN,
        CAP_NUM, 0, NONE, NONE, CAP_PLAYER, 0, 0, NONE, NONE, NONE, -4, CAP_PLAYER, CAP_NUM, CAP_NOUN, 0, 0, CAP_PLAYER,
        0, 0, CAP_NUM, 0, 0, CAP_PLAYER, 0, 0, CAP_PLAYER, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, NONE, NONE, -4,
        CAP_PLAYER, 0, CAP_NUM, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_PLAYER,
        CAP_NUM, 0, CAP_NUM, CAP_NUM, CAP_NUM, CAP_PLAYER, CAP_NUM, 0, CAP_ROOM, CAP_NOUN, 0};

const char *acts[NACTS] = {"quit", "save", "score", "state", "look", "what", "where", "who", "treatas", "message",
        "skip", "endparse", "killme", "finishparse", "abortparse", "failparse", "wait", "bleep", "whereami", "send",
        "announce", "get", "drop", "invent", "reply", "changesex", "sleep", "wake", "sit", "stand", "lie", "rdmode",
        "reset", "action", "move", "travel", "announceto", "actionto", "announcefrom", "actionfrom", "tell", "addval",
        "give", "inflict", "cure", "summon", "add", "sub", "checknear", "checkget", "destroy", "recover", "start",
        "cancel", "begin", "showtimer", "objannounce", "objaction", "contents", "force", "help", "stophelp", "fix",
        "objinvis", "objvis", "fight", "flee", "log", "combat", "wield", "follow", "lose", "stopfollow", "exits",
        "settask", "showtasks", "syntax", "setpre", "setpost", "senddaemon", "do", "interact", "autoexits", "setarr",
        "setdep", "respond", "error", "burn", "douse", "inc", "dec", "toprank", "deduct", "damage", "repair", "gstart"};
// Type of params...
const char tacp[NACTS][3] = {NONE, NONE, -5, 0, 0, CAP_NOUN, CAP_NUM, 0, NONE, NONE, CAP_NOUN, 0, 0, -5, 0, 0, CAP_VERB,
        0, 0, CAP_UMSG, 0, 0, CAP_NUM, 0, 0, NONE, NONE, NONE, NONE, NONE, CAP_NUM, 0, 0, CAP_NUM, 0, 0, NONE, CAP_NOUN,
        CAP_ROOM, 0, -1, CAP_UMSG, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, NONE, CAP_UMSG, 0, 0, CAP_PLAYER, 0, 0, NONE,
        NONE, NONE, NONE, NONE, -2, 0, 0, NONE, -1, CAP_UMSG, 0, CAP_ROOM, 0, 0, NONE, CAP_ROOM, CAP_UMSG, 0, CAP_ROOM,
        CAP_UMSG, 0, CAP_NOUN, CAP_UMSG, 0, CAP_NOUN, CAP_UMSG, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_NOUN, 0, 0, CAP_NOUN,
        CAP_PLAYER, 0, CAP_PLAYER, -3, 0, CAP_PLAYER, -3, 0, CAP_PLAYER, 0, 0, CAP_NUM, -4, CAP_PLAYER, CAP_NUM, -4,
        CAP_PLAYER, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_DAEMON_ID, CAP_NUM, 0,
        CAP_DAEMON_ID, 0, 0, CAP_DAEMON_ID, 0, 0, CAP_DAEMON_ID, 0, 0, CAP_NOUN, CAP_UMSG, 0, CAP_NOUN, CAP_UMSG, 0,
        CAP_NOUN, 0, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_PLAYER, 0, 0, NONE, -4, CAP_PLAYER, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0,
        0, CAP_PLAYER, 0, 0, NONE, CAP_UMSG, 0, 0, NONE, CAP_NOUN, 0, 0, CAP_PLAYER, 0, 0, NONE, NONE, NONE, CAP_NUM, 0,
        0, NONE, CAP_REAL, CAP_REAL, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_PLAYER, CAP_DAEMON_ID,
        CAP_NUM, CAP_VERB, 0, 0, CAP_PLAYER, 0, 0, -6, 0, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_PLAYER, CAP_UMSG, 0, CAP_UMSG,
        0, 0, CAP_UMSG, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, CAP_NOUN, 0, 0, NONE, CAP_PLAYER, CAP_NUM,
        0, CAP_NOUN, CAP_NUM, 0, CAP_NOUN, CAP_NUM, 0, CAP_DAEMON_ID, CAP_NUM, 0};

const char *syntax[NSYNTS] = {
        "none", "any", "noun", "adj", "prep", "player", "room", "syn", "text", "verb", "class", "number"};
// Length of --^
const short int syntl[NSYNTS] = {4, 3, 4, 3, 4, 6, 4, 3, 4, 4, 5, 6};

// Check to see if s is a room flag
int
isrflag(const char *s)
{
    for (int x = 0; x < NRFLAGS; x++) {
        if (strcmp(s, rflag[x]) == 0)
            return x;
    }
    return -1;
}

// Is it a FIXED object flag?
int
isoflag1(const char *s)
{
    for (int i = 0; i < NOFLAGS; i++) {
        if (strcmp(obflags1[i], s) == NULL)
            return i;
    }
    return -1;
}

// Is it an object parameter?
int
isoparm()
{
    for (int i = 0; i < NOPARMS; i++) {
        if (striplead(obparms[i], Word))
            return i;
    }
    return -1;
}

// Is it a state flag?
int
isoflag2(const char *s)
{
    for (int i = 0; i < NSFLAGS; i++) {
        if (strcmp(obflags2[i], s) == NULL)
            return i;
    }
    return -1;
}

int
iscond(const char *s)
{
    for (int i = 0; i < NCONDS; i++) {
        if (strcmp(conds[i], s) == 0)
            return i;
    }
    return -1;
}

int
isact(const char *s)
{
    for (int i = 0; i < NACTS; i++) {
        if (strcmp(acts[i], s) == 0)
            return i;
    }
    return -1;
}