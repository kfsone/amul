/*
          ####        ###     ###  ##     ##  ###
         ##  ##        ###   ###   ##     ##  ##            Amiga
        ##    ##       #########   ##     ##  ##            Multi
        ##    ##       #########   ##     ##  ##            User
        ########  ---  ## ### ##   ##     ##  ##            adventure
        ##    ##       ##     ##    ##   ##   ##            Language
       ####  ####     ####   ####   #######   ########


              ****    AMULCOM.C.......Adventure Compiler    ****
              ****               Main Program!              ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike

 Notes:

   When the LOCATE function is installed, as-well as as the 'i' words, we must
  have a user variable, 'located'. This will allow the user to fiddle and
  tinker with the locate function... We will also need a 'setword' so that
  you could write:

   locate id [pref=]c|h|a|e|i [state=]# in=ID|cont=ID|outside|REGardless[/#]

   Last Amendments: 26/08/90   12:30   OS   Installed GameTime= (title.txt)
            27/09/90   14:52   OS   Enhanced ObjProc.C (uses MC rtns)

*/

#include "amulcom.h"
#include "amulcom.strings.h"

#include "filesystem.h"
#include "modules.h"
#include "system.h"

#include <h/amigastubs.h>
#include <h/amul.acts.h>
#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.defs.h>
#include <h/amul.gcfg.h>
#include <h/amul.hash.h>
#include <h/amul.msgs.h>
#include <h/amul.stct.h>
#include <h/amul.strs.h>
#include <h/amul.test.h>
#include <h/amul.vars.h>
#include <h/amul.xtra.h>
#include <h/amulcom.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#ifndef _MSC_VER
#    include <alloca.h>
#else
#    include <malloc.h>
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Variables

static char compilerVersion[128];

/* Compiler specific variables... */

int    dmoves = 0; /* How many DMOVEs to check?	*/
int    rmn = 0;    /* Current room no.		*/
size_t FPos;       /* Used during TT/Lang writes	*/
char   Word[64];   /* For internal use only <grin>	*/
int    proc;       /* What we are processing	*/
char * syntab;     /* Synonym table, re-read	*/
long   wizstr;     /* Wizards strength		*/

struct Task *mytask, *FindTask();

char block[1024];  // scratch pad

struct _OBJ_STRUCT *obtab2, *objtab2, obj2, *osrch, *osrch2;

// counters
struct GameInfo   g_gameInfo;
struct GameConfig g_gameConfig;

FILE *ifp, *ofp1, *ofp2, *ofp3, *ofp4, *ofp5, *afp;

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
        alog(AL_FATAL, "Terminating due to %u errors", al_errorCount);
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
        alog(AL_FATAL, "File contained no data");
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
    FILE **fpp = NULL;
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
    if (afp != NULL)
        fclose(afp);
    afp = OpenGameFile(filename, "ab+");
}

/* Open file for reading */
void
fopenr(const char *filename)
{
    if (ifp != NULL)
        fclose(ifp);
    ifp = OpenGameFile(filename, "rb");
}

int
checkedfread(void *data, size_t objSize, size_t objCount, FILE *fp)
{
    int read = fread(data, objSize, objCount, fp);
    if (read != objCount) {
        alog(AL_FATAL, "Wrong write count: expected %d, got %d", objCount, read);
    }
    return read;
}

int
checkedfwrite(void *data, size_t objSize, size_t objCount, FILE *fp)
{
    int written = fwrite(data, objSize, objCount, fp);
    if (written != objCount) {
        alog(AL_FATAL, "Wrong write count: expected %d, got %d", objCount, written);
    }
    return written;
}

