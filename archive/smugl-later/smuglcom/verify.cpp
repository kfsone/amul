/*
 * verify.cpp -- program to help verify output of smuglcom
 * If you're hacking smuglcom, it can be very hard to tell that it's
 * producing properly sane output. This little thing makes it a might
 * easier. It may become redundant as SMUGL comes back online again,
 * but without it I'd never have gotten this far.
 */

#define SMUGLCOM 1
#define PORTS 1

#include <cstring>

#include "cl_vocab.hpp"
#include "smuglcom.hpp"
#include "virtuals.hpp"

FILE *fp;
char *mem;
size_t size;
char *rdesc;
ROOM *rmp;

counter_t nbobs;
BASIC_OBJ **bobs;
counter_t ncontainers;
CONTAINER *containers;

VOCAB VC;

#define CHUNKS 1024

int read_in(const char *s, size_t sized);

// Output a umsg
static const char *
umsg(msgno_t n)
{
    if (n < 0)
        return "(none)";
    else
        return umsgp + umsgip[n];
}

const char *options[] = {
    "-rooms", "-ranks", "-smsgs", "-umsgs", "-mobs", "-vocab", "-containers"
};

enum { mROOMS, mRANKS, mSMSGS, mUMSGS, mMOBS, mVOCAB, mCONT, nOPTS };

void Lrooms(), Lranks(), Lsmsgs(), Lumsgs(), Lmobs(), Lvocab(), Lcont();

