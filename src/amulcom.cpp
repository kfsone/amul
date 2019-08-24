/*
          ####        ###     ###  ##     ##  ###
         ##  ##        ###   ###   ##     ##  ##            Amiga
        ##    ##       #########   ##     ##  ##            Multi
        ##    ##       #########   ##     ##  ##            User
        ########  ---  ## ### ##   ##     ##  ##            adventure
        ##    ##       ##     ##    ##   ##   ##            Language
       ####  ####     ####   ####   #######   ########


amulcom.cpp :: AMUL Compiler. Copyright (C) KingFisher Software 1990-2019.
*/

#include "amulcom.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"

#include "filesystem.h"
#include "filesystem.inl.h"
#include "logging.h"
#include "modules.h"
#include "sourcefile.h"
#include "sourcefile.log.h"
#include "svparse.h"
#include "system.h"

#include <h/amigastubs.h>
#include <h/amul.acts.h>
#include <h/amul.cons.h>
#include <h/amul.defs.h>
#include <h/amul.gcfg.h>
#include <h/amul.msgs.h>
#include <h/amul.stct.h>
#include <h/amul.strs.h>
#include <h/amul.test.h>
#include <h/amul.vars.h>
#include <h/amul.vmop.h>
#include <h/amul.xtra.h>
#include <h/amulcom.h>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <map>
#include <string>
#include <sys/stat.h>
#include <vector>

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables

std::string compilerVersion { VERS " (" DATE ")" };

/* Compiler specific variables... */

size_t FPos;       /* Used during TT/Lang writes	*/
char   Word[64];   /* For internal use only <grin>	*/
int    proc;       /* What we are processing	*/
char * syntab;     /* Synonym table, re-read	*/
long   wizstr;     /* Wizards strength		*/

char block[1024];  // scratch pad

_OBJ_STRUCT *obtab2, obj2;

// counters
GameConfig g_gameConfig;

FILE *ifp, *ofp1, *ofp2, *ofp3, *ofp4, *ofp5, *afp;

bool exiting;

std::map<std::string, std::string> s_dmoveLookups;
std::vector<_ROOM_STRUCT> s_rooms;
std::map<std::string, roomid_t> s_roomIndex;

_ROOM_STRUCT *c_room;

///////////////////////////////////////////////////////////////////////////////////////////////////
//

void
CloseOutFiles()
{
    CloseFile(&ofp1);
    CloseFile(&ofp2);
    CloseFile(&ofp3);
    CloseFile(&ofp4);
    CloseFile(&ofp5);
    CloseFile(&afp);
}

void
checkErrorCount()
{
    if (GetLogErrorCount() > 30)
        LogFatal("Terminating due to ", GetLogErrorCount(), " errors");
}

bool
consumeComment(const char *text)
{
    if (*text == '*')
        LogInfo(text);
    return isCommentChar(*text);
}

char *
getword(char *from)
{
    char *to = Word;
    *to = 0;
    from = skipspc(from);
    for (const char *end = Word + sizeof(Word) - 1; *from && to < end; ++to, ++from) {
        char c = *to = tolower(*from);
        if (c == ' ' || c == '\t' || c == ' ') {
            c = *to = 0;
            break;
        }
    }

    // overflowed 'Word', add a trailing '\0' and drain remaining characters.
    *to = 0;
    for (;;) {
        switch (*from) {
        case 0:
        case ';':
        case ' ':
        case '\t':
            goto broke;
        default:
            ++from;
        }
    }

broke:
    return from;
}

/* Find the next real stuff in file */
bool
nextc(bool required)
{
    char c;
    do {
        while ((c = fgetc(ifp)) != EOF && isspace(c))
            ;
        if (c == ';' || c == '*')
            fgets(block, sizeof(block), ifp);
        if (c == '*')
            LogNote(block);
    } while (c != EOF && (c == '*' || c == ';' || isspace(c)));
    if (c == EOF && required) {
        LogFatal("File contained no data");
    }
    if (c == EOF)
        return false;
    ungetc(c, ifp);
    return true;
}

void
fatalOp(std::string_view verb, std::string_view noun)
{
    LogError("Can't ", verb, " ", noun);
}

FILE *
OpenGameFile(const char *filename, const char *mode)
{
    char filepath[MAX_PATH_LENGTH];
    safe_gamedir_joiner(filename);
    FILE *fp = fopen(filepath, mode);
    if (!fp)
        fatalOp("open", filepath);
    return fp;
}

/* Open file for reading */
void
fopenw(const char *filename)
{
    FILE **fpp = nullptr;
    if (!ofp1)
        fpp = &ofp1;
    else if (!ofp2)
        fpp = &ofp2;
    else if (!ofp3)
        fpp = &ofp3;
    else if (!ofp4)
        fpp = &ofp4;
    else if (!ofp5)
        fpp = &ofp5;
    else
        fatalOp("select", "file descriptor");
    *fpp = OpenGameFile(filename, "wb");
}

/* Open file for appending */
void
fopena(const char *filename)
{
    if (afp != nullptr)
        fclose(afp);
    afp = OpenGameFile(filename, "ab+");
}

/* Open file for reading */
void
fopenr(const char *filename)
{
    if (ifp != nullptr)
        fclose(ifp);
    ifp = OpenGameFile(filename, "rb");
}

int
checkedfread(void *data, size_t objSize, size_t objCount, FILE *fp)
{
    size_t read = fread(data, objSize, objCount, fp);
    if (read != objCount) {
        LogFatal("Wrong write count: expected ", objCount, ", got ", read);
    }
    return read;
}

int
checkedfwrite(void *data, size_t objSize, size_t objCount, FILE *fp)
{
    size_t written = fwrite(data, objSize, objCount, fp);
    if (written != objCount) {
        LogFatal("Wrong write count: expected ", objCount, ", got ", written);
    }
    return written;
}

void
skipblock()
{
    char c, lc;

    lc = 0;
    c = '\n';
    while (c != EOF && !(lc == '\n' && isEol(c))) {
        lc = c;
        c = fgetc(ifp);
    }
}

void
tidy(char *ptr)
{
    char *lastNonSpace = nullptr;
    while (*ptr) {
        if (*ptr == '\t' || *ptr == '\r')
            *ptr = ' ';
        if (!isspace(*ptr))
            lastNonSpace = ptr;
        ++ptr;
    }
    if (lastNonSpace)
        *(lastNonSpace + 1) = 0;
}

char *
getTidyBlock(FILE *fp)
{
    for (;;) {
        if (!fgets(block, sizeof(block), fp))
            return nullptr;

        tidy(block);

        char *p = skipspc(block);
        if (!consumeComment(p) && *p)
            return p;
    }
}

int
isVerb(const char *s)
{
    if (strlen(s) > IDL) {
        printf("Invalid verb ID (too long): %s", s);
        return -1;
    }

    vbptr = vbtab;
    for (size_t i = 0; i < g_gameConfig.numVerbs; i++, vbptr++) {
        if (stricmp(vbptr->id, s) == 0)
            return i;
    }
    if (stricmp(g_verb.id, s) == 0)
        return g_gameConfig.numVerbs + 1;

    return -1;
}

/* Return size of current file */
long
filesize()
{
    long now, s;

    now = ftell(ifp);
    fseek(ifp, 0, 2L);
    s = ftell(ifp) - now;
    fseek(ifp, now, 0L);
    return s;
}

/* Check to see if s is a room flag */
int
isRoomFlag(std::string_view name)
{
    for (int i = 0; i < NRFLAGS; i++) {
        if (name == rflag[i])
            return i;
	}
    return -1;
}

roomid_t
isRoomName(std::string_view token) noexcept
{
    std::string rmname { token };
    StringLower(rmname);
    if (auto it = s_roomIndex.find(rmname); it != s_roomIndex.end()) {
        return roomid_t(std::distance(s_roomIndex.begin(), it));
    }
    return -1;
}

/* Is it a FIXED object flag? */
int
isoflag1(const char *token)
{
    for (int i = 0; i < NOFLAGS; i++)
        if (strcmp(obflags1[i], token) == 0)
            return i;
    return -1;
}

/* Is it an object parameter? */
int
isoparm(char *token)
{
    for (int i = 0; i < NOPARMS; i++)
        if (striplead(obparms[i], token))
            return i;
    return -1;
}

/* Is it a state flag? */
int
isoflag2(const char *token)
{
    for (int i = 0; i < NSFLAGS; i++)
        if (strcmp(obflags2[i], token) == 0)
            return i;
    return -1;
}

