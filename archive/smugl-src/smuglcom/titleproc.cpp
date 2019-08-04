/*
 * titleproc.cpp -- process SYSTEM.txt
 */

#include <cctype>
#include <cstring>

#include "errors.hpp"
#include "smuglcom.hpp"

#define BAD_STRING "Invalid %s specified (%s).\n"

enum SecType { stOptions, stRanks };
static SecType Section = stOptions;
static int def_prompt_id = -1;

counter_t top_num_obj = 0;

extern char vername[];

enum {         // List of system options
    SO_NAME,   // name of this game
    SO_SESH,   // Session time limit
    SO_SEE1,   // Rank at which invises can see each other
    SO_SEE2,   // Rank at which visibles can see invises
    SO_MSGO,   // Minimum rank to use 'SuperGo'
    SO_RSCL,   // Rank scaling factor } in awarding points
    SO_TSCL,   // Time scaling factor } in a sanctuary
    SO_LOG,    // Game's log file
    SO_PORT,   // Which port to run the game on
    SO_NOISE,  // Define noise words
    SYSOPTS    // Number of system options
};

const char *sysopt[SYSOPTS] = {
    "name",      "session",   "iseeinvis", "seeinvis", "minsgo",
    "rankscale", "timescale", "log",       "port",     "noise",
};

static inline int
number(char *s)
{  // Wrapper for 'atoi'
    if (!isdigit(*s))
        return -1;
    return atoi(s);
}

static inline void
option_line(char *s)
{
    int i;
    // Check to see if we're changing section
    if (skiplead("[ranks]", s) != s) {
        Section = stRanks;
        return;
    }
    // Otherwise we should have an option line
    char *p;
    while (*(s = skipspc(s))) {
        // We're expecting wwwwwww=value or wwwwww="value"
        p = s;
        while (isalpha(*s))
            s++;

        // Ignore the word "mins" -- it happens, we don't care
        if (strncmp(p, "mins", (size_t)(s - p)) == 0)
            continue;

        // Field name should be followed by =value or ="value"
        if (*s != '=') {
            *s = 0;
            error("Unknown phrase \"%s\".\n", p);
            return;  // Next line
        }
        *(s++) = 0;

        // Look for this in the option-name list
        for (i = 0; i < SYSOPTS; i++)
            if (strcmp(p, sysopt[i]) == 0)
                break;
        if (i >= SYSOPTS) {  // It wasn't in the list
            error("Unkown option \"%s=\".\n", p);
            return;  // Next line
        }

        // Now extract the value for this option
        p = s;
        if (*p == '\"' || *p == '\'') {  // Quoted, string value
            s++;
            while (*s && *s != *p)
                s++;
            p++;
        } else  // Numeric/single word value
            while (!isspace(*s) && *s)
                s++;
        if (*s)
            // Insert our own "end of string"
            *(s++) = 0;

        // Now relate the value to it's option type & validate it
        switch (i) {       // What type?
            case SO_NAME:  // Adventure name
                memset(g_adname, 0, ADNAMEL);
                if (strlen(p) > ADNAMEL) {
                    warne("Adventure name too long! Truncated.\n");
                    *(p + ADNAMEL) = 0;
                }
                strcpy(g_adname, p);
                break;
            case SO_SESH:  // Session time
                mins = number(p);
                if (mins == -1) {
                    error(BAD_STRING, "session time", p);
                    break;
                }
                if (mins < 15) {
                    warne("Session time cannot be less than 15 minutes.\n");
                    mins = -1;
                }
                break;
            case SO_SEE1:  // Invisible see invis
                invis = number(p);
                if (invis == -1)
                    error(BAD_STRING, "'see invisible' level", p);
                break;
            case SO_SEE2:  // visible see invis
                invis2 = number(p);
                if (invis2 == -1)
                    error(BAD_STRING, "'visible can see invisible' level", p);
                break;
            case SO_MSGO:
                minsgo = number(p);
                if (minsgo == -1)
                    error(BAD_STRING, "minimum supergo rank", p);
                break;
            case SO_RSCL:
                rscale = number(p);
                if (rscale == -1)
                    error(BAD_STRING, "rank-scaling amount", p);
                break;
            case SO_TSCL:
                tscale = number(p);
                if (tscale == -1)
                    error(BAD_STRING, "time-scaling amount", p);
                break;
            case SO_LOG:
                memset(g_logname, 0, ADNAMEL);
                if (strlen(p) > ADNAMEL) {
                    error("Log-file name too long!\n");
                    break;
                }
                strcpy(g_logname, p);
                break;
            case SO_PORT:
                if ((port = number(p)) == -1)
                    error(BAD_STRING, "port number", p);
                else if (port < 1024)
                    warne("Port %d requires root priveliges to bind\n", port);
                break;
            case SO_NOISE:
                // Noise words: Fake Aliases
                // By pointing at a vocid of -1 we say:
                //  "I know this word, but it doesn't have a counterpart"
                // Which means it's a dud and can be ignored
                char *end;
                do {
                    if ((end = strchr(p, ',')) != nullptr)
                        *(end++) = 0;
                    if (!*p) {
                        error("Invalid 'noise=' string");
                        break;
                    }
                    vocid_t alias = new_word(p, true);
                    if (alias == -1) {
                        error("Bad/duplicate noise word '%s'", p);
                        break;
                    }
                    // Write the word id
                    fwrite(&alias, sizeof(alias), 1, ofp2);
                    // And now assign it a -1 meaning
                    alias = -1;
                    fwrite(&alias, sizeof(alias), 1, ofp2);
                    syns++;

                    p = end;
                } while (end && p && *p);
                break;
            default:
                error("** Int.err: sysopt[%ld]!\n", i);
                break;
        }
    }
}

