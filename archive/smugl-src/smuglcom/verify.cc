/*
 * verify.cpp -- program to help verify output of smuglcom
 * If you're hacking smuglcom, it can be very hard to tell that it's
 * producing properly sane output. This little thing makes it a might
 * easier. It may become redundant as SMUGL comes back online again,
 * but without it I'd never have gotten this far.
 */

static const char rcsid[] = "$Id: verify.cc,v 1.11 1999/06/08 15:36:54 oliver Exp $";

#define SMUGLCOM 1
#define	PORTS    1

#include "smuglcom.hpp"
#include "virtuals.hpp"

#include <cstring>

FILE *fp;
char *mem;
long size;
char *rdesc;
ROOM *rmp;

counter_t nbobs;
BASIC_OBJ **bobs;
counter_t ncontainers;
CONTAINER *containers;

VOCAB VC;

#define	CHUNKS	1024

int read_in(const char *s, size_t sized);

static __inline const char *
umsg(msgno_t n)                 /* Output a umsg */
    {
    if (n < 0)
        return "(none)";
    else return umsgp + umsgip[n];
    }

const char *options[] =
{
    "-rooms", "-ranks", "-smsgs", "-umsgs", "-mobs", "-vocab",
    "-containers"
};

enum
{
    mROOMS, mRANKS, mSMSGS, mUMSGS, mMOBS, mVOCAB, mCONT, nOPTS
};

void Lrooms(void), Lranks(void), Lsmsgs(void), Lumsgs(void),
    Lmobs(void), Lvocab(void), Lcont(void);

int
main(int argc, char *argv[])
    {
    int i, j;

	vc = &VC;

    /* Always read in the message file */
    msgs = read_in(umsgifn, sizeof(long));
    if (msgs < NSMSGS)
	printf("** Incomplete System Message file\n");
    umsgip = (long *)mem;
    mem = NULL;
    read_in(umsgfn, CHUNKS);
    umsgp = mem;
    mem = NULL;

    /* Next we're gonna need the vocab table */
    read_in_vocab(NULL);

    /* Always read the rooms file in */
    rooms = read_in(roomsfn, sizeof(ROOM));
    if (rooms <= 0)
        {
        printf("** Defunct rooms file\n");
        exit(1);
        }
    roomtab = (ROOM *) mem;
    mem = NULL;

    /* Always read in the object table */
    nouns = read_in(objsfn, sizeof(OBJ));
    if (nouns <= 0)
        {
        printf("** Defunct objects file\n");
        exit(1);
        }
    obtab = (OBJ *)mem;
    mem = NULL;

    /* Next, load in the containers and bobs index */
    if (read_in(bobfn, CHUNKS) <= 0)
        {
        printf("** Defunct 'Basic Objects' file\n");
        exit(1);
        }
    nbobs = ((long *)mem)[0];
    ncontainers = ((long *)mem)[1];
    mem += sizeof(counter_t) * 2;
    bobs = (BASIC_OBJ **)mem;
    mem += sizeof(BASIC_OBJ *) * nbobs;
    containers = (CONTAINER *)mem;
    mem = NULL;

    /* Finally, knock up the bobs index based on what we know */
    for (i = 0; i < rooms; i++) // Index rooms
        bobs[i] = (BASIC_OBJ *)(roomtab + i);
    for (i = 0; i < nouns; i++) // Index objects
        bobs[rooms + i] = (BASIC_OBJ *)(obtab + i);
    for (i = 1; i < rooms + nouns; i++) // Linked list
        {
        bobs[i - 1]->next = bobs[i];
        bobs[i]->next = NULL;
        }

    for (i = 1; i < argc; i++)
        {
	for (j = 0; j < nOPTS; j++)
            {
	    if (strcmp(options[j], argv[i]) == 0)
                {
		switch (j)
                    {
                  case mROOMS:
                        Lrooms();
                        break;
                  case mRANKS:
                        Lranks();
                        break;
                  case mSMSGS:
                        Lsmsgs();
                        break;
                  case mUMSGS:
                        Lumsgs();
                        break;
                  case mMOBS:
                        Lmobs();
                        break;
                  case mVOCAB:
                        Lvocab();
                        break;
                  case mCONT:
                        Lcont();
                        break;
                  default:
                        printf("Odd internal error - unknown case %d\n", j);
                        exit(2);
                    }
		break;
                }
            }
        }
    }

static void
describe_std_flags(flag_t std_flags)
    {
    printf(" std_flags: %08lx: ", std_flags);
    if (std_flags == 0)
        {
        printf("<none>\n");
        return;
        }
    int flags = 0;
    for (int i = 0; std_flag[i]; i++)
        {
        if (std_flags & (1 << i))
            {
            if (flags++)
                printf(" | ");
            printf("%s", std_flag[i]);
            }
        }
    printf("\n");
    }

void
Lcont(void)
    {
    printf("Total of %ld containers\n", ncontainers);
    for (int i = 0; i < ncontainers; i++)
        {
        printf("CONTAINER#%d: %s inside %s\n", i,
               word(bobs[containers[i].boSelf]->id),
               word(bobs[containers[i].boContainer]->id));
        }
    }