void
set_adj()
{
    if (strlen(Word) > IDL || strlen(Word) < 3) {
        LogFatal("Invalid adjective (length): ", Word);
    }
    if (g_gameConfig.numAdjectives == 0) {
        ZeroPad(Word, sizeof(Word));
        checkedfwrite(Word, IDL + 1, 1, afp);
        obj2.adj = 0;
        g_gameConfig.numAdjectives++;
        return;
    }
    fseek(afp, 0L, 0); /* Move to beginning */
    int i = 0;
    do {
        char id[IDL + 1];
        if (fread(id, IDL + 1, 1, afp) != 1)
            break;
        if (strcmp(Word, id) == 0) {
            obj2.adj = i;
            return;
        }
        i++;
    } while (!feof(afp));

    fseek(afp, 0L, 2); /* Move to end! */
    ZeroPad(Word, sizeof(Word));
    checkedfwrite(Word, IDL + 1, 1, afp); /* Add this adjective */
    obj2.adj = g_gameConfig.numAdjectives++;
}

[[noreturn]] void
objectInvalid(const char *s)
{
    LogFatal("Object #", g_gameConfig.numObjects + 1, ": ", obj2.id, ": invalid ", s, ": ", Word);
}

void
set_start()
{
    if (!isdigit(Word[0]))
        objectInvalid("start state");
    obj2.state = atoi(Word);
    if (obj2.state < 0 || obj2.state > 100)
        objectInvalid("start state");
}

void
set_holds()
{
    if (!isdigit(Word[0]))
        objectInvalid("holds= value");
    obj2.contains = atoi(Word);
    if (obj2.contains < 0 || obj2.contains > 1000000)
        objectInvalid("holds= state");
}

void
set_put()
{
    for (int i = 0; i < NPUTS; i++)
        if (stricmp(obputs[i], Word) == 0) {
            obj2.putto = i;
            return;
        }
    objectInvalid("put= flag");
}

void
set_mob()
{
    for (size_t i = 0; i < g_gameConfig.numMobPersonas; i++)
        if (stricmp(mobp[i].id, Word) == 0) {
            obj2.mobile = i;
            return;
        }
    objectInvalid("mobile= flag");
}

int
iscond(const char *s)
{
    for (int i = 0; i < NCONDS; i++)
        if (strcmp(conditions[i].name, s) == 0)
            return i;
    return -1;
}

int
isact(const char *s)
{
    for (int i = 0; i < NACTS; i++)
        if (strcmp(actions[i].name, s) == 0)
            return i;
    return -1;
}

int
isprep(const char *s)
{
    for (int i = 0; i < NPREP; i++)
        if (strcmp(s, prep[i]) == 0)
            return i;
    return -1;
}

bool
checkRankLine(const char *p)
{
    if (*p == 0) {
        LogError("Rank line ", g_gameConfig.numRanks, " incomplete");
        return false;
    }
    return true;
}

[[noreturn]] void
stateInvalid(const char *s)
{
    LogFatal("Object #", g_gameConfig.numObjects + 1, ": ", obj2.id, ": invalid ", s,
             " state line: ", block);
}

int
getObjectDescriptionID(const char *text)
{
    if (stricmp(text, "none") == 0)
        return -2;

    stringid_t id = -1;
    LookupTextString(text, &id);
    return id;
}

int
isNoun(const char *s)
{
    /// TODO: This should check the noun table...
    if (stricmp(s, "none") == 0)
        return -2;
    for (size_t i = 0; i < g_gameConfig.numObjects; i++)
        if (stricmp(s, obtab2[i].id) == 0)
            return i;
    return -1;
}

int
isContainer(const char *s)
{
    for (size_t i = 0; i < g_gameConfig.numObjects; i++)
        if (stricmp(s, obtab2[i].id) == 0 && obtab2[i].contains > 0)
            return i;
    return -1;
}

/* Room or container */
int
isloc(const char *s)
{
    int i;

    if ((i = isRoomName(s)) != -1)
        return i;
    if ((i = isContainer(s)) == -1) {
        if (isNoun(s) == -1)
            LogError("Invalid object start location: ", s);
        else
            LogError("Tried to start '", obj2.id, "' in non-container: ", s);
        return -1;
    }

    return -(INS + i);
}

char *
precon(char *s)
{
    while (*s) {
        s = skipspc(s);
        if (canSkipLead("if ", &s))
            continue;
        if (canSkipLead("the ", &s))
            continue;
        if (canSkipLead("i ", &s))
            continue;
        if (canSkipLead("am ", &s))
            continue;
        break;
    }
    return s;
}

char *
preact(char *s)
{
    while (*s) {
        s = skipspc(s);
        if (canSkipLead("then ", &s))
            continue;
        if (canSkipLead("goto ", &s))
            continue;
        if (canSkipLead("go to ", &s))
            continue;
        if (canSkipLead("set ", &s))
            continue;
        break;
    }
    return s;
}

/* Check a numeric arguments */
long
chknum(const char *s)
{
    long n;

    /// TODO: This tries to cope with, e.g. ">#rank", except
    /// this can't work because we're using atoi here...
    /* Is this a variable? (less than, greater than, etc) */
    if (*s == '>' || *s == '<' || *s == '-' || *s == '=')
        n = atoi(s + 1);
    else if (!isdigit(*s) && !isdigit(*(s + 1)))
        return -1000001;
    else
        n = atoi(s);
    if (n >= 1000000) {
        LogError("Number too large: ", s);
        return -1000001;
    }
    if (*s == '-')
        return (long)-n;
    if (*s == '>')
        return (long)(n | LESS);
    if (*s == '<')
        return (long)(n | MORE);
    return n;
}

static const char *optionalPrefixes[] = {  // noise we can skip
    "the ",  "of ",  "are ", "is ",  "has ", "next ", "with ", "to ", "set ",
    "from ", "for ", "by ",  "and ", "was ", "i ",    "am ",   "as ", nullptr
};

char *
skipOptionalPrefixes(char *p)
{
    while (*p) {
        p = skipspc(p);
        bool match = false;
        for (int i = 0; optionalPrefixes[i]; ++i) {
            if (canSkipLead(optionalPrefixes[i], &p)) {
                match = true;
                break;
            }
        }
        if (!match)
            break;
    }
    return p;
}

int
isgen(char c)
{
    if (c == 'M')
        return 0;
    if (c == 'F')
        return 1;
    return -1;
}

static const char *announceTypes[MAX_ANNOUNCE_TYPE] = {"global", "everyone", "outside", "here",
                                                       "others", "all",      "cansee",  "notsee"};
int
announceType(const char *s)
{
    for (int i = 0; i < MAX_ANNOUNCE_TYPE; ++i) {
        if (stricmp(s, announceTypes[i]) == 0)
            return i;
    }
    LogError("Invalid announcement target: ", s);
    return -1;
}

int
rdmode(char c)
{
    if (c == 'R')
        return RDRC;
    if (c == 'V')
        return RDVB;
    if (c == 'B')
        return RDBF;
    return -1;
}

int
spell(const char *s)
{
    if (strcmp(s, "glow") == 0)
        return SGLOW;
    if (strcmp(s, "invis") == 0)
        return SINVIS;
    if (strcmp(s, "deaf") == 0)
        return SDEAF;
    if (strcmp(s, "dumb") == 0)
        return SDUMB;
    if (strcmp(s, "blind") == 0)
        return SBLIND;
    if (strcmp(s, "cripple") == 0)
        return SCRIPPLE;
    if (strcmp(s, "sleep") == 0)
        return SSLEEP;
    if (strcmp(s, "sinvis") == 0)
        return SSINVIS;
    return -1;
}

int
is_stat(const char *s)
{
    if (strcmp(s, "sctg") == 0)
        return STSCTG;
    if (strncmp(s, "sc", 2) == 0)
        return STSCORE;
    if (strncmp(s, "poi", 3) == 0)
        return STSCORE;
    if (strncmp(s, "str", 3) == 0)
        return STSTR;
    if (strncmp(s, "stam", 4) == 0)
        return STSTAM;
    if (strncmp(s, "dext", 4) == 0)
        return STDEX;
    if (strncmp(s, "wis", 3) == 0)
        return STWIS;
    if (strncmp(s, "exp", 3) == 0)
        return STEXP;
    if (strcmp(s, "magic") == 0)
        return STMAGIC;
    return -1;
}

int
bvmode(char c)
{
    if (c == 'V')
        return TYPEV;
    if (c == 'B')
        return TYPEB;
    return -1;
}

stringid_t
getTextString(char *s, bool prefixed)
{
    if (prefixed) {
        s = skipspc(s);
        s = skiplead("msgid=", s);
        s = skiplead("msgtext=", s);
    }

    stringid_t id = -1;
    if (*s == '\"' || *s == '\'') {
        char *  start = s + 1;
        char *  end = strstop(start, *s);
        std::string_view text { start, size_t(end - start) };
        error_t err = AddTextString(text, true, &id);
        if (err != 0)
            LogFatal("Unable to add string literal: ", err);
    } else {
        getword(s);
        error_t err = LookupTextString(Word, &id);
        if (err != 0 && err != ENOENT)
            LogFatal("Error looking up string (", err, "): ", s);
    }

    return id;
}

