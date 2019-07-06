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

#include <ctime>
#include <fstream>
#include <string>

#include "amulcom.includes.h"
#include "context.h"
#include "parsers/parsers.h"

#include "h/amul.cons.h"  // Predefined Constants etc
#include "h/amul.vars.h"  // all INTERNAL variables

using namespace AMUL::Logging;
using namespace Compiler;

// Compiler specific variables...

int     dmoves;               // How many RF_CEMETERYs to check?
int     rmn;                  // Current room no.
int32_t FPos;                 // Used during TT/Lang writes
int     proc;                 // What we are processing
char *  data;                 // Pointer to data buffer
char *  data2;                // Secondary buffer area
char *  syntab;               // Synonym table, re-read
int32_t datal, datal2, mins;  // Length of data! & gametime
int32_t obmem;                // Size of Objects.TXT
int32_t vbmem;                // Size of Lang.Txt
int32_t wizstr;               // Wizards strength
char *  mobdat;               // Mobile data
int32_t moblen;               // Length

char Word[WORD_LEN];

extern std::string titleText;

char  fnm[150], was[128];
FILE *ofp5;

struct _OBJ_STRUCT2 *obtab2, *objtab2, obj2, *osrch, *osrch2;

void
CXBRK()
{
    printf("Ctrl-C Pressed: Terminating\n");
    quit();
}

//---------------------------------------------------------

static void
parseArguments(int argc, const char *argv[])
{
    if (argc > 6) {
        GetLogger().errorf("Usage:\n  %s [-d] [-q] [-r] [<game path>]\n", argv[0]);
        exit(0);
    }
    for (int n = 1; n < argc; n++) {
        if (strcmp("-d", argv[n]) == 0) {
            GetContext().m_skipDmoveCheck = true;
            continue;
        }
        if (strcmp("-q", argv[n]) == 0) {
            GetLogger().m_quiet = false;
            continue;
        }
        if (strcmp("-r", argv[n]) == 0) {
            GetContext().m_skipRoomRead = true;
            continue;
        }
        strcpy(dir, argv[n]);
        if (char c = dir[strlen(dir) - 1]; c != '/' && c != ':')
            strcat(dir, "/");
    }
}

//---------------------------------------------------------

int
isroom(const char *s)
{
    roomtab = rmtab;
    for (int r = 0; r < rooms; r++, roomtab++) {
        if (strcmp(roomtab->id, s) == 0)
            return r;
    }
    return -1;
}

void
set_adj()
{
    if (strlen(Word) > IDL || strlen(Word) < 3) {
        GetLogger().fatalf("Invalid adjective: %s", Word);
    }
    if (adjs == 0) {
        memset(dmove, 0, sizeof(dmove));
        strcpy(dmove, Word);
        obj2.adj = 0;

        fwrite(dmove, IDL + 1, 1, afp);

        adjs++;

        return;
    }

    fseek(afp, 0L, 0);  // Move to beginning
    for (int i = 0; !feof(afp); ++i) {
        if (fread(dmove, IDL + 1, 1, afp) != 1)
            continue;  // Read adj!
        if (strcmp(Word, dmove) == NULL) {
            obj2.adj = i;
            return;
        }
    }
    memset(dmove, 0, sizeof(dmove));
    strcpy(dmove, Word);
    fseek(afp, 0L, 2);               // Move to end!
    fwrite(dmove, IDL + 1, 1, afp);  // Add this adjective
    obj2.adj = adjs++;
}

void
object(const char *s)
{
    GetLogger().fatalf("Object #%d '%s' has invalid %s: %s", nouns + 1, obj2.id, s, Word);
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
    for (int i = 0; i < NPUTS; i++) {
        if (stricmp(obputs[i], Word) == NULL) {
            obj2.putto = i;
            return;
        }
    }
    object("put= flag");
}

void
set_mob()
{
    for (int i = 0; i < mobchars; i++) {
        if (stricmp(Word, (mobp + i)->id) == NULL) {
            obj2.mobile = i;
            return;
        }
    }
    object("mobile= flag");
}

static void
checkf(const char *s)
{
    sprintf(block, "%s%s", dir, s);
    if (ifp = fopen(block, "rb"); !ifp) {
        GetLogger().fatalf("Missing file: %s", block);
    }
    fclose(ifp);
    ifp = NULL;
}

void
checkFilePresence()
{
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
}

