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

#include "amulcom.includes.h"
#include "context.h"
#include "parsers/parsers.h"

#include "h/amul.cons.h"  // Predefined Constants etc
#include "h/amul.vars.h"  // all INTERNAL variables

using namespace AMUL::Logging;
using namespace Compiler;

// Compiler specific variables...

int		dmoves;				  // How many RF_CEMETERYs to check?
int		rmn;				  // Current room no.
int32_t readgot, FPos;		  // Used during TT/Lang writes
int		proc;				  // What we are processing
char *  data;				  // Pointer to data buffer
char *  data2;				  // Secondary buffer area
char *  syntab;				  // Synonym table, re-read
char	idt[IDL + 1];		  // Temporary ID store
int32_t datal, datal2, mins;  // Length of data! & gametime
int32_t obmem;				  // Size of Objects.TXT
int32_t vbmem;				  // Size of Lang.Txt
int32_t invis, invis2;		  // Invisibility Stuff
int32_t wizstr;				  // Wizards strength
char *  mobdat, *px;		  // Mobile data
int32_t moblen;				  // Length

Amiga::Task *mytask;

struct UMSG
{
	char	id[IDL + 1];
	int32_t fpos;
} umsg;

const char *skipspc(const char *);

char  fnm[150], was[128], *getword(char *), *sgetl(char *, char *), *skiplead(char *, char *);
char *skipline(char *);
FILE *ofp5;

struct _OBJ_STRUCT2 *obtab2, *objtab2, obj2, *osrch, *osrch2;

void
CXBRK()
{
	tx("\n[42m** Break pressed - memory released, files closed! "
	   "Tata!\n\n[0m");
	quit();
}

//---------------------------------------------------------

