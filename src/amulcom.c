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

   When the LOCATE function is installed, aswell as as the 'i' words, we must
  have a user variable, 'located'. This will allow the user to fiddle and
  tinker with the locate function... We will also need a 'setword' so that
  you could write:

   locate id [pref=]c|h|a|e|i [state=]# in=ID|cont=ID|outside|REGardless[/#]

   Last Amendments: 26/08/90   12:30   OS   Installed GameTime= (title.txt)
            27/09/90   14:52   OS   Enhanced ObjProc.C (uses MC rtns)

                                       */

#define COMPILER 1

#include "h/amulcom.h"
#include "h/amul.alog.h" /* Logging */
#include "h/amul.cons.h" /* Predefined Constants etc     */
#include "h/amul.defs.h" /* Defines in one nice file     */
#include "h/amul.incs.h" /* Include files tidily stored. */
#include "h/amul.lnks.h" /* (external) Linkage symbols   */
#include "h/amul.vars.h" /* all INTERNAL variables       */
#include "h/amul.xtra.h"

#include <stdlib.h>
#include <time.h>

/* Compiler specific variables... */

bool exiting = false;
bool verbose = false;
bool checkDmoves = false;
bool reuseRoomData = false;

int   dmoves = 0;          /* How many DMOVEs to check?	*/
int   rmn = 0;             /* Current room no.		*/
long  titlePos = 0;        /* Position of the title text in title.txt */
long  FPos;                /* Used during TT/Lang writes	*/
char  Word[64];            /* For internal use only <grin>	*/
int   errorCount;          /* Number of AL_ERRORs logged */
int   proc;                /* What we are processing	*/
long  startTime;           /* Bits for time etc		*/
char *data;                /* Pointer to data buffer	*/
char *data2;               /* Secondary buffer area	*/
char *syntab;              /* Synonym table, re-read	*/
char  idt[IDL + 1];        /* Temporary ID store		*/
long  datal, datal2, mins; /* Length of data! & gametime	*/
long  obmem;               /* Size of Objects.TXT		*/
long  vbmem;               /* Size of Lang.Txt		*/
int   invis, invis2;       /* Invisibility Stuff		*/
long  wizstr;              /* Wizards strength		*/
char *mobdat, *px;         /* Mobile data			*/
long  moblen;              /* Length			*/

struct Task *mytask, *FindTask();

struct UMSG {
    char id[IDL + 1];
    long fpos;
} umsg;

char  fnm[150];
FILE *ofp5;

struct _OBJ_STRUCT2 *obtab2, *objtab2, obj2, *osrch, *osrch2;

#if defined(_AMIGA_)
long ohd; /* Output handle for direct dos	*/
#endif

///////////////////////////////////////////////////////////////////////////////

void
close_ofps()
{
    if (ofp1 != NULL)
        fclose(ofp1);
    if (ofp2 != NULL)
        fclose(ofp2);
    if (ofp3 != NULL)
        fclose(ofp3);
    if (ofp4 != NULL)
        fclose(ofp4);
    if (ofp5 != NULL)
        fclose(ofp5);
    if (afp != NULL)
        fclose(afp);
    ofp1 = ofp2 = ofp3 = ofp4 = ofp5 = afp = NULL;
}

void
quit()
{
    close_ofps();
    if (exiting) {
        sprintf(block, "%s%s", dir, advfn);
        unlink(block);
    }
    unlink("ram:ODIDs");
    unlink("ram:umsg.tmp");
    unlink("ram:objh.tmp");
    if (mobdat)
        FreeMem(mobdat, moblen);
    mobdat = NULL;
    if (mobp)
        FreeMem(mobp, sizeof(mob) * mobchars);
    mobp = NULL;
    if (rmtab != 0)
        FreeMem(rmtab, sizeof(room) * rooms);
    rmtab = NULL;
    if (data != 0)
        FreeMem(data, datal);
    data = NULL;
    if (data2 != 0)
        FreeMem(data2, datal2);
    data2 = NULL;
    if (obtab2 != 0)
        FreeMem(obtab2, obmem);
    obtab2 = NULL;
    if (vbtab != 0)
        FreeMem(vbtab, vbmem);
    vbtab = NULL;
    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    exit(0);
}

void
checkErrorCount()
{
    if (al_errorCount > 30) {
        alog(AL_FATAL, "Terminating due to %u errors", al_errorCount);
    }
}

void
CXBRK()
{
    alog(AL_NOTE, "** CTRL-C pressed - memory released, files closed! Tata!");
    quit();
}

bool
com(const char *text)
{
    if (*text == '*')
        alog(AL_INFO, "%s", text);
    return isCommentChar(*text);
}