int
onoff(const char *p)
{
    if (stricmp(p, "on") == 0 || stricmp(p, "yes") == 0)
        return 1;
    return 0;
}

/* Note about matching actuals...

Before agreeing a match, remember to check that the relevant slot isn't
set to NONE.
Variable N is a wtype... If the phrases 'noun', 'noun1' or 'noun2' are used,
instead of matching the phrases WTYPE with n, match the relevant SLOT with
n...

So, if the syntax line is 'verb text player' the command 'tell noun2 text'
will call isactual with *s=noun2, n=WPLAYER.... is you read the 'actual'
structure definition, 'noun2' is type 'WNOUN'. WNOUN != WPLAYER, HOWEVER
the slot for noun2 (vbslot.wtype[4]) is WPLAYER, and this is REALLY what the
user is referring too.							     */

/* Get actual value! */
int
actualval(const char *s, int n)
{
    int i;

    // you can prefix runtime-variable references with these characters to modify the
    // resulting value, e.g. choosing a random number based on your rank.
    /// TODO: replace with something akin to SQL aggregate functions.
    /// TODO: rand(myrank)
    if (n != PREAL && strchr("?%^~`*#", *s)) {
        // These can only be applied to numbers except '*' which can be applied to rooms
        if (n != WNUMBER && !(n == WROOM && *s == '*'))
            return -1;
        // calculate random numbers based on the parameter
        if (*s == '~')
            return RAND0 + atoi(s + 1);
        if (*s == '`')
            return RAND1 + atoi(s + 1);
        // then it's a modifier: recurse and add our flags.
        i = actualval(s + 1, PREAL);
        if (i == -1)
            return -1;
        // The remaining prefixes can only be attached to properties of the player.
        if (*s == '#' && (i & IWORD || (i & MEPRM && i & (SELF | FRIEND | HELPER | ENEMY))))
            return PRANK + i;
        if ((i & IWORD) == 0)
            return -1;
        if (*s == '?')
            return OBVAL + i;
        if (*s == '%')
            return OBDAM + i;
        if (*s == '^')
            return OBWHT + i;
        if (*s == '*')
            return OBLOC + i;
        if (*s == '#')
            return PRANK + i;
        return -1;
    }
    if (!isalpha(*s))
        return -2;
    for (i = 0; i < NACTUALS; i++) {
        if (stricmp(s, actual[i].name) != 0)
            continue;
        /* If its not a slot label, and the wtypes match, we's okay! */
        if (!(actual[i].value & IWORD))
            return (actual[i].wtype == n || n == PREAL) ? actual[i].value : -1;

        /* Now we know its a slot label... check which: */
        switch (actual[i].value - IWORD) {
        case IVERB: /* Verb */
            if (n == PVERB || n == PREAL)
                return actual[i].value;
            return -1;
        case IADJ1: /* Adjective #1 */
            if (vbslot.wtype[0] == n)
                return actual[i].value;
            if (*(s + strlen(s) - 1) != '1' && vbslot.wtype[3] == n)
                return IWORD + IADJ2;
            if (n == PREAL)
                return actual[i].value;
            return -1;
        case INOUN1: /* noun 1 */
            if (vbslot.wtype[1] == n)
                return actual[i].value;
            if (*(s + strlen(s) - 1) != '1' && vbslot.wtype[4] == n)
                return IWORD + INOUN2;
            if (n == PREAL)
                return actual[i].value;
            return -1;
        case IADJ2:
            return (vbslot.wtype[3] == n || n == PREAL) ? actual[i].value : -1;
        case INOUN2:
            return (vbslot.wtype[4] == n || n == PREAL) ? actual[i].value : -1;
        default:
            return -1; /* Nah... Guru instead 8-) */
        }
    }
    return -2; /* It was no actual! */
}

[[noreturn]] void
badParameter(
        const VMOP *op, size_t paramNo, const char *category, const char *issue, const char *token)
{
    char msg[256];
    if (token) {
        snprintf(msg, sizeof(msg), "%s: '%s'", issue, token);
        issue = msg;
    }
    LogFatal(proc ? "verb" : "room", "=", proc ? g_verb.id : c_room->id, ": ",
            category, "=", op->name, ": parameter#", paramNo + 1, ": ", issue);
}

// Check the parameters accompanying a vmop
char *
checkParameter(char *p, const VMOP *op, size_t paramNo, const char *category, FILE *fp)
{
    int32_t      value = -1;
    const int8_t parameterType = op->parameters[paramNo];

    p = skipOptionalPrefixes(p);
    if (*p == 0) {
        badParameter(op, paramNo, category, "Unexpected end of arguments", nullptr);
    }
    static char token[4096];
    char *end = nullptr;
    if (*p != '\"' && *p != '\'') {
        end = strstop(p, ' ');
        WordCopier(token, p, end);
    } else {
        // Subsequent parsing will want to see the opening quote character
        char quote = *p;
        if (!*p || isEol(*p))
            badParameter(op,
                         paramNo,
                         category,
                         "Unexpected end of string (missing close quote?)",
                         nullptr);
        end = strstop(p + 1, quote);
        StrCopier(token, p, end);
        if (*end == quote)
            ++end;
    }
    p = end;

    /* Processing lang tab? */
    if ((parameterType >= 0 && parameterType <= 10) || parameterType == PREAL) {
        value = actualval(token, parameterType);
        if (value == -1) {
            /* it was an actual, but wrong type */
            badParameter(op, paramNo, category, "Invalid syntax slot label", token);
        }
        if (value != -2)
            goto write;
    }
    switch (parameterType) {
    case -6:
        value = onoff(token);
        break;
    case -5:
        value = bvmode(toupper(*token));
        break;
    case -4:
        value = is_stat(token);
        break;
    case -3:
        value = spell(token);
        break;
    case -2:
        value = rdmode(toupper(*token));
        break;
    case -1:
        value = announceType(token);
        break;
    case PROOM:
        value = isRoomName(token);
        break;
    case PVERB:
        value = isVerb(token);
        break;
    case PADJ:
        break;
    case PREAL:
    case PNOUN:
        value = isNoun(token);
        break;
    case PUMSG:
        value = getTextString(token, true);
        break;
    case PNUM:
        value = chknum(token);
        break;
    case PRFLAG:
        value = isRoomFlag(token);
        break;
    case POFLAG:
        value = isoflag1(token);
        break;
    case PSFLAG:
        value = isoflag2(token);
        break;
    case PSEX:
        value = isgen(toupper(*token));
        break;
    case PDAEMON:
        if ((value = isVerb(token)) == -1 || *token != '.')
            value = -1;
        break;
    default: {
        if (!(proc == 1 && parameterType >= 0 && parameterType <= 10)) {
            char param[32];
            snprintf(param, sizeof(param), "%d", parameterType);
            badParameter(op, paramNo, category, "INTERNAL ERROR: Unhandled parameter type", param);
        }
    }
    }
    if (parameterType == PREAL && value == -2)
        value = -1;
    else if (((value == -1 || value == -2) && parameterType != PNUM) || value == -1000001) {
        badParameter(op, paramNo, category, "Invalid/unrecognized value for position", token);
    }
write:
    FPos += checkedfwrite(&value, sizeof(value), 1, fp);
    return *p ? skipspc(p + 1) : p;
}

char *
checkParameters(char *p, const VMOP *op, FILE *fp, const char *category)
{
    for (size_t i = 0; i < op->parameterCount; i++) {
        if (!(p = checkParameter(p, op, i, category, fp)))
            return nullptr;
    }
    return p;
}

char *
checkActionParameters(char *p, const VMOP *op, FILE *fp)
{
    return checkParameters(p, op, fp, "action");
}

char *
checkConditionParameters(char *p, const VMOP *op, FILE *fp)
{
    return checkParameters(p, op, fp, "condition");
}

static void
chae_err(std::string_view word)
{
    LogError("Verb: ", g_verb.id, ": Invalid '#CHAE' flag: ", word);
}

/* Set the VT slots */
void
setslots(unsigned char i)
{
    vbslot.wtype[0] = WANY;
    vbslot.wtype[1] = i;
    vbslot.wtype[2] = i;
    vbslot.wtype[3] = WANY;
    vbslot.wtype[4] = i;
    vbslot.slot[0] = vbslot.slot[1] = vbslot.slot[2] = vbslot.slot[3] = vbslot.slot[4] = WANY;
}