static void
parseArguments(int argc, const char *argv[])
{
	if (argc > 6) {
		puts("!! Error !!\n\n Usage:\n   amulcom <game "
			 "path>\n");
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
		printf("\nInvalid adjective '%s'!\n\n\x07", Word);
		quit();
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
	fseek(afp, 0L, 2);				 // Move to end!
	fwrite(dmove, IDL + 1, 1, afp);  // Add this adjective
	obj2.adj = adjs++;
}

void
object(char *s)
{
	printf("\nObject #%d \"%s\" has invalid %s, '%s'!\n", nouns + 1, obj2.id, s, Word);
	quit();
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

void
checkf(char *s)
{
	sprintf(block, "%s%s", dir, s);
	if ((ifp = fopen(block, "rb")) == NULL) {
		printf("Missing: file \x1B[33;1m%s!!!\x1B[0m\n\n", block);
		exit(202);
	}
	fclose(ifp);
	ifp = NULL;
}

int
main(int argc, const char *argv[])
{
	sprintf(vername, "AMULCom v%d.%03d (%8s)", VERSION, REVISION, DATE);
	mytask = Amiga::FindTask(0L);  // find this process
	mytask->tc_Node.ln_Name = vername;

	puts("\x0C\n  [33mAMUL  Multi-User Games Language Copyright "
		 "(C) KingFisher "
		 "Software, 1991[0m\n");
	printf("                 [4mAMUL Compiler; %s[0m\n\n", vername);

	ofp1 = NULL;
	ofp2 = NULL;
	ofp3 = NULL;
	dir[0] = 0;

	// Check we have correct no. of parameters

	parseArguments(argc, argv);

	// Check the files/directories

	tx("\n%% Checking existence of files\x0d");
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
	tx("## All .TXT files located.    \n\n\x1B[1mCompiling game "
	   "files...\n\x1B[0m");

	tx("%% TITLE....:\x0d");
	opentxt("TITLE");
	title_proc();
	fclose(ifp);
	dmoves = 0;

	// Now load the rooms data.
	if (!GetContext().m_skipRoomRead) {
		tx("%% ROOMS....:\x0d");
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
	if ((rmtab = (_ROOM_STRUCT*)OS::Allocate(sizeof(room) * rooms)) == NULL) {
		puts("No memory for ROOM ID table!\n");
		quit();
	}
	if (fread((char *)rmtab, sizeof(room), rooms, ifp) != rooms) {
		puts("Failed to get right number of rooms from "
			 "files! Aborting!");
		quit();
	}
	if (!GetContext().m_skipDmoveCheck || dmoves != 0) {
		tx("%% RF_CEMETERYs...:\x0d");
		checkdmoves();
		fclose(ifp);
	}
	tx("%% RANKS....:\x0d");
	opentxt("RANKS");
	rank_proc();
	fclose(ifp);
	tx("%% SYSMSG...:\x0d");
	opentxt("SysMsg");
	smsg_proc();
	fclose(ifp);
	if (smsgs != NSMSGS) {
		printf("\x07!! %ld System message(s) missing!\n\n", NSMSGS - smsgs);
		quit();
	}
	tx("%% UMSG.....:\x0d");
	opentxt("UMSG");
	if (!umsg_proc())
		quit();
	else
		fclose(ifp);
	tx("%% MOBILES..:\x0d");
	opentxt("MOBILES");
	mob_proc1();
	fclose(ifp);
	tx("%% OBDESCS..:\x0d");
	opentxt("OBDESCS");
	obds_proc();
	fclose(ifp);
	tx("%% OBJECTS..:\x0d");
	opentxt("OBJECTS");
	objs_proc();
	fclose(ifp);
	tx("%% LANG.....:\x0d");
	opentxt("LANG");
	lang_proc();
	fclose(ifp);
	proc = 0;
	tx("%% TRAVEL...:\x0d");
	opentxt("TRAVEL");
	trav_proc();
	fclose(ifp);
	tx("%% SYNS.....:\x0d");
	opentxt("SYNS");
	syn_proc();
	fclose(ifp);

	tx("Execution finished normally\n\n");
	printf("Statistics for %s:\n\n", adname);
	printf("		Rooms: %6d	Ranks: %6d	Nouns: %6d\n", rooms, ranks, nouns);
	printf("		Adj's: %6d	Verbs: %6d	Syns : %6d\n", adjs, verbs, syns);
	printf("		T.T's: %6d	Umsgs: %6d	SMsgs: %6d\n", ttents, umsgs, NSMSGS);
	printf("		 [33mTotal items processed:[0;1m%7d\n\n[0m",
		   rooms + ranks + adjs + verbs + nouns + syns + ttents + umsgs + NSMSGS + mobs + mobchars);
	fopenw(Resources::Compiled::gameProfile());
	fprintf(ofp1,
			"%s\n%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld "
			"%ld %ld %ld %ld %ld",
			adname, rooms, ranks, verbs, syns, nouns, adjs, ttents, umsgs, clock, mins, invis,
			invis2, minsgo, mobs, rscale, tscale, mobchars);
	opentxt("TITLE");
	fseek(ifp, readgot, 0);
	do {
		block[0] = 0;
		readgot = fread(block, 1, 1020, ifp);
		fwrite(block, readgot, 1, ofp1);
	} while (readgot == 1020);

	GetContext().m_completed = true;

	quit();
}

void
checkdmoves()
{
	int			  n;
	_ROOM_STRUCT *roomptr;

	// Check RF_CEMETERY ptrs
	fopenr(Resources::Compiled::roomDesc());  // Open desc. file
	roomptr = rmtab;
	for (n = 0; n < rooms; n++) {
		if (roomptr->flags & RF_CEMETERY) {
			sprintf(block, "%-9s\x0d", roomptr->id);
			tx(block);
			fseek(ifp, roomptr->desptr, 0);
			fread(dmove, IDL, 1, ifp);  // Read the RF_CEMETERY name
			if (isroom(dmove) == -1)	// Is it a valid room?
			{
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
