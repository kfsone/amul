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

#include <cassert>
#include <charconv>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <map>
#include <set>
#include <string>
#include <sys/stat.h>
#include <vector>

#include "amigastubs.h"
#include "amul.acts.h"
#include "amul.argp.h"
#include "amul.cons.h"
#include "amul.defs.h"
#include "amul.msgs.h"
#include "amul.stct.h"
#include "amul.strs.h"
#include "amul.test.h"
#include "amul.vars.h"
#include "amul.vmop.h"
#include "amul.xtra.h"
#include "amulcom.fileparser.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.h"
#include "amulcom.strings.h"
#include "amulcom.version.h"
#include "amullib.h"
#include "filesystem.h"
#include "filesystem.inl.h"
#include "game.h"
#include "logging.h"
#include "modules.h"
#include "objflag.h"
#include "ptype.h"
#include "roomflag.h"
#include "sourcefile.h"
#include "sourcefile.log.h"
#include "spellid.h"
#include "statid.h"
#include "stringmanip.h"
#include "svparse.h"
#include "system.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables

constexpr string_view compilerVersion{ AMULCOM_VSTRING };

/* Compiler specific variables... */

char Word[64]; /* For internal use only <grin>  */
int proc;      /* What we are processing    */
long wizstr;   /* Wizards strength      */

char scratch[4096];  // scratch pad

static Syntax vbslot;

// counters
Game g_game;

thread_local FILE *ifp, *ofp1;

bool exiting;

std::map<std::string, std::string> s_dmoveLookups;
std::map<std::string, roomid_t> s_roomIdx;
std::map<std::string, adjid_t> s_adjectiveIdx;

Room *c_room;

using OptionalSourceFile = std::optional<SourceFile>;

///////////////////////////////////////////////////////////////////////////////////////////////////
//

// Stubs for g_game members etc that we don't use.

Avatar::Avatar() {}

///////////////////////////////////////////////////////////////////////////////////////////////////
//

void
CloseOutFiles()
{
    CloseFile(&ofp1);
}

void
CheckErrorCount()
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
            fgets(scratch, sizeof(scratch), ifp);
        if (c == '*')
            LogNote(scratch);
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
fatalOp(string_view verb, string_view noun)
{
    LogError("Can't ", verb, " ", noun);
}

FILE *
OpenGameFile(const char *filename, const char *mode)
{
    std::string filepath{};
    safe_gamedir_joiner(filename);
    FILE *fp = fopen(filepath.c_str(), mode);
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
    else
        fatalOp("select", "file descriptor");
    *fpp = OpenGameFile(filename, "wb");
}

/* Open file for reading */
void
fopenr(const char *filename)
{
    if (ifp != nullptr)
        fclose(ifp);
    ifp = OpenGameFile(filename, "rb");
}

constexpr auto checkedfread = [](FILE *fp, auto *data, size_t count) {
    ssize_t gotCount = fread(data, sizeof(*data), count, fp);
    if (size_t(gotCount) != count) {
        LogFatal("Wrong write count: expected ", count, ", got ", gotCount);
    }
    return sizeof(*data) * count;
};

constexpr auto checkedfwrite = [](FILE *fp, auto *data, size_t count) {
    auto gotCount = fwrite(data, sizeof(*data), count, fp);
    if (size_t(gotCount) != count) {
        LogFatal("Wrong write count: expected ", count, ", got ", gotCount);
    }
    return sizeof(*data) * count;
};

void
skipParagraph()
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
getTidiedLineToScratch(FILE *fp)
{
    for (;;) {
        if (!fgets(scratch, sizeof(scratch), fp))
            return nullptr;

        tidy(scratch);

        char *p = skipspc(scratch);
        if (!consumeComment(p) && *p)
            return p;
    }
}

/* Check to see if s is a room flag */
int
isRoomFlag(string_view name)
{
    for (int i = 0; i < NRFLAGS; i++) {
        if (name == rflag[i])
            return i;
    }
    return WNONE;
}

roomid_t
isRoomName(string_view token)
{
    std::string rmname{ token };
    StringLower(rmname);
    if (auto it = s_roomIdx.find(rmname); it != s_roomIdx.end()) {
        return it->second;
    }
    return WNONE;
}

/* Is it a FIXED object flag? */
int
IsObjectFlag(string_view token)
{
    for (int i = 0; i < NOFLAGS; i++)
        if (obflags1[i] == token)
            return i;
    return WNONE;
}

ObjectParameter
GetObjectParam(string_view &param)
{
    for (int i = 0; i < NOPARMS; i++) {
        if (RemovePrefix(param, obparms[i]))
            return ObjectParameter(i);
    }
    return OP_NONE;
}

/* Is it a state flag? */
int
IsObjectStateFlag(string_view token)
{
    for (int i = 0; i < NSFLAGS; i++)
        if (obflags2[i] == token)
            return i;
    return WNONE;
}

int
iscond(string_view condition)
{
    for (int i = 0; i < NCONDS; i++) {
        if (condition == conditions[i].name)
            return i;
    }
    return -1;
}

int
isact(string_view action)
{
    for (int i = 0; i < NACTS; i++)
        if (action == actions[i].name)
            return i;
    return -1;
}

bool
checkRankLine(const char *p)
{
    if (*p == 0) {
        LogError("Rank line ", g_game.numRanks, " incomplete");
        return false;
    }
    return true;
}

[[noreturn]] void
stateInvalid(const Object &obj, const char *s)
{
    LogFatal("Object #",
             g_game.numObjects + 1,
             ": ",
             obj.id,
             ": invalid ",
             s,
             " state line: ",
             scratch);
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

objid_t
isNoun(string_view noun)
{
    /// TODO: This should check the noun table...
    if (noun == "none")
        return -2;
    std::string word{ noun };
    StringLower(word);
    for (objid_t objId = 0; objId < objid_t(g_game.m_objects.size()); ++objId) {
        if (g_game.m_objects[objId].id == noun)
            return objId;
    }
    return WNONE;
}

objid_t
isContainer(string_view name)
{
    for (size_t i = 0; i < g_game.m_objects.size(); ++i) {
        if (g_game.m_objects[i].contains > 0 && name == g_game.m_objects[i].id)
            return objid_t(i);
    }
    return WNONE;
}

/* Room or container */
int
isloc(const Object &obj, const char *s)
{
    int i;

    if ((i = isRoomName(s)) != -1)
        return i;
    if ((i = isContainer(s)) == -1) {
        if (isNoun(s) == -1)
            LogError("Invalid object start location: ", s);
        else
            LogError("Tried to start '", obj.id, "' in non-container: ", s);
        return WNONE;
    }

    return -(INS + i);
}

static std::set<string_view> PreConditionWords = { "if", "the", "i", "am", "is" };

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

static std::set<string_view> PreActionWords = { "then", "go", "to", "goto", "set" };

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
        return (long) -n;
    if (*s == '>')
        return (long) (n | LESS);
    if (*s == '<')
        return (long) (n | MORE);
    return n;
}

static const std::set<string_view> OptionalPrefixes = { "the",  "of",  "are", "is",   "has", "next",
                                                        "with", "to",  "set", "from", "for", "by",
                                                        "and",  "was", "i",   "am",   "as" };

char *
skipOptionalPrefixes(char *p)
{
    while (*p) {
        p = skipspc(p);
        bool match = false;
        for (auto &prefix : OptionalPrefixes) {
            if (strncmp(p, prefix.data(), prefix.length()) == 0) {
                if (isspace(p[prefix.length()])) {
                    p += prefix.length() + 1;
                    match = true;
                    break;
                }
            }
        }
        if (!match)
            break;
    }
    return p;
}

int
parseGender(string_view token) noexcept
{
    if (!token.empty()) {
        switch (token.front()) {
            case 'm':
                return 0;
            case 'f':
                return 1;
        }
    }
    return WNONE;
}

int
announceType(string_view typeName)
{
    constexpr std::array<string_view, MAX_ANNOUNCE_TYPE> announceTypes{
        "global", "everyone", "outside", "here", "others", "all", "cansee", "notsee"
    };

    auto it = std::find(announceTypes.begin(), announceTypes.end(), typeName);
    if (it != announceTypes.end()) {
        return std::distance(announceTypes.begin(), it);
    }

    LogError("Invalid announcement target: ", typeName);
    return WNONE;
}

int
rdmode(string_view token)
{
    if (!token.empty()) {
        switch (token.front()) {
            case 'r':
                return RDRC;
            case 'v':
                return RDVB;
            case 'b':
                return RDBF;
        }
    }
    return WNONE;
}