/* Is 'text' a ptype */
int
iswtype(char *s)
{
    for (int i = 0; i < NSYNTS; i++) {
        char *end = s;
        if (!canSkipLead(syntax[i], &end))
            continue;
        switch (*end) {
        // exact match
        case 0:
            *s = 0;
            break;
        // x=y
        case '=':
            memmove(s, end + 1, strlen(end));
            break;
        // partial match, ignore
        default:
            continue;
        }
        // this makes 'WNONE' become -1 and 'WANY' become 0. Ick.
        return i - 1;
    }
    return -3;  /// TODO: INVALID_WTYPE
}

/* Declare a PROBLEM, and which verb its in! */
[[noreturn]] void
vbprob(const char *s, const char *s2)
{
    LogFatal("Verb: ", g_verb.id, " line: '", s2, "': ", s);
}

void
mobmis(const char *s)
{
    LogError("Mobile: ", mob.id, ": missing field: ", s);
    skipblock();
}

/* Fetch mobile message line */
stringid_t
getmobmsg(const char *s)
{
    for (;;) {
        char *p = getTidyBlock(ifp);
        if (feof(ifp))
            LogFatal("Mobile:", mob.id, ": Unexpected end of file");
        if (*p == 0 || isEol(*p)) {
            LogFatal("Mobile:", mob.id, ": Unexpected end of definition");
        }
        p = skipspc(p);
        if (*p == 0 || isEol(*p) || isCommentChar(*p))
            continue;

        if (!canSkipLead(s, &p)) {
            mobmis(s);
            return -1;
        }
        if (toupper(*p) == 'N') {
            p = skipline(p);
            return -2;
        }
        stringid_t n = getTextString(p, true);
        p = skipline(p);
        if (n == -1) {
            LogError("Mobile:", mob.id, ": Invalid '", s, "' line");
        }
        return n;
    }
}

///////////////////////////////////////////////////////////////////////////////

// Process ROOMS.TXT
void
room_proc()
{
    char filepath[MAX_PATH_LENGTH];
    ///TODO: make name constexpr
    MakeTextFileName(std::string{"Rooms"}, filepath);

    SourceFile src{filepath};
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath);
    }

    std::string text {};
    text.reserve(4096);

    s_rooms.reserve(512);

    while (!src.Eof()) {
        // Terminate if we've reached the error limit.
        checkErrorCount();

        // Try and consume some non-whitespace tokens
        if (!src.GetIDLine("room="))
            continue;

        // Because 'room' is a global, we have to clear it out.
        std::string rmname { src.line.front() };
        StringLower(rmname);

        _ROOM_STRUCT room {};
        strncpy(room.id, rmname.c_str(), sizeof(room.id));
        room.dmove = -1;
        room.flags = 0;
        room.shortDesc = -1;
        room.longDesc = -1;
        room.tabptr = -1;
        room.ttlines = 0;

        // If there are additional words on the line, they are room flags.

        for (auto it = src.line.begin() + 1; it != src.line.end(); ++it) {
            if (auto [ok, token] = removePrefix(*it, "dmove="); ok == true) {
                if (token.empty()) {
                    LogError(src, "room:", rmname, ": dmove= without a destination name");
                    continue;
                }
                if (s_dmoveLookups.find(rmname) != s_dmoveLookups.end()) {
                    LogError(src, "room:", rmname, ": multiple 'dmove's");
                    break;
                }
                std::string dmove { token };
                StringLower(dmove);
                s_dmoveLookups[rmname] = dmove;
                continue;
            }

            std::string flag { *it };
            StringLower(flag);
            int no = isRoomFlag(flag);
            if (no == -1) {
                LogError(src, "room:", rmname, ": invalid room flag: ", *it);
                continue;
            }
            if (room.flags & bitset(no)) {
                LogWarn(src, "room:", rmname, ": duplicate room flag: ", *it);
            }
            room.flags |= bitset(no);
        }

        if (src.GetLine() && !src.line.empty() && !src.line.front().empty()) {
            // Short description.
            AddTextString(src.line.front(), true, &room.shortDesc);

            // Check for a long description.
            text.clear();
            src.GetLines(text);
            if (!text.empty())
                AddTextString(text, false, &room.longDesc);
        }

        s_roomIndex[rmname] = s_rooms.size();
        s_rooms.push_back(room);

        ++g_gameConfig.numRooms;
    }

    for (auto & it : s_dmoveLookups) {
        checkErrorCount();
        const auto &rmname = it.first, &dmoveName = it.second;
        auto dmoveIt = s_roomIndex.find(dmoveName);
        if (dmoveIt == s_roomIndex.end()) {
            LogError("room:", rmname, ": could not resolve 'dmove': ", dmoveName);
            continue;
        }
        auto roomIt = s_roomIndex.find(rmname);
        if (roomIt == s_roomIndex.end())
            LogFatal("room:", rmname, ": internal error: room not present in index");
        s_rooms[roomIt->second].dmove = dmoveIt->second;
    }
}

/* Process RANKS.TXT */
void
rank_proc()
{
    nextc(true);

    fopenw(rankDataFile);

    while (!feof(ifp)) {
        if (!nextc(false))
            break;

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        p = getword(block);
        if (!checkRankLine(p))
            continue;

        if (strlen(Word) < 3 || p - block > RANKL) {
            LogFatal("Rank ", g_gameConfig.numRanks + 1, ": Invalid male rank: ", Word);
        }
        int n = 0;
        do {
            if (Word[n] == '_')
                Word[n] = ' ';
            rank.male[n] = rank.female[n] = tolower(Word[n]);
            n++;
        } while (Word[n - 1] != 0);

        p = getword(p);
        if (!checkRankLine(p))
            continue;
        if (strcmp(Word, "=") != 0 && (strlen(Word) < 3 || strlen(Word) > RANKL)) {
            LogFatal("Rank ", g_gameConfig.numRanks + 1, ": Invalid female rank: ", Word);
        }
        if (Word[0] != '=') {
            n = 0;
            do {
                if (Word[n] == '_')
                    Word[n] = ' ';
                rank.female[n] = tolower(Word[n]);
                n++;
            } while (Word[n - 1] != 0);
        }

        p = getword(p);
        if (!checkRankLine(p))
            continue;
        if (!isdigit(Word[0])) {
            LogError("Invalid score value: ", Word);
            continue;
        }
        rank.score = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid strength value: ", Word);
            continue;
        }
        rank.strength = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid stamina value: ", Word);
            continue;
        }
        rank.stamina = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid dexterity value: ", Word);
            continue;
        }
        rank.dext = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid wisdom value: ", Word);
            continue;
        }
        rank.wisdom = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid experience value: ", Word);
            continue;
        }
        rank.experience = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid magic points value: ", Word);
            continue;
        }
        rank.magicpts = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid max weight value: ", Word);
            continue;
        }
        rank.maxweight = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid max inventory value: ", Word);
            continue;
        }
        rank.numobj = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid kill points value: ", Word);
            continue;
        }
        rank.minpksl = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            LogError("Invalid task number: ", Word);
            continue;
        }
        rank.tasks = atoi(Word);

        p = skipspc(p);
        if (*p == '\"')
            p++;
        strcpy(block, p);
        p = block;
        while (*p != 0 && *p != '\"')
            p++;
        /* Greater than prompt length? */
        if (p - block > 10) {
            LogError("Rank #", g_gameConfig.numRanks + 1, ": prompt too long: ", block);
            continue;
        }
        if (block[0] == 0)
            strcpy(rank.prompt, "$ ");
        else
            WordCopier(rank.prompt, block, p);

        wizstr = rank.strength;
        checkedfwrite(rank.male, sizeof(rank), 1, ofp1);
        g_gameConfig.numRanks++;
    }
}