const char *
getword(const char *from)
{
    char *to = Word;
    *to = 0;
    from = skipspc(from);
    for (const char *end = Word + sizeof(Word) - 1; to < end; ++to, ++from) {
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
        case '\t': goto broke;
        default: ++from;
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
            fgets(block, 1024, ifp);
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
fatalOp(char *s, char *t)
{
    alog(AL_ERROR, "Can't %s %s", s, t);
}

/* Open file for reading */
void
fopenw(char *s)
{
    if (*s == '-')
        strcpy(fnm, s + 1);
    else
        sprintf(fnm, "%s%s", dir, s);
    FILE *fp = fopen(fnm, "wb");
    if (!fp)
        fatalOp("write", fnm);
    if (!ofp1)
        ofp1 = fp;
    else if (!ofp2)
        ofp2 = fp;
    else if (!ofp3)
        ofp3 = fp;
    else if (!ofp4)
        ofp4 = fp;
    else if (!ofp5)
        ofp5 = fp;
    else
        fatalOp("select", "file descriptor");
}

/* Open file for appending */
void
fopena(char *s)
{
    if (afp != NULL)
        fclose(afp);
    if (*s == '-')
        strcpy(fnm, s + 1);
    else
        sprintf(fnm, "%s%s", dir, s);
    if ((afp = fopen(fnm, "rb+")) == NULL)
        fatalOp("create", fnm);
}

/* Open file for reading */
void
fopenr(char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    if (*s != '-')
        sprintf(fnm, "%s%s", dir, s);
    else
        strcpy(fnm, s + 1);
    if ((ifp = fopen(fnm, "rb")) == NULL)
        fatalOp("open", fnm);
}

/* Open file for reading */
FILE *
rfopen(const char *s)
{
    FILE *fp;

    if (*s != '-')
        sprintf(fnm, "%s%s", dir, s);
    else
        strcpy(fnm, s + 1);
    if ((fp = fopen(fnm, "rb")) == NULL)
        fatalOp("open", fnm);
    return fp;
}

/* Update room entries after TT */
void
ttroomupdate()
{
    fseek(afp, 0, 0L);
    fwrite(rmtab->id, sizeof(room), rooms, afp);
}

void
opentxt(char *s)
{
    sprintf(block, "%s%s.TXT", dir, s);
    if ((ifp = fopen(block, "r")) == NULL) {
        alog(AL_FATAL, "Missing file: %s", block);
    }
}

void
skipblock()
{
    char c, lc;

    lc = 0;
    c = '\n';
    while (c != EOF && !(c == lc && lc == '\n')) {
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

int
is_verb(const char *s)
{
    int i;

    if (verbs == 0) {
        alog(AL_FATAL, "Attempted to look up verb '%s' with no verbs defined", s);
    }
    if (strlen(s) > IDL) {
        printf("Invalid verb ID (too long): %s", s);
        return -1;
    }

    vbptr = vbtab;
    for (i = 0; i < verbs; i++, vbptr++) {
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

void
blkget(long *s, char **p, long off)
{
    *s = filesize() + off;
    if ((*p = (char *)AllocMem(*s, MEMF_PUBLIC)) == NULL) {
        alog(AL_FATAL, "Out of memory");
    }
    fread((*p) + off, 1, *s, ifp);
    *((*p + *s) - 2) = 0;
    *((*p + *s) - 1) = 0;
    // repcrlf((*p)+off);	///TODO: HANDLE \r
}

/* Check to see if s is a room flag */
int
isrflag(const char *s)
{
    int _x;
    for (_x = 0; _x < NRFLAGS; _x++)
        if (strcmp(s, rflag[_x]) == 0)
            return _x;
    return -1;
}

int
isroom(const char *s)
{
    int r;
    roomtab = rmtab;
    for (r = 0; r < rooms; r++)
        if (strcmp(roomtab->id, s) == 0)
            return r;
        else
            roomtab++;
    return -1;
}

/* Is it a FIXED object flag? */
int
isoflag1(const char *s)
{
    int i;
    for (i = 0; i < NOFLAGS; i++)
        if (strcmp(obflags1[i], s) == 0)
            return i;
    return -1;
}

/* Is it an object parameter? */
int
isoparm()
{
    int i;
    for (i = 0; i < NOPARMS; i++)
        if (striplead(obparms[i], Word))
            return i;
    return -1;
}

/* Is it a state flag? */
int
isoflag2(const char *s)
{
    int i;
    for (i = 0; i < NSFLAGS; i++)
        if (strcmp(obflags2[i], s) == 0)
            return i;
    return -1;
}

void
set_adj()
{
    int i;

    if (strlen(Word) > IDL || strlen(Word) < 3) {
        alog(AL_FATAL, "Invalid adjective (length): %s", Word);
    }
    if (adjs == 0) {
        for (i = 0; i < IDL + 1; i++)
            dmove[i] = 0;
        strcpy(dmove, Word);
        obj2.adj = 0;
        fwrite(dmove, IDL + 1, 1, afp);
        adjs++;
        return;
    }
    fseek(afp, 0L, 0); /* Move to beginning */
    i = 0;
    do {
        if (fread(dmove, IDL + 1, 1, afp) != 1)
            continue; /* Read adj! */
        if (strcmp(Word, dmove) == 0) {
            obj2.adj = i;
            return;
        }
        i++;
    } while (!feof(afp));
    for (i = 0; i < IDL + 1; i++)
        dmove[i] = 0;
    strcpy(dmove, Word);
    fseek(afp, 0L, 2);              /* Move to end! */
    fwrite(dmove, IDL + 1, 1, afp); /* Add this adjective */
    obj2.adj = adjs++;
}

void
object(char *s)
{
    alog(AL_FATAL, "Object #%d: %s: invalid %s: %s", nouns + 1, obj2.id, s, Word);
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
    int i;

    for (i = 0; i < NPUTS; i++)
        if (stricmp(obputs[i], Word) == 0) {
            obj2.putto = i;
            return;
        }
    object("put= flag");
}

void
set_mob()
{
    int i;
    for (i = 0; i < mobchars; i++)
        if (stricmp(Word, (mobp + i)->id) == 0) {
            obj2.mobile = i;
            return;
        }
    object("mobile= flag");
}

int
iscond(char *s)
{
    int i;
    for (i = 0; i < NCONDS; i++)
        if (strcmp(conds[i], s) == 0)
            return i;
    return -1;
}

int
isact(char *s)
{
    int i;
    for (i = 0; i < NACTS; i++)
        if (strcmp(acts[i], s) == 0)
            return i;
    return -1;
}

int
isprep(char *s)
{
    int i;
    for (i = 0; i < NPREP; i++)
        if (strcmp(s, prep[i]) == 0)
            return i;
    return -1;
}

int
getno(const char *s)
{
    char *p = skipspc(block);
    p = skiplead(s, block);
    if (!p) {
        alog(AL_ERROR, "Missing %s entry", s);
        return -1;
    }
    p = getword(p);
    if (!isdigit(Word[0])) {
        alog(AL_ERROR, "Invalid %s entry", s);
        return -1;
    }
    return atoi(Word);
}

///////////////////////////////////////////////////////////////////////////////

void
title_proc()
{
    nextc(true);
    fgets(block, 1000, ifp);
    repspc(block);
    char *p = block;
    if (!canSkipLead("name=", &p)) {
        alog(AL_FATAL, "Invalid title.txt: missing name= line");
    }
    int length = strlen(p);
    *(p + length - 1) = 0;  // remove newline
    if (length > 40) {
        *(p + 40) = 0;
        alog(AL_WARN, "Game name too long: trunvate to %s", block);
    }
    strcpy(adname, block);
    fgets(block, 1000, ifp);
    repspc(block);
    mins = getno("gametime=");
    if (mins < 15) {
        mins = 15;
        alog(AL_WARN, "Game time too short, falling back to %d minutes", mins);
    }

    fgets(block, 1000, ifp);
    repspc(block);
    p = block;
    if (!canSkipLead("invisible=", &p)) {
        alog(AL_FATAL, "Missing invisible= line");
    }
    int reads = sscanf(p, "%d %d", &invis, &invis2);
    if (reads != 2) {
        alog(AL_ERROR, "Invalid invisible= line: %s", block);
    }

    fgets(block, 1000, ifp);
    repspc(block);
    minsgo = getno("min sgo=");

    /* Get the Scaleing line. */
    fgets(block, 1000, ifp);
    repspc(block);
    rscale = getno("rankscale="); /* Process RankScale= */
    fgets(block, 1000, ifp);
    repspc(block);
    tscale = getno("timescale="); /* Process TimeScale= */

    titlePos = ftell(ifp);
}

/* Process ROOMS.TXT */
void
room_proc()
{
    char c, lastc, *p, *p2;
    int  n;

    nextc(true);

    fopenw(rooms1fn);
    fopenw(rooms2fn);

    do {
        p = block;
        while ((c = fgetc(ifp)) != EOF && !isspace(c))
            *(p++) = c;
        if (c == EOF)
            break;
        *p = 0; /* Set null byte */
        p = skipspc(block);
        if (*p == 0)
            continue;
        striplead("room=", block);
        if (strlen(block) < 3 || strlen(block) > IDL) {
            alog(AL_FATAL, "Invalid ID (length): %s", block);
        }
        strcpy(room.id, block);
        /* Do the flags */
        room.flags = 0;
        room.tabptr = -1;
        temp[0] = 0;
        if (c != '\n') {
            fgets(block, 1024, ifp);
            p = block;
            n = -1;
            do {
                while (isspace(*p) && *p != 0)
                    p++;
                if (*p == 0)
                    continue;
                p2 = p;
                while (!isspace(*p2) && *p2 != 0)
                    p2++;
                *p2 = 0;
                if (n == 0) {
                    /* Get dmove param */
                    strcpy(temp, p);
                    dmoves++;
                    p = p2 + 1;
                    n = -1;
                    continue;
                }
                if ((n = isrflag(p)) == -1) {
                    alog(AL_FATAL, "Invalid flag: %s", p);
                }
                n -= NRNULL;
                if (n >= 0)
                    room.flags = (room.flags | bitset(n));
                p = p2 + 1;
            } while (*p != 0);
        }

        lastc = '\n';
        fseek(ofp2, 0, 1);
        room.desptr = ftell(ofp2);
        n = 0;
        if (temp[0] != 0)
            fwrite(temp, IDL, 1, ofp2); /* save dmove */
        while ((c = fgetc(ifp)) != EOF && !(c == '\n' && lastc == '\n')) {
            if (lastc == '\n' && c == 9)
                continue;
            fputc((lastc = c), ofp2);
            n++;
        };
        fputc(0, ofp2);
        fwrite(room.id, sizeof(room), 1, ofp1);
        ++rooms;
        nextc(false);
    } while (c != EOF);
}

void
checkdmoves()
{
    struct _ROOM_STRUCT *roomptr;

    /* Check DMOVE ptrs */
    fopenr(rooms2fn); /* Open desc. file */
    roomptr = rmtab;
    for (int n = 0; n < rooms; n++) {
        if (roomptr->flags & DMOVE) {
            printf("%-9s\r", roomptr->id);
            fseek(ifp, roomptr->desptr, 0);
            fread(dmove, IDL, 1, ifp); /* Read the DMOVE name */
            if (isroom(dmove) == -1) {
                alog(AL_ERROR, "%-9s: invalid dmove: %s", roomptr->id, dmove);
            }
        }
        roomptr++;
    }
}

bool
chkline(char *p)
{
    if (*p == 0) {
        alog(AL_ERROR, "Rank line %d incomplete", ranks);
        return false;
    }
    return true;
}

/* Process RANKS.TXT */
void
rank_proc()
{
    nextc(true);

    fopenw(ranksfn);

    do {
        fgets(block, 1024, ifp);
        if (feof(ifp))
            break;
        if (com(block) || isEol(block[0]))
            continue;
        tidy(block);
        if (block[0] == 0)
            continue;
        char *p = getword(block);
        if (chkline(p) != 0)
            continue;
        rank.male[0] = 0;
        rank.female[0] = 0;
        if (strlen(Word) < 3 || strlen(Word) > RANKL) {
            alog(AL_FATAL, "Rank %d: Invalid male rank: %s", ranks, Word);
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
            alog(AL_FATAL, "Rank %d: Invalid female rank: %s", ranks, Word);
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
        *(p++) = 0;
        /* Greater than prompt length? */
        if (p - block > 10) {
            alog(AL_ERROR, "Rank %d prompt too long: %s", ranks, block);
            continue;
        }
        if (block[0] == 0)
            strcpy(rank.prompt, "$ ");
        else
            strcpy(rank.prompt, block);

        wizstr = rank.strength;
        fwrite(rank.male, sizeof(rank), 1, ofp1);
        ranks++;
    } while (!feof(ifp));
}

void
obds_proc()
{
    char lastc;

    obdes = 0;
    fopenw(obdsfn);
    close_ofps(); /* Create file */
    if (!nextc(false)) {
        alog(AL_INFO, "No long object descriptions");
        return;
    }
    fopenw("-ram:ODIDs");
    fopenw(obdsfn);
    char c;
    do {
        fgets(block, 1024, ifp);
        tidy(block);
        striplead("desc=", block);
        getword(block);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid obj. description id: %s", Word);
            skipblock();
            continue;
        }
        strcpy(objdes.id, Word);
        fseek(ofp2, 0, 2);
        objdes.descrip = ftell(ofp2);
        fwrite(objdes.id, sizeof(objdes), 1, ofp1);
        lastc = '\n';
        while ((c = fgetc(ifp)) != EOF && !(c == '\n' && lastc == '\n')) {
            if ((lastc == EOF || lastc == '\n') && c == 9)
                continue;
            fputc((lastc = c), ofp2);
        };
        fputc(0, ofp2);
        obdes++;
        nextc(false);
    } while (c != EOF);
}

/*
void
sort_objs()
{	int i,j,k,nts; long *rmtab,*rmptr;

    if(ifp!=NULL) fclose(ifp); ifp=NULL;
    close_ofps(); fopenr(statfn);	blkget(&datal,&data,NULL); fclose(ifp); ifp=NULL;
    close_ofps(); fopenr(objrmsfn); blkget(&datal2,&data2,NULL); fclose(ifp); ifp=NULL;
    close_ofps(); fopenw(objsfn);	fopenw(statfn); fopenw(objrmsfn); fopenw(ntabfn);
    ifp=NULL;

    printf("Sorting Objects...:\r"); objtab2=obtab2; nts=0; k=0;

    statab=(struct _OBJ_STATE *)data; rmtab=(long *)data2;
    for(i=0; i<nouns; i++)
    {
        if(*(objtab2=(obtab2+i))->id==0)
        {
            printf("@! skipping %ld states, %ld rooms.\n",objtab2->nstates,objtab2->nrooms);
            statab += objtab2->nstates;
            rmtab  += objtab2->nrooms;
            continue;
        }
        strcpy(nountab.id,objtab2->id); nts++;
        nountab.num_of=0; osrch=objtab2; statep=statab; rmptr=rmtab;
        for(j=i; j<nouns; j++, osrch++)
        {
            if(*(osrch->id)!=0 && stricmp(nountab.id,osrch->id)==0)
            {
                fwrite((char *)osrch,  sizeof(obj),   1,               ofp1);
                fwrite((char *)statep, sizeof(state), osrch->nstates,  ofp2);
                fwrite((char *)rmptr,  sizeof(long),  osrch->nrooms,   ofp3);
                nountab.num_of++; *osrch->id=0; if(osrch!=objtab) k++;
                statep+=osrch->nstates; rmptr+=osrch->nrooms;
                if(osrch==objtab2) { statab=statep; rmtab=rmptr; objtab2++; i++; }
            }
            else statep+=osrch->nstates; rmptr+=osrch->nrooms;
        }

        fwrite((char *)&nountab, sizeof(nountab), 1, ofp4);
    }
    printf("%20s\r%ld objects moved.\n"," ",k);
    FreeMem(data, datal); FreeMem(data2, datal2); data=data2=NULL;
    fopenr(objsfn); fread((char *)obtab2, sizeof(obj), nouns, ifp);
}
*/

void
statinv(char *s)
{
    alog(AL_FATAL, "Object #%d: %s: invalid %s state line: %s", nouns + 1, obj2.id, s, block);
}

void
is_desid()
{
    int   i;
    FILE *fp;
    if (stricmp(Word, "none") == 0) {
        state.descrip = -2;
        return;
    }
    if ((fp = fopen("ram:ODIDs", "rb+")) == NULL)
        fatalOp("open", "ram:ODIDs");
    for (i = 0; i < obdes; i++) {
        fread(objdes.id, sizeof(objdes), 1, fp);
        state.descrip = objdes.descrip;
        if (stricmp(Word, objdes.id) == 0) {
            fclose(fp);
            return;
        }
    }
    fclose(fp);
    state.descrip = -1;
}

void
text_id(char *p, char c)
{
    char *ptr;
    FILE *fp;

    strcpy(block, p);
    p = block;
    while (*p != c && *p != 0)
        p++;
    if (*p == 0)
        *(p + 1) = 0;
    *(p++) = '\n';
    if (*(p - 2) == '{')
        ptr = p - 1;
    else
        ptr = p;

    sprintf(temp, "%s%s", dir, obdsfn); /* Open output file */
    if ((fp = fopen(temp, "rb+")) == NULL)
        fatalOp("open", temp);
    fseek(fp, 0, 2L);
    state.descrip = ftell(fp); /* Get pos */
    if (fwrite(block, ptr - block, 1, fp) != 1) {
        fclose(fp);
        fatalOp("write", temp);
    }
    fputc(0, fp);
    strcpy(block, p);
    fclose(fp);
}

void
state_proc()
{
    int   flag;
    char *p;

    state.weight = state.value = state.flags = 0;
    state.descrip = -1;

    tidy(block);
    if (block[0] == 0)
        return;

    /* Get the weight of the object */
    striplead("weight=", block);
    p = getword(block);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("weight value on");
    state.weight = atoi(Word);
    if (obj2.flags & OF_SCENERY)
        state.weight = wizstr + 1;

    /* Get the value of it */
    p = skipspc(p);
    striplead("value=", p);
    p = getword(p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("value entry on");
    state.value = atoi(Word);

    /* Get the strength of it (hit points)*/
    p = skipspc(p);
    striplead("str=", p);
    p = getword(p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("strength entry on");
    state.strength = atoi(Word);

    /* Get the damage it does as a weapon*/
    p = skipspc(p);
    striplead("dam=", p);
    p = getword(p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("damage entry on");
    state.damage = atoi(Word);

    /* Description */
    p = skipspc(p);
    striplead("desc=", p);
    if (*p == 0)
        statinv("incomplete");
    if (*p == '\"' || *p == '\'') {
        text_id(p + 1, *p);
        p = block;
    } else {
        p = getword(p);
        is_desid(); /* Is it valid? */
    }
    if (state.descrip == -1) {
        char tmp[128];
        snprintf(tmp, "desc= ID (%s) on", Word);
        statinv(tmp);
    }
    while (*p != 0) {
        p = getword(p);
        if (Word[0] == 0)
            break;
        if ((flag = isoflag2(Word)) == -1)
            statinv("flag on");
        state.flags = (state.flags | bitset(flag));
    }
    fwrite((char *)&state.weight, sizeof(state), 1, ofp2);
    obj2.nstates++;
}

int
isnoun(char *s)
{
    int i;

    objtab2 = obtab2;
    if (stricmp(s, "none") == 0)
        return -2;
    for (i = 0; i < nouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == 0)
            return i;
    return -1;
}

int
iscont(char *s)
{
    int i;

    objtab2 = obtab2;
    for (i = 0; i < nouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == 0 && objtab2->contains > 0)
            return i;
    return -1;
}

/* Room or container */
int
isloc(char *s)
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
    char *s2;

    s2 = s;

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

char *
preact(char *s)
{
    char *s2;

    s2 = s;
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

long
chknum(char *p)
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

char *
optis(char *p)
{
    char *p2;
    p2 = p;

    p = skiplead("the ", p);
    p = skiplead("of ", p);
    p = skiplead("are ", p);
    p = skiplead("is ", p);
    p = skiplead("has ", p);
    p = skiplead("next ", p);
    p = skiplead("with ", p);
    p = skiplead("to ", p);
    p = skiplead("set ", p);
    p = skiplead("from ", p);
    p = skiplead("for ", p);
    p = skiplead("by ", p);
    p = skiplead("and ", p);
    p = skiplead("was ", p);
    p = skiplead("i ", p);
    p = skiplead("am ", p);
    p = skiplead("as ", p);
    p = skipspc(p);
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
antype(char *s)
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
isnounh(char *s)
{
    int   i, l, j;
    FILE *fp;
    long  orm;

    if (stricmp(s, "none") == 0)
        return -2;
    fp = (FILE *)rfopen(objrmsfn);
    l = -1;
    objtab2 = obtab2;

    for (i = 0; i < nouns; i++, objtab2++) {
        if (stricmp(s, objtab2->id) != 0)
            continue;
        fseek(fp, (long)objtab2->rmlist, 0L);
        for (j = 0; j < objtab2->nrooms; j++) {
            fread((char *)&orm, 4, 1, fp);
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
        return RDRC;
    if (c == 'V')
        return RDVB;
    if (c == 'B')
        return RDBF;
    return -1;
}

int
spell(char *s)
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
stat(char *s)
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
user is refering too.							     */

/* Get actual value! */
int
actualval(char *s, int n)
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
        case IADJ2: return (vbslot.wtype[3] == n || n == -70) ? actual[i].value : -1;
        case INOUN2: return (vbslot.wtype[4] == n || n == -70) ? actual[i].value : -1;
        default: return -1; /* Nah... Guru instead 8-) */
        }
    }
    return -2; /* It was no actual! */
}

int
msgline(char *s)
{
    FILE *fp;
    long  pos;
    char  c;
    fp = afp;
    afp = NULL;
    fopena(umsgfn);
    fseek(afp, 0, 2L);
    pos = ftell(afp);

    fwrite(s, strlen(s) - 1, 1, afp);
    if ((c = *(s + strlen(s) - 1)) != '{') {
        fputc(c, afp);
        fputc('\n', afp);
    }
    fputc(0, afp);
    fopena(umsgifn);
    fseek(afp, 0, 2L);
    fwrite((char *)&pos, sizeof(long), 1, afp);
    fclose(afp);
    afp = fp;
    return NSMSGS + (umsgs++);
}

/* Check FP for umsg id! */
int
isumsg(char *s, FILE *fp)
{
    int i;

    if (*s == '$') {
        i = atoi(s + 1);
        if (i < 1 || i > NSMSGS) {
            alog(AL_ERROR, "Invalid system message ID: %s", s);
        }
        return i - 1;
    }
    if (umsgs == 0)
        return -1;
    fseek(fp, 0, 0L); /* Rewind file */
    for (i = 0; i < umsgs; i++) {
        fread(umsg.id, sizeof(umsg), 1, fp);
        if (stricmp(umsg.id, s) == 0)
            return i + NSMSGS;
    }
    return -1;
}

int
chkumsg(char *s)
{
    int   r;
    FILE *fp;

    if (*s != '$' && umsgs == 0)
        return -1;

    if ((fp = fopen("ram:umsg.tmp", "rb+")) == NULL) {
        alog(AL_FATAL, "Unable to access umsg.tmp file");
    }
    r = isumsg(s, fp);
    fclose(fp);
    return r;
}

int
ttumsgchk(char *s)
{
    s = skiplead("msgid=", s);
    s = skiplead("msgtext=", s);
    s = skipspc(s);
    if (*s == '\"' || *s == '\'')
        return msgline(s + 1);
    return chkumsg(s);
}

int
onoff(char *p)
{
    if (stricmp(p, "on") == 0 || stricmp(p, "yes") == 0)
        return 1;
    return 0;
}

char *
chkp(char *p, char t, int c, int z, FILE *fp)
{
    char qc, *p2;
    long x;

    p = optis(p);
    p2 = (p = skipspc(p)); /* Strip crap out */
    if (*p == 0) {
        alog(AL_FATAL, "%s '%s' has incomplete C&A line: (%s='%s')", (proc == 1) ? "Verb" : "Room",
             (proc == 1) ? verb.id : roomtab->id, (z == 1) ? "condition" : "action",
             (z == 1) ? conds[c] : acts[c]);
    }
    if (*p != '\"' && *p != '\'')
        while (*p != 32 && *p != 0)
            p++;
    else {
        qc = *(p++); /* Search for same CLOSE quote */
        while (*p != 0 && *p != qc)
            p++;
    }
    if (*p != 0)
        *p = 0;
    else
        *(p + 1) = 0;
    /* Processing lang tab? */
    if ((t >= 0 && t <= 10) || t == -70) {
        x = actualval(p2, t);
        if (x == -1) {
            /* it was an actual, but wrong type */
            alog(AL_ERROR, "Invalid slot label, '%s', after %s '%s' in verb '%s'", p2,
                 (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c], verb.id);
            return NULL;
        }
        if (x != -2)
            goto write;
    }
    switch (t) {
    case -6: x = onoff(p2); break;
    case -5: x = bvmode(toupper(*p2)); break;
    case -4: x = stat(p2); break;
    case -3: x = spell(p2); break;
    case -2: x = rdmode(toupper(*p2)); break;
    case -1: x = antype(p2); break;
    case PROOM: x = isroom(p2); break;
    case PVERB: x = is_verb(p2); break;
    case PADJ: break;
    case -70:
    case PNOUN: x = isnounh(p2); break;
    case PUMSG: x = ttumsgchk(p2); break;
    case PNUM: x = chknum(p2); break;
    case PRFLAG: x = isrflag(p2); break;
    case POFLAG: x = isoflag1(p2); break;
    case PSFLAG: x = isoflag2(p2); break;
    case PSEX: x = isgen(toupper(*p2)); break;
    case PDAEMON:
        if ((x = is_verb(p2)) == -1 || *p2 != '.')
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
        alog(AL_ERROR, "Invalid parameter '%s' after %s '%s' in %s '%s'", p2,
             (z == 1) ? "condition" : "action", (z == 1) ? conds[c] : acts[c],
             (proc == 1) ? "verb" : "room", (proc == 1) ? (verb.id) : (rmtab + rmn)->id);
        return NULL;
    }
write:
    fwrite((char *)&x, 4, 1, fp);
    FPos += 4; /* Writes a LONG */
    *p = 32;
    return skipspc(p);
}

char *
chkaparms(char *p, int c, FILE *fp)
{
    int i;

    if (nacp[c] == 0)
        return p;
    for (i = 0; i < nacp[c]; i++)
        if ((p = chkp(p, tacp[c][i], c, 0, fp)) == NULL)
            return NULL;
    return p;
}

char *
chkcparms(char *p, int c, FILE *fp)
{
    int i;

    if (ncop[c] == 0)
        return p;
    for (i = 0; i < ncop[c]; i++)
        if ((p = chkp(p, tcop[c][i], c, 1, fp)) == NULL)
            return NULL;
    return p;
}

void
objs_proc()
{
    char *p, *s;
    int   roomno;

    nouns = adjs = 0;

    /* Clear files */
    fopenw(adjfn);
    close_ofps();  // create file
    fopenw(objsfn);
    fopenw(statfn);
    fopenw(objrmsfn);
    fopena(adjfn);

    if (!nextc(false)) {
        return;
    } /* Nothing to process: ///TODO: Warning */
    blkget(&obmem, (char **)&obtab2, 32 * sizeof(obj2));
    objtab2 = obtab2 + 32;
    s = (char *)objtab2;

    do {
        checkErrorCount();

        do
            p = s = extractLine(s, block);
        while (*s != 0 && (block[0] == 0 || com(block)));
        if (*s == 0 || block[0] == 0)
            continue;
        tidy(block);
        if (block[0] == 0)
            continue;
        striplead("noun=", block);
        p = getword(block);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            alog(AL_FATAL, "Invalid object id (length): %s", Word);
            Word[IDL + 1] = 0;
        }
        obj2.adj = obj2.mobile = -1;
        obj2.idno = nouns;
        obj2.state = obj2.nrooms = obj2.contains = obj2.flags = obj2.putto = 0;
        obj2.rmlist = (long *)ftell(ofp3);
        strcpy(obj2.id, Word);

        /* Get the object flags */
        do {
            p = getword(p);
            if (Word[0] == 0)
                continue;
            if ((roomno = isoflag1(Word)) != -1)
                obj2.flags = (obj2.flags | bitset(roomno));
            else {
                if ((roomno = isoparm()) == -1) {
                    alog(AL_ERROR, "Object: %s: Invalid parameter: %s", obj2.id, Word);
                    continue;
                }
                switch (bitset(roomno)) {
                case OP_ADJ: set_adj(); break;
                case OP_START: set_start(); break;
                case OP_HOLDS: set_holds(); break;
                case OP_PUT: set_put(); break;
                case OP_MOB:
                    set_mob();
                    mobs++;
                    break;
                default:
                    alog(AL_FATAL, "Internal Error: Code for object-parameter '%s' missing",
                         obparms[roomno]);
                }
            }
        } while (Word[0] != 0);

        /* Get the room list */

        p = block;
        *p = '+';
        *(p + 1) = 0;
        roomno = 0;
        do {
            p = getword(p);
            if (Word[0] == '+') {
                do
                    s = extractLine(s, block);
                while (*s != 0 && block[0] != 0 && com(block));
                if (*s == 0) {
                    alog(AL_FATAL, "Unexpected end of file in Objects.TXT");
                }
                p = block;
                Word[0] = ' ';
                continue;
            }
            if (Word[0] == 0)
                continue;
            if ((roomno = isloc(Word)) == -1) {
                roomno = -1;
                continue;
            }
            fwrite((char *)&roomno, 1, 4, ofp3);
            obj2.nrooms++;
        } while (Word[0] != 0);
        if (obj2.nrooms == 0 && roomno == 0) {
            alog(AL_ERROR, "Object: %s: no location given", obj2.id);
        }
        obj2.nstates = 0;
        do {
            do
                s = extractLine(s, block);
            while (block[0] != 0 && com(block));
            if (block[0] == 0 || block[0] == '\n')
                break;
            state_proc();
            block[0] = '-';
        } while (block[0] != 0 && block[0] != '\n');
        if (obj2.nstates == 0 || obj2.nstates > 100)
            object("amount of states (i.e. none)");
        if ((long)(obtab2 + (nouns)) > (long)s)
            alog(AL_FATAL, "table exceeded data");
        *(obtab2 + (nouns++)) = obj2;
    } while (*s != 0);
    /*
    close_ofps();
    sort_objs();
    */
    fwrite((char *)obtab2, sizeof(obj2), nouns, ofp1);
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
    int   strip, lines, nvbs, i, ntt, t, r;
    char *p;
    long *l;

    nextc(true); /* Move to first text */
    fopenw(ttfn);
    fopenw(ttpfn);
    fopena(rooms1fn);
    ntt = t = 0;

    do {
    loop1:
        checkErrorCount();
        fgets(block, 1000, ifp);
        if (feof(ifp))
            continue;
        tidy(block);
        if (block[0] == 0 || com(block))
            goto loop1;
        p = block;
        getword(block);
        striplead("room=", Word);
        if ((rmn = isroom(Word)) == -1) {
            alog(AL_ERROR, "No such room: %s", Word);
            skipblock();
            goto loop1;
        }
        if (roomtab->tabptr != -1) {
            alog(AL_ERROR, "Multiple tt entries for room: %s", roomtab->id);
            skipblock();
            goto loop1;
        }
    vbloop:
        do
            fgets(block, 1000, ifp);
        while (block[0] != 0 && com(block));
        if (block[0] == 0 || block[0] == '\n') {
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
        lines = 0;
        verb.id[0] = 0;
        roomtab->tabptr = t;
        roomtab->ttlines = 0;
    vbproc: /* Process verb list */
        nvbs = 0;
        tt.pptr = (long *)-1;
        l = (long *)temp;
        p = block;
        /* Break verb list down to verb no.s */
        do {
            p = getword(p);
            if (Word[0] == 0)
                break;
            if ((*l = is_verb(Word)) == -1) {
                alog(AL_ERROR, "Room: %s: Invalid verb: %s", roomtab->id, Word);
            }
            l++;
            nvbs++;
        } while (Word[0] != 0);
        if (nvbs == 0) {
            alog(AL_FATAL, "Room has empty verb[s]= line: %s", roomtab->id);
        }
        /* Now process each instruction line */
        do {
        xloop:
            strip = 0;
            r = -1;
            block[0] = 0;
            fgets(block, 1000, ifp);
            if (feof(ifp))
                break;
            if (block[0] == 0 || block[0] == '\n') {
                strip = -1;
                continue;
            }
            tidy(block);
            if (block[0] == 0 || com(block))
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
            l = (long *)temp;
            for (i = 0; i < nvbs; i++) {
                if (i < nvbs - 1)
                    tt.pptr = (long *)-2;
                else
                    tt.pptr = (long *)-1;
                tt.verb = *(l++);
                fwrite((char *)&tt.verb, sizeof(tt), 1, ofp1);
                roomtab->ttlines++;
                t++;
                ttents++;
            }
            lines++;
        next:
            strip = 0;
        } while (strip == 0 && !feof(ifp));
        if (strip == 1 && !feof(ifp))
            goto vbproc;
        nextc(false);
        ntt++;
    } while (!feof(ifp));
    if (errorCount == 0 && ntt != rooms && verbose) {
        roomtab = rmtab;
        for (i = 0; i < rooms; i++, roomtab++) {
            if (roomtab->tabptr == -1 && (roomtab->flags & DEATH) != DEATH) {
                alog(AL_WARN, "No TT entry for room: %s", roomtab->id);
            }
        }
    }
    ttroomupdate();
}

void
chae_err()
{
    alog(AL_ERROR, "Verb: %s: Invalid '#CHAE' flag: %s", verb.id, Word);
}

/* From and To */
int
chae_proc(char *f, char *t)
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

    for (i = 0; i < nsynts; i++) {
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
vbprob(char *s, char *s2)
{
    alog(AL_FATAL, "Verb: %s line: '%s': %s", verb.id, s2, s);
}

/* Process LANG.TXT */
void
lang_proc()
{
    char lastc, *p, *p2, *s1, *s2;
    /* n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2) */
    int           n, cs, s, r;
    unsigned long of2p, of3p;

    verbs = 0;
    nextc(true);
    fopenw(lang1fn);
    close_ofps();
    fopena(lang1fn);
    ofp1 = afp;
    afp = NULL;
    fopenw(lang2fn);
    fopenw(lang3fn);
    fopenw(lang4fn);

    blkget(&vbmem, (char **)&vbtab, 64 * (sizeof(verb)));
    vbptr = vbtab + 64;
    s1 = (char *)vbptr;
    vbptr = vbtab;
    of2p = ftell(ofp2);
    of3p = ftell(ofp3);
    FPos = ftell(ofp4);

    do {
        checkErrorCount();
        verbs++;
        p = block;
    loop:
        do {
            s1 = extractLine((s2 = s1), block);
            *(s1 - 1) = 0;
        } while (com(block) && *s1 != 0);
        if (*s1 == 0) {
            verbs--;
            break;
        }
        tidy(block);
        if (block[0] == 0)
            goto loop;
        p = skiplead("verb=", block);
        p = getword(p);
        if (Word[0] == 0) {
            alog(AL_ERROR, "verb= line without a verb?");
            continue;
        }
        if (strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid verb ID (too long): %s", Word);
            do {
                s1 = extractLine((s2 = s1), block);
                *(s1 - 1) = 0;
            } while (*s1 != 0 && block[0] != 0);
            if (*s1 == 0)
                break;
            goto loop;
        }

        strcpy(verb.id, Word);

        p2 = verb.sort;
        *(p2++) = -1;
        *(p2++) = 'C';
        *(p2++) = 'H';
        *(p2++) = 'A';
        *(p2++) = 'E';
        *(p2++) = -1;
        *(p2++) = 'C';
        *(p2++) = 'H';
        *(p2++) = 'A';
        *(p2++) = 'E';

        verb.flags = VB_TRAVEL;
        if (*p == 0 || *p == ';' || *p == '*')
            goto noflags;
        p = getword(p);
        if (strcmp("travel", Word) == 0) {
            verb.flags = 0;
            p = getword(p);
        }
        if (strcmp("dream", Word) == 0) {
            verb.flags += VB_DREAM;
            p = getword(p);
        }
        if (Word[0] == 0 || Word[0] == ';' || Word[0] == '*')
            goto noflags;

        if (chae_proc(Word, verb.sort) == -1)
            goto noflags;
        p = getword(p);
        if (Word[0] != 0 && Word[0] != ';' && Word[0] != '*')
            chae_proc(Word, verb.sort2);

    noflags:
        verb.ents = 0;
        verb.ptr = (struct _SLOTTAB *)of2p;

    stuffloop:
        do {
            s2 = s1;
            s1 = extractLine(s1, block);
            *(s1 - 1) = 0;
        } while (*s1 != 0 && block[0] != 0 && com(block));
        if (*s1 == 0 || block[0] == 0) {
            if (verb.ents == 0 && (verb.flags & VB_TRAVEL))
                alog(AL_WARN, "Verb has no entries: %s", verb.id);
            goto write;
        }

        tidy(block);
        if (block[0] == 0)
            goto stuffloop;

        if ((p = skiplead("syntax=", block)) == block) {
            vbprob("no syntax= line", s2);
            goto stuffloop;
        }

        /* Syntax line loop */
    synloop:
        setslots(WNONE);
        verb.ents++;
        p = skiplead("verb", p);
        p2 = getword(p);
        p2 = skipspc(p2);

        /* If syntax line is 'syntax=verb any' or 'syntax=none' */
        if (*p2 == 0 && strcmp("any", Word) == 0) {
            setslots(WANY);
            goto endsynt;
        }
        if (*p2 == 0 && strcmp("none", Word) == 0) {
            setslots(WNONE);
            goto endsynt;
        }

    sp2: /* Syntax line processing */
        p = skipspc(p);
        if (*p == ';' || *p == '|' || *p == '*')
            goto endsynt;
        Word[0] = 0;
        p = getword(p);
        if (Word[0] == 0)
            goto endsynt;
        if ((n = iswtype(Word)) == -3) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid phrase on syntax line: %s", Word);
            vbprob(tmp, s2);
            goto commands;
        }
        if (Word[0] == 0) {
            s = WANY;
            goto skipeq;
        }

        /* First of all, eliminate illegal combinations */
        if (n == WNONE || n == WANY) { /* you cannot say none=fred any=fred etc */
            char tmp[128];
            snprintf(tmp, "Tried to define %s on syntax line", syntax[n]);
            vbprob(tmp, s2);
            goto endsynt;
        }
        if (n == WPLAYER && strcmp(Word, "me") != 0 && strcmp(Word, "myself") != 0) {
            vbprob("Tried to specify player other than self", s2);
            goto endsynt;
        }

        /* Now check that the 'tag' is the correct type of word */

        s = -1;
        switch (n) {
        case WADJ:
        /* Need ISADJ() - do TT entry too */
        case WNOUN: s = isnoun(Word); break;
        case WPREP: s = isprep(Word); break;
        case WPLAYER:
            if (strcmp(Word, "me") == 0 || strcmp(Word, "myself") == 0)
                s = -3;
            break;
        case WROOM: s = isroom(Word); break;
        case WSYN:
            alog(AL_WARN, "Synonyms not supported yet");
            s = WANY;
            break;
        case WTEXT: s = chkumsg(Word); break;
        case WVERB: s = is_verb(Word); break;
        case WCLASS: s = WANY;
        case WNUMBER:
            if (Word[0] == '-')
                s = atoi(Word + 1);
            else
                s = atoi(Word);
        default: alog(AL_ERROR, "Internal Error: Invalid w-type");
        }

        if (n == WNUMBER && (s > 100000 || -s > 100000)) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "Invalid number: %d", s);
            vbprob(tmp, s2);
        }
        if (s == -1 && n != WNUMBER) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "%s=: Invalid setting: %s", syntax[n + 1], Word);
            vbprob(tmp, s2);
        }
        if (s == -3 && n == WNOUN)
            s = -1;

    skipeq: /* (Skipped the equals signs) */
        /* Now fit this into the correct slot */
        cs = 1; /* Noun1 */
        switch (n) {
        case WADJ:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                vbprob("Invalid NOUN NOUN ADJ combination", s2);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("Invalid NOUN ADJ NOUN ADJ combination", s2);
                n = -5;
                break;
            }
            if (vbslot.wtype[0] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("More than two adjectives on a line", s2);
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
                vbprob("Invalid noun arrangement", s2);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE)
                cs = 4;
            break;
        case WPREP:
            if (vbslot.wtype[2] != WNONE) {
                vbprob("Invalid PREP arrangement", s2);
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
                snprintf(block, "No free noun slot for %s entry", syntax[n + 1]);
                vbprob(tmp, s2);
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

        do {
            s2 = s1;
            s1 = extractLine(s1, block);
            *(s1 - 1) = 0;
        } while (*s1 != 0 && block[0] != 0 && com(block));
        if (block[0] == 0 || *s1 == 0) {
            lastc = 1;
            goto writeslot;
        }
        tidy(block);
        if ((p = skiplead("syntax=", block)) != block) {
            lastc = 0;
            goto writeslot;
        }
        if (*p == 0)
            goto commands;

        vbslot.ents++;
        r = -1;
        vt.pptr = (long *)FPos;

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
            strcpy(Word, Word + 1);
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
                snprintf(tmp, "Invalid condition, '%s'", Word);
                vbprob(tmp, s2);
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
                vbprob("Missing action", s2);
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
            vbprob(tmp, s2);
            goto commands;
        }
    doaction:
        p = skipspc(p);
        if ((p = chkaparms(p, vt.action, ofp4)) == NULL) {
            goto commands;
        }
        vt.action = 0 - (vt.action + 1);

    writecna: /* Write the C & A lines */
        fwrite((char *)&vt.condition, sizeof(vt), 1, ofp3);
        proc = 0;
        of3p += sizeof(vt);
        goto commands;

    writeslot:
        fwrite(vbslot.wtype, sizeof(vbslot), 1, ofp2);
        proc = 0;
        of2p += sizeof(vbslot);
        if (lastc > 1)
            goto commands;
        if (lastc == 0)
            goto synloop;

        lastc = '\n';
    write:
        fwrite(verb.id, sizeof(verb), 1, ofp1);
        proc = 0;
        *(vbtab + (verbs - 1)) = verb;
        if ((long)(vbtab + (verbs - 1)) > (long)s1)
            alog(AL_FATAL, "table overtaking s1");
    } while (*s1 != 0);
}

void
umsg_proc()
{
    char *s;

    umsgs = 0;
    fopenw("-ram:umsg.tmp");
    close_ofps();
    fopena(umsgifn);
    ofp1 = afp;
    afp = NULL;
    fseek(ofp1, 0, 2L);
    fopena(umsgfn);
    ofp2 = afp;
    afp = NULL;
    fseek(ofp2, 0, 2L);
    fopena("-ram:umsg.tmp");
    if (!nextc(false)) {
        /// TODO: Tell the user
        return;
    } /* None to process */
    blkget(&datal, &data, 0L);
    s = data;

    do {
    loop:
        do
            s = extractLine(s, block);
        while (com(block) && *s != 0);
        if (*s == 0)
            break;
        tidy(block);
        if (block[0] == 0)
            goto loop;
        striplead("msgid=", block);
        getword(block);
        if (Word[0] == 0)
            goto loop;

        if (Word[0] == '$') {
            alog(AL_ERROR, "Invalid message ID: %s ('$' reserved or system messages", Word);
            skipblock();
            goto loop;
        }
        if (strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid message ID (too long): %s", Word);
            skipblock();
            goto loop;
        }
        umsgs++; /* Now copy the text across */
        strcpy(umsg.id, Word);
        umsg.fpos = ftell(ofp2);
        fwrite(umsg.id, sizeof(umsg), 1, afp);
        fwrite((char *)&umsg.fpos, 4, 1, ofp1);
        do {
            while (*s != 0 && com(s))
                s = skipline(s);
            if (*s == 0 || *s == 13) {
                *s = 0;
                break;
            }
            if (*s == 9)
                s++;
            if (*s == 13) {
                block[0] = 13;
                continue;
            }
            s = extractLine(s, block);
            if (block[0] == 0)
                break;
            umsg.fpos = strlen(block);
            if (block[umsg.fpos - 1] == '{')
                block[--umsg.fpos] = 0;
            else
                strcat(block + (umsg.fpos++) - 1, "\n");
            fwrite(block, 1, umsg.fpos, ofp2);
        } while (*s != 0 && block[0] != 0);
        fputc(0, ofp2);
    } while (*s != 0);
    FreeMem(data, datal);
    data = NULL;
    datal = NULL;
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

void
smsg_proc()
{
    char *s;
    long  id, pos;
    smsgs = 0;

    if (!nextc(false))
        return; /* Nothing to process! */
    fopenw(umsgifn);
    fopenw(umsgfn); /* Text and index */

    blkget(&datal, &data, 0L);
    s = data;

    do {
        checkErrorCount();

        do {
            s = extractLine(s, block);
        } while (isCommentChar(block[0]));
        if (block[0] == 0)
            continue;

        tidy(block);
        if (block[0] == 0)
            continue;

        getWordAfter("msgid=", block);
        if (Word[0] == 0)
            break;

        bool valid = true;

        if (Word[0] != '$') {
            alog(AL_ERROR, "Invalid system message id: %s (must begin with '$'", Word);
            valid = false;
        } else if (atoi(Word + 1) != smsgs + 1) {
            alog(AL_ERROR, "Message %s out of sequence", Word);
            valid = false;
        } else if (smsgs >= NSMSGS) {
            alog(AL_FATAL, "Unexpected system message (last should be %d)", NSMSGS);
        }

        id = ++smsgs; /* Now copy the text across */

        pos = ftell(ofp2);
        fwrite((char *)&pos, 4, 1, ofp1);

        do {
            while (com(s))
                s = skipline(s);
            if (isLineEnding(*s))
                break;
            if (*s == '\t')  // expected but optional indent
                s++;
            s = extractLine(s, block);
            if (block[0] == 0)
                continue;
            pos = strlen(block);
            if (block[pos - 1] == '{')
                block[--pos] = 0;
            else
                strcat(block + (pos++) - 1, "\n");
            fwrite(block, 1, pos, ofp2);
        } while (*s != 0);
        fputc(0, ofp2);
    } while (*s != 0);
    FreeMem(data, datal);
    data = NULL;
    datal = NULL;
}

/*
    Routines to process/handle Synonyms
                        */

void
syn_proc()
{
    char *    s, *t;
    short int no;
    short int x;

    syns = 0;
    if (!nextc(false))
        return;
    fopenw(synsfn);
    fopenw(synsifn);

    blkget(&datal, &data, 0L);
    s = data;

    do {
        do
            s = extractLine(s, block);
        while (com(block));

        tidy(block);
        if (block[0] == 0)
            continue;
        t = getword(block);
        t = skipspc(t);

        if ((no = isnoun(Word)) < 0) {
            if ((x = is_verb(Word)) == -1) {
                alog(AL_ERROR, "Invalid verb/noun: %s", Word);
                continue;
            }
            no = -(2 + x);
        }

        while (*t != 0) {
            t = getword(t);
            if (Word[0] == 0)
                break;
            fwrite((char *)&no, 1, sizeof(short int), ofp2);
            fprintf(ofp1, "%s%c", Word, 0);
            syns++;
        }
    } while (*s != 0);
    FreeMem(data, datal);
    data = NULL;
    datal = NULL;
}

/* Mobiles.Txt Processor */

/* Pass 1: Indexes mobile names */
void
mobmis(char *s)
{
    alog(AL_ERROR, "Mobile: %s: missing field: %s", mob.id, s);
    skipblock();
}

int
badmobend()
{
    return -1;
}

/* Fetch mobile message line */
int
getmobmsg(char *s)
{
    char *q;
    int   n;

loop:
    if (com(px)) {
        px = skipline(px);
        goto loop;
    }
    if (*px == 0 || *px == 13 || *px == 10) {
        alog(AL_ERROR, "Mobile: %s: unexpected end of definition", s);
        return -1;
    }
    px = skipspc(px);
    if (*px == 0 || *px == 13 || *px == 10)
        goto loop;

    if ((q = skiplead(s, px)) == px) {
        mobmis(s);
        return -1;
    }
    if (toupper(*q) == 'N') {
        px = skipline(px);
        return -2;
    }
    n = ttumsgchk(q);
    px = skipline(px);
    if (n == -1) {
        alog(AL_ERROR, "Mobile: %s: Invalid '%s' line", mob.id, s);
    }
    return n;
}

/* Pass 2: Indexes commands mobiles have access to */
void
mob_proc1()
{
    char *p, *s1, *s2;
    long  n;

    mobchars = 0;
    fopenw(mobfn);
    if (!nextc(false))
        return;

    blkget(&moblen, &mobdat, 0L);
    p = mobdat;
    repspc(mobdat);

    do {
        while (*p != 0 && *p != '!')
            p = skipline(p);
        if (*p == 0)
            break;
        p = extractLine(p, block);
        mobchars++;
        s1 = getword(block + 1);
        strcpy(mob.id, Word);
        do {
            s1 = skipspc(s1);
            if (*s1 == 0 || *s1 == ';')
                break;
            if ((s2 = skiplead("dead=", s1)) != s1) {
                s1 = getword(s2);
                mob.dead = atoi(Word);
                continue;
            }
            if ((s2 = skiplead("dmove=", s1)) != s1) {
                s1 = getword(s2);
                mob.dmove = isroom(Word);
                if (mob.dmove == -1) {
                    alog(AL_ERROR, "Mobile: %s: invalid dmove: %s", mob.id, Word);
                }
                continue;
            }
        } while (*s1 != 0 && *s1 != ';' && Word[0] != 0);

        p = extractLine(p, block);
        tidy(block);
        s1 = block;
        mob.dmove = -1;

        if ((s2 = skiplead("speed=", s1)) == s1) {
            mobmis("speed=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.speed = atoi(Word);
        if ((s2 = skiplead("travel=", s1)) == s1) {
            mobmis("travel=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.travel = atoi(Word);
        if ((s2 = skiplead("fight=", s1)) == s1) {
            mobmis("speed=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.fight = atoi(Word);
        if ((s2 = skiplead("act=", s1)) == s1) {
            mobmis("act=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.act = atoi(Word);
        if ((s2 = skiplead("wait=", s1)) == s1) {
            mobmis("wait=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.wait = atoi(Word);
        if (mob.travel + mob.fight + mob.act + mob.wait != 100) {
            alog(AL_ERROR, "Mobile: %s: Travel+Fight+Act+Wait values not equal to 100%.", mob.id);
        }
        if ((s2 = skiplead("fear=", s1)) == s1) {
            mobmis("fear=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.fear = atoi(Word);
        if ((s2 = skiplead("attack=", s1)) == s1) {
            mobmis("attack=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.attack = atoi(Word);
        if ((s2 = skiplead("hitpower=", s1)) == s1) {
            mobmis("hitpower=");
            continue;
        }
        s1 = getword(s2);
        s1 = skipspc(s1);
        mob.hitpower = atoi(Word);

        px = p;
        if ((n = getmobmsg("arrive=")) == -1)
            continue;
        mob.arr = n;
        if ((n = getmobmsg("depart=")) == -1)
            continue;
        mob.dep = n;
        if ((n = getmobmsg("flee=")) == -1)
            continue;
        mob.flee = n;
        if ((n = getmobmsg("strike=")) == -1)
            continue;
        mob.hit = n;
        if ((n = getmobmsg("miss=")) == -1)
            continue;
        mob.miss = n;
        if ((n = getmobmsg("dies=")) == -1)
            continue;
        mob.death = n;
        p = px;

        fwrite(mob.id, sizeof(mob), 1, ofp1);
    } while (*p != 0);

    if (mobchars != 0) {
        if ((mobp = (struct _MOB_ENT *)AllocMem(sizeof(mob) * mobchars, MEMF_PUBLIC)) == NULL) {
            alog(AL_FATAL, "Out of memory");
        }
        close_ofps();
        fopena(mobfn);
        fread((char *)mobp, sizeof(mob) * mobchars, 1, afp);
    }
}
/*mob_proc2()
{*/

void
checkf(char *s)
{
    sprintf(block, "%s%s", dir, s);
    if ((ifp = fopen(block, "rb")) == NULL) {
        alog(AL_FATAL, "Missing file: %s", block);
    }
    fclose(ifp);
    ifp = NULL;
}

void
argue(int argc, const char **argv)
{
    int n;
    for (n = 2; n <= argc; n++) {
        if (strcmp("-d", argv[n - 1]) == 0) {
            checkDmoves = true;
            continue;
        }
        if (strcmp("-q", argv[n - 1]) == 0) {
            verbose = true;
            continue;
        }
        if (strcmp("-r", argv[n - 1]) == 0) {
            reuseRoomData = true;
            continue;
        }
        strcpy(dir, argv[n - 1]);
        char c = dir[strlen(dir) - 1];
        if (c != '/' && c != ':')
            strcat(dir, "/");
    }
}

/*---------------------------------------------------------*/

static void
checkFilesExist()
{
    alog(AL_INFO, "Checking for files");
    checkf("Title.TXT");
    checkf("Rooms.TXT");
    checkf("Ranks.TXT");
    checkf("Obdescs.TXT");
    checkf("Objects.TXT");
    checkf("Lang.TXT");
    checkf("Travel.TXT");
    checkf("SysMsg.TXT");
    checkf("UMsg.TXT");
    checkf("Reset.TXT");
    checkf("Syns.TXT");
    checkf("Mobiles.TXT");
    alog(AL_INFO, "All files found");
}

void
compileSection(const char *name, bool isText, void (*procFn)())
{
    alog(AL_NOTE, "Compiling: %s", name);
    if (isText)
        opentxt(name);
    procFn();
    if (ifp) {
        fclose(ifp);
        ifp = NULL;
    }
    close_ofps();
    if (errorCount > 0) {
        alog(AL_FATAL, "Terminating due to errors.");
    }
}

void
compileGame()
{
    // title section defines the core game parameters, so we check it first
    compileSection("title", true, title_proc);

    // next check the system message file because this probably won't change
    // often and so should be fairly fire-and-forget
    compileSection("sysmsg", true, smsg_proc);
    if (smsgs != NSMSGS) {
        alog(AL_FATAL, "%d system message(s) missing", NSMSGS - smsgs);
    }

    // Next, rooms, they're kind of fundamental
    if (!reuseRoomData) {
        compileSection("rooms", true, room_proc);
    }
    fopenr(rooms1fn); /* Check DMOVE ptrs */
    if (reuseRoomData) {
        fseek(ifp, 0, SEEK_END);
        rooms = ftell(ifp) / sizeof(*rmtab);
        fseek(ifp, 0, SEEK_SET);
        rewind(ifp);
    }
    if ((rmtab = (struct _ROOM_STRUCT *)AllocMem(sizeof(room) * rooms, MEMF_PUBLIC)) == NULL) {
        alog(AL_FATAL, "Out of memory for room id table");
    }
    int roomsInFile = fread(rmtab, sizeof(*rmtab), rooms, ifp);
    if (roomsInFile != rooms) {
        alog(AL_FATAL, "Roomtable appears to be corrupted. Recompile.");
    }
    if (checkDmoves && dmoves != 0) {
        compileSection("DMOVEs", false, checkdmoves);
    }

    // list of available player ranks
    compileSection("ranks", true, rank_proc);

    // user-defined messages next so that we can reference them in
    // subsequent files.
    compileSection("umsg", true, umsg_proc);

    // descriptions of npc classes (aka mobiles) needs to come
    // next so we can imbue objects with them later
    compileSection("mobiles", true, mob_proc1);

    // long and shared object descriptions
    compileSection("obdescs", true, obds_proc);

    // the actual object definitions
    compileSection("objects", true, objs_proc);

    // now we can declare language structure, esp verbs
    compileSection("lang", true, lang_proc);
    proc = 0;  // done processing language

    // compile the links between rooms
    compileSection("travel", true, trav_proc);

    // define synonyms (aliases)
    compileSection("syns", true, syn_proc);
}

/*---------------------------------------------------------*/

int
main(int argc, const char **argv)
{
    sprintf(vername, "AMULCom v%d.%03d (%8s)", VERSION, REVISION, DATE);
    mytask = FindTask(0L);
    mytask->tc_Node.ln_Name = vername;

    alog(AL_INFO, "  AMUL  Multi-User Games Language Copyright (C) KingFisher Software, 1991");
    alog(AL_INFO, "                 AMUL Compiler; %s", vername);

    ofp1 = NULL;
    ofp2 = NULL;
    ofp3 = NULL;
    dir[0] = 0;

#if defined(_AMIGA_)
    ohd = Output();
#endif

    /* Check we have correct no. of parameters */

    if (argc > 6) {
        alog(AL_ERROR, "Usage: amulcom <game path>");
        exit(0);
    }
    if (argc > 1)
        argue(argc, argv);
    if (verbose) {
        alogLevel(AL_NOTE);
    }

    /* Check the files/directories */

    checkFilesExist();

    compileGame();
    alog(AL_NOTE, "Execution finished normally");
    alog(AL_INFO, "Statistics for %s:", adname);
    alog(AL_INFO, "		Rooms: %6d	Ranks: %6d	Nouns: %6d", rooms, ranks, nouns);
    alog(AL_INFO, "		Adj's: %6d	Verbs: %6d	Syns : %6d", adjs, verbs, syns);
    alog(AL_INFO, "		T.T's: %6d	Umsgs: %6d	SMsgs: %6d", ttents, umsgs, NSMSGS);
    alog(AL_INFO, "		 Total items processed:%7d",
         rooms + ranks + adjs + verbs + nouns + syns + ttents + umsgs + NSMSGS + mobs + mobchars);
    fopenw(advfn);
    time(&startTime);
    fprintf(ofp1, "%s\n%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", adname,
            rooms, ranks, verbs, syns, nouns, adjs, ttents, umsgs, startTime, mins, invis, invis2,
            minsgo, mobs, rscale, tscale, mobchars);

    // Copy the text from title.txt into the profile file
    opentxt("TITLE");
    fseek(ifp, titlePos, 0);
    for (;;) {
        int bytes = fread(block, 1, sizeof(block), ifp);
        if (bytes <= 0)
            break;
        bytes = fwrite(block, bytes, 1, ofp1);
        if (bytes != 1)
            alog(AL_FATAL, "Error writing title text");
    }

    exiting = true;

    quit();
}