int
parseSpellName(string_view spellName) noexcept
{
    static const std::map<std::string_view, int> spellNameIndex{
        { "glow", SGLOW },   { "invis", SINVIS },   { "deaf", SDEAF },
        { "dumb", SDUMB },   { "blind", SBLIND },   { "cripple", SCRIPPLE },
        { "sleep", SSLEEP }, { "sinvis", SSINVIS }, { "superinvis", SSINVIS },
    };

    if (auto it = spellNameIndex.find(spellName); it != spellNameIndex.end())
        return it->second;

    return WNONE;
}

int
is_stat(string_view statName) noexcept
{
    static const std::map<std::string_view, int> statNameToId{
        { "sctg"sv, STSCTG },      { "score"sv, STSCORE },  { "points"sv, STSCORE },
        { "str"sv, STSTR },        { "strength"sv, STSTR }, { "sta"sv, STSTAM },
        { "stamina"sv, STSTAM },   { "dex"sv, STDEX },      { "dexterity"sv, STDEX },
        { "wis"sv, STWIS },        { "wisdom"sv, STWIS },   { "exp"sv, STEXP },
        { "experience"sv, STEXP }, { "mag"sv, STMAGIC },    { "magic"sv, STMAGIC },
    };

    if (auto it = statNameToId.find(statName); it != statNameToId.end())
        return it->second;
    return WNONE;
}

int
parseVerbosity(string_view token) noexcept
{
    if (!token.empty()) {
        switch (token.front()) {
            case 'v':
                return TYPEV;
            case 'b':
                return TYPEB;
        }
    }
    return WNONE;
}

stringid_t
getTextString(string_view string)
{
    stringid_t id = WNONE;
    if (!string.empty() && (string.front() == '"' || string.front() == '\'')) {
        string.remove_prefix(1);
        error_t err = AddTextString(string, true, &id);
        if (err != 0)
            LogFatal("Unable to add string literal: ", err);
    } else {
        RemovePrefix(string, "msg=");
        RemovePrefix(string, "msgid=");
        error_t err = LookupTextString(string, &id);
        if (err == ENOENT)
            id = WNONE;
        else if (err != 0)
            LogFatal("Error looking up string '", string, "': ", err);
    }
    return id;
}