// void
// sort_objs()
//{
//    int   i, j, k, nts;
//    long *rmtab, *rmptr;
//
//    if (ifp != nullptr)
//        fclose(ifp);
//    ifp = nullptr;
//    closeOutFiles();
//    fopenr(objectStateFile);
//    blkget(&datal, &data, nullptr);
//    fclose(ifp);
//    ifp = nullptr;
//    closeOutFiles();
//    fopenr(objrmsfn);
//    blkget(&datal2, &data2, nullptr);
//    fclose(ifp);
//    ifp = nullptr;
//    closeOutFiles();
//    fopenw(objsfn);
//    fopenw(objectStateFile);
//    fopenw(objrmsfn);
//    fopenw(ntabfn);
//    ifp = nullptr;
//
//    printf("Sorting Objects...:\r");
//    objtab2 = obtab2;
//    nts = 0;
//    k = 0;
//
//    statab = (_OBJ_STATE *)data;
//    rmtab = (long *)data2;
//    for (i = 0; i < nouns; i++) {
//        if (*(objtab2 = (obtab2 + i))->id == 0) {
//            printf("@! skipping %ld states, %ld rooms.\n", objtab2->nstates, objtab2->nrooms);
//            statab += objtab2->nstates;
//            rmtab += objtab2->nrooms;
//            continue;
//        }
//        strcpy(nountab.id, objtab2->id);
//        nts++;
//        nountab.num_of = 0;
//        osrch = objtab2;
//        statep = statab;
//        rmptr = rmtab;
//        for (j = i; j < nouns; j++, osrch++) {
//            if (*(osrch->id) != 0 && stricmp(nountab.id, osrch->id) == 0) {
//                fwrite((char *)osrch, sizeof(obj), 1, ofp1);
//                fwrite((char *)statep, sizeof(state), osrch->nstates, ofp2);
//                fwrite((char *)rmptr, sizeof(long), osrch->nrooms, ofp3);
//                nountab.num_of++;
//                *osrch->id = 0;
//                if (osrch != objtab)
//                    k++;
//                statep += osrch->nstates;
//                rmptr += osrch->nrooms;
//                if (osrch == objtab2) {
//                    statab = statep;
//                    rmtab = rmptr;
//                    objtab2++;
//                    i++;
//                }
//            } else
//                statep += osrch->nstates;
//            rmptr += osrch->nrooms;
//        }
//
//        fwrite((char *)&nountab, sizeof(nountab), 1, ofp4);
//    }
//    printf("%20s\r%ld objects moved.\n", " ", k);
//    ReleaseMem(datal);
//    ReleaseMem(datal2);
//    data = data2 = nullptr;
//    fopenr(objsfn);
//    fread((char *)obtab2, sizeof(obj), nouns, ifp);
//}

