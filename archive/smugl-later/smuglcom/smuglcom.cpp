/*
 * smuglcom.cpp -- Compiler main source file
 *
 * Copyright (C) Oliver Smith, 1991-2. Copyright (C) Kingfisher s/w 1991-2
 *        Program Designed, Developed and Written By: Oliver Smith.
 */

#define DEF
#define SMUGLCOM 1

#include "smuglcom/smuglcom.hpp"
#include "include/actuals.hpp"
#include "smuglcom/virtuals.hpp"
#include <sys/stat.h>
#if defined(WIN32)
#include <direct.h>
#endif
#include <stdexcept>

static void usage(void);
static void argue(int argc, char *argv[]);
static void checkf(const char *s);

extern const char vername[];
extern long i_alloc;

const char *txtfile[TXTFILES] = { "sysmsg",  "system",  "umsg", "rooms",  "mobiles",
                                  "obdescs", "objects", "lang", "travel", "syns" };

typedef void (*voidfunc)(void);
voidfunc tf_func[TXTFILES] = {
    smsg_proc, sys_proc,  umsg_proc, room_proc, mob_proc1,
    obds_proc, objs_proc, lang_proc, trav_proc, syn_proc,
};

struct VOCAB VC;

int
main(int argc, char *argv[])
{  // Main program - entry point
    ofp1 = ofp2 = ofp3 = NULL;
    g_dir[0] = 0;
    warn = 1;
    vc = &VC;
    vc->extras = 0;  // No extra entries in voacb table

    if (argc > 6)
        usage();
    if (argc > 1)
        argue(argc, argv);

    printf("SMUGL Compiler: %s\n", vername);

    // Ensure that TF_SYSTEM exists, if not, create one
    checksys();
    errabort();

    // Create the Data subdirectory
    sprintf(temp, "%sData", g_dir);
#if !defined(WIN32)
    mkdir(temp, 0777);
#else
    _mkdir(temp);
#endif

    for (int i = 0; i < TXTFILES; i++) {
        section(i);
        if (i == TF_LANG)
            proc = 1;
        try {
            tf_func[i]();
        } catch (std::exception &e) {
            error(e.what());
            errabort();
        }
        if (i == TF_LANG)
            proc = 0;
        if (ifp)
            fclose(ifp);
        ifp = NULL;
    }

    // Write miscellaneous stuff to disk
    finish_rooms();
    save_vocab_index();
    save_basic_objs();

    printf("\nSuccessful: Rooms=%ld: Verbs=%ld Objects=%ld. Total Items=%ld\n",
           rooms,
           verbs,
           nouns,
           rooms + ranks + verbs + nouns + syns + ttents + i_alloc + mobs + mobchars + vc->items);

    fopenw(statsfn);
    fprintf(ofp1, "%ld %ld", rooms, nouns);
    close_ofps();

    fopenw(advfn);
    time(&compiled);

    // Now print the hash statistics, if desired
    if (inc_hash_stats)
        hash_stats();

    // Game name and Logfile name
    fwrite(g_adname, ADNAMEL + 1, 1, ofp1);
    fwrite(g_logname, ADNAMEL + 1, 1, ofp1);
    // Various parameters
    fprintf(ofp1,
            "%ld %ld %d %d %d %d %d %d\n",
            compiled,
            mins,
            invis,
            invis2,
            minsgo,
            rscale,
            tscale,
            port);
    exi = 1;
    quit(NULL);
}

static void
version(void)
{  // Display current version information
    printf("SMUGL: Simple Multi-User Games Language, (C) KingFisher Software 1996-1997\n");
    printf("AMUL:  Amiga Multi-User games Language, (C) KingFisher Software 1990-1993\n\n");
    printf("SMUGL Compiler: %s\n", vername);
    printf("Written By Oliver Smith (oliver@kfs.org)\n");
}

static void
usage(void)
{
    printf("Usage:\n	smuglcom [-v] [-q] [-s] <adv path>\n\n -v = Display version information\n "
           "-q = Quiet (no warnings)\n -s = Include Vocab hash stats\n");
    exit(0);
}

static void
argue(int argc, char *argv[])
{  // Handle the program arguments
    char c;
    int n;

    if (!strcmp(argv[1], "-?"))
        usage();
    for (n = 2; n <= argc; n++) {
        if (!strcmp("-v", argv[n - 1]) || !strcmp("--version", argv[n - 1])) {
            version();
            exit(0);
        }
        if (!strcmp("-q", argv[n - 1])) {
            warn = 0;
            continue;
        }

        if (!strcmp("-s", argv[n - 1])) {
            inc_hash_stats = 1;
            continue;
        }
        strcpy(g_dir, argv[n - 1]);
        c = g_dir[strlen(g_dir) - 1];
        if (c != PATH_SEP_CHAR && c != ':')
            strcat(g_dir, PATH_SEP);
    }
}

int
iscond(const char *s)
{  // Test 's' is a condition name
    int i;

    for (i = 0; i < CONDITIONS; i++)
        if (!strcmp(cond[i].name, s))
            return i;
    return -1;
}

int
isact(const char *s)
{  // Test 's' is an action name
    int i;

    for (i = 0; i < ACTIONS; i++)
        if (!strcmp(action[i].name, s))
            return i;
    return -1;
}

void section(int i)  // Start a new compile section
{
    printf("%s:", txtfile[i]);
    opentxt(txtfile[i]);
    needcr = true;
    fflush(stdout);
}

/* tx(string)
 * Prints a partial string and forces it to be displayed. Under most
 * Unix environments, the output buffering means that nothing would
 * be displayed until an error occured.
 */
void
tx(const char *s)
{
    printf("%s", s);
    fflush(stdout);
}