stringid_t
getTextString(char *s, bool prefixed)
{
    if (prefixed) {
        s = skipspc(s);
        s = skiplead("msgid=", s);
    }

    stringid_t id = -1;
    if (*s == '\"' || *s == '\'') {
        char *start = s + 1;
        char *end = strstop(start, *s);
        string_view text{ start, size_t(end - start) };
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

bool
onoff(string_view p)
{
    return (p == "on" || p == "yes");
}

/* Note about matching actuals...

Before agreeing a match, remember to check that the relevant slot isn't
set to NONE.
Variable N is a wtype... If the phrases 'noun', 'noun1' or 'noun2' are used,
instead of matching the phrases WTYPE with n, match the relevant SLOT with
n...

So, if the syntaxes line is 'verb text player' the command 'tell noun2 text'
will call isactual with *s=noun2, n=WPLAYER.... is you read the 'actual'
structure definition, 'noun2' is type 'WNOUN'. WNOUN != WPLAYER, HOWEVER
the slot for noun2 (vbslot.wtype[4]) is WPLAYER, and this is REALLY what the
user is referring too.                               */

/* Get actual value! */
amulid_t
actualval(string_view token, amulid_t n)
{
    // you can prefix runtime-variable references with these characters to modify
    // the resulting value, e.g. choosing a random number based on your rank.
    /// TODO: replace with something akin to SQL aggregate functions.
    /// TODO: rand(myrank)
    char front = token.front();
    if (n != PREAL && strchr("?%^~`*#", front)) {
        // These can only be applied to numbers except '*' which can be applied to
        // rooms
        if (n != WNUMBER && !(n == WROOM && front == '*'))
            return -1;
        // calculate random numbers based on the parameter
        if (front == '~')
            return RAND0 + atoi(token.data() + 1);
        if (front == '`')
            return RAND1 + atoi(token.data() + 1);
        // then it's a modifier: recurse and add our flags.
        auto i = actualval(token.substr(1), PREAL);
        if (i == WNONE)
            return WNONE;
        // The remaining prefixes can only be attached to properties of the player.
        if (front == '#' && (i & IWORD || (i & MEPRM && i & (SELF | FRIEND | HELPER | ENEMY))))
            return PRANK + i;
        if (!(i & IWORD))
            return WNONE;
        if (front == '?')
            return OBVAL + i;
        if (front == '%')
            return OBDAM + i;
        if (front == '^')
            return OBWHT + i;
        if (front == '*')
            return OBLOC + i;
        if (front == '#')
            return PRANK + i;
        return WNONE;
    }
    if (!isalpha(front))
        return -2;
    for (int i = 0; i < int(NACTUALS); i++) {
        auto &cur = actual[i];
        if (cur.m_name != token)
            continue;
        /* If its not a slot label, and the wtypes match, we's okay! */
        if (!(cur.m_value & IWORD))
            return (cur.m_wtype == n || n == PREAL) ? cur.m_value : -1;

        /* Now we know its a slot label... check which: */
        switch (cur.m_value - IWORD) {
            case IVERB: /* Verb */
                if (n == PVERB || n == PREAL)
                    return cur.m_value;
                return WNONE;

            case IADJ1: /* Adjective #1 */
                if (vbslot.wtype[0] == n)
                    return cur.m_value;
                if (token.back() != '1' && vbslot.wtype[3] == n)
                    return IWORD + IADJ2;
                if (n == PREAL)
                    return cur.m_value;
                return WNONE;

            case INOUN1: /* noun 1 */
                if (vbslot.wtype[1] == n)
                    return cur.m_value;
                if (token.back() != '1' && vbslot.wtype[4] == n)
                    return IWORD + INOUN2;
                if (n == PREAL)
                    return cur.m_value;
                return WNONE;
            case IADJ2:
                return (vbslot.wtype[3] == n || n == PREAL) ? cur.m_value : WNONE;

            case INOUN2:
                return (vbslot.wtype[4] == n || n == PREAL) ? cur.m_value : WNONE;

            default:
                return WNONE;
        }
    }
    // We didn't recognize the thing at all
    return WINVALID;
}

void
badParameter(string_view srcId,
             const VMOP *op,
             size_t paramNo,
             const char *category,
             const char *issue,
             string_view token,
             OptionalSourceFile src = OptionalSourceFile{})
{
    char msg[256];
    if (!token.empty()) {
        snprintf(msg, sizeof(msg), "%s: '%s'", issue, std::string{ token }.c_str());
        issue = msg;
    }
    if (!src) {
        LogError(proc ? "verb" : "room",
                 "=",
                 srcId,
                 ": ",
                 category,
                 "=",
                 op->name,
                 ": parameter#",
                 paramNo + 1,
                 ": ",
                 issue);
    } else {
        src->Error(proc ? "verb" : "room",
                   "=",
                   srcId,
                   ": ",
                   category,
                   "=",
                   op->name,
                   ": parameter#",
                   paramNo + 1,
                   ": ",
                   issue);
    }
}

// Check the parameters accompanying a vmop
bool
checkParameter(string_view srcId,
               string_view token,
               const VMOP *op,
               VMOper *oper,
               size_t paramNo,
               const char *category,
               OptionalSourceFile src = OptionalSourceFile{})
{
    amulid_t value = WNONE;
    const int8_t parameterType = op->parameters[paramNo];

    /* Processing lang tab? */
    if ((parameterType >= 0 && parameterType <= 10) || parameterType == PREAL) {
        value = actualval(token, parameterType);
        if (value == WNONE) {
            /* it was an actual, but wrong type */
            badParameter(srcId, op, paramNo, category, "Invalid syntax slot label", token, src);
            return false;
        }
        if (value != -2) {
            oper->m_args[paramNo] = value;
            return true;
        }
    }
    switch (parameterType) {
        case -6:
            value = onoff(token) ? 1 : 0;
            break;
        case -5:
            value = parseVerbosity(token);
            break;
        case -4:
            value = is_stat(token);
            break;
        case -3:
            value = parseSpellName(token);
            break;
        case -2:
            value = rdmode(token);
            break;
        case -1:
            value = announceType(token);
            break;
        case PROOM:
            value = isRoomName(token);
            break;
        case PVERB:
            value = IsVerb(token);
            break;
        case PADJ:
            break;
        case PREAL:
        case PNOUN:
            value = isNoun(token);
            break;
        case PUMSG:
            value = getTextString(token);
            break;
        case PNUM:
            value = chknum(token.data());
            break;
        case PRFLAG:
            value = isRoomFlag(token);
            break;
        case POFLAG:
            value = IsObjectFlag(token);
            break;
        case PSFLAG:
            value = IsObjectStateFlag(token);
            break;
        case PSEX:
            value = parseGender(token);
            break;
        case PDAEMON:
            if ((value = IsVerb(token)) == -1 || token.front() != '.')
                value = -1;
            break;
        default: {
            if (!(proc == 1 && parameterType >= 0 && parameterType <= 10)) {
                char param[32];
                snprintf(param, sizeof(param), "%d", parameterType);
                badParameter(srcId,
                             op,
                             paramNo,
                             category,
                             "INTERNAL ERROR: Unhandled parameter type",
                             param,
                             src);
                return false;
            }
        }
    }
    if (parameterType == PREAL && value == -2)
        value = -1;
    else if (((value == -1 || value == -2) && parameterType != PNUM) || value == -1000001) {
        badParameter(srcId,
                     op,
                     paramNo,
                     category,
                     "Invalid/unrecognized value for position",
                     token,
                     src);
        return false;
    }

    oper->m_args[paramNo] = value;
    return true;
}

string_view
selectToken(string_view srcId, const VMOP *op, size_t paramNo, const char *category, char *&p)
{
    p = skipOptionalPrefixes(p);
    if (!p || *p == '\0') {
        badParameter(srcId, op, paramNo, category, "Unexpected end of arguments", "");
        return "";
    }

    char *begin = p;
    char *end = nullptr;
    if (*p != '\"' && *p != '\'') {
        end = strstop(p, ' ');
        p = end;
    } else {
        // Subsequent parsing will want to see the opening quote character
        char quote = *(p++);
        if (!*p || isEol(*p))
            badParameter(srcId,
                         op,
                         paramNo,
                         category,
                         "Unexpected end of string (missing close quote?)",
                         std::string_view{begin, size_t(p - begin)});
        end = strstop(p, quote);
        if (*end == quote)
            p = end + 1;
    }
    return string_view{ begin, size_t(end - begin) };
}

char *
checkParameters(string_view srcId, char *p, const VMOP *op, VMOper *ins, const char *category)
{
    for (size_t i = 0; i < op->parameterCount; i++) {
        string_view token = selectToken(srcId, op, i, category, p);
        if (!p || !checkParameter(srcId, token, op, ins, i, category))
            return nullptr;
    }
    return p;
}

ssize_t
checkParameters(string_view srcId,
                SourceFile::iterator begin,
                SourceFile::iterator end,
                const VMOP *op,
                VMOper *ins,
                const char *category,
                SourceFile &src)
{
    auto it = begin;
    for (size_t i = 0; i < op->parameterCount; ++i, ++it) {
        for (;;) {
            if (it == end) {
                src.Error("Unexpected end of ", category, "parameters");
                return -1;
            }
            if (OptionalPrefixes.find(*it) == OptionalPrefixes.end())
                break;
            ++it;
        }
        if (!checkParameter(srcId, *it, op, ins, i, category))
            return -1;
    }
    return std::distance(begin, it);
}

char *
checkActionParameters(string_view srcId, char *p, const VMOP *op, VMLine *line)
{
    return checkParameters(srcId, p, op, &(line->action), "action");
}

auto
checkActionParameters(string_view srcId,
                      SourceFile::iterator begin,
                      SourceFile::iterator end,
                      VMLine *line,
                      SourceFile &src)
{
    return checkParameters(
            srcId, begin, end, &actions[line->action.m_op], &line->action, "action", src);
}

char *
checkConditionParameters(const char *srcId, char *p, const VMOP *op, VMLine *line)
{
    return checkParameters(srcId, p, op, &(line->condition), "condition");
}

auto
checkConditionParameters(string_view srcId,
                         SourceFile::iterator begin,
                         SourceFile::iterator end,
                         VMLine *line,
                         SourceFile &src)
{
    return checkParameters(srcId,
                           begin,
                           end,
                           &conditions[line->condition.m_op],
                           &line->condition,
                           "condition",
                           src);
}

static void
chae_err(const Verb &verb, string_view word)
{
    LogError("Verb: ", verb.id, ": Invalid '#CHAE' flag: ", word);
}

/* Set the VT slots */
void
setslots(const WType i)
{
    vbslot.wtype[0] = WANY;
    vbslot.wtype[1] = i;
    vbslot.wtype[2] = i;
    vbslot.wtype[3] = WANY;
    vbslot.wtype[4] = i;
    vbslot.slot[0] = vbslot.slot[1] = vbslot.slot[2] = vbslot.slot[3] = vbslot.slot[4] = WANY;
}

/* Is 'text' a ptype */
WType
iswtype(char *s)
{
    for (WType i = WType(0); i < WType(NSYNTS); i = WType(int(i) + 1)) {
        char *end = s;
        if (!canSkipLead(syntaxes[i], &end))
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
        return WType(int(i) + int(WNONE));
    }
    return WINVALID;
}

/* Declare a PROBLEM, and which verb its in! */
static void
vbprob(const Verb &verb, const char *s, const char *s2)
{
    LogError("Verb: ", verb.id, " line: '", s2, "': ", s);
}

///////////////////////////////////////////////////////////////////////////////

// Process ROOMS.TXT
void
room_proc(const std::string &filepath)
{
    SourceFile src{ filepath };
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath, ": ", err);
    }

    std::string text{};
    text.reserve(4096);

    g_game.m_rooms.reserve(512);

    while (!src.Eof()) {
        // Terminate if we've reached the error limit.
        CheckErrorCount();

        // Try and consume some non-whitespace tokens
        if (!src.GetIDLine("room=")) {
            continue;
        }

        // Because 'room' is a global, we have to clear it out.
        std::string rmname{ src.line.front() };
        StringLower(rmname);

        Room room{};
        strncpy(room.id, rmname.c_str(), sizeof(room.id));

        // If there are additional words on the line, they are room flags.

        for (auto it = src.line.begin() + 1; it != src.line.end(); ++it) {
            auto token = *it;
            if (RemovePrefix(token, "dmove=")) {
                if (token.empty()) {
                    LogError(src, "room:", rmname, ": dmove= without a destination name");
                    continue;
                }
                if (s_dmoveLookups.find(rmname) != s_dmoveLookups.end()) {
                    LogError(src, "room:", rmname, ": multiple 'dmove's");
                    break;
                }
                std::string dmove{ token };
                StringLower(dmove);
                s_dmoveLookups[rmname] = dmove;
                continue;
            }

            std::string flag{ *it };
            StringLower(flag);
            int no = isRoomFlag(flag);
            if (no == -1) {
                LogError(src, "room:", rmname, ": invalid room flag: ", *it);
                continue;
            }
            if (no == 0)
                continue;
            auto bit = bitset(no - 1);
            if (room.flags & bit) {
                LogWarn(src, "room:", rmname, ": duplicate room flag: ", *it);
            }
            room.flags |= bit;
        }

        if (src.GetLine() && !src.line.empty() && !src.line.front().empty()) {
            if (src.line.front().front() == '\t')
                src.line.front().remove_prefix(1);
            // Short description.
            AddTextString(src.line.front(), false, &room.shortDesc);

            // Check for a long description.
            text.clear();
            src.GetLines(text);
            if (!text.empty())
                AddTextString(text, false, &room.longDesc);
        }

        s_roomIdx[rmname] = g_game.m_rooms.size();
        g_game.m_rooms.push_back(room);

        ++g_game.numRooms;
    }

    for (auto &it : s_dmoveLookups) {
        CheckErrorCount();
        const auto &rmname = it.first, &dmoveName = it.second;
        auto dmoveIt = s_roomIdx.find(dmoveName);
        if (dmoveIt == s_roomIdx.end()) {
            LogError("room:", rmname, ": could not resolve 'dmove': ", dmoveName);
            continue;
        }
        auto roomIt = s_roomIdx.find(rmname);
        if (roomIt == s_roomIdx.end())
            LogFatal("room:", rmname, ": internal error: room not present in index");
        g_game.m_rooms[roomIt->second].dmove = dmoveIt->second;
    }
}

