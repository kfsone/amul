/* Mobiles.Txt Processor */
#include "amulcominc.h"

/*=* Pass 1: Indexes mobile names *=*/

void
mob_proc1()
{
	char *  p, *s1, *s2;
	int32_t n;

	err = 0;
	mobchars = 0;
	fopenw(mobfn);
	if (nextc(0) == -1)
		return;

	blkget(&moblen, &mobdat, 0L);
	p = mobdat;
	repspc(mobdat);

	do {
	ldo:
		while (*p != 0 && *p != '!')
			p = skipline(p);
		if (*p == 0)
			break;
		p = sgetl(p, block);
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
					printf("## Mobile '%s': invalid DMove '%s'.\n", mob.id, Word);
					err++;
				}
				continue;
			}
		} while (*s1 != 0 && *s1 != ';' && Word[0] != 0);

		p = sgetl(p, block);
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
			printf("## Mobile '%s': Travel+Fight+Act+Wait <> 100%! Please check!\n", mob.id);
			err++;
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

	if (err != 0) {
		printf("\n\n!! Aborting due to %ld errors !!\n\n", err);
		quit();
	}
	close_ofps();
	if (mobchars != 0) {
		if ((mobp = (struct _MOB_ENT *)AllocMem(sizeof(mob) * mobchars, MEMF_PUBLIC)) == NULL) {
			printf("### FATALY OUT OF MEMORY!\n");
			quit();
		}
		fopena(mobfn);
		fread((char *)mobp, sizeof(mob) * mobchars, 1, afp);
		close_ofps();
	}
}

void
mobmis(char *s)
{
	printf("## Mobile '%s': missing %s field.\n", mob.id, s);
	err++;
	skipblock();
}

int
badmobend()
{
	return -1;
}

/*=* Fetch mobile message line *=*/
int
getmobmsg(char *s)
{
	char *q;
	int   n;

loop:
	if (com(px) == -1) {
		px = skipline(px);
		goto loop;
	}
	if (*px == 0 || *px == 13 || *px == 10) {
		err++;
		printf("## Mobile '%s': Unexpected end of mobile!\n");
		return -1;
	}
	px = skipspc(px);
	if (*px == 0 || *px == 13 || *px == 10)
		goto loop;

	if ((q = skiplead(s, px)) == px) {
		mobmis(s);
		err++;
		return -1;
	}
	if (toupper(*q) == 'N') {
		px = skipline(px);
		return -2;
	}
	n = ttumsgchk(q);
	px = skipline(px);
	if (n == -1) {
		printf("## Mobile '%s': Bad text on '%s' line!\n", mob.id, s);
		err++;
	}
	return n;
}

/*=* Pass 2: Indexes commands mobiles have access to *=*/

/*mob_proc2()
{*/
