// title file parser
//
// The title file was for the login/motd/splash text, but it then
// became a place to also put configuration.

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

static int
getno(const char *title)
{
	remspc(block);
	if (!striplead(title, block)) {
		GetLogger().errorf("Missing '%s' entry.", title);
		return -1;
	}
	const char* p = getword(block);
	strcpy(block, p);
	if (!isdigit(Word[0])) {
		GetLogger().errorf("Invalid '%s' entry.", title);
		return -1;
	}
	return atoi(Word);
}


void
title_proc()
{
	nextc(1);
	fgets(block, 1000, ifp);
	repspc(block);
	remspc(block);
	if (!striplead("name=", block)) {
		tx("Invalid title.txt; missing 'name=' line!\n");
		quit();
	}
	block[strlen(block) - 1] = 0; /* Remove \n */
	if (strlen(block) > 40) {
		block[40] = 0;
		printf("Adventure name too long!            \nTruncated to %40s...\n", block);
	}
	strcpy(adname, block);
	fgets(block, 1000, ifp);
	repspc(block);
	mins = getno("gametime=");
	if (mins < 15) {
		tx("!! Minimum game time of 15 minutes inforced!\n");
		mins = 15;
	}

	fgets(block, 1000, ifp);
	repspc(block);
	invis = getno("invisible=");
	remspc(block);
	getword(block);
	if (!isdigit(Word[0])) {
		GetLogger().error("Invalid rank for visible players to see invisible players/objects.");
	} else
		invis2 = atoi(Word);

	fgets(block, 1000, ifp);
	repspc(block);
	minsgo = getno("min sgo=");

	/*-* Get the Scaleing line. *-*/
	fgets(block, 1000, ifp);
	repspc(block);
	rscale = getno("rankscale="); /* Process RankScale= */
	tscale = getno("timescale="); /* Process TimeScale= */

	readgot = ftell(ifp);

	GetContext().terminateOnErrors();
}
