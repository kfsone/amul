/*
		  ####         ###     ### ##     ## ####
		 ##  ##         ###   ###  ##     ##  ##            Amiga
		##    ##        #########  ##     ##  ##            Multi
		##    ##        #########  ##     ##  ##            User
		########  ----  ## ### ##  ##     ##  ##            adventure
		##    ##        ##     ##  ##     ##  ##            Language
	   ####  ####      ####   ####  #######  #########


			  ****       AMUL.C.......Adventure System      ****
			  ****               Main Program!              ****

	Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.
*/

#include "amulinc.h"
#include "h/amigastubs.h"
#include "h/amul.cons.h"
#include "h/amul.h"
#include "h/os.h"

static inline void
init()
{
	sprintf(vername, "AMUL v%d.%d (%s)", VERSION, REVISION, DATE);
	lverb = -1;
	iverb = -1;
	ip = 1;
	needcr = NO;
	addcr = NO;
	MyFlag = ROLE_PLAYER;
}

static inline void
parseCommandLine(int argc, const char *argv[])
{
	if (argc > 1 && argv[1][0] != '-') {
		printf("\n\x07!! Invalid argument, %s!\n", argv[1]);
		quit();
	}
	if (argc > 1) {
		switch (toupper(*(argv[1] + 1))) {
		case 3: /* Daemon processor */ MyFlag = ROLE_DAEMON; break;
		case 4: MyFlag = ROLE_NPCS; break;
		default: /* None specified */
		{
			txs("\nInvalid parameter '%s'!\n", argv[1]);
			quit();
		}
		}
	}
}

static inline void
processStatus(int connectionNo, const char *prefix, const char *note)
{
	char buffer[512];
	if (note) {
		snprintf(buffer, sizeof(buffer), "%s #%2d: %s: %s", vername, connectionNo, prefix, note);
	} else {
		snprintf(buffer, sizeof(buffer, "%s #%2d: %s", vername, connectionNo, prefix));
	}
	/// TODO: Use this value
}

bool
match(const char *lhs, const char *rhs)
{
	for (;;) {
		char lhsc = *(lhs++);
		char rhsc = *(rhs++);
		// when either string ends, so must the other.
		if (lhsc == 0 || rhsc == 0) {
			return (lhsc == rhsc);
		}
		if (tolower(lhsc) == tolower(rhsc))
			continue;
		// if they didn't match and either character is
		// an underscore, consider it a match if the other
		// character is a space.
		if ((lhsc == '_' || rhsc == '_') && (lhsc == ' ' || rhsc == ' '))
			continue;
		return false;
	}
}

/* Main Program */
int
main(int argc, char *argv[])
{
	int i;

	processStatus(Af, "Initializing", nullptr);

	init();

	parseCommandLine(argc, argv);

	if ((ob = (char *)OS::AllocateClear(5000)) == NULL)
		memfail("IO buffers");
	if ((ow = (char *)OS::AllocateClear(3000)) == NULL)
		memfail("IO buffers");
	if ((input = (char *)OS::AllocateClear(400)) == NULL)
		memfail("IO buffers");
	if ((amanPort = Amiga::FindPort(managerName)) == NULL) {
		tx("Manager not running!\n");
		quit();
	}
	if ((replyPort = Amiga::CreatePort(0L, 0L)) == NULL)
		memfail("user port");
	if ((repbk = Amiga::CreatePort(0L, 0L)) == NULL)
		memfail("comms reply");
	if ((amanrep = Amiga::CreatePort(0L, 0L)) == NULL)
		memfail("aman port");
	if ((amul = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
		memfail("comms port");
	if ((amanp = (Aport *)OS::AllocateClear(sizeof(*amul))) == NULL)
		memfail("comms port");
	Am->mn_Length = sizeof(*amul);
	Am->mn_Node.ln_Type = NT_MESSAGE;
	Am->mn_ReplyPort = amanrep;
	switch (MyFlag) /* What type of line? */
	{
	case ROLE_DAEMON: Af = MAXU; break;
	case ROLE_NPCS: Af = MAXU + 1; break;
	}
	*amanp = *amul;
	link = 1;
	SendIt(MSG_CONNECT, -10, NULL); /* Go for a connection! */
	lstat = (struct LS *)Ad;
	me2 = lstat + Af;
	me2->IOlock = Af;
	ip = 0;
	usr = (struct _PLAYER *)Ap;
	me = usr + Af;
	me2->rep = replyPort;
	if (Ad == -'R') {
		tx("\n...Reset In Progress...\n");
		Amiga::Delay(40);
		quit();
	}
	reset(); /* Read in data files */
	if (Af < 0) {
		sys(NOSLOTS);
		pressret();
		quit();
	}

	processStatus(Af, "Connecting", nullptr);
	me2->unum = Af;
	me2->buf = ob;
	*ob = 0;
	me2->IOlock = -1;
	*ob = 0;
	SendIt(MSG_FREE, NULL, NULL);
	iocheck();

	/* Special processors go HERE: */

	if (Af >= MAXU)
		Special_Proc();

	/* Clear room flags, and send scenario */
	rset = (1 << Af);
	rclr = -1 - rset;
	for (i = 0; i < rooms; i++)
		*(rctab + i) = (*(rctab + i) & rclr);

	do /* Print the title */
	{
		i = fread(block, 1, 900, ifp);
		block[i] = 0;
		tx(block);
	} while (i == 900);
	fclose(ifp);
	ifp = NULL;

	getid(); /*  GET USERS INFO */

	processStatus(Af, "Player", me->name);

	last_him = last_her = it = -1;

	do {
		died = 0;
		actor = -1;
		fol = 0;
		needcr = NO;
		addcr = NO;
		if (last_him != -1 && (lstat + last_him)->state != US_CONNECTED)
			last_him = -1;
		if (last_her != -1 && (lstat + last_her)->state != US_CONNECTED)
			last_her = -1;
		iocheck();
		tx((rktab + me->rank)->prompt);
		needcr = YES;
		block[0] = 0;
		Inp(input, 390);
		if (exeunt != 0)
			break;
		if (stricmp(input, "help") == NULL) {
			sys(HELP);
			continue;
		}
		if (input[0] == '/') {
			internal(input + 1);
			continue;
		}
		if (stricmp(input, "***debug") == NULL) {
			debug = debug ^ 1;
			continue;
		}
		if (input[0] == 0)
			continue;
	gloop:
		failed = NO;
		forced = 0;
		died = 0;
		exeunt = 0;
		if (grab() == -1)
			break;
		iocheck();
		if (forced != 0 && died == 0 && exeunt == 0)
			goto gloop;
	} while (exeunt == 0 && died == 0);

quitgame: /* Quite the game, tidily. */
	if (died == 0)
		action(acp(EXITED), AGLOBAL);
	else
		action(acp(HEDIED), AGLOBAL);
	forced = 0;
	exeunt = 0;
	died = 0;
	if (me->plays == 1)
		sys(BYEPASSWD);
	if (dmove[0] != 0)
		dropall(isroom(dmove));
	else
		dropall(me2->room);
	LoseFollower(); /* Lose who ever is following us. */
	quit();
}
