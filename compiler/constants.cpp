#include "amulcom.includes.h"

#include "h/amul.instructions.h"

// remember to update amul.defs.h

const char *rflag[NRFLAGS] = {  // room flags
        "light",  "dmove",  "startloc", "randobjs",  "dark",     "small",    "death",
        "nolook", "silent", "hide",     "sanctuary", "hideaway", "peaceful", "noexits"};

const char *obflags1[NOFLAGS] = {  // object flags
        "opens", "scenery", "counter", "flammable", "shines", "fire", "invis", "smell"};

const char *obparms[NOPARMS] = {  // object parameters
        "adj=", "start=", "holds=", "put=", "mobile="};

const char *obflags2[NSFLAGS] = {  // object state-flags
        "lit", "open", "closed", "weapon", "opaque", "scaled", "alive"};

const char *syntax[NSYNTS] = {"none", "any", "noun", "adj",  "prep",  "player",
                              "room", "syn", "text", "verb", "class", "number"};
// Length of --^
const short int syntl[NSYNTS] = {4, 3, 4, 3, 4, 6, 4, 3, 4, 4, 5, 6};

// Check to see if s is a room flag
int
isrflag(const char *s) noexcept
{
    for (int x = 0; x < NRFLAGS; x++) {
        if (strcmp(s, rflag[x]) == 0)
            return x;
    }
    return -1;
}

// Is it a FIXED object flag?
int
isoflag1(const char *s) noexcept
{
    for (int i = 0; i < NOFLAGS; i++) {
        if (strcmp(obflags1[i], s) == 0)
            return i;
    }
    return -1;
}

// Is it an object parameter?
int
isoparm() noexcept
{
    for (int i = 0; i < NOPARMS; i++) {
        if (striplead(obparms[i], Word))
            return i;
    }
    return -1;
}

// Is it a state flag?
int
isoflag2(const char *s) noexcept
{
    for (int i = 0; i < NSFLAGS; i++) {
        if (strcmp(obflags2[i], s) == 0)
            return i;
    }
    return -1;
}

condid_t
getCondition(std::string token) noexcept
{
    for (int i = 0; i < NCONDS; i++) {
        if (conditions[i].name == token)
            return i;
    }
    return -1;
}

actionid_t
getAction(std::string token) noexcept
{
    for (int i = 0; i < NACTS; i++) {
        if (actions[i].name == token)
            return i;
    }
    return -1;
}