void
state_proc()
{
    state.weight = state.value = state.flags = 0;
    state.description = -1;

    /* Get the weight of the object */
    char *p = getWordAfter("weight=", block);
    if (Word[0] == 0)
        stateInvalid("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        stateInvalid("weight value on");
    state.weight = atoi(Word);
    if (obj2.flags & OF_SCENERY)
        state.weight = wizstr + 1;

    /* Get the value of it */
    p = getWordAfter("value=", p);
    if (Word[0] == 0)
        stateInvalid("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        stateInvalid("value entry on");
    state.value = atoi(Word);

    /* Get the strength of it (hit points)*/
    p = getWordAfter("str=", p);
    if (Word[0] == 0)
        stateInvalid("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        stateInvalid("strength entry on");
    state.strength = atoi(Word);

    /* Get the damage it does as a weapon*/
    p = getWordAfter("dmg=", p);
    if (Word[0] == 0)
        stateInvalid("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        stateInvalid("damage entry on");
    state.damage = atoi(Word);

    /* Description */
    p = skiplead("desc=", skipspc(p));
    if (*p == 0)
        stateInvalid("incomplete");
    if (*p == '\"' || *p == '\'') {
        state.description = getTextString(p, false);
        char quote = *p;
        p = strstop(p + 1, *p);
        if (*p == quote)
            ++p;
    } else {
        p = getword(p);
        state.description = getObjectDescriptionID(Word);
    }
    if (state.description == -1) {
        char tmp[128];
        snprintf(tmp, sizeof(tmp), "desc= ID (%s) on", &Word[0]);
        stateInvalid(tmp);
    }
    while (*(p = skipspc(p)) != 0) {
        p = getword(p);
        if (Word[0] == 0)
            break;
        int flag = isoflag2(Word);
        if (flag == -1)
            stateInvalid("flag on");
        state.flags |= bitset(flag);
    }
    checkedfwrite(&state.weight, sizeof(state), 1, ofp2);
    obj2.nstates++;
    g_gameConfig.numObjStates++;
}

void
objs_proc()
{
    /* Clear files */
    fopenw(objectDataFile);
    fopenw(objectStateFile);
    fopenw(objectRoomFile);
    fopena(adjectiveDataFile);

    obtab2 = (_OBJ_STRUCT *)AllocateMem(filesize() + 128 * sizeof(obj2));
    if (obtab2 == nullptr)
        LogFatal("Out of memory");

    if (!nextc(false)) {
        return;
    } /* Nothing to process: ///TODO: Warning */

    while (!feof(ifp)) {
        checkErrorCount();

        if (!nextc(false))
            break;

        char *cur = getTidyBlock(ifp);
        if (!cur)
            continue;

        cur = getWordAfter("noun=", cur);
        if (Word[0] == 0) {
            LogError("noun= line with no noun");
            skipblock();
            continue;
        }
		LogDebug("noun=", Word);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            LogError("Invalid object name (length): ", cur);
            skipblock();
            continue;
        }

        obj2.adj = obj2.mobile = -1;
        obj2.idno = g_gameConfig.numObjects;
        obj2.state = 0;
        obj2.nrooms = 0;
        obj2.contains = 0;
        obj2.flags = 0;
        obj2.putto = 0;
        obj2.rmlist = (roomid_t *)ftell(ofp3);
        strncpy(obj2.id, Word, sizeof(obj2.id));
        /// TODO: Register noun

        while (*cur) {
            cur = getword(cur);
            if (!Word[0])
                break;

            int idNo = isoflag1(Word);
            if (idNo != -1)
                obj2.flags |= idNo;
            else {
                idNo = isoparm(Word);
                if (idNo == -1) {
                    LogError("object:", obj2.id, ": Invalid parameter: ", Word);
                    continue;
                }
                switch (bitset(idNo)) {
                case OP_ADJ:
                    set_adj();
                    break;
                case OP_START:
                    set_start();
                    break;
                case OP_HOLDS:
                    set_holds();
                    break;
                case OP_PUT:
                    set_put();
                    break;
                case OP_MOB:
                    set_mob();
                    g_gameConfig.numMobs++;
                    break;
                default:
                    LogFatal("Internal Error: Code for object-parameter '", obparms[idNo], "' missing");
                }
            }
        }

        /* Get the room list */

        bool continuation = true;
        while (continuation) {
            char *p = getTidyBlock(ifp);
            if (!p)
                LogFatal("object:", obj2.id, ": unexpected end of file");

            while (*p) {
                continuation = false;
                p = getword(p);
                if (strcmp(Word, "+") == 0) {
                    continuation = true;
                    continue;
                }
                int roomNo = isloc(Word);
                if (roomNo == -1) {
                    LogError("object:", obj2.id, ": invalid room: ", Word);
                }
                checkedfwrite(&roomNo, 1, sizeof(roomNo), ofp3);
                obj2.nrooms++;
            }
        }

        if (obj2.nrooms == 0)
            LogError("object:", obj2.id, ": no location given");

        obj2.nstates = 0;
        for (;;) {
            char *p = getTidyBlock(ifp);
            if (!p)
				break;
            if (!*p || isEol(*p))
                break;
            state_proc();
        }

        if (obj2.nstates == 0)
            LogError("object:", obj2.id, ": no states defined");
        if (obj2.nstates > 100)
            LogError("object:", obj2.id, ": too many states defined (", obj2.nstates, ")");

        *(obtab2 + (g_gameConfig.numObjects++)) = obj2;
    }

    /*
    closeOutFiles();
    sort_objs();
    */
    checkedfwrite(obtab2, sizeof(obj2), g_gameConfig.numObjects, ofp1);
}

/*
     Travel Processing Routines for AMUL, Copyright (C) Oliver Smith, '90
     --------------------------------------------------------------------
  Warning! All source code in this file is copyright (C) KingFisher Software
*/

/* Process TRAVEL.TXT */
void
trav_proc()
{
    int strip;
    proc = 0;

    nextc(true); /* Move to first text */
    fopenw(travelTableFile);
    fopenw(travelParamFile);
    fopena(roomDataFile);

    assert(g_gameConfig.numTTEnts == 0);
    int ntt = 0, t = 0, r = 0;
    int ttNumVerbs = 0;

    verbid_t verbsUsed[200];

    while (!feof(ifp)) {
        checkErrorCount();

        if (!nextc(false))
            break;

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        p = getWordAfter("room=", p);
        if (Word[0] == 0) {
            LogError("travel: empty room= line");
            skipblock();
            continue;
        }
		LogDebug("room=", Word);

        const roomid_t rmn = isRoomName(Word);
        if (rmn == -1) {
            LogError("No such room: ", Word);
            skipblock();
            continue;
        }
        c_room = &s_rooms[rmn];
        if (c_room->tabptr != -1) {
            LogError("room:", Word, ": Multiple tt entries for room");
            skipblock();
            continue;
        }

    vbloop:
        do
            fgets(block, sizeof(block), ifp);
        while (block[0] != 0 && consumeComment(block));
        if (block[0] == 0 || isEol(block[0])) {
            /* Only complain if room is not a death room */
            if ((c_room->flags & DEATH) != DEATH) {
                LogInfo("room:", c_room->id, ": No tt entries for room");
                c_room->tabptr = -2;
                ntt++;
                continue;
            }
        }
        tidy(block);
        if (!striplead("verb=", block) && !striplead("verbs=", block)) {
            LogError("room:", c_room->id, ": Missing verb[s]= entry");
            goto vbloop;
        }
        g_verb.id[0] = 0;
        c_room->tabptr = t;
        c_room->ttlines = 0;
    vbproc: /* Process verb list */
        tt.pptr = (oparg_t *)-1;
        p = block;
        ttNumVerbs = 0;  // number of verbs in this tt entry
        /* Break verb list down to verb no.s */
        do {
            p = getword(p);
            if (Word[0] == 0)
                break;
            verbsUsed[ttNumVerbs] = isVerb(Word);
            if (verbsUsed[ttNumVerbs] == -1) {
                LogError("room:", c_room->id, ": Invalid verb: ", Word);
            }
            ttNumVerbs++;

        } while (Word[0] != 0);
        if (!ttNumVerbs) {
            LogFatal("room:", c_room->id, ": empty verb[s]= line: %s", block);
        }
        /* Now process each instruction line */
        do {
        xloop:
            strip = 0;
            r = -1;
            block[0] = 0;
            fgets(block, sizeof(block), ifp);
            if (feof(ifp))
                break;
            if (block[0] == 0 || isEol(block[0])) {
                strip = -1;
                continue;
            }
            tidy(block);
            if (block[0] == 0 || consumeComment(block))
                goto xloop;
            if (striplead("verb=", block) || striplead("verbs=", block)) {
                strip = 1;
                break;
            }
            p = precon(block); /* Strip pre-condition opts */
        notloop:
            p = getword(p);
            if (strcmp(Word, ALWAYSEP) == 0) {
                tt.condition = CALWAYS;
                tt.action = -(1 + AENDPARSE);
                goto write;
            }
            if (strcmp(Word, "not") == 0 || strcmp(Word, "!") == 0) {
                r = -1 * r;
                goto notloop;
            }
        notlp2:
            if (Word[0] == '!') {
                strcpy(Word, Word + 1);
                r = -1 * r;
                goto notlp2;
            }
            if ((tt.condition = iscond(Word)) == -1) {
                tt.condition = CALWAYS;
                if ((tt.action = isRoomName(Word)) != -1)
                    goto write;
                if ((tt.action = isact(Word)) == -1) {
                    LogError("room:", c_room->id, ": invalid tt condition: ", Word);
                    goto xloop;
                }
                goto gotohere;
            }
            p = skipspc(p);
            if ((p = checkConditionParameters(p, &conditions[tt.condition], ofp2)) == nullptr) {
                goto next;
            }
            if (r == 1)
                tt.condition = -1 - tt.condition;
            if (*p == 0) {
                LogError("room:", c_room->id, ": tt entry is missing an action");
                goto xloop;
            }
            p = preact(p);
            p = getword(p);
            if ((tt.action = isRoomName(Word)) != -1)
                goto write;
            if ((tt.action = isact(Word)) == -1) {
                LogError("room:", c_room->id, ": unrecognized tt action: ", Word);
                goto xloop;
            }
        gotohere:
            if (tt.action == ATRAVEL) {
                LogError("room:", c_room->id, ": Tried to call action 'travel' from travel table");
                goto xloop;
            }
            p = skipspc(p);
            if ((p = checkActionParameters(p, &actions[tt.action], ofp2)) == nullptr) {
                goto next;
            }
            tt.action = 0 - (tt.action + 1);
        write:
            // this is some weird-ass kind of encoding where -1 means "more", and "-2" means "last"
            for (int verbNo = 0; verbNo < ttNumVerbs; ++verbNo) {
                oparg_t paramid = (verbNo + 1 < ttNumVerbs) ? -1 : -2;
                tt.pptr = (oparg_t *)(uintptr_t)paramid;
                tt.verb = verbsUsed[verbNo];
                checkedfwrite(&tt.verb, sizeof(tt), 1, ofp1);
                c_room->ttlines++;
                g_gameConfig.numTTEnts++;
            }
        next:
            strip = 0;
        } while (strip == 0 && !feof(ifp));
        if (strip == 1 && !feof(ifp))
            goto vbproc;
        ntt++;
    }

    c_room = nullptr;

    if (GetLogErrorCount() == 0 && ntt != g_gameConfig.numRooms) {
        for (auto &room : s_rooms) {
            if (room.tabptr == -1 && (room.flags & DEATH) != DEATH) {
                LogWarn("room:", room.id, ": no travel entry");
            }
        }
    }
}

/* From and To */
int
chae_proc(const char *f, char *t)
{
    int n;

    if ((*f < '0' || *f > '9') && *f != '?') {
        chae_err(Word);
        return -1;
    }

    if (*f == '?') {
        *(t++) = -1;
        f++;
    } else {
        n = atoi(f);
        while (isdigit(*f) && *f != 0)
            f++;
        if (*f == 0) {
            chae_err(Word);
            return -1;
        }
        *(t++) = (char)n;
    }

    for (n = 1; n < 5; n++) {
        if (*f == 'c' || *f == 'h' || *f == 'a' || *f == 'e') {
            *(t++) = toupper(*f);
            f++;
        } else {
            chae_err(Word);
            return -1;
        }
    }

    return 0;
}

void
getVerbFlags(_VERB_STRUCT *verbp, char *p)
{
    static char defaultChae[] = {-1, 'C', 'H', 'A', 'E', -1, 'C', 'H', 'A', 'E'};
    memcpy(verbp->precedences, defaultChae, sizeof(verbp->precedences));

    verbp->flags = VB_TRAVEL;
    int precedence = 0;
    while (*(p = skipspc(p)) != 0 && !isCommentChar(*p)) {
        p = getword(p);
        if (strcmp("travel", Word) == 0) {
            verbp->flags = 0;
            continue;
        }
        if (strcmp("dream", Word) == 0) {
            verbp->flags += VB_DREAM;
            continue;
        }
        // So we expect it to be a precedence specifier
        if (precedence < 2) {
            if (chae_proc(Word, verbp->precedence[precedence]) == -1)
                return;
            ++precedence;
            continue;
        }

        LogError("Expected verb flag/precedence, got: ", Word);
    }
}

void
registerTravelVerbs(char *p)
{
    while (!isEol(*p) && *p) {
        p = skipspc(p);
        p = getword(p);
        if (Word[0] == 0)
            break;
        int extant = isVerb(Word);
        if (extant != -1) {
            if (vbptr[extant].flags | VB_TRAVEL) {
                LogError("Redefinition of travel verb: ", Word);
                continue;
            }
            vbptr[extant].flags |= VB_TRAVEL;
            LogDebug("Added TRAVEL to existing verb: ", Word);
            continue;
        }
        /// TODO: size check
        strncpy(g_verb.id, Word, sizeof(g_verb.id));
        checkedfwrite(&g_verb, sizeof(g_verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameConfig.numVerbs++)) = g_verb;
        LogDebug("Added TRAVEL verb: ", Word);
    }
}

/* Process LANG.TXT */
void
lang_proc()
{
    char lastc;
    /* n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2) */
    int n, cs, s, r;

    nextc(true);
    fopenw(verbDataFile);
    CloseOutFiles();
    fopena(verbDataFile);
    ofp1 = afp;
    afp = nullptr;
    fopenw(verbSlotFile);
    fopenw(verbTableFile);
    fopenw(verbParamFile);

    vbtab = (_VERB_STRUCT *)AllocateMem(filesize() + 128 * sizeof(g_verb));
    vbptr = vbtab;

    size_t of2p = ftell(ofp2);
    size_t of3p = ftell(ofp3);
    FPos = ftell(ofp4);

    while (!feof(ifp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        // check for 'travel' line which allows specification of travel verbs

        if (canSkipLead("travel=", &p)) {
            registerTravelVerbs(p);
            continue;
        }

        p = getWordAfter("verb=", p);
        if (!Word[0]) {
            LogError("verb= line without a verb?");
            skipblock();
            continue;
        }

        if (strlen(Word) > IDL) {
            LogError("Invalid verb ID (too long): ", Word);
            skipblock();
            continue;
        }

        memset(&g_verb, 0, sizeof(g_verb));
        strncpy(g_verb.id, Word, sizeof(g_verb.id));

        ++g_gameConfig.numVerbs;
        LogDebug("verb#", g_gameConfig.numVerbs, ":", g_verb.id);

        getVerbFlags(&g_verb, p);

        p = getTidyBlock(ifp);
        if (!p)
            LogError("Unexpected end of file during verb: ", g_verb.id);
        if (!*p || isEol(*p)) {
            if (g_verb.ents == 0 && (g_verb.flags & VB_TRAVEL)) {
                LogWarn("Verb has no entries: ", g_verb.id);
            }
            goto write;
        }

        if (!canSkipLead("syntax=", &p)) {
            vbprob("no syntax= line", block);
            skipblock();
            continue;
        }

        /* Syntax line loop */
    synloop : {
        setslots(WNONE);
        g_verb.ents++;
        p = skiplead("verb", p);
        char *qualifier = getword(p);
        qualifier = skipspc(qualifier);

        /* If syntax line is 'syntax=verb any' or 'syntax=none' */
        if (*qualifier == 0 && strcmp("any", Word) == 0) {
            setslots(WANY);
            goto endsynt;
        }
        if (*qualifier == 0 && strcmp("none", Word) == 0) {
            setslots(WNONE);
            goto endsynt;
        }
    }

    sp2: /* Syntax line processing */
        p = skipspc(p);
        if (consumeComment(p) || *p == '|')
            goto endsynt;

        p = getword(p);
        if (Word[0] == 0)
            goto endsynt;

        if ((n = iswtype(Word)) == -3) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid phrase on syntax line: %s", Word);
            vbprob(tmp, block);
            goto commands;
        }
        if (Word[0] == 0) {
            s = WANY;
            goto skipeq;
        }

        /* First of all, eliminate illegal combinations */
        if (n == WNONE || n == WANY) { /* you cannot say none=fred any=fred etc */
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Tried to define %s on syntax line", syntax[n]);
            vbprob(tmp, block);
            goto endsynt;
        }
        if (n == WPLAYER && strcmp(Word, "me") != 0 && strcmp(Word, "myself") != 0) {
            vbprob("Tried to specify player other than self", block);
            goto endsynt;
        }

        /* Now check that the 'tag' is the correct type of word */

        s = -1;
        switch (n) {
        case WADJ:
        /* Need ISADJ() - do TT entry too */
        case WNOUN:
            s = isNoun(Word);
            break;
        case WPREP:
            s = isprep(Word);
            break;
        case WPLAYER:
            if (strcmp(Word, "me") == 0 || strcmp(Word, "myself") == 0)
                s = -3;
            break;
        case WROOM:
            s = isRoomName(Word);
            break;
        case WSYN:
            LogWarn("Synonyms not supported yet");
            s = WANY;
            break;
        case WTEXT:
            s = getTextString(Word, false);
            break;
        case WVERB:
            s = isVerb(Word);
            break;
        case WCLASS:
            s = WANY;
        case WNUMBER:
            if (Word[0] == '-')
                s = atoi(Word + 1);
            else
                s = atoi(Word);
        default:
            LogError("Internal Error: Invalid w-type");
        }

        if (n == WNUMBER && (s > 100000 || -s > 100000)) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "Invalid number: %d", s);
            vbprob(tmp, block);
        }
        if (s == -1 && n != WNUMBER) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "%s=: Invalid setting: %s", syntax[n + 1], Word);
            vbprob(tmp, block);
        }
        if (s == -3 && n == WNOUN)
            s = -1;

    skipeq: /* (Skipped the equals signs) */
        /* Now fit this into the correct slot */
        cs = 1; /* Noun1 */
        switch (n) {
        case WADJ:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                vbprob("Invalid NOUN NOUN ADJ combination", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("Invalid NOUN ADJ NOUN ADJ combination", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[0] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("More than two adjectives on a line", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[0] != WNONE)
                cs = 3;
            else
                cs = 0;
            break;
        case WNOUN:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                vbprob("Invalid noun arrangement", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE)
                cs = 4;
            break;
        case WPREP:
            if (vbslot.wtype[2] != WNONE) {
                vbprob("Invalid PREP arrangement", block);
                n = -5;
                break;
            }
            cs = 2;
            break;
        case WPLAYER:
        case WROOM:
        case WSYN:
        case WTEXT:
        case WVERB:
        case WCLASS:
        case WNUMBER:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                char tmp[128];
                snprintf(block, sizeof(block), "No free noun slot for %s entry", syntax[n + 1]);
                vbprob(tmp, block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE)
                cs = 4;
            break;
        }
        if (n == -5)
            goto sp2;
        /* Put the bits into the slots! */
        vbslot.wtype[cs] = n;
        vbslot.slot[cs] = s;
        goto sp2;

    endsynt:
        vbslot.ents = 0;
        vbslot.ptr = (_VBTAB *)of3p;

    commands:
        lastc = 'x';
        proc = 0;

        p = getTidyBlock(ifp);
        if (!p || !*p || isEol(*p)) {
            lastc = 1;
            goto writeslot;
        }

        if (canSkipLead("syntax=", &p)) {
            lastc = 0;
            goto writeslot;
        }

        vbslot.ents++;
        r = -1;
        vt.pptr = (oparg_t *)FPos;

        /* Process the condition */
    notloop:
        p = precon(p);
        p = getword(p);

        /* always endparse */
        if (strcmp(Word, ALWAYSEP) == 0) {
            vt.condition = CALWAYS;
            vt.action = -(1 + AENDPARSE);
            goto writecna;
        }
        if (strcmp(Word, "not") == 0 || (Word[0] == '!' && Word[1] == 0)) {
            r = -1 * r;
            goto notloop;
        }

        /* If they forgot space between !<condition>, eg !toprank */
    notlp2:
        if (Word[0] == '!') {
            memmove(Word, Word + 1, sizeof(Word) - 1);
            Word[sizeof(Word) - 1] = 0;
            r = -1 * r;
            goto notlp2;
        }

        if ((vt.condition = iscond(Word)) == -1) {
            proc = 1;
            if ((vt.action = isact(Word)) == -1) {
                if ((vt.action = isRoomName(Word)) != -1) {
                    vt.condition = CALWAYS;
                    goto writecna;
                }
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "Invalid condition, '%s'", Word);
                vbprob(tmp, block);
                proc = 0;
                goto commands;
            }
            vt.condition = CALWAYS;
            goto doaction;
        }
        p = skipspc(p);
        proc = 1;
        if ((p = checkConditionParameters(p, &conditions[vt.condition], ofp4)) == nullptr) {
            goto commands;
        }
        if (*p == 0) {
            if ((vt.action = isact(conditions[vt.condition].name)) == -1) {
                vbprob("Missing action", block);
                goto commands;
            }
            vt.action = 0 - (vt.action + 1);
            vt.condition = CALWAYS;
            goto writecna;
        }
        if (r == 1)
            vt.condition = -1 - vt.condition;
        p = preact(p);
        p = getword(p);
        if ((vt.action = isact(Word)) == -1) {
            if ((vt.action = isRoomName(Word)) != -1)
                goto writecna;
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid action, '%s'", Word);
            vbprob(tmp, block);
            goto commands;
        }
    doaction:
        p = skipspc(p);
        if ((p = checkActionParameters(p, &actions[vt.action], ofp4)) == nullptr) {
            goto commands;
        }
        vt.action = 0 - (vt.action + 1);

    writecna: /* Write the C & A lines */
        checkedfwrite((char *)&vt.condition, sizeof(vt), 1, ofp3);
        proc = 0;
        of3p += sizeof(vt);
        goto commands;

    writeslot:
        checkedfwrite(vbslot.wtype, sizeof(vbslot), 1, ofp2);
        proc = 0;
        of2p += sizeof(vbslot);
        if (lastc > 1)
            goto commands;
        if (lastc == 0)
            goto synloop;

        lastc = '\n';
    write:
        checkedfwrite(&g_verb, sizeof(g_verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameConfig.numVerbs - 1)) = g_verb;
    }
}

