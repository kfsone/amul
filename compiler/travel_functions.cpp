// travel parsing helpers, used by both the travel and the language parser

#include "amulcom.includes.h"
#include "h/amul.cons.h"

#include "constants.h"
#include "extras.h"

#include <algorithm>
#include <string>
#include <unordered_set>

using namespace AMUL::Logging;
using namespace Compiler;

const char *
precon(const char *s) noexcept
{
    const char *s2 = s;

    if ((s = skiplead("if ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    if ((s = skiplead("the ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    if ((s = skiplead("i ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    s = skiplead("am ", s);

    return s;
}

const char *
preact(const char *s) noexcept
{
    const char *s2 = s;
    if ((s = skiplead("then ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    if ((s = skiplead("goto ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    if ((s = skiplead("go to ", s)) != s2) {
        s = skipspc(s);
        s2 = s;
    }
    s = skiplead("set ", s);
    return s;
}

int32_t
chknum(const char *p) noexcept
{
    int32_t n;

    if (!isdigit(*p) && !isdigit(*(p + 1)))
        return -1000001;
    if (*p == '>' || *p == '<' || *p == '-' || *p == '=')
        n = atoi(p + 1);
    else
        n = atoi(p);
    if (n >= 1000000) {
        printf("\x07\n*** Number %d exceeds limits!", n);
        return -1000001;
    }
    if (*p == '-')
        return (int32_t)-n;
    if (*p == '>')
        return (int32_t)(n + LESS);
    if (*p == '<')
        return (int32_t)(n + MORE);
    return n;
}

static const std::unordered_set<std::string> optionalPrefixes = {
        "the",  "of",  "are", "is",  "has", "next", "with", "to", "set",
        "from", "for", "by",  "and", "was", "i",    "am",   "as"};

static const char *
skipOptionalPrefixes(const char *p) noexcept
{
    for (; p;) {
        p = skipspc(p);
        if (isLineEnding(*p))
            break;
        const char *nextWord = strpbrk(p, " \n;");
        if (!nextWord)
            nextWord = p + strlen(p);
        if (optionalPrefixes.find(std::string(p, nextWord)) == optionalPrefixes.end())
            break;
    }
    return p;
}

const char *
chkp(const char *p, char t, int c, int z, FILE *fp) noexcept
{
    int32_t x;

    // skip optional prefixes
    p = skipOptionalPrefixes(p);
    if (*p == 0) {
        GetLogger().fatalf(
                "%s: %s: incomplete condition/action line: %s='%s'", (proc == 1) ? "Verb" : "Room",
                (proc == 1) ? verb.id : roomtab->id, (z == 1) ? "condition" : "action",
                (z == 1) ? conditions[c].name.c_str()
                         : actions[c].name.c_str());  /// TODO: take this as a parameter
    }
    std::string token{};
    if (*p != '\"' && *p != '\'') {
        const char *end = strstop(p, ' ');
        token.assign(p, end);
        p = end;
    } else {
        const char *endquote = strstop(p + 1, *p);  // look for matching close quote
        token.assign(p + 1, endquote);
        p = endquote + 1;
    }

    const char *start = token.c_str();
    if ((t >= 0 && t <= 10) || t == -70) {  // processing language table?
        x = actualval(token.c_str(), t);
        if (x == -1) {  // If it was an actual, but wrong type
            GetLogger().errorf(
                    "Invalid slot label, '%s', after %s '%s' in verb '%s'.\n", start,
                    (z == 1) ? "condition" : "action",
                    (z == 1) ? conditions[c].name.c_str() : actions[c].name.c_str(), verb.id);
            return NULL;
        }
        if (x != -2)
            goto write;
    }

    switch (t) {
    case -6: x = onoff(start); break;
    case -5: x = bvmode(toupper(*start)); break;
    case -4: x = isstat(start); break;
    case -3: x = isspell(start); break;
    case -2: x = rdmode(toupper(*start)); break;
    case -1: x = antype(start); break;
    case CAP_ROOM: x = isroom(start); break;
    case CAP_VERB: x = is_verb(start); break;
    case CAP_ADJ: break;
    case -70:
    case CAP_NOUN: x = isnounh(start); break;
    case CAP_UMSG: x = ttumsgchk(start); break;
    case CAP_NUM: x = chknum(start); break;
    case CAP_ROOM_FLAG: x = isrflag(start); break;
    case CAP_OBJ_FLAG: x = isoflag1(start); break;
    case CAP_STAT_FLAG: x = isoflag2(start); break;
    case CAP_GENDER: x = isgender(toupper(*start)); break;
    case CAP_DAEMON_ID:
        if ((x = is_verb(start)) == -1 || *start != '.')
            x = -1;
        break;
    default: {
        if (!(proc == 1 && t >= 0 && t <= 10)) {
            GetLogger().fatalf(
                    "Internal error, invalid PTYPE (val: %d) in %s %s: %s = %s", t,
                    (proc == 1) ? "verb" : "room", (proc == 1) ? verb.id : (rmtab + rmn)->id,
                    (z == 1) ? "condition" : "action",
                    (z == 1) ? conditions[c].name.c_str() : actions[c].name.c_str());
        }
    }
    }
    if (t == -70 && x == -2)
        x = -1;
    else if (((x == -1 || x == -2) && t != CAP_NUM) || x == -1000001) {
        GetLogger().errorf(
                "\x07\nInvalid parameter, '%s', after %s '%s' in %s '%s'.\n", start,
                (z == 1) ? "condition" : "action",
                (z == 1) ? conditions[c].name.c_str() : actions[c].name.c_str(),
                (proc == 1) ? "verb" : "room", (proc == 1) ? (verb.id) : (rmtab + rmn)->id);
        return NULL;
    }
write:
    fwritesafe(x, fp);
    FPos += 4;  // Writes a LONG
    return skipspc(p + 1);
}

int
isgender(char c) noexcept
{
    if (c == 'M')
        return 0;
    if (c == 'F')
        return 1;
    return -1;
}

int
antype(const char *s) noexcept
{
    if (strcmp(s, "global") == 0)
        return AGLOBAL;
    if (strcmp(s, "everyone") == 0)
        return AEVERY1;
    if (strcmp(s, "outside") == 0)
        return AOUTSIDE;
    if (strcmp(s, "here") == 0)
        return AHERE;
    if (strcmp(s, "others") == 0)
        return AOTHERS;
    if (strcmp(s, "all") == 0)
        return AALL;
    printf("\x07\nInvalid anouncement-group, '%s'...\n", s);
    return -1;
}

// Test noun state, checking rooms
int
isnounh(const char *s)
{
    int     i, l, j;
    int32_t orm;

    if (stricmp(s, "none") == 0)
        return -2;
    FILE *fp = rfopen(Resources::Compiled::objLoc());
    l = -1;
    objtab2 = obtab2;

    for (i = 0; i < nouns; i++, objtab2++) {
        if (stricmp(s, objtab2->id) != 0)
            continue;
        fseek(fp, (long)(uintptr_t)(objtab2->rmlist), 0L);
        for (j = 0; j < objtab2->nrooms; j++) {
            freadsafe(orm, fp);
            if (orm == rmn) {
                l = i;
                i = nouns + 1;
                j = objtab2->nrooms;
                break;
            }
        }
        if (i < nouns)
            l = i;
    }
    fclose(fp);
    return l;
}

int
rdmode(char c)
{
    if (c == 'R')
        return RD_VERBOSE_ONCE;
    if (c == 'V')
        return RD_VERBOSE;
    if (c == 'B')
        return RD_TERSE;
    return -1;
}

int
isspell(const char *s)
{
    if (strcmp(s, "glow") == 0)
        return SPELL_GLOW;
    if (strcmp(s, "invis") == 0)
        return SPELL_INVISIBLE;
    if (strcmp(s, "deaf") == 0)
        return SPELL_DEAFEN;
    if (strcmp(s, "dumb") == 0)
        return SPELL_MUTE;
    if (strcmp(s, "blind") == 0)
        return SPELL_BLIND;
    if (strcmp(s, "cripple") == 0)
        return SPELL_CRIPPLE;
    if (strcmp(s, "sleep") == 0)
        return SPELL_SLEEP;
    if (strcmp(s, "sinvis") == 0)
        return SPELL_SUPER_INVIS;
    return -1;
}

int
isstat(const char *s)
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
        return VERBOSE;
    if (c == 'B')
        return TERSE;
    return -1;
}

const char *
chkaparms(const char *p, int actionNo, FILE *fp)
{
    auto &instruction = actions[actionNo];
    for (int i = 0; i < instruction.parameterCount; i++)
        if ((p = chkp(p, instruction.parameters[i], actionNo, 0, fp)) == NULL)
            return NULL;
    return p;
}

const char *
chkcparms(const char *p, int conditionNo, FILE *fp)
{
    auto &instruction = conditions[conditionNo];
    for (int i = 0; i < instruction.parameterCount; i++)
        if ((p = chkp(p, instruction.parameters[i], conditionNo, 1, fp)) == NULL)
            return NULL;
    return p;
}

int
onoff(const char *p)
{
    if (stricmp(p, "on") == 0 || stricmp(p, "yes") == 0)
        return 1;
    return 0;
}