/* Update room entries after TT */
void
ttroomupdate()
{
    fseek(afp, 0, 0L);
    checkedfwrite(rmtab->id, sizeof(room), g_gameInfo.numRooms, afp);
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
    char *lastNonSpace = NULL;
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
getTidyBlock(FILE *ifp)
{
    for (;;) {
        if (!fgets(block, sizeof(block), ifp))
            return NULL;

        tidy(block);

        char *p = skipspc(block);
        if (!consumeComment(p) && *p)
            return p;
    }
}

int
is_verb(const char *s)
{
    int i;

    if (g_gameInfo.numVerbs == 0) {
        alog(AL_FATAL, "Attempted to look up verb '%s' with no verbs defined", s);
    }
    if (strlen(s) > IDL) {
        printf("Invalid verb ID (too long): %s", s);
        return -1;
    }

    vbptr = vbtab;
    for (i = 0; i < g_gameInfo.numVerbs; i++, vbptr++) {
        if (stricmp(vbptr->id, s) == 0)
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

/* Check to see if s is a room flag */
int
isrflag(const char *s)
{
    for (int i = 0; i < NRFLAGS; i++)
        if (strcmp(s, rflag[i]) == 0)
            return i;
    return -1;
}

int
isroom(const char *s)
{
    struct _ROOM_STRUCT *rp = rmtab;
    for (int r = 0; r < g_gameInfo.numRooms; r++, rp++) {
        if (strcmp(rp->id, s) == 0)
            return r;
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
        alog(AL_FATAL, "Invalid adjective (length): %s", Word);
    }
    if (g_gameInfo.numAdjectives == 0) {
        ZeroPad(Word, sizeof(Word));
        checkedfwrite(Word, IDL + 1, 1, afp);
        obj2.adj = 0;
        g_gameInfo.numAdjectives++;
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
    obj2.adj = g_gameInfo.numAdjectives++;
}

void
object(const char *s)
{
    alog(AL_FATAL, "Object #%d: %s: invalid %s: %s", g_gameInfo.numNouns + 1, obj2.id, s, Word);
}

void
set_start()
{
    if (!isdigit(Word[0]))
        object("start state");
    obj2.state = atoi(Word);
    if (obj2.state < 0 || obj2.state > 100)
        object("start state");
}

void
set_holds()
{
    if (!isdigit(Word[0]))
        object("holds= value");
    obj2.contains = atoi(Word);
    if (obj2.contains < 0 || obj2.contains > 1000000)
        object("holds= state");
}

void
set_put()
{
    for (int i = 0; i < NPUTS; i++)
        if (stricmp(obputs[i], Word) == 0) {
            obj2.putto = i;
            return;
        }
    object("put= flag");
}

void
set_mob()
{
    for (int i = 0; i < g_gameInfo.numMobPersonas; i++)
        if (stricmp(mobp[i].id, Word) == 0) {
            obj2.mobile = i;
            return;
        }
    object("mobile= flag");
}

int
iscond(const char *s)
{
    for (int i = 0; i < NCONDS; i++)
        if (strcmp(conds[i], s) == 0)
            return i;
    return -1;
}

int
isact(const char *s)
{
    for (int i = 0; i < NACTS; i++)
        if (strcmp(acts[i], s) == 0)
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

static int
getNo(const char *prefix, const char *from)
{
    int result = 0;
    if (sscanf(from, "%d", &result) != 1) {
        alog(AL_FATAL, "Invalid '%s' entry: %s", prefix, from);
    }
    return result;
}

void
stripNewline(char *text)
{
    size_t len = strlen(text);
    while (len-- > 0 && isEol(text[len])) {
        text[len] = 0;
    }
}

static void
getBlock(const char *linetype, void (*callback)(const char *, const char *))
{
    for (;;) {
        if (!fgets(block, sizeof(block), ifp)) {
            alog(AL_FATAL, "Invalid title.txt: Missing '%s' line", linetype);
        }

        repspc(block);
        stripNewline(block);
        char *p = skipspc(block);
        if (!*p || isCommentChar(*p))
            continue;

        if (!canSkipLead(linetype, &p)) {
            alog(AL_FATAL, "Invalid title.txt:Expected '%s' got: %s", linetype, p);
        }

        callback(linetype, p);
        break;
    }
}

static int blockValue;

void
blockNoTrampoline(const char *prefix, const char *value)
{
    blockValue = getNo(prefix, value);
}

void
getBlockNo(const char *prefix, int *into)
{
    // TODO: This is awful, use a context.
    getBlock(prefix, blockNoTrampoline);
    *into = blockValue;
}

bool
chkline(const char *p)
{
    if (*p == 0) {
        alog(AL_ERROR, "Rank line %" PRIu64 " incomplete", g_gameInfo.numRanks);
        return false;
    }
    return true;
}

void
statinv(const char *s)
{
    alog(AL_FATAL, "Object #%" PRIu64 ": %s: invalid %s state line: %s", g_gameInfo.numNouns + 1,
         obj2.id, s, block);
}

int
getObjectDescriptionID(const char *text)
{
    if (stricmp(text, "none") == 0)
        return -2;

    stringid_t id = -1;
    LookupTextString(text, STRING_OBJECT_DESC, &id);
    return id;
}

int
isnoun(const char *s)
{
    int i;

    objtab2 = obtab2;
    if (stricmp(s, "none") == 0)
        return -2;
    for (i = 0; i < g_gameInfo.numNouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == 0)
            return i;
    return -1;
}

int
iscont(const char *s)
{
    int i;

    objtab2 = obtab2;
    for (i = 0; i < g_gameInfo.numNouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == 0 && objtab2->contains > 0)
            return i;
    return -1;
}

/* Room or container */
int
isloc(const char *s)
{
    int i;

    if ((i = isroom(s)) != -1)
        return i;
    if ((i = iscont(s)) == -1) {
        if (isnoun(s) == -1)
            alog(AL_ERROR, "Invalid object start location: %s", s);
        else
            alog(AL_ERROR, "Tried to start '%s' in non-container '%s'", obj2.id, s);
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

long
chknum(const char *p)
{
    long n;

    if (!isdigit(*p) && !isdigit(*(p + 1)))
        return -1000001;
    if (*p == '>' || *p == '<' || *p == '-' || *p == '=')
        n = atoi(p + 1);
    else
        n = atoi(p);
    if (n >= 1000000) {
        alog(AL_ERROR, "Number too large: %d", n);
        return -1000001;
    }
    if (*p == '-')
        return (long)-n;
    if (*p == '>')
        return (long)(n + LESS);
    if (*p == '<')
        return (long)(n + MORE);
    return n;
}

static const char *optionalPrefixes[] = {  // noise we can skip
        "the ",  "of ",  "are ", "is ",  "has ", "next ", "with ", "to ", "set ",
        "from ", "for ", "by ",  "and ", "was ", "i ",    "am ",   "as ", NULL};

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

int
antype(const char *s)
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
    alog(AL_ERROR, "Invalid announcement-group: %s", s);
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
    objtab2 = obtab2;

    for (i = 0; i < g_gameInfo.numNouns; i++, objtab2++) {
        if (stricmp(s, objtab2->id) != 0)
            continue;
        fseek(fp, (long)(uintptr_t)objtab2->rmlist, 0L);  /// TODO: Dispose
        for (j = 0; j < objtab2->nrooms; j++) {
            fread(&orm, 4, 1, fp);
            if (orm == rmn) {
                l = i;
                i = g_gameInfo.numNouns + 1;
                j = objtab2->nrooms;
                break;
            }
        }
        if (i < g_gameInfo.numNouns)
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
            alog(AL_FATAL, "Unable to add string literal: %d", err);
    } else {
        getword(s);
        error_t err = LookupTextString(Word, STRING_MESSAGE, &id);
        if (err != 0 && err != ENOENT)
            alog(AL_FATAL, "Error looking up string (%d): %s", err, s);
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

    if (n != -70 && (*s == '?' || *s == '%' || *s == '^' || *s == '~' || *s == '`')) {
        if (n != WNUMBER)
            return -1;
        if (*s == '~')
            return RAND0 + atoi(s + 1);
        if (*s == '`')
            return RAND1 + atoi(s + 1);
        i = actualval(s + 1, -70);
        if (i == -1)
            return -1;
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
            return (actual[i].wtype == n || n == -70) ? actual[i].value : -1;

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
            return (vbslot.wtype[3] == n || n == -70) ? actual[i].value : -1;
        case INOUN2:
            return (vbslot.wtype[4] == n || n == -70) ? actual[i].value : -1;
        default:
            return -1; /* Nah... Guru instead 8-) */
        }
    }
    return -2; /* It was no actual! */
}

char *
chkp(char *p, char t, int c, int z, FILE *fp)
{
    int32_t x = -1;

    p = skipOptionalPrefixes(p);
    if (*p == 0) {
        alog(AL_FATAL, "%s '%s' has incomplete C&A line: (%s='%s')", (proc == 1) ? "Verb" : "Room",
             (proc == 1) ? verb.id : roomtab->id, (z == 1) ? "condition" : "action",
             (z == 1) ? conds[c] : acts[c]);
    }
    static char token[4096];
    char *end = NULL;
    if (*p != '\"' && *p != '\'') {
        end = strstop(p, ' ');
        WordCopier(token, p, end);
    } else {
		// Subsequent parsing will want to see the opening quote character
        char quote = *p;
        if (!*p || isEol(*p))
            alog(AL_FATAL, "Unterminated string");
        end = strstop(p+1, quote);
        StrCopier(token, p, end);
        if (*end == quote)
            ++end;
    }
    p = end;

    /* Processing lang tab? */
    if ((t >= 0 && t <= 10) || t == -70) {
        x = actualval(token, t);
        if (x == -1) {
            /* it was an actual, but wrong type */
            alog(AL_ERROR, "Invalid slot label, '%s', after %s '%s' in verb '%s'", token,
                 (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c], verb.id);
            return NULL;
        }
        if (x != -2)
            goto write;
    }
    switch (t) {
    case -6:
        x = onoff(token);
        break;
    case -5:
        x = bvmode(toupper(*token));
        break;
    case -4:
        x = is_stat(token);
        break;
    case -3:
        x = spell(token);
        break;
    case -2:
        x = rdmode(toupper(*token));
        break;
    case -1:
        x = antype(token);
        break;
    case PROOM:
        x = isroom(token);
        break;
    case PVERB:
        x = is_verb(token);
        break;
    case PADJ:
        break;
    case -70:
    case PNOUN:
        x = isnounh(token);
        break;
    case PUMSG:
        x = getTextString(token, true);
        break;
    case PNUM:
        x = chknum(token);
        break;
    case PRFLAG:
        x = isrflag(token);
        break;
    case POFLAG:
        x = isoflag1(token);
        break;
    case PSFLAG:
        x = isoflag2(token);
        break;
    case PSEX:
        x = isgen(toupper(*token));
        break;
    case PDAEMON:
        if ((x = is_verb(token)) == -1 || *token != '.')
            x = -1;
        break;
    default: {
        if (!(proc == 1 && t >= 0 && t <= 10)) {
            alog(AL_FATAL, "Internal error: Invalid PTYPE (val: %d) in %s %s (%s = %s)", t,
                 (proc == 1) ? "verb" : "room", (proc == 1) ? verb.id : (rmtab + rmn)->id,
                 (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c]);
        }
    }
    }
    if (t == -70 && x == -2)
        x = -1;
    else if (((x == -1 || x == -2) && t != PNUM) || x == -1000001) {
        alog(AL_ERROR, "Invalid parameter '%s' after %s '%s' in %s '%s'", token,
             (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c],
             (proc == 1) ? "verb" : "room", (proc == 1) ? (verb.id) : (rmtab + rmn)->id);
        return NULL;
    }
write:
    FPos += checkedfwrite(&x, sizeof(x), 1, fp);
    return *p ? skipspc(p + 1) : p;
}

char *
chkaparms(char *p, int c, FILE *fp)
{
    for (int i = 0; i < nacp[c]; i++)
        if ((p = chkp(p, tacp[c][i], c, 0, fp)) == NULL)
            return NULL;
    return p;
}

char *
chkcparms(char *p, int c, FILE *fp)
{
    for (int i = 0; i < ncop[c]; i++)
        if ((p = chkp(p, tcop[c][i], c, 1, fp)) == NULL)
            return NULL;
    return p;
}

void
chae_err()
{
    alog(AL_ERROR, "Verb: %s: Invalid '#CHAE' flag: %s", verb.id, Word);
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
    int i;

    for (i = 0; i < NSYNTS; i++) {
        if (strcmp(s, syntax[i]) == 0) {
            *s = 0;
            return i - 1;
        }
        if (strncmp(s, syntax[i], syntl[i]) != 0)
            continue;
        if (*(s + syntl[i]) != '=')
            continue;
        strcpy(s, s + syntl[i] + 1);
        return i - 1;
    }
    return -3;
}

/* Declare a PROBLEM, and which verb its in! */
void
vbprob(const char *s, const char *s2)
{
    alog(AL_FATAL, "Verb: %s line: '%s': %s", verb.id, s2, s);
}

void
mobmis(const char *s)
{
    alog(AL_ERROR, "Mobile: %s: missing field: %s", mob.id, s);
    skipblock();
}

/* Fetch mobile message line */
stringid_t
getmobmsg(const char *s)
{
    for (;;) {
        char *p = getTidyBlock(ifp);
        if (feof(ifp))
            alog(AL_FATAL, "Mobile:%s: Unexpected end of file", mob.id);
        if (*p == 0 || isEol(*p)) {
            alog(AL_FATAL, "Mobile: %s: unexpected end of definition", mob.id);
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

error_t
consumeMessageFile(
        FILE *ifp, const char *prefix, enum StringType stype, bool (*checkerFn)(const char *))
{
    if (!nextc(false))
        return ENOENT;

    while (!feof(ifp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        p = getWordAfter(prefix, p);
        alog(AL_DEBUG, "%s%s", prefix, Word);
        if (strlen(Word) < 2 || strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid %s ID: %s", prefix, Word);
            skipblock();
        } else if (checkerFn && !checkerFn(Word)) {
            skipblock();
        } else if (!checkerFn && Word[0] == '$') {
            alog(AL_ERROR, "Invalid ID (can't begin with '$'): %s", Word);
            skipblock();
        }

        TextStringFromFile(Word, ifp, stype, NULL);
    }

    return 0;
}

///////////////////////////////////////////////////////////////////////////////

void
_getAdventureName(const char *prefix, const char *value)
{
    strncpy(g_gameConfig.gameName, value, sizeof(g_gameConfig.gameName));
    if (strlen(value) > sizeof(g_gameConfig.gameName) - 1)
        alog(AL_WARN, "Game name too long, truncated to: %s", g_gameConfig.gameName);
}

void
title_proc()
{
    nextc(true);

    getBlock("name=", _getAdventureName);
    getBlockNo("resettime=", &g_gameConfig.gameDuration_m);
    if (g_gameConfig.gameDuration_m < 15) {
        g_gameConfig.gameDuration_m = 15;
        alog(AL_WARN, "resettime= too short: falling back to %" PRIu64 " minutes",
             g_gameConfig.gameDuration_m);
    }

    getBlockNo("seeinvis=", &g_gameConfig.seeInvisRank);
    getBlockNo("seesuperinvis=", &g_gameConfig.seeSuperInvisRank);

    getBlockNo("sgorank=", &g_gameConfig.superGoRank);

    getBlockNo("rankscale=", &g_gameConfig.rankScale);
    getBlockNo("timescale=", &g_gameConfig.timeScale);

    // Make message 0 be the splash screen
    if (TextStringFromFile("$title", ifp, STRING_FILE, NULL) != 0) {
        alog(AL_FATAL, "Could not write splash text to message file");
    }
}

/*
     System Message processing routines for AMUL, (C) KingFisher Software
     --------------------------------------------------------------------

 Notes:

    System messages MUST be listed in order, and MUST all exist! These
      should be supplied with the package, so the user has a set of defaults.
      We could write all the default system messages into AMULCOM, but this
      would simply be a waste of space!

*/

int currentSysMessage = 0;

bool
check_smsg(const char *token)
{
    if (*token != '$') {
        alog(AL_ERROR, "Invalid system message ID (must start with '$'): %s", token);
        return false;
    }
    int messageID = atoi(token + 1);
    if (messageID != currentSysMessage + 1) {
        alog(AL_ERROR, "Out-of-order system message ID: %s (expected $%d)", token,
             currentSysMessage);
        return false;
    }
    ++currentSysMessage;
    return true;
}

void
smsg_proc()
{
    (void)consumeMessageFile(ifp, "msgid=", STRING_MESSAGE, check_smsg);
    if (currentSysMessage != NSMSGS)
        alog(AL_ERROR, "sysmsg.txt is incomplete.");
}

void
umsg_proc()
{
    error_t err = consumeMessageFile(ifp, "msgid=", STRING_MESSAGE, NULL);
    if (err == ENOENT) {
        /// TODO: Tell the user
        return;
    }
}

void
obds_proc()
{
    error_t err = consumeMessageFile(ifp, "desc=", STRING_OBJECT_DESC, NULL);
    if (err == ENOENT) {
        alog(AL_INFO, "No long object descriptions");
        return;
    }
}

void
saveRoomDmove(const char *dmove)
{
    /// TODO: Implement
}

void
load_rooms()
{
    fopenr(roomDataFile);  // load rooms
    if (reuseRoomData) {
        fseek(ifp, 0, SEEK_END);
        g_gameInfo.numRooms = ftell(ifp) / sizeof(*rmtab);
        fseek(ifp, 0, SEEK_SET);
        rewind(ifp);
    }
    rmtab = (struct _ROOM_STRUCT *)AllocateMem(sizeof(room) * g_gameInfo.numRooms);
    if (rmtab == NULL) {
        alog(AL_FATAL, "Out of memory for room id table");
    }
    size_t roomsInFile = fread(rmtab, sizeof(*rmtab), g_gameInfo.numRooms, ifp);
    if (roomsInFile != g_gameInfo.numRooms) {
        alog(AL_FATAL, "Room table appears to be corrupted. Recompile.");
    }
}

// Process ROOMS.TXT
void
room_proc()
{
    if (reuseRoomData)
        return;

    nextc(true);

    fopenw(roomDataFile);

    do {
        checkErrorCount();

        if (!nextc(false))
            break;

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        p = getWordAfter("room=", p);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            *p = 0;
            alog(AL_ERROR, "Invalid ID (length): %s", p);
            skipblock();
            continue;
        }
        memset(&room, 0, sizeof(room));
        strncpy(room.id, Word, sizeof(room.id));

        // If there are additional words on the line, they are room flags.
        room.flags = 0;
        room.tabptr = -1;
        for (;;) {
            p = skipspc(p);
            if (!*p)
                break;
            p = getword(p);
            char *dmove = Word;
            if (canSkipLead("dmove", &dmove)) {
                if (room.dmove[0] != 0)
                    alog(AL_ERROR, "room:%s: duplicate dmove flag", room.id);
                else
                    WordCopier(room.dmove, dmove, strstop(dmove, ' '));
                continue;
            }
            int no = isrflag(Word);
            if (no == -1) {
                alog(AL_ERROR, "room:%s: Invalid room flag: %s", room.id, Word);
                continue;
            }
            if (room.flags & bitset(no)) {
                alog(AL_WARN, "room:%s: Duplicate room flag: %s", room.id, Word);
            }
            room.flags |= bitset(no);
        }

        error_t err = TextStringFromFile(NULL, ifp, STRING_ROOM_DESC, &room.descid);
        if (err != 0) {
            alog(AL_ERROR, "room:%s: Unable to write description", room.id);
            skipblock();
            continue;
        }
        checkedfwrite(room.id, sizeof(room), 1, ofp1);

        ++g_gameInfo.numRooms;
    } while (!feof(ifp));
}

void
checkdmoves()
{
    if (!checkDmoves || dmoves == 0)
        return;
    /// TODO: Check dmoves
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
        if (chkline(p) != 0)
            continue;

        if (strlen(Word) < 3 || p - block > RANKL) {
            alog(AL_FATAL, "Rank %" PRIu64 ": Invalid male rank: %s", g_gameInfo.numRanks + 1,
                 Word);
        }
        int n = 0;
        do {
            if (Word[n] == '_')
                Word[n] = ' ';
            rank.male[n] = rank.female[n] = tolower(Word[n]);
            n++;
        } while (Word[n - 1] != 0);

        p = getword(p);
        if (chkline(p) != 0)
            continue;
        if (strcmp(Word, "=") != 0 && (strlen(Word) < 3 || strlen(Word) > RANKL)) {
            alog(AL_FATAL, "Rank %d: Invalid female rank: %s", g_gameInfo.numRanks + 1, Word);
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
        if (chkline(p) != 0)
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
            alog(AL_ERROR, "Rank %" PRIu64 " prompt too long: %s", g_gameInfo.numRanks + 1, block);
            continue;
        }
        if (block[0] == 0)
            strcpy(rank.prompt, "$ ");
        else
            WordCopier(rank.prompt, block, p);

        wizstr = rank.strength;
        checkedfwrite(rank.male, sizeof(rank), 1, ofp1);
        g_gameInfo.numRanks++;
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
//    statab = (struct _OBJ_STATE *)data;
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
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("weight value on");
    state.weight = atoi(Word);
    if (obj2.flags & OF_SCENERY)
        state.weight = wizstr + 1;

    /* Get the value of it */
    p = getWordAfter("value=", p);
    if (Word[0] == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("value entry on");
    state.value = atoi(Word);

    /* Get the strength of it (hit points)*/
    p = getWordAfter("str=", p);
    if (Word[0] == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("strength entry on");
    state.strength = atoi(Word);

    /* Get the damage it does as a weapon*/
    p = getWordAfter("dmg=", p);
    if (Word[0] == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("damage entry on");
    state.damage = atoi(Word);

    /* Description */
    p = skiplead("desc=", skipspc(p));
    if (*p == 0)
        statinv("incomplete");
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
        statinv(tmp);
    }
    while (*(p = skipspc(p)) != 0) {
        p = getword(p);
        if (Word[0] == 0)
            break;
        int flag = isoflag2(Word);
        if (flag == -1)
            statinv("flag on");
        state.flags |= bitset(flag);
    }
    checkedfwrite(&state.weight, sizeof(state), 1, ofp2);
    obj2.nstates++;
}

void
objs_proc()
{
    /* Clear files */
    fopenw(objectDataFile);
    fopenw(objectStateFile);
    fopenw(objectRoomFile);
    fopena(adjectiveDataFile);

    obtab2 = (struct _OBJ_STRUCT *)AllocateMem(filesize() + 128 * sizeof(obj2));
    if (obtab2 == NULL)
        alog(AL_FATAL, "Out of memory");
    objtab2 = obtab2;

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
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid object name (length): %s", cur);
            skipblock();
            continue;
        }

        obj2.adj = obj2.mobile = -1;
        obj2.idno = g_gameInfo.numNouns;
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
                    g_gameInfo.numMobs++;
                    break;
                default:
                    alog(AL_FATAL, "Internal Error: Code for object-parameter '%s' missing",
                         obparms[idNo]);
                }
            }
        }

        /* Get the room list */

        bool continuation = true;
        while (continuation) {
            char *p = getTidyBlock(ifp);
            if (!p)
                alog(AL_FATAL, "object:%s: unexpected end of file", obj2.id);

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
                alog(AL_FATAL, "object:%s: unexpected end of file", obj2.id);
            if (!*p || isEol(*p))
                break;
            state_proc();
        }

        if (obj2.nstates == 0)
            alog(AL_ERROR, "object:%s: no states defined");
        if (obj2.nstates > 100)
            alog(AL_ERROR, "object:%s: too many states defined (%d)", obj2.nstates);

        *(obtab2 + (g_gameInfo.numNouns++)) = obj2;
    }

    /*
    closeOutFiles();
    sort_objs();
    */
    checkedfwrite(obtab2, sizeof(obj2), g_gameInfo.numNouns, ofp1);
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
    fopenw(travelTableFile);
    fopenw(travelParamFile);
    fopena(roomDataFile);

    assert(g_gameInfo.numTTEnts == 0);
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
            alog(AL_ERROR, "travel: empty room= line");
            skipblock();
            continue;
        }

        rmn = isroom(Word);
        if (rmn == -1) {
            alog(AL_ERROR, "No such room: %s", Word);
            skipblock();
            continue;
        }
        if (roomtab->tabptr != -1) {
            alog(AL_ERROR, "Multiple tt entries for room: %s", roomtab->id);
            skipblock();
            continue;
        }

    vbloop:
        do
            fgets(block, sizeof(block), ifp);
        while (block[0] != 0 && consumeComment(block));
        if (block[0] == 0 || isEol(block[0])) {
            /* Only complain if room is not a death room */
            if ((roomtab->flags & DEATH) != DEATH) {
                alog(AL_INFO, "No tt entries for room: %s", roomtab->id);
                roomtab->tabptr = -2;
                ntt++;
                continue;
            }
        }
        tidy(block);
        if (!striplead("verb=", block) && !striplead("verbs=", block)) {
            alog(AL_ERROR, "Missing verb[s]= entry for room: %s", roomtab->id);
            goto vbloop;
        }
        verb.id[0] = 0;
        roomtab->tabptr = t;
        roomtab->ttlines = 0;
    vbproc: /* Process verb list */
        tt.pptr = (opparam_t *)-1;
        p = block;
        ttNumVerbs = 0;  // number of verbs in this tt entry
        /* Break verb list down to verb no.s */
        do {
            p = getword(p);
            if (Word[0] == 0)
                break;
            verbsUsed[ttNumVerbs] = is_verb(Word);
            if (verbsUsed[ttNumVerbs] == -1) {
                alog(AL_ERROR, "Room: %s: Invalid verb: %s", roomtab->id, Word);
            }
            ttNumVerbs++;

        } while (Word[0] != 0);
        if (!ttNumVerbs) {
            alog(AL_FATAL, "Room has empty verb[s]= line: %s", roomtab->id);
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
                if ((tt.action = isroom(Word)) != -1)
                    goto write;
                if ((tt.action = isact(Word)) == -1) {
                    alog(AL_ERROR, "Room: %s: invalid tt condition: %s", Word, roomtab->id);
                    goto xloop;
                }
                goto gotohere;
            }
            p = skipspc(p);
            if ((p = chkcparms(p, tt.condition, ofp2)) == NULL) {
                goto next;
            }
            if (r == 1)
                tt.condition = -1 - tt.condition;
            if (*p == 0) {
                alog(AL_ERROR, "Room's tt entry is missing an action", roomtab->id);
                goto xloop;
            }
            p = preact(p);
            p = getword(p);
            if ((tt.action = isroom(Word)) != -1)
                goto write;
            if ((tt.action = isact(Word)) == -1) {
                alog(AL_ERROR, "Room %s has unrecognized tt action: %s", Word, (rmtab + rmn)->id);
                goto xloop;
            }
        gotohere:
            if (tt.action == ATRAVEL) {
                alog(AL_ERROR, "Room %s: Tried to call action 'travel' from travel table",
                     roomtab->id);
                goto xloop;
            }
            p = skipspc(p);
            if ((p = chkaparms(p, tt.action, ofp2)) == NULL) {
                goto next;
            }
            tt.action = 0 - (tt.action + 1);
        write:
            roomtab = rmtab + rmn;

            // this is some weird-ass kind of encoding where -1 means "more", and "-2" means "last"
            for (int verbNo = 0; verbNo < ttNumVerbs; ++verbNo) {
                opparam_t paramid = (verbNo + 1 < ttNumVerbs) ? -1 : -2;
                tt.pptr = (opparam_t *)(uintptr_t)paramid;
                tt.verb = verbsUsed[verbNo];
                checkedfwrite(&tt.verb, sizeof(tt), 1, ofp1);
                roomtab->ttlines++;
                g_gameInfo.numTTEnts++;
            }
        next:
            strip = 0;
        } while (strip == 0 && !feof(ifp));
        if (strip == 1 && !feof(ifp))
            goto vbproc;
        ntt++;
    }

    if (al_errorCount == 0 && ntt != g_gameInfo.numRooms) {
        roomtab = rmtab;
        for (i = 0; i < g_gameInfo.numRooms; i++, roomtab++) {
            if (roomtab->tabptr == -1 && (roomtab->flags & DEATH) != DEATH) {
                alog(AL_WARN, "No TT entry for room: %s", roomtab->id);
            }
        }
    }
    ttroomupdate();
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
getVerbFlags(struct _VERB_STRUCT *verbp, char *p)
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
        if (precedence < 2 && chae_proc(Word, verbp->precedence[precedence]) == -1)
            return;

        alog(AL_ERROR, "Expected verb flag or precedence, got: %s", Word);
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
        ///TODO: size check
        strncpy(verb.id, Word, sizeof(verb.id));
        checkedfwrite(verb.id, sizeof(verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameInfo.numVerbs++)) = verb;
        alog(AL_DEBUG, "Added TRAVEL verb: %s", Word);
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
    afp = NULL;
    fopenw(verbSlotFile);
    fopenw(verbTableFile);
    fopenw(verbParamFile);

    vbtab = (struct _VERB_STRUCT *)AllocateMem(filesize() + 128 * sizeof(verb));
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
            alog(AL_ERROR, "verb= line without a verb?");
            skipblock();
            continue;
        }

        if (strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid verb ID (too long): %s", Word);
            skipblock();
            continue;
        }

        memset(&verb, 0, sizeof(verb));
        strncpy(verb.id, Word, sizeof(verb.id));

        ++g_gameInfo.numVerbs;
        alog(AL_DEBUG, "verb#%04d:%s", g_gameInfo.numVerbs, verb.id);

        getVerbFlags(&verb, p);

        p = getTidyBlock(ifp);
        if (!p)
            alog(AL_ERROR, "Unexpected end of file during verb: %s", verb.id);
        if (!*p || isEol(*p)) {
            if (verb.ents == 0 && (verb.flags & VB_TRAVEL)) {
                alog(AL_WARN, "Verb has no entries: %s", verb.id);
            }
            goto write;
        }

        if (!canSkipLead("syntax=", &p)) {
            vbprob("no syntax= line", block);
            skipblock();
            continue;
        }

        /* Syntax line loop */
    synloop:
        setslots(WNONE);
        verb.ents++;
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
            s = isnoun(Word);
            break;
        case WPREP:
            s = isprep(Word);
            break;
        case WPLAYER:
            if (strcmp(Word, "me") == 0 || strcmp(Word, "myself") == 0)
                s = -3;
            break;
        case WROOM:
            s = isroom(Word);
            break;
        case WSYN:
            alog(AL_WARN, "Synonyms not supported yet");
            s = WANY;
            break;
        case WTEXT:
            s = getTextString(Word, false);
            break;
        case WVERB:
            s = is_verb(Word);
            break;
        case WCLASS:
            s = WANY;
        case WNUMBER:
            if (Word[0] == '-')
                s = atoi(Word + 1);
            else
                s = atoi(Word);
        default:
            alog(AL_ERROR, "Internal Error: Invalid w-type");
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
        vbslot.ptr = (struct _VBTAB *)of3p;

    commands:
        lastc = 'x';
        proc = 0;

        p = getTidyBlock(ifp);
        if (!p)
            alog(AL_FATAL, "verb:%s: unexpected end of file", verb.id);
        if (!*p || isEol(*p)) {
            lastc = 1;
            goto writeslot;
        }

        if (canSkipLead("syntax=", &p)) {
            lastc = 0;
            goto writeslot;
        }

        vbslot.ents++;
        r = -1;
        vt.pptr = (opparam_t *)FPos;

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
            memmove(Word, Word+1, sizeof(Word) - 1);
            Word[sizeof(Word) - 1] = 0;
            r = -1 * r;
            goto notlp2;
        }

        if ((vt.condition = iscond(Word)) == -1) {
            proc = 1;
            if ((vt.action = isact(Word)) == -1) {
                if ((vt.action = isroom(Word)) != -1) {
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
        if ((p = chkcparms(p, vt.condition, ofp4)) == NULL) {
            goto commands;
        }
        if (*p == 0) {
            if ((vt.action = isact(conds[vt.condition])) == -1) {
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
            if ((vt.action = isroom(Word)) != -1)
                goto writecna;
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid action, '%s'", Word);
            vbprob(tmp, block);
            goto commands;
        }
    doaction:
        p = skipspc(p);
        if ((p = chkaparms(p, vt.action, ofp4)) == NULL) {
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
        checkedfwrite(verb.id, sizeof(verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameInfo.numVerbs - 1)) = verb;
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
            g_gameInfo.numSynonyms++;
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
    struct _MOB *mobd = &mob.mob;

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
                mobd->dmove = isroom(Word);
                if (mobd->dmove == -1) {
                    alog(AL_ERROR, "Mobile: %s: invalid dmove: %s", mob.id, Word);
                }
                continue;
            }
        }

        p = getTidyBlock(ifp);
        if (!p)
            alog(AL_FATAL, "mob: !%s: unexpected end of file", mob.id);

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
        g_gameInfo.numMobPersonas++;
    }

    if (g_gameInfo.numMobPersonas != 0) {
        mobp = (struct _MOB_ENT *)AllocateMem(sizeof(mob) * g_gameInfo.numMobPersonas);
        if (mobp == NULL) {
            alog(AL_FATAL, "Out of memory");
        }
        CloseOutFiles();

        fopenr(mobileDataFile);
        checkedfread(mobp, sizeof(mob), g_gameInfo.numMobPersonas, ifp);
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

struct Context *
NewContext(const char *filename)
{
    struct Context *context = calloc(sizeof(struct Context), 1);
    if (context == NULL) {
        alog(AL_FATAL, "Out of memory for context");
    }

    if (EINVAL == path_join(context->filename, sizeof(context->filename), gameDir, filename)) {
        alog(AL_FATAL, "Path length exceeds limit for %s/%s", gameDir, filename);
    }

    return context;
}
*/

struct CompilePhase {
    const char *name;
    bool        isText;
    void (*handler)();
};

struct CompilePhase phases[] = {
        {"title", true, title_proc},      // game "title" (and config)
        {"sysmsg", true, smsg_proc},      // system messages
        {"umsg", true, umsg_proc},        // user-defined string messages
        {"obdescs", true, obds_proc},     // object description strings
        {"rooms", true, room_proc},       // room table
        {"roomread", false, load_rooms},  // re-load room table
        {"dmoves", false, checkdmoves},   // check cemeteries
        {"ranks", true, rank_proc},       // rank table
        {"mobiles", true, mob_proc1},     // npc classes so we can apply to objects
        {"objects", true, objs_proc},     // objects
        {"lang", true, lang_proc},        // language
        {"travel", true, trav_proc},      // travel table
        {"syns", true, syn_proc},         // synonyms for other things
        {NULL, false, NULL}               // terminator
};

void
compilePhase(const struct CompilePhase *phase)
{
    alog(AL_INFO, "Compiling: %s", phase->name);
    if (phase->isText) {
        char filename[MAX_PATH_LENGTH];
        snprintf(filename, sizeof(filename), "%s.txt", phase->name);
        ifp = OpenGameFile(filename, "r");
    }

    phase->handler();

    if (ifp) {
        fclose(ifp);
        ifp = NULL;
    }

    CloseOutFiles();

    if (al_errorCount > 0) {
        alog(AL_FATAL, "Terminating due to errors.");
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
compilerModuleInit(struct Module *module)
{
    snprintf(
            compilerVersion, sizeof(compilerVersion), "AMULCom v%d.%03d (%8s)", VERSION, REVISION,
            DATE);
    return 0;
}

error_t
compilerModuleStart(struct Module *module)
{
    // set process name
    mytask = FindTask(0L);
    mytask->tc_Node.ln_Name = compilerVersion;
    alog(AL_INFO, "AMUL Compiler %s", compilerVersion);

    alog(AL_DEBUG, "Game Directory: %s", gameDir);
    alog(AL_DEBUG, "Log Verbosity : %s", alogGetLevelName());
    alog(AL_DEBUG, "Check DMoves  : %s", checkDmoves ? "true" : "false");
    alog(AL_DEBUG, "Reuse Rooms   : %s", reuseRoomData ? "true" : "false");

    struct stat sb;

    for (const struct CompilePhase *phase = &phases[0]; phase->name; ++phase) {
        if (phase->isText == false)
            continue;

        char filename[MAX_PATH_LENGTH];
        snprintf(filename, sizeof(filename), "%s.txt", phase->name);

        char filepath[MAX_PATH_LENGTH];
        safe_gamedir_joiner(filename);

        int err = stat(filepath, &sb);
        if (err != 0)
            alog(AL_FATAL, "Missing file (%d): %s", err, filepath);
    }

    return 0;
}

error_t
compilerModuleClose(struct Module *module, error_t err)
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
    NewModule(
            false, MOD_COMPILER, compilerModuleInit, compilerModuleStart, compilerModuleClose, NULL,
            NULL);
}

/*---------------------------------------------------------*/

error_t
amulcom_main()
{
    InitStrings();

    InitCompilerModule();

    StartModules();

    compileGame();

    g_gameInfo.numStrings = GetStringCount();

    alog(AL_NOTE, "Execution finished normally");
    alog(AL_INFO, "Statistics for %s:", g_gameConfig.gameName);
    alog(AL_INFO, "		Rooms: %6" PRIu64 "	Ranks:   %6" PRIu64 "	Nouns: %6" PRIu64 "",
         g_gameInfo.numRooms, g_gameInfo.numRanks, g_gameInfo.numNouns);
    alog(AL_INFO, "		Adj's: %6" PRIu64 "	Verbs:   %6" PRIu64 "	Syns : %6" PRIu64 "",
         g_gameInfo.numAdjectives, g_gameInfo.numVerbs, g_gameInfo.numSynonyms);
    alog(AL_INFO, "		T.T's: %6" PRIu64 "	Strings: %6" PRIu64 "", g_gameInfo.numTTEnts,
         g_gameInfo.numStrings);

    fopenw(gameDataFile);
    checkedfwrite(&g_gameConfig, sizeof(g_gameConfig), 1, ofp1);
    checkedfwrite(&g_gameInfo, sizeof(g_gameInfo), 1, ofp1);
    CloseOutFiles();

    exiting = true;

    return 0;
}
