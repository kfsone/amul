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
#include "amulcom.ctxLog.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"

#include "filesystem.h"
#include "filesystem.inl.h"
#include "modules.h"
#include "system.h"

#include <h/amigastubs.h>
#include <h/amul.acts.h>
#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.defs.h>
#include <h/amul.gcfg.h>
#include <h/amul.lcst.h>
#include <h/amul.msgs.h>
#include <h/amul.stct.h>
#include <h/amul.strs.h>
#include <h/amul.test.h>
#include <h/amul.vars.h>
#include <h/amul.vmop.h>
#include <h/amul.xtra.h>
#include <h/amulcom.h>

#include <h/room_struct.h>

#include <cassert>
#include <fcntl.h>
#include <cstdlib>
#include <sys/stat.h>
#include <ctime>
#include <string>

const char *g_contextFile;
const char *g_contextPhase;
const char *g_contextElement;

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables

std::string compilerVersion { VERS " (" DATE ")" };

/* Compiler specific variables... */

int    dmoves = 0; /* How many DMOVEs to check?	*/
int    rmn = 0;    /* Current room no.		*/
size_t FPos;       /* Used during TT/Lang writes	*/
char   Word[64];   /* For internal use only <grin>	*/
int    proc;       /* What we are processing	*/
char * syntab;     /* Synonym table, re-read	*/
long   wizstr;     /* Wizards strength		*/

char block[1024];  // scratch pad

// counters
GameConfig g_gameConfig;

FILE *ifp, *ofp1, *ofp2, *ofp3, *ofp4, *ofp5, *afp;

struct _OBJ_STRUCT *obtab2, obj2;

bool exiting;
bool reuseRoomData;
bool checkDmoves;

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
    if (al_errorCount > 30) {
        afatal("Terminating due to %u errors", al_errorCount);
    }
}

bool
consumeComment(const char *text)
{
    if (*text == '*')
        alog(AL_INFO, "%s", text);
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
            alog(AL_NOTE, "%s", block);
    } while (c != EOF && (c == '*' || c == ';' || isspace(c)));
    if (c == EOF && required) {
        afatal("File contained no data");
    }
    if (c == EOF)
        return false;
    fseek(ifp, -1, 1); /* Move back 1 char */
    return true;
}

void
fatalOp(const char *verb, const char *noun)
{
    alog(AL_ERROR, "Can't %s %s", verb, noun);
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
        afatal("Wrong write count: expected %d, got %d", objCount, read);
    }
    return read;
}