/* Process RANKS.TXT */
void
rank_proc(const std::string & /*filepath*/)
{
    nextc(true);

    while (!feof(ifp)) {
        if (!nextc(false))
            break;

        char *p = getTidiedLineToScratch(ifp);
        if (!p)
            continue;

        p = getword(scratch);
        if (!checkRankLine(p))
            continue;

        if (strlen(Word) < 3 || p - scratch > RANKL) {
            LogFatal("Rank ", g_game.numRanks + 1, ": Invalid male rank: ", Word);
        }

        Rank rank{};
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
            LogFatal("Rank ", g_game.numRanks + 1, ": Invalid female rank: ", Word);
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
        strcpy(scratch, p);
        p = scratch;
        while (*p != 0 && *p != '\"')
            p++;
        /* Greater than prompt length? */
        if (p - scratch > 10) {
            LogError("Rank #", g_game.numRanks + 1, ": prompt too long: ", scratch);
            continue;
        }
        if (scratch[0] == 0)
            strcpy(rank.prompt, "$ ");
        else
            WordCopier(rank.prompt, scratch, p);

        wizstr = rank.strength;
        g_game.m_ranks.push_back(rank);
        g_game.numRanks++;
    }
}

constexpr auto StateError = [](const SourceFile &src, const Object &obj, const auto &... args) {
    LogError(src.filepath,
             ":",
             src.lineNo,
             ":",
             obj.id,
             ":state#",
             uint32_t(obj.nstates),
             ": ",
             args...);
    return false;
};

/// TODO: constexpr lambda
template<typename T>
bool
parseObjectStateAttribute(SourceFile &src, Object &obj, string_view prefix, T *into)
{
    if (src.Eol()) {
        return StateError(src, obj, "Premature end of state line (expected ", prefix, " value)");
    }
    auto cur = src.PopFront();
    int64_t value{ 0 };
    bool valid = ToInt(cur, value);
    if (!valid || value > std::numeric_limits<T>::max()) {
        return StateError(src,
                          obj,
                          "Invalid attribute value: ",
                          cur,
                          " (expected a positive numeric ",
                          prefix,
                          " value)");
    }
    if constexpr (std::numeric_limits<T>::min() == 0) {
        if (value < 0) {
            return StateError(
                    src, obj, "Invalid ", prefix, " value (expected a positive value): ", value);
        }
    }
    *into = static_cast<T>(value);
    return true;
};

bool
parseObjectStateAttributes(SourceFile &src, Object &obj, ObjState &state)
{
    if (!parseObjectStateAttribute(src, obj, "weight=", &state.weight))
        return false;
    if (!parseObjectStateAttribute(src, obj, "value=", &state.value))
        return false;
    if (!src.Eol())
        RemovePrefix(src.line.front(), "str=");
    if (!parseObjectStateAttribute(src, obj, "health=", &state.value))
        return false;
    if (!parseObjectStateAttribute(src, obj, "dmg=", &state.damage))
        return false;

    return true;
}

bool
parseObjectState(SourceFile &src, Object &obj)
{
    ObjState state{};
    // scenery objects don't have stat attributes
    if (!(obj.flags & OF_SCENERY)) {

        if (!parseObjectStateAttributes(src, obj, state))
            return false;
    } else {
        state.weight = ~0U;
    }

    if (src.Eol())
        return StateError(src, obj, "Expected state description id/string");

    auto cur = src.PopFront();
    RemovePrefix(cur, "desc=");
    state.description = getTextString(cur);

    while (!src.Eol()) {
        cur = src.PopFront();
        if (int flag = IsObjectStateFlag(cur); flag != WNONE) {
            state.flags |= bitset(flag);
        } else {
            StateError(src, obj, "Unrecognized object state flag: ", cur);
            return false;
        }
    }

    g_game.m_objectStates.push_back(state);
    obj.nstates++;
    g_game.numObjStates++;

    return true;
}

bool
processObjectStates(SourceFile &src, Object &obj)
{
    while (src.GetLineTerms()) {
        CheckErrorCount();
        parseObjectState(src, obj);
    }
    return true;
}

constexpr auto ObjectError = [](const SourceFile &src, const Object &obj, const auto &... args) {
    LogError(src.filepath, ":", src.lineNo, ":", obj.id, ": ", args...);
    return false;
};

bool
processObjectAdj(const SourceFile &src, Object &obj, string_view cur)
{
    if (cur.length() < 3 || cur.length() > IDL) {
        return ObjectError(src, obj, "Invalid adjective (length): ", cur);
    }

    if (auto it = s_adjectiveIdx.find(Word); it != s_adjectiveIdx.cend()) {
        obj.adj = it->second;
        return true;
    }

    Adjective adj{};
    strncpy(adj.word, cur.data(), std::min(sizeof(adj.word), cur.length()));
    adjid_t id = s_adjectiveIdx[std::string{ cur }] = adjid_t(g_game.m_adjectives.size());
    g_game.m_adjectives.push_back(adj);
    g_game.numAdjectives++;

    return true;
}

bool
processObjectStartState(const SourceFile &src, Object &obj, string_view cur)
{
    int stateNo{ 0 };
    auto result = std::from_chars(cur.data(), cur.data() + cur.size(), stateNo);
    if (result.ec == std::errc::invalid_argument) {
        return ObjectError(src, obj, "Invalid start state (expected a numeric value): ", cur);
    }
    if (stateNo < 0) {
        return ObjectError(src, obj, "Invalid start state (expected a positive value): ", cur);
    }
    obj.state = stateNo;
    return true;
}

bool
processObjectHolds(const SourceFile &src, Object &obj, string_view cur)
{
    int holds{ 0 };
    auto result = std::from_chars(cur.data(), cur.data() + cur.size(), holds);
    if (result.ec == std::errc::invalid_argument) {
        return ObjectError(src, obj, "Invalid holds= value (expected a numeric value): ", cur);
    }
    if (holds < 0) {
        return ObjectError(src, obj, "Invalid holds= value (expected a positive value): ", cur);
    }
    if (holds > 1000000) {
        return ObjectError(
                src, obj, "Invalid holds= value (expected 0 <= value <= 1000000): ", holds);
    }
    obj.contains = holds;
    return true;
}

bool
processObjectPrep(const SourceFile &src, Object &obj, string_view cur)
{
    for (int i = 0; i < NPUTS; i++) {
        if (cur == obputs[i]) {
            obj.putto = i;
            return true;
        }
    }
    return ObjectError(src, obj, "Unrecognized object put= preposition type: ", cur);
}

bool
processObjectMob(const SourceFile &src, Object &obj, string_view cur)
{
    for (int i = 0; i < int(g_game.numNPCClasses); i++) {
        if (cur == g_game.m_npcClasses[i].id) {
            obj.npc = i;
            return true;
        }
    }
    return ObjectError(src, obj, "Unrecognized npc= identity: ", cur);
}

void
processObjectParam(const SourceFile &src, Object &obj, ObjectParameter paramNo, string_view cur)
{
    switch (paramNo) {
        case OP_NONE:
            break;
        case OP_ADJ:
            processObjectAdj(src, obj, cur);
            break;
        case OP_START:
            processObjectStartState(src, obj, cur);
            break;
        case OP_HOLDS:
            processObjectHolds(src, obj, cur);
            break;
        case OP_PUT:
            processObjectPrep(src, obj, cur);
            break;
        case OP_MOB:
            if (processObjectMob(src, obj, cur))
                g_game.numNPCs++;
            break;
        default:
            ObjectError(src,
                        obj,
                        "Internal Error: object-parameter ",
                        obparms[paramNo],
                        " not supported");
    }
}

bool
processObjectFlags(SourceFile &src, Object &obj)
{
    while (!src.Eol()) {
        auto cur = src.PopFront();

        if (auto flagbit = IsObjectFlag(cur); flagbit != WNONE) {
            obj.flags |= bitset(flagbit);
            continue;
        }
        if (auto paramNo = GetObjectParam(cur); paramNo != OP_NONE) {
            processObjectParam(src, obj, paramNo, cur);
            continue;
        }
        return ObjectError(src, obj, "Invalid parameter: ", cur);
    }
    return true;
}

bool
processObjectRooms(SourceFile &src, Object &obj)
{
    static std::set<roomid_t> locations{};
    locations.clear();

    if (!src.GetLineTerms()) {
        return ObjectError(src, obj, "Unexpected end of object (expected object locations)");
    }

    // Allow the line to start with "rooms=" or "room="
    if (!RemovePrefix(src.line.front(), "rooms="))
        RemovePrefix(src.line.front(), "room=");

    bool valid = true;
    while (!src.Eol()) {
        auto cur = src.PopFront();
        if (roomid_t roomNo = isRoomName(cur); roomNo != WNONE) {
            locations.insert(roomNo);
            continue;
        }
        if (objid_t objId = isContainer(cur); objId != WNONE) {
            locations.insert(-(INS + objId));
            continue;
        }
        ObjectError(src, obj, "Unrecognized start room/container: ", cur);
        valid = false;
    }

    if (locations.empty()) {
        ObjectError(src, obj, "No start locations.");
        valid = false;
    } else if (valid) {
        g_game.m_objectLocations.insert(
                g_game.m_objectLocations.end(), locations.begin(), locations.end());
        obj.nrooms = uint16_t(locations.size());
        g_game.numObjLocations += obj.nrooms;
    }

    return valid;
}