void
compileGame()
{
    GetLogger().info("-- title");
    opentxt("TITLE");
    title_proc();
    fclose(ifp);
    dmoves = 0;

    // Now load the rooms data.
    if (!GetContext().m_skipRoomRead) {
        GetLogger().info("-- rooms");
        opentxt("ROOMS");
        room_proc();
        fclose(ifp);
    }
    fopenr(Resources::Compiled::roomData());  // Check RF_CEMETERY ptrs
    if (GetContext().m_skipRoomRead) {
        fseek(ifp, 0, 2L);
        rooms = ftell(ifp) / sizeof(room);
        rewind(ifp);
    }
    if ((rmtab = (_ROOM_STRUCT *)OS::Allocate(sizeof(room) * rooms)) == NULL) {
        GetLogger().fatal("Out of memory for room id table");
    }
    if (fread((char *)rmtab, sizeof(room), rooms, ifp) != rooms) {
        GetLogger().fatal("Invalid rooms file");
    }
    if (!GetContext().m_skipDmoveCheck || dmoves != 0) {
        GetLogger().info("-- cemetery check");
        checkdmoves();
        fclose(ifp);
    }
    GetLogger().info("-- ranks");
    opentxt("RANKS");
    rank_proc();
    fclose(ifp);

    GetLogger().info("-- system messages");
    opentxt("SysMsg");
    smsg_proc();
    fclose(ifp);
    if (smsgs != NSMSGS) {
        GetLogger().fatalf("%u system message(s) missing", NSMSGS - smsgs);
    }

    GetLogger().info("-- user messages");
    opentxt("UMSG");
    if (!umsg_proc())
        quit();
    else
        fclose(ifp);

    GetLogger().info("-- mobiles");
    opentxt("MOBILES");
    mob_proc1();
    fclose(ifp);

    GetLogger().info("-- object descriptions");
    opentxt("OBDESCS");
    obds_proc();
    fclose(ifp);

    GetLogger().info("-- objects");
    opentxt("OBJECTS");
    objs_proc();
    fclose(ifp);

    GetLogger().info("-- language table");
    opentxt("LANG");
    lang_proc();
    fclose(ifp);

    proc = 0;
    GetLogger().info("-- travel table");
    opentxt("TRAVEL");
    trav_proc();
    fclose(ifp);

    GetLogger().info("-- synonyms");
    opentxt("SYNS");
    syn_proc();
    fclose(ifp);
}

void
reportStatistics()
{
    GetLogger().infof("Statistics for %s:\n\n", adname);
    GetLogger().infof("		Rooms: %6u	Ranks: %6u	Nouns: %6u", rooms, ranks, nouns);
    GetLogger().infof("		Adj's: %6u	Verbs: %6u	Syns : %6u", adjs, verbs, syns);
    GetLogger().infof("		T.T's: %6u	Umsgs: %6u	SMsgs: %6u", ttents, umsgs, NSMSGS);
    GetLogger().infof("		Total items processed: %u",
            rooms + ranks + adjs + verbs + nouns + syns + ttents + umsgs + NSMSGS + mobs + mobchars);
}

void
writeProfile()
{
    opentxt("TITLE");
    {
        std::ofstream profile(Resources::Compiled::gameProfile());
        profile << adname << "\n";
        profile << rooms << ranks << verbs << syns << nouns << adjs << ttents << umsgs << time(nullptr) << mins << invis
                << invis2 << minsgo << mobs << rscale << tscale << mobchars;
        profile << "\n";
        profile << titleText;
    }
}

int
main(int argc, const char *argv[])
{
    // capture game compile time
    sprintf(vername, "amulcom v%d.%03d (%8s)", VERSION, REVISION, DATE);
    OS::SetProcessName(vername);

    puts("AMUL  Multi-User Games Language Copyright (C) KingFisher Software, 1991-2019\n");
    printf("AMUL Compiler: %s\n\n", vername);

    // Parse commandline
    parseArguments(argc, argv);

    // Check the files/directories
    GetLogger().info("-- Checking existence of files:");
    checkFilePresence();

    GetLogger().info("-- Compiling game files:");
    compileGame();
    GetContext().m_completed = true;

    writeProfile();
    GetLogger().info("++ Compilation Successful");

    if (!GetLogger().m_quiet) {
        reportStatistics();
    }

    quit();
}

void
checkdmoves()
{
    int           n;
    _ROOM_STRUCT *roomptr;

    // Check RF_CEMETERY ptrs
    fopenr(Resources::Compiled::roomDesc());  // Open desc. file
    roomptr = rmtab;
    for (n = 0; n < rooms; n++) {
        if (roomptr->flags & RF_CEMETERY) {
            printf("%-9s\x0d", roomptr->id);
            fseek(ifp, roomptr->desptr, 0);
            fread(dmove, IDL, 1, ifp);  // Read the RF_CEMETERY name
            if (isroom(dmove) == -1) {  // Is it a valid room?
                GetLogger().errorf("%-9s - invalid dmove: %s", roomptr->id, dmove);
            }
        }
        roomptr++;
    }
    GetContext().terminateOnErrors();
}

int
isprep(const char *s)
{
    for (int i = 0; i < NPREP; i++) {
        if (strcmp(s, prep[i]) == 0)
            return i;
    }
    return -1;
}