/* Routines to process/handle Synonyms */

void
syn_proc()
{
    if (!nextc(false))
        return;

    fopenw(synonymDataFile);
    fopenw(synonymIndexFile);

    while (!feof(ifp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        /// TODO: Read syns file.

        p = getword(p);
        if (!*p) {
            LogError("Invalid synonym line: ", block);
            continue;
        }
        int id = isNoun(Word);
        if (id < 0) {
            id = isVerb(Word);
            if (id == -1) {
                LogError("Invalid verb/noun: ", Word);
                continue;
            }
            id = -(2 + id);
        }

        for (;;) {
            p = getword(p);
            if (Word[0] == 0)
                break;
            checkedfwrite(&id, 1, sizeof(id), ofp2);
            fprintf(ofp1, "%s%c", Word, 0);
            g_gameConfig.numSynonyms++;
        }
    }
}

/* Mobiles.Txt Processor */

/* Pass 1: Indexes mobile names */

void
mob_proc1()
{
    fopenw(mobileDataFile);
    if (!nextc(false))
        return;

    // "mob" is the mobile entity
    // "mobp" is a pointer to the runtime portion
    _MOB *mobd = &mob.mob;

    while (!feof(ifp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(ifp);
        if (!p || *p != '!')
            continue;

        // skip the '!'
        p = getword(p + 1);
        memset(&mob, 0, sizeof(mob));
        strcpy(mob.id, Word);

        for (;;) {
            p = skipspc(p);
            if (!*p)
                break;

            if (canSkipLead("dead=", &p)) {
                p = getword(p);
                mobd->deadstate = atoi(Word);
                continue;
            }
            if (canSkipLead("dmove=", &p)) {
                p = getword(p);
                mobd->dmove = isRoomName(Word);
                if (mobd->dmove == -1) {
                    LogError("Mobile:", mob.id, ": invalid dmove: ", Word);
                }
                continue;
            }
        }

        p = getTidyBlock(ifp);
        if (!p)
            LogFatal("mob:!", mob.id, ": unexpected end of file");

        if (!canSkipLead("speed=", &p)) {
            mobmis("speed=");
            continue;
        }
        p = getword(p);
        mobd->speed = atoi(Word);

        if (!canSkipLead("travel=", &p)) {
            mobmis("travel=");
            continue;
        }
        p = getword(p);
        mobd->travel = atoi(Word);

        if (!canSkipLead("fight=", &p)) {
            mobmis("speed=");
            continue;
        }
        p = getword(p);
        mobd->fight = atoi(Word);

        if (!canSkipLead("act=", &p)) {
            mobmis("act=");
            continue;
        }
        p = getword(p);
        mobd->act = atoi(Word);

        if (!canSkipLead("wait=", &p)) {
            mobmis("wait=");
            continue;
        }
        p = getword(p);
        mobd->wait = atoi(Word);

        if (mobd->travel + mobd->fight + mobd->act + mobd->wait != 100) {
            LogError("Mobile:", mob.id, ": Travel+Fight+Act+Wait values not equal to 100%.");
        }

        if (!canSkipLead("fear=", &p)) {
            mobmis("fear=");
            continue;
        }
        p = getword(p);
        mobd->fear = atoi(Word);

        if (!canSkipLead("attack=", &p)) {
            mobmis("attack=");
            continue;
        }
        p = getword(p);
        mobd->attack = atoi(Word);

        if (!canSkipLead("hitpower=", &p)) {
            mobmis("hitpower=");
            continue;
        }
        p = getword(p);
        mobd->hitpower = atoi(Word);

        if ((mobd->arr = getmobmsg("arrive=")) == -1)
            continue;
        if ((mobd->dep = getmobmsg("depart=")) == -1)
            continue;
        if ((mobd->flee = getmobmsg("flee=")) == -1)
            continue;
        if ((mobd->hit = getmobmsg("strike=")) == -1)
            continue;
        if ((mobd->miss = getmobmsg("miss=")) == -1)
            continue;
        if ((mobd->death = getmobmsg("dies=")) == -1)
            continue;

        checkedfwrite(&mob, sizeof(mob), 1, ofp1);
        g_gameConfig.numMobPersonas++;
    }

    if (g_gameConfig.numMobPersonas != 0) {
        mobp = (_MOB_ENT *)AllocateMem(sizeof(mob) * g_gameConfig.numMobPersonas);
        if (mobp == nullptr) {
            LogFatal("Out of memory");
        }
        CloseOutFiles();

        fopenr(mobileDataFile);
        checkedfread(mobp, sizeof(mob), g_gameConfig.numMobPersonas, ifp);
    }
}

/* Pass 2: Indexes commands mobiles have access to */
/*mob_proc2()
{*/

/*---------------------------------------------------------*/

/*
struct Context {
    const char  filename[512];
    const char *start;
    const char *end;
    const char *cur;
    size_t      lineNo;
    const char *lineStart;
};

Context *
NewContext(const char *filename)
{
    Context *context = calloc(sizeof(Context), 1);
    if (context == nullptr) {
        LogFatal("Out of memory for context");
    }

    if (EINVAL == path_join(context->filename, sizeof(context->filename), gameDir, filename)) {
        LogFatal("Path length exceeds limit for %s/%s", gameDir, filename);
    }

    return context;
}
*/

extern void title_proc(), smsg_proc(), umsg_proc(), obds_proc();

struct CompilePhase {
    const char *name;
    bool        isText;
    void (*handler)();
} phases[] = {
    { "title", true, title_proc },      // game "title" (and config)
    { "sysmsg", false, smsg_proc },      // system messages
    { "umsg", false, umsg_proc },        // user-defined string messages
    { "obdescs", false, obds_proc },     // object description strings
    { "rooms", true, room_proc },       // room table
    { "ranks", true, rank_proc },       // rank table
    { "mobiles", true, mob_proc1 },     // npc classes so we can apply to objects
    { "objects", true, objs_proc },     // objects
    { "lang", true, lang_proc },        // language
    { "travel", true, trav_proc },      // travel table
    { "syns", true, syn_proc },         // synonyms for other things
    { nullptr, false, nullptr }         // terminator
};

void
compilePhase(const CompilePhase *phase)
{
    LogInfo("Compiling: ", phase->name);
    if (phase->isText) {
        char filepath[MAX_PATH_LENGTH];
        MakeTextFileName(phase->name, filepath);
        FILE *fp = fopen(filepath, "r");
        if (!fp)
            LogFatal("Could not open ", phase->name, " file: ", filepath);
        ifp = fp;
    }

    phase->handler();

    if (ifp) {
        fclose(ifp);
        ifp = nullptr;
    }

    CloseOutFiles();

    if (GetLogErrorCount() > 0) {
        LogFatal("Terminating due to errors.");
    }
}

void
compileGame()
{
    for (size_t phaseNo = 0; phases[phaseNo].name; ++phaseNo) {
        compilePhase(&phases[phaseNo]);
    }
}

error_t
compilerModuleInit(Module *module)
{
    return 0;
}

void
setProcessTitle(const char *title)
{
#if defined(AMIGA)
    // set process name
    mytask = FindTask(0L);
    mytask->tc_Node.ln_Name = compilerVersion;
    return;
#endif
#if defined(_MSC_VER)
    (void)title;  /// TODO: Implement Win32
#else
    (void)title;  /// TODO: Implement posix version
#endif
}

void
createDataDir()
{
    char filepath[MAX_PATH_LENGTH] {0};
    if (path_joiner(filepath, gameDir, "Data") != 0)
        LogFatal("Internal error preparing Data directory path");
#if defined(_MSC_VER)
    if (mkdir(filepath) < 0 && errno != EEXIST)
#else
    if (mkdir(filepath, 0776) < 0 && errno != EEXIST)
#endif
        LogFatal("Unable to create Data directory: ", filepath, ": ", strerror(errno));
}

error_t
compilerModuleStart(Module *module)
{
    setProcessTitle(compilerVersion.c_str());
    LogInfo("AMUL Compiler: ", compilerVersion);

    LogDebug("Game Directory: ", gameDir);
    LogDebug("Log Verbosity : ", GetLogLevelName(GetLogLevel()));

    createDataDir();

    for (const CompilePhase *phase = &phases[0]; phase->name; ++phase) {
        if (phase->isText == false)
            continue;

        char filepath[MAX_PATH_LENGTH];
        MakeTextFileName(phase->name, filepath);

        struct stat sb {0};
        error_t err = stat(filepath, &sb);
        if (err != 0)
            LogFatal("Missing file (", err, "): ", filepath);
    }

    return 0;
}

error_t
compilerModuleClose(Module *module, error_t err)
{
    CloseOutFiles();
    CloseFile(&ifp);

    // If we didn't complete compilation, delete the profile file.
    if (err != 0 || !exiting) {
        UnlinkGameFile(gameDataFile);
    }

    ReleaseMem((void **)&obtab2);
    ReleaseMem((void **)&vbtab);
    ReleaseMem((void **)&mobp);

    return 0;
}

void
InitCompilerModule()
{
    NewModule(MOD_COMPILER,
              compilerModuleInit,
              compilerModuleStart,
              compilerModuleClose,
              nullptr,
              nullptr);
}

/*---------------------------------------------------------*/

error_t
amulcom_main()
{
    InitStrings();

    InitCompilerModule();

    StartModules();

    compileGame();

    g_gameConfig.numStrings = GetStringCount();
    g_gameConfig.stringBytes = GetStringBytes();

    LogNote("Execution finished normally");
    LogInfo("Statistics for ", g_gameConfig.gameName, ":");
    LogInfo("Rooms: ", g_gameConfig.numRooms,
            ", Ranks: ", g_gameConfig.numRanks,
            ", Objects: ", g_gameConfig.numObjects,
            ", Adjs: ", g_gameConfig.numAdjectives,
            ", Verbs: ", g_gameConfig.numVerbs,
            ", Syns: ", g_gameConfig.numSynonyms,
            ", TT Ents: ", g_gameConfig.numTTEnts,
            ", Strings: ", g_gameConfig.numStrings,
            ", Text: ", g_gameConfig.stringBytes);

    fopenw(gameDataFile);
    checkedfwrite(&g_gameConfig, sizeof(g_gameConfig), 1, ofp1);
    CloseOutFiles();

    exiting = true;

    return 0;
}