void
objs_proc(const std::string &filepath)
{
    SourceFile src{ filepath };
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath, ": ", err);
    }

    g_game.m_rooms.reserve(512);

    while (!src.Eof()) {
        // Terminate if we've reached the error limit.
        CheckErrorCount();

        if (!src.GetIDLine("object="))
            continue;

        auto id = src.PopFront();

        Object obj{};
        obj.adj = obj.npc = WNONE;
        obj.idno = g_game.numObjects;
        obj.state = 0;
        obj.nrooms = 0;
        obj.nstates = 0;
        obj.contains = 0;
        obj.flags = 0;
        obj.putto = 0;
        obj.roomsOffset = g_game.m_objectLocations.size();
        strncpy(obj.id, id.data(), std::min(sizeof(obj.id), id.size()));

        if (!processObjectFlags(src, obj)) {
            src.SkipBlock();
            continue;
        }

        if (!processObjectRooms(src, obj)) {
            src.SkipBlock();
            continue;
        }
        if (!processObjectStates(src, obj)) {

            src.SkipBlock();
            continue;
        }

        if (obj.nstates == 0) {
            ObjectError(src, obj, "No states defined");
        } else if (obj.state > obj.nstates) {
            ObjectError(src,
                        obj,
                        "Invalid start state (",
                        obj.state,
                        "): Object has ",
                        obj.nstates,
                        " state(s)");
        } else if (obj.state == obj.nstates) {
            ObjectError(src,
                        obj,
                        "Invalid start state (",
                        obj.state,
                        "). States are 0-indexed. Did you mean ",
                        obj.state - 1,
                        "?");
        } else {
            g_game.m_objects.push_back(obj);
            g_game.numObjects++;
        }
    }

    /*
    closeOutFiles();
    sort_objs();
    */
}

/*
     Travel Processing Routines for AMUL, Copyright (C) Oliver Smith, '90
     --------------------------------------------------------------------
  Warning! All source code in this file is copyright (C) KingFisher Software
*/

template<typename... Args>
void
RoomError(SourceFile src, Args &&... args)
{
    LogError(src.filepath, ":", src.lineNo, ":", c_room->id, ": ", forward<Args>(args)...);
}

bool
getTravelVerbs(SourceFile &src, std::set<verbid_t> &verbs)
{
    verbs.clear();
    for (auto it : src.line) {
        if (verbid_t verb = IsVerb(it); verb != WNONE) {
            if (!(GetVerb(verb).flags & VB_TRAVEL))
                RoomError(src, "Verb is not a TRAVEL verb: ", it);
            verbs.insert(verb);
        } else {
            RoomError(src, "Unrecognized verb: ", it);
            return false;
        }
    }
    if (verbs.empty()) {
        RoomError(src, "Empty verb[s]= line");
        return false;
    }
    return true;
}

bool
processTravelCnALine(SourceFile &src, TravelLine &tt)
{
    auto it = src.line.begin(), end = src.line.end();
    bool truthiness = true;
    while (it != end) {
        while (!it->empty() && it->front() == '!') {
            truthiness = !truthiness;
            it->remove_prefix(1);
        }
        if (it->empty())
            ++it;
        if (PreConditionWords.find(*it) != PreConditionWords.end()) {
            ++it;
            continue;
        }
        if (*it == "not") {
            ++it;
            truthiness = !truthiness;
            continue;
        }
        break;
    }

    if (it == end) {
        RoomError(src, "No condition or action present on line");
        return false;
    }

    std::string word{ *(it) };
    StringLower(word);
    if (word == ALWAYSEP) {
        tt.condition.m_op = CALWAYS;
        tt.action.m_op = AENDPARSE;
        return true;
    }

    tt.condition.m_op = iscond(word);
    if (tt.condition.m_op != WNONE) {
        ++it;
        auto consumed = checkConditionParameters(c_room->id, it, end, &tt, src);
        if (consumed < 0)
            return false;
        it += consumed;
    } else {
        tt.condition.m_op = CALWAYS;
        if (roomid_t roomId = isRoomName(word); roomId != WNONE) {
            tt.action.m_op = AGOTO_ROOM;
            tt.action.m_args[0] = roomId;
            return true;
        }
    }

    while (it != end && PreActionWords.find(*it) != PreActionWords.end()) {
        ++it;
    }

    if (it == end) {
        RoomError(src, "Missing action/room.");
        return false;
    }

    word = *(it++);
    StringLower(word);

    if (roomid_t roomId = isRoomName(word); roomId != WNONE) {
        tt.action.m_op = AGOTO_ROOM;
        tt.action.m_args[0] = roomId;
        return true;
    }
    tt.action.m_op = isact(word);
    if (tt.action.m_op == WNONE) {
        RoomError(src, "Unrecognized action/room: ", word);
        return false;
    }

    if (tt.action.m_op == ATRAVEL) {
        RoomError(src, "Tried to call action 'travel' from travel table");
        return false;
    }

    auto consumed = checkActionParameters(c_room->id, it, end, &tt, src);
    if (consumed < 0)
        return false;
    it += consumed;

    if (it != end) {
        RoomError(src, "Unexpected additional words after end of condition: ", *it);
        return false;
    }

    return true;
}

bool
processTravelRoom(SourceFile &src, std::string roomName)
{
    const roomid_t rmn = isRoomName(roomName);
    if (rmn == WNONE) {
        src.Error("Unrecognized room name: ", roomName);
        return false;
    }

    c_room = &g_game.m_rooms[rmn];
    if (c_room->ttlines != 0) {
        src.Error("Travel table for room ", roomName, " already defined.");
        return false;
    }

    if (c_room->flags & DEATH) {
        RoomError(src, "DEATH rooms can't have a travel table.");
        return false;
    }

    c_room->ttOffset = g_game.m_travel.size();

    std::set<verbid_t> verbs{};
    while (src.GetLineTerms()) {
        if (RemovePrefix(src.line.front(), "verb=") || RemovePrefix(src.line.front(), "verbs=")) {
            if (!getTravelVerbs(src, verbs))
                return false;
            continue;
        }
        if (verbs.empty()) {
            RoomError(src, "Missing verb[s]= line before first condition/action line.");
            return false;
        }

        TravelLine tt{};
        if (!processTravelCnALine(src, tt))
            return false;

        for (auto verb : verbs) {
            tt.verb = verb;
            g_game.m_travel.push_back(tt);
            c_room->ttlines++;
            g_game.numTTEnts++;
        }
    }

    return true;
}

// travel.txt: Describes connections or obstacles between rooms.
// Players move around the game using "travel" commands, typically "north", "up",
// etc. Because rooms don't have coordinates or dimensions, there's no way the game
// could tell what those links are.
//
// The travel table is relatively simple:
//
//    room=<room name>
//    verb=<verb>        # or 'verbs='
//          <room name>
//        | <action <action params>>
//        | <condition <condition params>> <room name>
//        | <condition <condition params>> <action <aciton params>>
//
// e.g.
//
//    room=northroom
//    verb=north
//        error "You can't go further north."
//    verb=south
//        if got umbrella in state 1 then error "Can't get the umbrella thru the
//        door."
//        goto southroom
//
//    room=southroom
//    verbs=north out
//        if got umbrella in state 1 then error "Can't get the umbrella thru the
//        door."
//        goto northroom

void
trav_proc(const std::string &filepath)
{
    SourceFile src{ filepath };
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath);
    }

    std::set<verbid_t> currentVerbs{};

    size_t roomsProcessed{ 0 };
    while (!src.Eof()) {

        CheckErrorCount();

        if (!src.GetIDLine("room="))
            continue;
        if (!processTravelRoom(src, std::string{ src.line.front() })) {
            src.SkipBlock();
        }

        ++roomsProcessed;
    }

    c_room = nullptr;

    if (GetLogErrorCount() == 0 && roomsProcessed != g_game.numRooms) {
        for (auto &room : g_game.m_rooms) {
            if (room.ttlines == 0 && (room.flags & DEATH) != DEATH) {
                LogWarn("room:", room.id, ": no travel entry");
            }
        }
    }
}