static int
chkline(char *p)
// Test for incomplete rank line
{
    if (*p)
        return 0;
    error("Rank line %ld incomplete!\n", ranks);
    return 1;
}

static void
badrank(const char *s)
// Complain about a bad rank line
{
    error("%3ld/%s: Invalid number for %s - \"%s\".\n", ranks, rank.male, s, Word);
}

static bool
rank_item(const char *type, char *&s, long &val)
{
    s = getword(s);
    if (chkline(s))
        return false;
    if (!isdigit(*Word)) {
        badrank(type);
        return false;
    } else
        val = atol(Word);
    return true;
}

static inline void
rank_line(char *s)
{
    int i;
    s = skipspc(s);
    if (!*s)
        return;

    ranks++;
    *rank.male = *rank.female = 0;
    rank.prompt = def_prompt_id;

    s = getword(s);
    if (chkline(s))
        return;
    if (strlen(Word) < 3 || strlen(Word) > RANKL) {
        error("%s rank name \"%s\" is invalid.\n", "Male", Word);
        Word[RANKL] = 0;
    }
    i = 0;
    do {
        if (Word[i] == '_')
            Word[i] = ' ';
        rank.male[i] = rank.female[i] = Word[i];
        i++;
    } while (Word[i - 1]);

    s = getword(s);
    if (chkline(s))
        return;
    if (strcmp(Word, "=") != 0L && (strlen(Word) < 3 || strlen(Word) > RANKL)) {
        error("%s rank name \"%s\" is invalid.\n", "Female", Word);
        Word[RANKL] = 0;
    }
    if (Word[0] != '=') {
        i = 0;
        do {
            rank.female[i] = (Word[i] == '_') ? ' ' : Word[i];
        } while (Word[i++]);
    }

    // Read in all the numeric values
    // Use 'if's short circuiting to only test
    // until one value fails
    if (!rank_item("score", s, rank.score) || !rank_item("strength", s, rank.strength) ||
        !rank_item("stamina", s, rank.stamina) || !rank_item("dexterity", s, rank.dext) ||
        !rank_item("wisdom", s, rank.wisdom) || !rank_item("experience", s, rank.experience) ||
        !rank_item("magic points", s, rank.magicpts) ||
        !rank_item("max weight carried", s, rank.maxweight) ||
        !rank_item("max. objects carried", s, rank.numobj) ||
        !rank_item("player kill value", s, rank.minpksl) ||
        !rank_item("task number", s, rank.tasks))
        return;

    if (rank.numobj > top_num_obj)
        top_num_obj = rank.numobj;

    s = skipspc(s);
    // Do we have a prompt?
    if (*s == '\"' || *s == '\'') {
        char *quot = s;
        s++;
        while (*s && *s != *quot)
            s++;
        quot++;
        *(s++) = 0;
        if (*quot) {
            rank.prompt = add_msg(nullptr);
            fwrite(quot, (size_t)(s - quot) + 1, 1, msgfp);
        }
    } else if (*s) {
        error("Bad prompt value for rank %d - missing quotes?\n", ranks);
        return;
    }
    if (rank.prompt == -1) {
        if (def_prompt_id == -1) {
            def_prompt_id = add_msg(nullptr);
            fwrite("> ", 3, 1, msgfp);
        }
        rank.prompt = def_prompt_id;
    }

    wizstr = rank.strength;

    fwrite(&rank, sizeof(rank), 1, ofp1);
}

