#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

static void
rank_error(const char *what)
{
	GetLogger().errorf("rank:%d (%s/%s): Invalid %s %s: \"%s\"", ranks, rank.male, rank.female,
					   what, Word);
}

/* Process RANKS.TXT */
void
rank_proc()
{
	char *p;
	int   n;

	nextc(1);
	fopenw(ranksfn);
	putchar('\n');

	ranks = 0;
	n = 0;

	do {
		fgets(block, 1024, ifp);
		if (feof(ifp))
			continue;
		if (com(block) == -1 || block[0] == '\n')
			continue;
		tidy(block);
		if (block[0] == 0)
			continue;
		p = getword(block);
		if (chkline(p) != 0)
			continue;
		ranks++;
		rank.male[0] = 0;
		rank.female[0] = 0;
		if (strlen(Word) < 3 || strlen(Word) > RANKL) {
			GetLogger().errorf("Invalid male rank: %s", Word);
		}
		n = 0;
		do {
			if (Word[n] == '_')
				Word[n] = ' ';
			rank.male[n] = rank.female[n] = tolower(Word[n]);
			n++;
		} while (Word[n - 1] != 0);

		p = getword(p);
		if (chkline(p) != 0)
			continue;
		if (strcmp(Word, "=") != NULL && strlen(Word) < 3 || strlen(Word) > RANKL) {
			GetLogger().errorf("Invalid female rank: %s", Word);
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
			rank_error("number for score");
			continue;
		}
		rank.score = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for strength");
			continue;
		}
		rank.strength = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for stamina");
			continue;
		}
		rank.stamina = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for dexterity");
			continue;
		}
		rank.dext = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for wisdom");
			continue;
		}
		rank.wisdom = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for experience");
			continue;
		}
		rank.experience = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for magic points");
			continue;
		}
		rank.magicpts = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for max carry weight");
			continue;
		}
		rank.maxweight = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for max carry items");
			continue;
		}
		rank.numobj = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("number for min. points per kill");
			continue;
		}
		rank.minpksl = atoi(Word);

		p = getword(p);
		if (!isdigit(Word[0])) {
			rank_error("task number");
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
		if (p - block > 10) /* Greater than prompt length? */
		{
			rank_error("prompt string (too long)");
			continue;
		}
		if (block[0] == 0)
			strcpy(rank.prompt, "$ ");
		else
			strcpy(rank.prompt, block);

		wizstr = rank.strength;
		fwrite(rank.male, sizeof(rank), 1, ofp1);
	} while (!feof(ifp));

	GetContext().terminateOnErrors();
	close_ofps();
}

chkline(char *p)
{
	if (*p == 0) {
		GetLogger().errorf("Rank line #%u incomplete.", ranks);
		return 1;
	}
	return 0;
}