/* From and To */
int
chae_proc(const Verb &verb, const char *f, char *t)
{
    int n;

    if ((*f < '0' || *f > '9') && *f != '?') {
        chae_err(verb, Word);
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
            chae_err(verb, Word);
            return -1;
        }
        *(t++) = (char) n;
    }

    for (n = 1; n < 5; n++) {
        if (*f == 'c' || *f == 'h' || *f == 'a' || *f == 'e') {
            *(t++) = toupper(*f);
            f++;
        } else {
            chae_err(verb, Word);
            return -1;
        }
    }

    return 0;
}

void
getVerbFlags(Verb *verbp, char *p)
{
    static char defaultChae[] = { -1, 'C', 'H', 'A', 'E', -1, 'C', 'H', 'A', 'E' };
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
            if (chae_proc(*verbp, Word, verbp->precedence[precedence]) == -1)
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
        int extant = IsVerb(Word);
        if (extant != -1) {
            std::cout << "found " << Word << " it's " << extant << " with "
                      << g_game.m_verbs[extant].flags << "\n";
            if (g_game.m_verbs[extant].flags | VB_TRAVEL) {
                LogError("Redefinition of travel verb: ", Word);
                continue;
            }
            g_game.m_verbs[extant].flags |= VB_TRAVEL;
            LogDebug("Added TRAVEL to existing verb: ", Word);
            continue;
        }
        /// TODO: size check
        Verb verb{};
        strncpy(verb.id, Word, sizeof(verb.id));
        verb.flags |= VB_TRAVEL;
        proc = 0;
        g_game.m_verbs.push_back(verb);
        ++g_game.numVerbs;
        LogDebug("Added TRAVEL verb: ", Word);
    }
}

pair<char *, verbid_t>
getLangVerb(FILE *fp)
{
    char *p = getTidiedLineToScratch(fp);
    if (!p)
        return make_pair(nullptr, WNONE);

    // check for 'travel' line which allows specification of travel verbs

    if (canSkipLead("travel=", &p)) {
        registerTravelVerbs(p);
        return make_pair(nullptr, WNONE);
    }

    p = getWordAfter("verb=", p);
    if (!Word[0]) {
        LogError("verb= line without a verb?");
        return make_pair(nullptr, WINVALID);
    }

    if (strlen(Word) > IDL) {
        LogError("Invalid verb ID (too long): ", Word);
        return make_pair(nullptr, WINVALID);
    }

    auto verbId = IsVerb(Word);
    if (verbId != WNONE) {
        auto &verb = g_game.m_verbs[verbId];
        if (!(verb.flags & VB_TRAVEL) || verb.ents != 0) {
            vbprob(verb, "Redefinition of verb", scratch);
            return make_pair(nullptr, WINVALID);
        }
    } else {
        verbId = g_game.m_verbs.size();
        g_game.m_verbs.push_back(Verb{});
    }

    auto &verb = g_game.m_verbs[verbId];
    strncpy(verb.id, Word, sizeof(verb.id));
    verb.slotOffset = g_game.m_verbSlots.size();

    LogDebug("verb#", verbId, "/", g_game.m_verbs.size(), ":", verb.id);

    return make_pair(p, verbId);
}

/* Process LANG.TXT */
void
lang_proc(const std::string & /*filepath*/)
{
    char lastc;
    /* n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2) */
    WType n{ WNONE };  /// TODO: This has some -ve wtype values.
    int cs, s;

    nextc(true);

    VMLine vmline{};

    while (!feof(ifp) && nextc(false)) {
        CheckErrorCount();

        auto [p, verbId] = getLangVerb(ifp);
        if (!p) {
            if (verbId != WNONE)
                skipParagraph();
            continue;
        }

        auto &verb = g_game.m_verbs[verbId];

        getVerbFlags(&verb, p);

        p = getTidiedLineToScratch(ifp);
        if (!p)
            LogError("Unexpected end of file during verb: ", verb.id);
        if (!*p || isEol(*p)) {
            if (verb.ents == 0 && (verb.flags & VB_TRAVEL)) {
                LogWarn("Verb has no entries: ", verb.id);
            }
            goto write;
        }

        if (!canSkipLead("syntax=", &p)) {
            vbprob(verb, "no syntax= line", scratch);
            skipParagraph();
            continue;
        }

        /* Syntax line loop */
    synloop:
        vbslot = Syntax{};
        vbslot.lineOffset = g_game.m_vmLines.size();
        setslots(WNONE);
        verb.ents++;
        p = skiplead("verb", p);

        {
            char *qualifier = getword(p);
            qualifier = skipspc(qualifier);

            /* If syntaxes line is 'syntaxes=verb any' or 'syntaxes=none' */
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

        if ((n = iswtype(Word)) == WType(WINVALID)) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid phrase on syntax line: %s", Word);
            vbprob(verb, tmp, scratch);
            goto commands;
        }
        if (Word[0] == 0) {
            s = WANY;
            goto skipeq;
        }

        /* First of all, eliminate illegal combinations */
        if (n == WNONE || n == WANY) { /* you cannot say none=fred any=fred etc */
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Tried to define %s on syntax line", syntaxes[n]);
            vbprob(verb, tmp, scratch);
            goto endsynt;
        }
        if (n == WPLAYER && strcmp(Word, "me") != 0 && strcmp(Word, "myself") != 0) {
            vbprob(verb, "Tried to specify player other than self", scratch);
            goto endsynt;
        }

        /* Now check that the 'tag' is the correct type of word */

        s = WNONE;
        switch (n) {
            case WADJ:
                /* Need ISADJ() - do TT entry too */
                break;
            case WNOUN:
                s = isNoun(Word);
                break;
            case WPREP:
                s = IsPreposition(Word);
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
                s = IsVerb(Word);
                break;
            case WCLASS:
                s = WANY;
                break;
            case WNUMBER:
                if (Word[0] == '-')
                    s = atoi(Word + 1);
                else
                    s = atoi(Word);
                break;
            default:
                LogError("Internal Error: Invalid w-type");
        }

        if (n == WNUMBER && (s > 100000 || -s > 100000)) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "Invalid number: %d", s);
            vbprob(verb, tmp, scratch);
        }
        if (s == WNONE && n != WNUMBER) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "%s=: Invalid setting: %s", syntaxes[n + 1], Word);
            vbprob(verb, tmp, scratch);
        }
        if (s == -3 && n == WNOUN)
            s = -1;

    skipeq: /* (Skipped the equals signs) */
        /* Now fit this into the correct slot */
        cs = 1; /* Noun1 */
        switch (n) {
            case WADJ:
                if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                    vbprob(verb, "Invalid NOUN NOUN ADJ combination", scratch);
                    n = WINVALID;
                    break;
                }
                if (vbslot.wtype[1] != WNONE && vbslot.wtype[3] != WNONE) {
                    vbprob(verb, "Invalid NOUN ADJ NOUN ADJ combination", scratch);
                    n = WINVALID;
                    break;
                }
                if (vbslot.wtype[0] != WNONE && vbslot.wtype[3] != WNONE) {
                    vbprob(verb, "More than two adjectives on a line", scratch);
                    n = WINVALID;
                    break;
                }
                if (vbslot.wtype[0] != WNONE)
                    cs = 3;
                else
                    cs = 0;
                break;
            case WNOUN:
                if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                    vbprob(verb, "Invalid noun arrangement", scratch);
                    n = WINVALID;
                    break;
                }
                if (vbslot.wtype[1] != WNONE)
                    cs = 4;
                break;
            case WPREP:
                if (vbslot.wtype[2] != WNONE) {
                    vbprob(verb, "Invalid PREP arrangement", scratch);
                    n = WINVALID;
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
                    snprintf(scratch,
                             sizeof(scratch),
                             "No free noun slot for %s entry",
                             syntaxes[n + 1]);
                    vbprob(verb, tmp, scratch);
                    n = WINVALID;
                    break;
                }
                if (vbslot.wtype[1] != WNONE)
                    cs = 4;
                break;
            default:
                break;
        }
        if (n == WINVALID)
            goto sp2;
        /* Put the bits into the slots! */
        vbslot.wtype[cs] = n;
        vbslot.slot[cs] = s;
        goto sp2;

    endsynt:
        vbslot.ents = 0;
        vbslot.lineOffset = g_game.m_vmLines.empty() ? 0 : g_game.m_vmLines.size();

    commands:
        lastc = 'x';
        proc = 0;

        p = getTidiedLineToScratch(ifp);
        if (!p || !*p || isEol(*p)) {
            lastc = 1;
            goto writeslot;
        }

        if (canSkipLead("syntax=", &p)) {
            lastc = 0;
            goto writeslot;
        }

        // we're committed to this being a c&a line so count it now.
        vmline = VMLine{};
        vbslot.ents++;

        /* Process the condition */
    notloop:
        p = precon(p);
        p = getword(p);

        /* always endparse */
        if (strcmp(Word, ALWAYSEP) == 0) {
            vmline.condition.m_op = CALWAYS;
            vmline.action.m_op = AENDPARSE;
            goto writecna;
        }
        if (strcmp(Word, "not") == 0 || (Word[0] == '!' && Word[1] == 0)) {
            vmline.notCondition = !vmline.notCondition;
            goto notloop;
        }

        /* If they forgot space between !<condition>, eg !toprank */
    notlp2:
        if (Word[0] == '!') {
            memmove(Word, Word + 1, sizeof(Word) - 1);
            Word[sizeof(Word) - 1] = 0;
            vmline.notCondition = !vmline.notCondition;
            goto notlp2;
        }

        if ((vmline.condition.m_op = iscond(Word)) == WNONE) {
            proc = 1;
            if ((vmline.action.m_op = isact(Word)) == WNONE) {
                if (roomid_t roomId = isRoomName(Word); roomId != WNONE) {
                    vmline.condition.m_op = CALWAYS;
                    vmline.action.m_op = AGOTO_ROOM;
                    vmline.action.m_args[0] = roomId;
                    goto writecna;
                }
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "Invalid condition, '%s'", Word);
                vbprob(verb, tmp, scratch);
                proc = 0;
                goto commands;
            }
            vmline.condition.m_op = CALWAYS;
            goto doaction;
        }
        p = skipspc(p);
        proc = 1;
        if ((p = checkConditionParameters(
                     verb.id, p, &conditions[vmline.condition.m_op], &vmline)) == nullptr) {
            goto commands;
        }
        if (*p == 0) {
            if ((vmline.action.m_op = isact(conditions[vmline.condition.m_op].name)) == WNONE) {
                vbprob(verb, "Missing action", scratch);
                goto commands;
            }
            vmline.condition.m_op = CALWAYS;
            goto writecna;
        }
        p = preact(p);
        p = getword(p);
        if ((vmline.action.m_op = isact(Word)) == WNONE) {
            if (roomid_t roomId = isRoomName(Word); roomId != WNONE) {
                vmline.action.m_op = AGOTO_ROOM;
                vmline.action.m_args[0] = roomId;
                goto writecna;
            }
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid action, '%s'", Word);
            vbprob(verb, tmp, scratch);
            goto commands;
        }
    doaction:
        p = skipspc(p);
        if (!(p = checkActionParameters(verb.id, p, &actions[vmline.action.m_op], &vmline))) {
            goto commands;
        }

    writecna: /* Write the C & A lines */
        g_game.m_vmLines.push_back(vmline);
        g_game.numVerbOps++;
        proc = 0;
        goto commands;

    writeslot:
        g_game.m_verbSlots.push_back(vbslot);
        g_game.numVerbSlots++;
        proc = 0;
        if (lastc > 1)
            goto commands;
        if (lastc == 0)
            goto synloop;

        lastc = '\n';
    write:
        g_game.numVerbs++;
        proc = 0;
    }
}