void
sys_proc()
{
    char *next_line, *prev_line;

    nextc(1);
    next_line = data = cleanget();
    *g_adname = 0;
    mins = invis = invis2 = minsgo = rscale = tscale = port = -2;
    strcpy(g_logname, "smugl.log");
    fopenw(ranksfn);  // ofp1
    fopenw(synsifn);  // ofp2

    do {
        next_line = skipline(prev_line = next_line);
        if (*prev_line == 0) {
            // Blank line always ends a section and puts us back to
            // stOptions
            Section = stOptions;
            continue;
        }
        switch (Section) {
            case stOptions:
                option_line(prev_line);
                break;
            case stRanks:
                rank_line(prev_line);
                break;
            default:
                // We don't know this section type
                error("Internal Error: Unknown section type %d\n", Section);
                errabort();
                break;
        }
    } while (*next_line);

    // Finally; check for default values, etc
    if (!*g_adname)
        error("Missing adventure name.\n");
    if (mins == -2) {
        warne("No %s entry. Default %s enforced: %s.\n",
              sysopt[SO_SESH],
              "session time",
              "15 minutes");
        mins = 15;
    }
    if (invis == -2) {
        warne("No %s entry. Default %s enforced: %s.\n", sysopt[SO_SEE1], "seeinvis value", "100");
        invis = 100;
    }
    if (invis2 == -2) {
        warne("No %s entry. Default %s enforced: %s.\n", sysopt[SO_SEE2], "iseeinvis value", "100");
        invis2 = 100;
    }
    if (port == -2) {
        warne("No %s entry. Default %s enforced: %s\n", sysopt[SO_PORT], "login port", "9000");
        port = 9000;
    }
    if (minsgo == -2)
        warne("No %s entry. %s%s will be disabled.\n", sysopt[SO_MSGO], "SUPER", "GOing");
    if (rscale == -2)
        warne("No %s entry. %s%s will be disabled.\n",
              sysopt[SO_RSCL],
              "rank",
              "-scaling of values");
    if (tscale == -2)
        warne("No %s entry. %s%s will be disabled.\n",
              sysopt[SO_TSCL],
              "timed",
              "-scaling of values");

    errabort();  // Abort if an error
}

/* Check for the presence of the SYSTEM.txt file; if it doesn't exist,
 * and we don't already have an error, then create a file with defaults
 */
void
checksys()
{
    int w;
    sprintf(g_block, "%s%s.txt", g_dir, txtfile[TF_SYSTEM]);
    if ((ifp = fopen(g_block, "rb"))) {
        fclose(ifp);
        ifp = nullptr;
        return;
    }

    // If we've already encountered an error, then treat this
    // as an error also, rather than creating files all over
    // the place.
    if (err) {
        error("Missing: file %s!\n", g_block);
        return;
    }
    w = warn;
    warn = 0;
    warne("Missing: file %s!\n", g_block);
    if (!(ofp1 = fopen(g_block, "wb")))
        quit("Unable to create file %s.\n", g_block);
    warne("Creating DEFAULT %s.txt file.\n", g_block);
    fprintf(ofp1,
            ";\n"
            "; Default %s.txt\n"
            "; Generated by %s\n"
            ";\n"
            "; This file defines various run-time options and also defines ranks.\n"
            ";\n"
            "\n"
            "; Name of this adventure:\n"
            "%s=\"SMUGL Game\"\n\n"
            "; Length of a 'session' (minutes between resets)\n"
            "%s=30\n\n"
            "; Port number to run the game on\n"
            "%s=9000\n\n"
            "; Minimum rank that can 'SuperGo'. To 'SuperGo', you simply enter the\n"
            "; name of a room instead of a verb, and you are taken there immediately.\n"
            "%s=0\n\n"
            "; Determine the scaling rates of objects. 'Rank Scaling' allows you to have\n"
            "; a number of objects which are valuable to new players, but decreasingly\n"
            "; worthless the more experienced a player becomes. 'Time Scaling' allows\n"
            "; you to base an objects value on how far into the game you are.\n"
            "%s=40     ; Top-Rank players loose 40%% of scaled object values\n"
            "%s=50     ; Scaled objects loose 50%% of value at start of game\n\n"
            "; Specify rank required to see an invisible player if YOU are invisible\n"
            "%s=2\n\n"
            "; Specify rank required to see an insivible player when you are VISIBLE\n"
            "%s=3\n"
            "; These lines describe the local vocabulary of white noise and\n"
            "; 'special case' words that will be understood by the parser\n"
            "%s=\"the,a,an,at,as,that,to,for,from\"  ; Words we ignore\n"
            "%s=\"this,is,in,into,using,with,on,go\" ; Can have several lines of these\n\n"
            "; Identify the beginning of the RANKS section\n"
            "; See documentation for explanation of fields"
            "[ranks]\n"
            ";male  female score  str stam dext wsdm exp  mag max g obj pkill task prompt\n"
            ";     (= means same as male)\n"
            ";---------------------------------------------------------------------------\n"
            "newguy newgirl    0   25   30   30   30   0    0  4000   5    10    0 ': '\n"
            "bold   =        500   50   50   55   60  25   10  8000  10    50    0 ': '\n"
            "hero   heroine 1500   75   75   80   80  50   30 15000  20   100    1 '* '\n"
            "wizard witch   3000  100  100  100  100 100  100 50000  30  1000    2 '-* '\n"
            "\n"
            "; (Blank line terminates 'ranks:' section)\n"
            "\n"
            "; -- End of file --\n",
            txtfile[TF_SYSTEM],
            vername,
            sysopt[SO_NAME],
            sysopt[SO_SESH],
            sysopt[SO_PORT],
            sysopt[SO_MSGO],
            sysopt[SO_RSCL],
            sysopt[SO_TSCL],
            sysopt[SO_SEE1],
            sysopt[SO_SEE2],
            sysopt[SO_NOISE],
            sysopt[SO_NOISE]);

    fclose(ofp1);
    ofp1 = nullptr;
    warn = w;
}