void
Lrooms(void)
    {
    int i, j;
    for (rmp = roomtab, i = 0; i < rooms; i++, rmp++)
        {
	printf("ROOM %d: %s\n", i, word(rmp->id));
	printf(" ttlines: %d\n", rmp->ttlines);
        describe_std_flags(rmp->std_flags);
	printf(" flags:   %04lx ", rmp->flags);
	for (j = 0; rflag[j]; j++)
            {
	    if (rmp->flags & (1 << j))
		printf("%02x:%s ", 1 << j, rflag[j]);
            }
	printf("\n");
	printf(" s_descrip: %ld, l_descrip: %ld\n", rmp->s_descrip, rmp->l_descrip);
	printf(" tabptr:    %ld\n", rmp->tabptr);
	printf(" dmove:     %ld\n", rmp->dmove);
        printf(" Short:     %s\n", umsg(rmp->s_descrip));
        printf(" Long :     %s\n", umsg(rmp->l_descrip));
        printf(" Contents:  %ld", rmp->contents);
        if (rmp->contents > 0)
            {
            j = rmp->conTent;
            for (j = rmp->conTent; j != -1; j = containers[j].conNext)
                {
                basic_obj bob = containers[j].boSelf;
                printf(" %s", word(bobs[bob]->id));
                }
            }
        printf("\n\n");
        }

    free((char *) roomtab);
    }

void
Lranks(void)
    {
    int i;

    printf("Calling 'read_in'\n");
    ranks = read_in(ranksfn, sizeof(RANKS));
    printf("ranks = %ld\n", ranks);
    ranktab = (RANKS *)mem;
    for (i = 0; i < ranks; i++, ranktab++)
        {
	printf("RANK %d: [%s][%s]\n",
	       i, ranktab->male, ranktab->female);
	printf(" score:      %ld\n", ranktab->score);
	printf(" strength:   %ld\n", ranktab->strength);
	printf(" stamina:    %ld\n", ranktab->stamina);
	printf(" dext:       %ld\n", ranktab->dext);
	printf(" wisdom:     %ld\n", ranktab->wisdom);
	printf(" experience: %ld\n", ranktab->experience);
	printf(" magicpts:   %ld\n", ranktab->magicpts);
	printf(" maxweight:  %ld\n", ranktab->maxweight);
	printf(" numobj:     %ld\n", ranktab->numobj);
	printf(" minpksl:    %ld\n", ranktab->minpksl);
	printf(" tasks:      %ld\n", ranktab->tasks);
	printf(" prompt:     %s\n", umsg(ranktab->prompt));
        }
    free(mem);
    mem = NULL;
    }

void
Lsmsgs(void)
    {
    long i;
    for (i = 0; i < msgs && i < NSMSGS; i++)
        {
	printf("System Message #%ld:\n", i);
	printf("> %s\n", umsg(i));
        }
    }

void
Lumsgs(void)
    {
    long i;
    for (i = NSMSGS; i < msgs; i++)
        {
	printf("Message #%ld:\n", i);
	printf("> %s\n", umsg(i));
        }
    }

#define MDIS(x,y)	printf(" %s = %d\n", x, mobp->y)
void
Lmobs(void)
    {
    int i = 0;
    mobs = read_in(mobfn, sizeof(MOB_ENT));
    mobp = (MOB_ENT *)mem;
    for (i = 0; i < mobs; i++, mobp++)
        {
	printf("MOB #%d: %s\n", i, word(mobp->id));
	printf(" dead = %d\n", mobp->dead);
	printf(" speed = %d\n", mobp->speed);
	printf(" travel = %d\n", mobp->travel);
	printf(" fight = %d\n", mobp->fight);
	printf(" act = %d\n", mobp->act);
	printf(" wait = %d\n", mobp->wait);
	printf(" fear = %d\n", mobp->fear);
	printf(" attack = %d\n", mobp->attack);
	printf(" flags = %ld\n", mobp->flags);
	printf(" dmove = %ld\n", mobp->dmove);
	printf(" hitpower = %d\n", mobp->hitpower);
	printf(" arr = %s\n", umsg(mobp->arr));
	printf(" dep = %s\n", umsg(mobp->dep));
	printf(" flee = %s\n", umsg(mobp->flee));
	printf(" hit = %s\n", umsg(mobp->hit));
	printf(" miss = %s\n", umsg(mobp->miss));
	printf(" death = %s\n", umsg(mobp->death));
        }
    free(mem);
    mem = NULL;
    }

void
Lvocab(void)
    {
    long i, j, errs = 0;
    printf("By 'index':\n");
    for (i = 0; i < VC.items; i++)
        {
        printf("%s, ", word(i));
        }
    printf("\n");
    printf("By 'hash':\n");
    for (i = 0; i < VOCROWS; i++)
        {
        printf("%ld:[", i); fflush(stdout);
        for (j = 0; j < VC.hash_size[i]; j++)
            {
            printf("%s ", word(VC.hash[i][j])); fflush(stdout);
            }
        printf("]. ");
        }
    printf("\n");
    printf("Running sanity check:\n");
    /* Go through all the entries in the reverse index,
     * look the word up via the hash, and see if we get back
     * where we started */
    for (i = 0, errs = 0; i < VC.items; i++)
        {
        j = is_word(word(i));
        if (j != i)
            {
            printf(" * %ld(%s) returns %ld(%s)\n",
                   i, word(i), j, word(j));
	    errs++;
            }
        }
    if (errs)
     printf("It's broke.\n");
    else printf("It works fine.\n");
    }

int
read_in(const char *s, size_t sized)
    {
    char tmp[100];
    int cnt = 0;
    size_t bytes = 0;

    if (fp)
	fclose(fp);
    fp = 0L;
    if (mem)
	free(mem);
    mem = 0L;
    sprintf(tmp, "Data" PATH_SEP "%s", s);
    fp = fopen(tmp, "rb");
    if (fp == NULL)
        {
	printf("Can't read file '%s'\n", tmp);
	exit(1);
        }

    mem = (char *)malloc((size_t)((size = sized) + 1));
    while ((bytes = fread(mem + (cnt * sized), 1, sized, fp)) == sized)
        {
	mem = (char *)realloc(mem, size + sized + 1);
	bzero(mem + size, sized + 1);
	size += sized;
	cnt++;
        }
    fclose(fp);
    fp = NULL;
    return cnt;
    }