/* Routines to process/handle Synonyms */

constexpr auto AliasError = [](const SourceFile &src, string_view original, const auto &... args) {
    LogError(src.filepath, ":", src.lineNo, ":", original, ": ", args...);
    return false;
};

void
syn_proc(const std::string &filepath)
{
    SourceFile src{ filepath };
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath, ": ", err);
    }

    Synonym syn{};

    while (!src.Eof()) {
        CheckErrorCount();
        if (!src.GetLineTerms())
            continue;

        std::string token{ src.PopFront() };
        StringLower(token);

        if (src.line.empty()) {
            AliasError(src, token, "expecting one or more alias word");
            continue;
        }

        syn.type = SYN_NOUN;
        syn.aliases = isNoun(token);
        if (syn.aliases == WNONE) {
            syn.aliases = IsVerb(token);
            if (syn.aliases == WNONE) {
                AliasError(src, token, "expected an existing verb or noun");
                continue;
            }
            syn.type = SYN_VERB;
        }

        while (!src.Eol()) {
            auto alias = src.PopFront();
            if (alias.length() > IDL) {
                AliasError(src,
                           token,
                           "invlaid length: alias must be under ",
                           IDL,
                           " characters long: ",
                           alias);
                continue;
            }
            std::string word{ alias };
            StringLower(word);
            strncpy(syn.word, word.c_str(), sizeof(syn.word));
            g_game.m_synonyms.push_back(syn);
        }
    }

    g_game.numSynonyms = g_game.m_synonyms.size();
}

/* Mobiles.Txt Processor */

/* Pass 1: Indexes npc names */

constexpr auto NpcError = [](const SourceFile &src, const NPCClass &npc, const auto &... args) {
    LogError(src.filepath, ":", src.lineNo, ":", npc.id, ": ", args...);
    CheckErrorCount();
    return false;
};

bool
consumeNpcIDLine(SourceFile &src, NPCClass &npc)
{
    auto id = src.PopFront();
    strncpy(npc.id, id.data(), std::min(sizeof(npc.id), id.length()));

    while (!src.Eol()) {
        auto flag = src.PopFront();
        if (RemovePrefix(flag, "dead=")) {
            if (npc.deadstate != WNONE) {
                NpcError(src, npc, "Multiple dead= entries");
                continue;
            }
            int64_t state = WNONE;
            if (!ToInt(flag, state) || state < 0 || state > 127) {
                NpcError(src,
                         npc,
                         "Expected positive integer value between 0 and ",
                         127,
                         " for dead=, got ",
                         flag);
                continue;
            }
            npc.deadstate = char(state);
            continue;
        }
        if (RemovePrefix(flag, "dmove=")) {
            if (npc.dmove != WNONE) {
                NpcError(src, npc, "Multiple dmove= entries");
                continue;
            }
            if (npc.dmove = isRoomName(flag); npc.dmove == -1) {
                NpcError(src, npc, "Unrecognized room after dmove=: ", flag);
                continue;
            }
            continue;
        }
        NpcError(src, npc, "Unrecognized NPC flag: ", flag);
    }

    return true;
}

constexpr auto getNpcStatValue = [](SourceFile &src,
                                    NPCClass &npc,
                                    string_view prefix,
                                    int max,
                                    auto &into) {
    if (src.Eol()) {
        return NpcError(src, npc, "Premature end of line, expected ", prefix);
    }
    auto text = src.PopFront();
    if (!RemovePrefix(text, prefix)) {
        return NpcError(
                src, npc, "Expected ", prefix, " (prefixes are not optional here), got: ", text);
    }
    int64_t value{ 0 };
    if (!ToInt(text, value) || value < 0 || value > max) {
        return NpcError(src,
                        npc,
                        "Error parsing ",
                        prefix,
                        ", expcted number between 0 and ",
                        max,
                        ", got: ",
                        text);
    }
    into = char(value);
    return true;
};

bool
consumeNpcStatsLine(SourceFile &src, NPCClass &npc)
{
    if (!src.GetLineTerms()) {
        return NpcError(src, npc, "Unexpected end of NPC");
    }

    if (!getNpcStatValue(src, npc, "speed="sv, 127, npc.speed))
        return false;

    if (!getNpcStatValue(src, npc, "travel=", 100, npc.travel))
        return false;

    if (!getNpcStatValue(src, npc, "fight=", 100, npc.fight))
        return false;
    if (!getNpcStatValue(src, npc, "act=", 100, npc.act))
        return false;
    if (!getNpcStatValue(src, npc, "wait=", 100, npc.wait))
        return false;
    auto sum = int64_t(npc.travel) + int64_t(npc.fight) + int64_t(npc.act) + int64_t(npc.wait);
    if (sum != 100) {
        NpcError(src, npc, "Travel+Fight+Act+Wait values add up to ", sum, "%, must add to 100% ");
        return false;
    }

    if (!getNpcStatValue(src, npc, "fear=", 100, npc.fear))
        return false;
    if (!getNpcStatValue(src, npc, "attack=", 100, npc.attack))
        return false;
    if (!getNpcStatValue(src, npc, "hitpower=", 100, npc.hitpower))
        return false;

    return true;
}

