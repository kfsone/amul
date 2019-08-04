/*
 * smuglcom.cpp -- Compiler main source file
 *
 * Copyright (C) Oliver Smith, 1991-2. Copyright (C) Kingfisher s/w 1991-2
 *        Program Designed, Developed and Written By: Oliver Smith.
 */

#define DEF
#define SMUGLCOM 1

#include "smuglcom.hpp"
#include "actuals.hpp"
#include "errors.hpp"
#include "fileio.hpp"
#include "virtuals.hpp"

#include "cl_vocab.hpp"

#include <cstring>

static void usage();
static void argue(int argc, const char *argv[]);
static void checkf(const char *s);

extern char vername[];
extern long i_alloc;

using voidfunc = void (*)();
voidfunc tf_func[TXTFILES] = {
    smsg_proc, sys_proc,  umsg_proc, room_proc, mob_proc1,
    obds_proc, objs_proc, lang_proc, trav_proc, syn_proc,
};

VOCAB VC;

int
main(int argc, const char *argv[])
{  // Main program - entry point
    int i;

    set_version_string();

    ofp1 = ofp2 = ofp3 = nullptr;
    dir[0] = 0;
    warn = 1;
    vc = &VC;
    vc->extras = 0;  // No extra entries in voacb table

    if (argc > 6)
        usage();
    if (argc > 1)
        argue(argc, argv);

    printf("SMUGL Compiler: %s\n", vername);

    // Check for the presence of all the text files except system
    for (i = 0; i < TXTFILES; i++) {
        if (i != TF_SYSTEM)
            checkf(txtfile[i]);
    }

    // Ensure that TF_SYSTEM exists, if not, create one
    checksys();
    errabort();

    // Create the Data subdirectory
    sprintf(temp, "%sData", dir);
    mkdir(temp, 0777);

    for (i = 0; i < TXTFILES; i++) {
        section(i);
        if (i == TF_LANG)
            proc = 1;
        tf_func[i]();
        if (i == TF_LANG)
            proc = 0;
        if (ifp)
            fclose(ifp);
        ifp = nullptr;
    }

    // Write miscellaneous stuff to disk
    finish_rooms();
    save_vocab_index();
    save_basic_objs();

    printf("\nSuccessful: Rooms=%d: Verbs=%d Objects=%d. Total Items=%ld\n",
           rooms,
           verbs,
           nouns,
           rooms + ranks + verbs + nouns + syns + ttents + i_alloc + mobs + mobchars + vc->items);

    fopenw(statsfn);
    fprintf(ofp1, "%d %d", rooms, nouns);
    close_ofps();

    fopenw(advfn);
    time(&compiled);

    // Now print the hash statistics, if desired
    if (inc_hash_stats)
        hash_stats();

    // Game name and Logfile name
    fwrite(adname, ADNAMEL + 1, 1, ofp1);
    fwrite(logname, ADNAMEL + 1, 1, ofp1);
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
    quit();
}

// Display current version information
static void
version()
{
    printf("SMUGL: Simple Multi-User Games Language, (C) KingFisher Software 1996-1997\n");
    printf("AMUL:  Amiga Multi-User games Language, (C) KingFisher Software 1990-1993\n\n");
    printf("SMUGL Compiler: %s\n", vername);
    printf("Written By Oliver Smith (oliver@kfs.org)\n");
}

static void
usage()
{
    printf("Usage:\n	smuglcom [-v] [-q] [-s] <adv path>\n\n -v = Display version information\n "
           "-q = Quiet (no warnings)\n -s = Include Vocab hash stats\n");
    exit(0);
}

// Handle the program arguments
static void
argue(int argc, const char *argv[])
{
    char c;
    if (!strcmp(argv[1], "-?"))
        usage();
    for (int n = 2; n <= argc; n++) {
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
        strcpy(dir, argv[n - 1]);
        if ((c = dir[strlen(dir) - 1]) != PATH_SEP_CHAR && c != ':')
            strcat(dir, PATH_SEP);
    }
}

// Check whether a file exists
static void
checkf(const char *s)
{
    sprintf(block, "%s%s.txt", dir, s);
    if ((ifp = fopen(block, "rb")))
        fclose(ifp);
    else
        error("Missing: file %s!\n", block);
    ifp = nullptr;
}

// Test 's' is a condition name
int
iscond(const char *s)
{
    int i;
    for (i = 0; i < CONDITIONS; i++)
        if (!strcmp(cond[i].name, s))
            return i;
    return -1;
}

// Test 's' is an action name
int
isact(const char *s)
{
    int i;
    for (i = 0; i < ACTIONS; i++)
        if (!strcmp(action[i].name, s))
            return i;
    return -1;
}

// Start a new compile section
void
section(int i)
{
    printf("%s:", txtfile[i]);
    opentxt(txtfile[i]);
    needcr = TRUE;
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