int
checkedfwrite(void *data, size_t objSize, size_t objCount, FILE *fp)
{
    size_t written = fwrite(data, objSize, objCount, fp);
    if (written != objCount) {
        afatal("Wrong write count: expected %d, got %d", objCount, written);
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

/* Check to see if s is a room flag */
int
isRoomFlag(const char *token)
{
    for (int i = 0; i < NRFLAGS; ++i) {
        if (strcmp(token, rflag[i]) == 0)
            return i;
    }
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
        afatal("Invalid adjective (length): %s", Word);
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
    afatal("Object #%d: %s: invalid %s: %s", g_gameConfig.numObjects + 1, obj2.id, s, Word);
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
    for (int i = 0; i < g_gameConfig.numMobPersonas; i++)
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
        alog(AL_ERROR, "Rank line %" PRIu64 " incomplete", g_gameConfig.numRanks);
        return false;
    }
    return true;
}

[[noreturn]] void
stateInvalid(const char *s)
{
    afatal("Object #%" PRIu64 ": %s: invalid %s state line: %s", g_gameConfig.numObjects + 1,
           obj2.id, s, block);
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
isnoun(const char *s)
{
    /// TODO: This should check the noun table...
    if (stricmp(s, "none") == 0)
        return -2;
    for (int i = 0; i < g_gameConfig.numObjects; i++)
        if (stricmp(s, obtab2[i].id) == 0)
            return i;
    return -1;
}

int
iscont(const char *s)
{
    for (int i = 0; i < g_gameConfig.numObjects; i++)
        if (stricmp(s, obtab2[i].id) == 0 && obtab2[i].contains > 0)
            return i;
    return -1;
}

/* Room or container */
int
isloc(const char *s)
{
    if (roomid_t rid = RoomIdx::Lookup(s); rid != -1) {
        return rid;
    }
    if (int i = iscont(s); i != -1) {
        return -(INS + i);
    }
    if (isnoun(s) == -1)
        alog(AL_ERROR, "Invalid object start location: %s", s);
    else
        alog(AL_ERROR, "Tried to start '%s' in non-container '%s'", obj2.id, s);
    return -1;
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
        alog(AL_ERROR, "Number too large: %s", s);
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
        if (stricmp(s, announceTypes[i]))
            return i;
    }
    alog(AL_ERROR, "Invalid announcement target: %s", s);
    return -1;
}

/* Test noun state, checking rooms */
int
isnounh(const char *s)
{
    int  i, l, j;
    long orm;

    if (stricmp(s, "none") == 0)
        return -2;
    FILE *fp = OpenGameFile(objectRoomFile, "rb");
    l = -1;

    for (i = 0; i < g_gameConfig.numObjects; i++) {
        const _OBJ_STRUCT &object = obtab2[i];
        if (stricmp(s, object.id) != 0)
            continue;
        fseek(fp, (uintptr_t)object.rmlist, 0L);  /// TODO: Dispose
        for (j = 0; j < object.nrooms; j++) {
            fread(&orm, 4, 1, fp);
            if (orm == rmn) {
                l = i;
                i = g_gameConfig.numObjects + 1;
                j = obj2.nrooms;
                break;
            }
        }
        if (i < g_gameConfig.numObjects)
            l = i;
    }
    fclose(fp);
    return l;
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
        error_t err = AddTextString(start, end, true, &id);
        if (err != 0)
            afatal("Unable to add string literal: %d", err);
    } else {
        getword(s);
        error_t err = LookupTextString(Word, &id);
        if (err != 0 && err != ENOENT)
            afatal("Error looking up string (%d): %s", err, s);
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
actualval(_SLOTTAB &vbslot, const char *s, int n)
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
        i = actualval(vbslot, s + 1, PREAL);
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
    if (!token)
        ctxLog(AL_FATAL, category, ": ", op->name, ": ", "param #", paramNo + 1, ": ", issue);
    else
        ctxLog(AL_FATAL, category, ": ", op->name, ": ", "param #", paramNo + 1, ": ", issue, ": '", token, "'");
    exit(1);
}

// Check the parameters accompanying a vmop
char *
checkParameter(_SLOTTAB &vbslot, char *p, const VMOP *op, size_t paramNo, const char *category, FILE *fp)
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
        value = actualval(vbslot, token, parameterType);
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
        value = RoomIdx::Lookup(token);
        break;
    case PVERB:
        value = VerbIdx::Lookup(token);
        break;
    case PADJ:
        break;
    case PREAL:
    case PNOUN:
        value = isnounh(token);
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
        if ((value = VerbIdx::Lookup(token)) == -1 || *token != '.')
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
checkParameters(_SLOTTAB &vbslot, char *p, const VMOP *op, FILE *fp, const char *category)
{
    for (size_t i = 0; i < op->parameterCount; i++) {
        if (!(p = checkParameter(vbslot, p, op, i, category, fp)))
            return nullptr;
    }
    return p;
}

char *
checkActionParameters(_SLOTTAB &vbslot, char *p, const VMOP *op, FILE *fp)
{
    return checkParameters(vbslot, p, op, fp, "action");
}

char *
checkConditionParameters(_SLOTTAB &vbslot, char *p, const VMOP *op, FILE *fp)
{
    return checkParameters(vbslot, p, op, fp, "condition");
}

void
chae_err()
{
    ctxLog(AL_ERROR, "Invalid precende flag: ", Word);
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
    ctxLog(AL_FATAL, "'", s2, "': ", s);
}

void
mobmis(const char *s)
{
    ctxLog(AL_ERROR, "Missing field: ", s);
    skipblock();
}

/* Fetch mobile message line */
stringid_t
getmobmsg(const char *s)
{
    for (;;) {
        char *p = getTidyBlock(ifp);
        if (feof(ifp))
            afatal("Mobile:%s: Unexpected end of file", mob.id);
        if (*p == 0 || isEol(*p)) {
            afatal("Mobile: %s: unexpected end of definition", mob.id);
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
            alog(AL_ERROR, "Mobile: %s: Invalid '%s' line", mob.id, s);
        }
        return n;
    }
}

///////////////////////////////////////////////////////////////////////////////

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
            afatal("Rank %" PRIu64 ": Invalid male rank: %s", g_gameConfig.numRanks + 1, Word);
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
            afatal("Rank %d: Invalid female rank: %s", g_gameConfig.numRanks + 1, Word);
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
            alog(AL_ERROR, "Invalid score value: %s", Word);
            continue;
        }
        rank.score = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid strength value: %s", Word);
            continue;
        }
        rank.strength = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid stamina value: %s", Word);
            continue;
        }
        rank.stamina = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid dexterity value: %s", Word);
            continue;
        }
        rank.dext = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid wisdom value: %s", Word);
            continue;
        }
        rank.wisdom = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid experience value: %s", Word);
            continue;
        }
        rank.experience = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid magic points value: %s", Word);
            continue;
        }
        rank.magicpts = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid max weight value: %s", Word);
            continue;
        }
        rank.maxweight = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid max inventory value: %s", Word);
            continue;
        }
        rank.numobj = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid kill points value: %s", Word);
            continue;
        }
        rank.minpksl = atoi(Word);

        p = getword(p);
        if (!isdigit(Word[0])) {
            alog(AL_ERROR, "Invalid task number: %s", Word);
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
            alog(AL_ERROR, "Rank %" PRIu64 " prompt too long: %s", g_gameConfig.numRanks + 1,
                 block);
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
//    if (ifp != NULL)
//        fclose(ifp);
//    ifp = NULL;
//    closeOutFiles();
//    fopenr(objectStateFile);
//    blkget(&datal, &data, NULL);
//    fclose(ifp);
//    ifp = NULL;
//    closeOutFiles();
//    fopenr(objrmsfn);
//    blkget(&datal2, &data2, NULL);
//    fclose(ifp);
//    ifp = NULL;
//    closeOutFiles();
//    fopenw(objsfn);
//    fopenw(objectStateFile);
//    fopenw(objrmsfn);
//    fopenw(ntabfn);
//    ifp = NULL;
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
//    data = data2 = NULL;
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
        afatal("Out of memory");

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
            alog(AL_ERROR, "noun= line with no noun");
            skipblock();
            continue;
        }
		alog(AL_DEBUG, "noun=%s", Word);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid object name (length): %s", cur);
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
                    alog(AL_ERROR, "object:%s: Invalid parameter: %s", obj2.id, Word);
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
                    afatal("Internal Error: Code for object-parameter '%s' missing", obparms[idNo]);
                }
            }
        }

        /* Get the room list */

        bool continuation = true;
        while (continuation) {
            char *p = getTidyBlock(ifp);
            if (!p)
                afatal("object:%s: unexpected end of file", obj2.id);

            while (*p) {
                continuation = false;
                p = getword(p);
                if (strcmp(Word, "+") == 0) {
                    continuation = true;
                    continue;
                }
                int roomNo = isloc(Word);
                if (roomNo == -1) {
                    alog(AL_ERROR, "object:%s: invalid room: %s", obj2.id, Word);
                }
                checkedfwrite(&roomNo, 1, sizeof(roomNo), ofp3);
                obj2.nrooms++;
            }
        }

        if (obj2.nrooms == 0)
            alog(AL_ERROR, "object:%s: no location given", obj2.id);

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
            alog(AL_ERROR, "object:%s: no states defined");
        if (obj2.nstates > 100)
            alog(AL_ERROR, "object:%s: too many states defined (%d)", obj2.nstates);

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
    int strip, i;
    proc = 0;

    nextc(true); /* Move to first text */

    assert(g_gameConfig.numTTEnts == 0);
    int ntt = 0, t = 0, r = 0;

    std::vector<verbid_t> verbsUsed;

    while (!feof(ifp)) {
        checkErrorCount();

        if (!nextc(false))
            break;

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        p = getWordAfter("room=", p);
        if (Word[0] == 0) {
            alog(AL_ERROR, "travel: empty room= line");
            skipblock();
            continue;
        }

        std::string word;
        AssignWord(word);
        Room *room = RoomIdx::Find(word);
        if (!room) {
            alog(AL_ERROR, "No such room: %s", word.c_str());
            skipblock();
            continue;
        }

        if (!room->travel.empty()) {
            alog(AL_ERROR, "Multiple travel table entries for room: %s", word.c_str());
            skipblock();
            continue;
        }

        ScopeContext roomCtx { word };

    vbloop:
        do
            fgets(block, sizeof(block), ifp);
        while (block[0] != 0 && consumeComment(block));
        if (block[0] == 0 || isEol(block[0])) {
            /* Only complain if room is not a death room */
            if ((room->flags & DEATH) != DEATH) {
                ctxLog(AL_INFO, "Room block with no travel entries");
                continue;
            }
        }
        tidy(block);
        if (!striplead("verb=", block) && !striplead("verbs=", block)) {
            ctxLog(AL_ERROR, "Missing verb[s]= entry");
            goto vbloop;
        }

    vbproc: /* Process verb list */
        p = block;
        verbsUsed.clear();
        /* Break verb list down to verb no.s */
        do {
            p = getword(p);
            if (Word[0] == 0)
                break;
            auto verbId = VerbIdx::Lookup(Word);
            if (verbId == -1) {
                ctxLog(AL_ERROR, "Invalid verb: ", Word);
            }
            verbsUsed.push_back(verbId);
        } while (Word[0] != 0);

        if (verbsUsed.empty()) {
            ctxLog(AL_ERROR, "Empty verb[s]= line");
            skipblock();
            continue;
        }

        /* Now process each instruction line */
        do {
            VMCnA instruction{};

            strip = 0;
            r = -1;
            fgets(block, sizeof(block), ifp);
            if (feof(ifp))
                break;
            if (block[0] == 0 || isEol(block[0])) {
                strip = -1;
                continue;
            }
            tidy(block);
            if (block[0] == 0 || consumeComment(block))
                continue;
            if (striplead("verb=", block) || striplead("verbs=", block)) {
                strip = 1;
                break;
            }
            p = precon(block); /* Strip pre-condition opts */

        notloop:
            p = getword(p);
            if (strcmp(Word, ALWAYSEP) == 0) {
                instruction.condition = CALWAYS;
                instruction.action = -(1 + AENDPARSE);
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
            if ((instruction.condition = iscond(Word)) == -1) {
                instruction.condition = CALWAYS;
                if ((instruction.action = RoomIdx::Lookup(Word)) != -1)
                    goto write;
                if ((instruction.action = isact(Word)) == -1) {
                    ctxLog(AL_ERROR, "Invalid condition: ", Word);
                    continue;
                }
                goto gotohere;
            }
            p = skipspc(p);
            if ((p = checkConditionParameters(vbslot, p, &conditions[instruction.condition], ofp2)) == nullptr) {
                goto next;
            }
            if (r == 1)
                instruction.condition = -1 - instruction.condition;
            if (*p == 0) {
                ctxLog(AL_ERROR, "Travel line is missing an action: ", block);
                continue;
            }
            p = preact(p);
            p = getword(p);
            if ((instruction.action = isRoom(Word)) != -1)
                goto write;
            if ((instruction.action = isact(Word)) == -1) {
                ctxLog(AL_ERROR, "Unrecognized action: ", Word);
                continue;
            }

        gotohere:
            if (instruction.action == ATRAVEL) {
                ctxLog(AL_ERROR, "'travel' action is illegal in travel table");
                continue;
            }
            p = skipspc(p);
            if ((p = checkActionParameters(vbslot, p, &actions[instruction.action], ofp2)) == nullptr) {
                goto next;
            }
            instruction.action = 0 - (instruction.action + 1);
        write:

            // this is some weird-ass kind of encoding where -1 means "more", and "-2" means "last"
            for (int verbNo = 0; verbNo < ttNumVerbs; ++verbNo) {
                room->travel.emplace_back(verbsUsed[i], instruction);
                g_gameConfig.numTTEnts++;
            }
        next:
            strip = 0;
        } while (strip == 0 && !feof(ifp));
        if (strip == 1 && !feof(ifp))
            goto vbproc;
    }

    for (auto&& room : RoomIdx) {
        if (room.travel.empty() && (room.flags & DEATH) != DEATH) {
            ctxLog(AL_WARN, "No TT entry for room: ", room.id);
            checkErrorCount();
        }
    }
}