bool
/* Fetch npc message line */
consumeNpcMessageLine(SourceFile &src, NPCClass &npc, string_view prefix, stringid_t &into)
{
    if (!src.GetLineTerms()) {
        return NpcError(src, npc, "Unexpected end of NPC: Expected ", prefix);
    }
    auto value = src.PopFront();
    if (!RemovePrefix(value, prefix)) {
        return NpcError(src,
                        npc,
                        "Expected ",
                        prefix,
                        "\"text\" or prefix=<msg id> (prefixes are not optional "
                        "here), got: ",
                        value);
    }
    if (value == "none") {
        into = WNONE;
        return true;
    }
    into = getTextString(value);
    if (into == WNONE) {
        return NpcError(src, npc, "UUnrecognized ", prefix, " message id: ", value);
    }
    return true;
}

void
npc_proc(const std::string &filepath)
{
    SourceFile src{ filepath };
    if (auto err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath, ": ", err);
    }

    while (!src.Eof()) {
        CheckErrorCount();
        if (!src.GetIDLine("npc="))
            continue;

        NPCClass npc{};
        if (!consumeNpcIDLine(src, npc)) {
            src.SkipBlock();
            continue;
        }
        if (!consumeNpcStatsLine(src, npc)) {
            src.SkipBlock();
            continue;
        }
        if (!consumeNpcMessageLine(src, npc, "arrive=", npc.arr) ||
            !consumeNpcMessageLine(src, npc, "depart=", npc.dep) ||
            !consumeNpcMessageLine(src, npc, "flee=", npc.flee) ||
            !consumeNpcMessageLine(src, npc, "strike=", npc.hit) ||
            !consumeNpcMessageLine(src, npc, "miss=", npc.miss) ||
            !consumeNpcMessageLine(src, npc, "dies=", npc.death)) {
            src.SkipBlock();
            continue;
        }

        LogWarn("///TODO: NPC Commands");
        src.SkipBlock();

        g_game.m_npcClasses.emplace_back(npc);
        g_game.numNPCClasses++;
    }
}

/* Pass 2: Indexes commands npcs have access to */
/*npc_proc2()
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

    if (EINVAL == path_join(context->filename, sizeof(context->filename), gameDir,
filename)) { LogFatal("Path length exceeds limit for %s/%s", gameDir, filename);
    }

    return context;
}
*/

extern void title_proc(const std::string &), smsg_proc(const std::string &),
        umsg_proc(const std::string &), obds_proc(const std::string &);

struct CompilePhase {
    const char *name;
    bool openFile;
    void (*handler)(const std::string &filename);
} phases[] = {
    { "title", true, title_proc },    // game "title" (and config)
    { "sysmsg", false, smsg_proc },   // system messages
    { "umsg", false, umsg_proc },     // user-defined string messages
    { "obdescs", false, obds_proc },  // object description strings
    { "rooms", false, room_proc },    // room table
    { "ranks", true, rank_proc },     // rank table
    { "npcs", false, npc_proc },      // npc classes so we can apply to objects
    { "objects", false, objs_proc },  // objects
    { "lang", true, lang_proc },      // language
    { "travel", false, trav_proc },   // travel table
    { "syns", false, syn_proc },      // synonyms for other things
    { nullptr, false, nullptr }       // terminator
};

void
compilePhase(const CompilePhase *phase)
{
    LogInfo("Compiling: ", phase->name);
    std::string filepath{};
    MakeTextFileName(phase->name, filepath);
    if (phase->openFile) {
        FILE *fp = fopen(filepath.c_str(), "r");
        if (!fp)
            LogFatal("Could not open ", phase->name, " file: ", filepath);
        ifp = fp;
    }

    phase->handler(filepath);

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
compilerModuleInit(Module * /*module*/)
{
    return 0;
}

void
createDataDir()
{
    std::string filepath{};
    if (PathJoin(filepath, gameDir, "Data") != 0)
        LogFatal("Internal error preparing Data directory path");
#if defined(_MSC_VER)
    if (mkdir(filepath.c_str()) < 0 && errno != EEXIST)
#else
    if (mkdir(filepath.c_str(), 0776) < 0 && errno != EEXIST)
#endif
        LogFatal("Unable to create Data directory: ", filepath, ": ", strerror(errno));
    LogInfo("Created ", filepath);
}

extern void CreateDefaultFiles();

error_t
compilerModuleStart(Module * /*module*/)
{
    LogInfo("AMUL Compiler: ", compilerVersion);

    LogDebug("Game Directory: ", gameDir);
    LogDebug("Log Verbosity : ", GetLogLevelName(GetLogLevel()));

    CreateDefaultFiles();

    g_game.m_stringIndex.reserve(1024);
    g_game.m_strings.reserve(256 * 1024);
    g_game.m_rooms.reserve(1024);
    g_game.m_ranks.reserve(20);
    g_game.m_objects.reserve(512);
    g_game.m_objectLocations.reserve(1024);
    g_game.m_objectStates.reserve(1024);
    g_game.m_adjectives.reserve(256);
    g_game.m_synonyms.reserve(512);
    g_game.m_verbs.reserve(128);
    g_game.m_verbSlots.reserve(256);
    g_game.m_vmLines.reserve(1024);
    g_game.m_travel.reserve(1024);

    createDataDir();

    for (const CompilePhase *phase = &phases[0]; phase->name; ++phase) {
        if (phase->openFile == false)
            continue;

        std::string filepath{};
        MakeTextFileName(phase->name, filepath);

        struct stat sb;
        error_t err = stat(filepath.c_str(), &sb);
        if (err != 0)
            LogFatal("Missing file (", err, "): ", filepath);
    }

    return 0;
}

error_t
compilerModuleClose(Module * /*module*/, error_t err)
{
    CloseOutFiles();
    CloseFile(&ifp);

    // If we didn't complete compilation, delete the profile file.
    if (err != 0 || !exiting) {
        UnlinkGameFile(gameFile);
    }

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

constexpr auto checkedSave = [](const char *label, const auto &container) {
    char desc[64];
    strcpy(desc, label);
    checkedfwrite(ofp1, desc, sizeof(desc));
    size_t size = container.size();
    checkedfwrite(ofp1, &size, 1);
    auto written = checkedfwrite(ofp1, container.data(), size);
    LogDebug("wrote ", label, ": ", container.size(), ": ", written, " bytes");
    assert((written & (sizeof(int) - 1)) == 0);
};

error_t
Game::Save()
{
    fopenw(gameFile);

    // Round the string table up to pointer alignment.
    auto stringPadding = sizeof(uintptr_t) - (stringBytes & (sizeof(uintptr_t) - 1));
    for (size_t i = 0; i < stringPadding; ++i) {
        m_strings.push_back('\0');
        stringBytes++;
    }

    const auto written = checkedfwrite(ofp1, dynamic_cast<GameConfig *>(this), 1);
    assert((written & (sizeof(uintptr_t) - 1)) == 0);

    assert(m_stringIndex.size() == numStrings);
    checkedSave("string index", m_stringIndex);
    checkedSave("string bytes", m_strings);

    checkedSave("ranks", m_ranks);

    checkedSave("rooms", m_rooms);
    checkedSave("travel", m_travel);

    checkedSave("adjectives", m_adjectives);

    checkedSave("objects", m_objects);
    checkedSave("object locations", m_objectLocations);
    checkedSave("object states", m_objectStates);

    checkedSave("synonyms", m_synonyms);

    checkedSave("verbs", m_verbs);
    checkedSave("syntax lines", m_verbSlots);
    checkedSave("language code", m_vmLines);
    checkedSave("npc classes", m_npcClasses);

    CloseOutFiles();

    return 0;
}

/*---------------------------------------------------------*/

error_t
amulcom_main()
{
    InitCompilerModule();

    // Strings is effectively a sub-module of the compiler
    InitStrings();

    StartModules();

    compileGame();

    g_game.numStrings = GetStringCount();
    g_game.stringBytes = GetStringBytes();

    LogNote("Execution finished normally");
    LogInfo("Statistics for ", g_game.gameName, ":");
    LogInfo("Rooms: ",
            g_game.numRooms,
            ", Ranks: ",
            g_game.numRanks,
            ", Objects: ",
            g_game.numObjects,
            ", Adjs: ",
            g_game.numAdjectives,
            ", Verbs: ",
            g_game.numVerbs,
            ", Syns: ",
            g_game.numSynonyms,
            ", TT Ents: ",
            g_game.numTTEnts,
            ", Strings: ",
            g_game.numStrings,
            ", Text: ",
            g_game.stringBytes);

    g_game.Save();

    exiting = true;

    return 0;
}

int
main(int argc, const char **argv)
{
    CommandLine cmdline = { argc, argv, nullptr };

    InitModules();

    ERROR_CHECK(InitLogging());
    ERROR_CHECK(InitCommandLine(&cmdline));

    const error_t retval = amulcom_main();

    CloseModules(retval);

    return retval;
}