int
main(int argc, char *argv[])
{
    int i, j;

    vc = &VC;

    // Always read in the message file
    msgs = read_in(umsgifn, sizeof(long));
    if (msgs < NSMSGS)
        printf("** Incomplete System Message file\n");
    umsgip = (long *) mem;
    mem = nullptr;
    read_in(umsgfn, CHUNKS);
    umsgp = mem;
    mem = nullptr;

    // Next we're gonna need the vocab table
    read_in_vocab(nullptr);

    // Always read the rooms file in
    rooms = read_in(roomsfn, sizeof(ROOM));
    if (rooms <= 0) {
        printf("** Defunct rooms file\n");
        exit(1);
    }
    roomtab = (ROOM *) mem;
    mem = nullptr;

    // Always read in the object table
    nouns = read_in(objsfn, sizeof(OBJ));
    if (nouns <= 0) {
        printf("** Defunct objects file\n");
        exit(1);
    }
    obtab = (OBJ *) mem;
    mem = nullptr;

    // Next, load in the containers and bobs index
    if (read_in(bobfn, CHUNKS) <= 0) {
        printf("** Defunct 'Basic Objects' file\n");
        exit(1);
    }
    nbobs = ((long *) mem)[0];
    ncontainers = ((long *) mem)[1];
    mem += sizeof(counter_t) * 2;
    bobs = (BASIC_OBJ **) mem;
    mem += sizeof(BASIC_OBJ *) * nbobs;
    containers = (CONTAINER *) mem;
    mem = nullptr;

    // Finally, knock up the bobs index based on what we know
    for (i = 0; i < rooms; i++)
        // Index rooms
        bobs[i] = (BASIC_OBJ *) (roomtab + i);
    for (i = 0; i < nouns; i++)
        // Index objects
        bobs[rooms + i] = (BASIC_OBJ *) (obtab + i);
    for (i = 1; i < rooms + nouns; i++)
    // Linked list
    {
        bobs[i - 1]->next = bobs[i];
        bobs[i]->next = nullptr;
    }

    for (i = 1; i < argc; i++) {
        for (j = 0; j < nOPTS; j++) {
            if (strcmp(options[j], argv[i]) == 0) {
                switch (j) {
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
    printf(" std_flags: %08x: ", std_flags);
    if (std_flags == 0) {
        printf("<none>\n");
        return;
    }
    int flags = 0;
    for (int i = 0; std_flag[i]; i++) {
        if (std_flags & (1 << i)) {
            if (flags++)
                printf(" | ");
            printf("%s", std_flag[i]);
        }
    }
    printf("\n");
}

void
Lcont()
{
    printf("Total of %d containers\n", ncontainers);
    for (int i = 0; i < ncontainers; i++) {
        printf("CONTAINER#%d: %s inside %s\n",
               i,
               word(bobs[containers[i].boSelf]->id),
               word(bobs[containers[i].boContainer]->id));
    }
}

void
Lrooms()
{
    int i, j;
    for (rmp = roomtab, i = 0; i < rooms; i++, rmp++) {
        printf("ROOM %d: %s\n", i, word(rmp->id));
        printf(" ttlines: %d\n", rmp->ttlines);
        describe_std_flags(rmp->std_flags);
        printf(" flags:   %04x ", rmp->flags);
        for (j = 0; rflag[j]; j++) {
            if (rmp->flags & (1 << j))
                printf("%02x:%s ", 1 << j, rflag[j]);
        }
        printf("\n");
        printf(" s_descrip: %d, l_descrip: %d\n", rmp->s_descrip, rmp->l_descrip);
        printf(" tabptr:    %ld\n", rmp->tabptr);
        printf(" dmove:     %d\n", rmp->dmove);
        printf(" Short:     %s\n", umsg(rmp->s_descrip));
        printf(" Long :     %s\n", umsg(rmp->l_descrip));
        printf(" Contents:  %d", rmp->contents);
        if (rmp->contents > 0) {
            for (j = rmp->conTent; j != -1; j = containers[j].conNext) {
                basic_obj bob = containers[j].boSelf;
                printf(" %s", word(bobs[bob]->id));
            }
        }
        printf("\n\n");
    }

    free(roomtab);
}

void
Lranks()
{
    int i;

    printf("Calling 'read_in'\n");
    ranks = read_in(ranksfn, sizeof(RANKS));
    printf("ranks = %d\n", ranks);
    ranktab = (RANKS *) mem;
    for (i = 0; i < ranks; i++, ranktab++) {
        printf("RANK %d: [%s][%s]\n", i, ranktab->male, ranktab->female);
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
    mem = nullptr;
}

void
Lsmsgs()
{
    long i;
    for (i = 0; i < msgs && i < NSMSGS; i++) {
        printf("System Message #%ld:\n", i);
        printf("> %s\n", umsg(i));
    }
}

void
Lumsgs()
{
    long i;
    for (i = NSMSGS; i < msgs; i++) {
        printf("Message #%ld:\n", i);
        printf("> %s\n", umsg(i));
    }
}

#define MDIS(x, y) printf(" %s = %d\n", x, mobp->y)
void
Lmobs()
{
    int i = 0;
    mobs = read_in(mobfn, sizeof(MOB_ENT));
    mobp = (MOB_ENT *) mem;
    for (i = 0; i < mobs; i++, mobp++) {
        printf("MOB #%d: %s\n", i, word(mobp->id));
        printf(" dead = %d\n", mobp->dead);
        printf(" speed = %d\n", mobp->speed);
        printf(" travel = %d\n", mobp->travel);
        printf(" fight = %d\n", mobp->fight);
        printf(" act = %d\n", mobp->act);
        printf(" wait = %d\n", mobp->wait);
        printf(" fear = %d\n", mobp->fear);
        printf(" attack = %d\n", mobp->attack);
        printf(" flags = %d\n", mobp->flags);
        printf(" dmove = %d\n", mobp->dmove);
        printf(" hitpower = %d\n", mobp->hitpower);
        printf(" arr = %s\n", umsg(mobp->arr));
        printf(" dep = %s\n", umsg(mobp->dep));
        printf(" flee = %s\n", umsg(mobp->flee));
        printf(" hit = %s\n", umsg(mobp->hit));
        printf(" miss = %s\n", umsg(mobp->miss));
        printf(" death = %s\n", umsg(mobp->death));
    }
    free(mem);
    mem = nullptr;
}

void
Lvocab()
{
    printf("By 'index':\n");
    for (counter_t i = 0; i < VC.items; i++) {
        printf("%s, ", word(i));
    }
    printf("\n");
    printf("By 'hash':\n");
    for (uint32_t i = 0; i < VOCAB_ROWS; i++) {
        printf("%d:[", i);
        fflush(stdout);
        for (counter_t j = 0; j < VC.hash_size[i]; j++) {
            printf("%s ", word(VC.hash[i][j]));
            fflush(stdout);
        }
        printf("]. ");
    }
    printf("\n");
    printf("Running sanity check:\n");
    /* Go through all the entries in the reverse index,
     * look the word up via the hash, and see if we get back
     * where we started */
    long errs = 0;
    for (counter_t i = 0; i < VC.items; i++) {
        vocid_t j = is_word(word(i));
        if (j != i) {
            printf(" * %d(%s) returns %d(%s)\n", i, word(i), j, word(j));
            errs++;
        }
    }
    if (errs)
        printf("It's broke.\n");
    else
        printf("It works fine.\n");
}

int
read_in(const char *s, size_t sized)
{
    char tmp[100];
    int cnt = 0;
    size_t bytes = 0;

    if (fp)
        fclose(fp);
    fp = nullptr;
    if (mem)
        free(mem);
    mem = nullptr;
    sprintf(tmp, "Data" PATH_SEP "%s", s);
    fp = fopen(tmp, "rb");
    if (fp == nullptr) {
        printf("Can't read file '%s'\n", tmp);
        exit(1);
    }

    mem = (char *) malloc((size_t)((size = sized) + 1));
    while ((bytes = fread(mem + (cnt * sized), 1, sized, fp)) == sized) {
        mem = (char *) realloc(mem, size + sized + 1);
        memset(mem + size, 0, sized + 1);
        size += sized;
        cnt++;
    }
    fclose(fp);
    fp = nullptr;
    return cnt;
}