/* From and To */
int
chae_proc(const char *f, char *t)
{
    int n;

    if ((*f < '0' || *f > '9') && *f != '?') {
        chae_err();
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
            chae_err();
            return -1;
        }
        *(t++) = (char)n;
    }

    for (n = 1; n < 5; n++) {
        if (*f == 'c' || *f == 'h' || *f == 'a' || *f == 'e') {
            *(t++) = toupper(*f);
            f++;
        } else {
            chae_err();
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

        alog(AL_ERROR, "Expected verb flag/precedence, got: %s", Word);
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
        int extant = is_verb(Word);
        if (extant != -1) {
            if (vbptr[extant].flags | VB_TRAVEL) {
                alog(AL_ERROR, "Redefinition of travel verb: %s", Word);
                continue;
            }
            vbptr[extant].flags |= VB_TRAVEL;
            alog(AL_DEBUG, "Added TRAVEL to existing verb %s", Word);
            continue;
        }
        /// TODO: size check
        strncpy(g_verb.id, Word, sizeof(g_verb.id));
        checkedfwrite(&g_verb, sizeof(g_verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameConfig.numVerbs++)) = g_verb;
        alog(AL_DEBUG, "Added TRAVEL verb: %s", Word);
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
            alog(AL_ERROR, "Invalid synonym line: %s", block);
            continue;
        }
        int id = isnoun(Word);
        if (id < 0) {
            id = is_verb(Word);
            if (id == -1) {
                alog(AL_ERROR, "Invalid verb/noun: %s", Word);
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
                mobd->dmove = isRoom(Word);
                if (mobd->dmove == -1) {
                    alog(AL_ERROR, "Mobile: %s: invalid dmove: %s", mob.id, Word);
                }
                continue;
            }
        }

        p = getTidyBlock(ifp);
        if (!p)
            afatal("mob: !%s: unexpected end of file", mob.id);

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
            alog(AL_ERROR, "Mobile: %s: Travel+Fight+Act+Wait values not equal to 100%.", mob.id);
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
            afatal("Out of memory");
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
    if (context == NULL) {
        afatal("Out of memory for context");
    }

    if (EINVAL == path_join(context->filename, sizeof(context->filename), gameDir, filename)) {
        afatal("Path length exceeds limit for %s/%s", gameDir, filename);
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
    { "sysmsg", true, smsg_proc },      // system messages
    { "umsg", true, umsg_proc },        // user-defined string messages
    { "obdescs", true, obds_proc },     // object description strings
    { "rooms", true, room_proc },       // room table
    { "roomread", false, load_rooms },  // re-load room table
    { "dmoves", false, checkdmoves },   // check cemeteries
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
    alog(AL_INFO, "Compiling: %s", phase->name);
    SetContext(phase->name);
    if (phase->isText) {
        char filepath[MAX_PATH_LENGTH];
        MakeTextFileName(phase->name, filepath);
        FILE *fp = fopen(filepath, "r");
        if (!fp)
            afatal("Could not open %s file: %s", phase->name, filepath);
        ifp = fp;
    }

    phase->handler();

    if (ifp) {
        fclose(ifp);
        ifp = nullptr;
    }

    CloseOutFiles();

    if (al_errorCount > 0) {
        afatal("Terminating due to errors.");
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

error_t
compilerModuleStart(Module *module)
{
    setProcessTitle(compilerVersion.c_str());
    alog(AL_INFO, "AMUL Compiler %s", compilerVersion.c_str());

    alog(AL_DEBUG, "Game Directory: %s", gameDir);
    alog(AL_DEBUG, "Log Verbosity : %s", alogGetLevelName());
    alog(AL_DEBUG, "Check DMoves  : %s", checkDmoves ? "true" : "false");
    alog(AL_DEBUG, "Reuse Rooms   : %s", reuseRoomData ? "true" : "false");

    struct stat sb {
        0
    };

    for (const CompilePhase *phase = &phases[0]; phase->name; ++phase) {
        if (phase->isText == false)
            continue;

        char filepath[MAX_PATH_LENGTH];
        MakeTextFileName(phase->name, filepath);
        int err = stat(filepath, &sb);
        if (err != 0)
            afatal("Missing file (%d): %s", err, filepath);
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
    ReleaseMem((void **)&rmtab);

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

    alog(AL_NOTE, "Execution finished normally");
    alog(AL_INFO, "Statistics for %s:", g_gameConfig.gameName);
    alog(AL_INFO, "		Rooms: %6" PRIu64 "	Ranks:   %6" PRIu64 " Objects: %6" PRIu64 "",
         g_gameConfig.numRooms, g_gameConfig.numRanks, g_gameConfig.numObjects);
    alog(AL_INFO, "		Adj's: %6" PRIu64 "	Verbs:   %6" PRIu64 "    Syns: %6" PRIu64 "",
         g_gameConfig.numAdjectives, g_gameConfig.numVerbs, g_gameConfig.numSynonyms);
    alog(AL_INFO, "		T.T's: %6" PRIu64 "	Strings: %6" PRIu64 "    Text: %6" PRIu64 "",
         g_gameConfig.numTTEnts, g_gameConfig.numStrings, g_gameConfig.stringBytes);

    fopenw(gameDataFile);
    checkedfwrite(&g_gameConfig, sizeof(g_gameConfig), 1, ofp1);
    CloseOutFiles();

    exiting = true;

    return 0;
}
