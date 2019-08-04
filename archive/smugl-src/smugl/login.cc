// Handle the process of getting the users name (and password),
// and preparing the user to enter the game
// Starts where the player's connection has already been accepted,
// ends when the players name/passwd has been confirmed, and the
// player has been shown all introductory text, and moved into their
// start location

static const char rcsid[] = "$Id: login.cc,v 1.11 1999/06/08 15:36:50 oliver Exp $";

#include <cctype>
#include <cstring>

#include "consts.hpp"
#include "io.hpp"
#include "ipc.hpp"
#include "libprotos.hpp"
#include "login.hpp"
#include "misc.hpp"
#include "ranks.hpp"
#include "rooms.hpp"
#include "smugl.hpp"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

char new_name[NAMEL + 1];

static inline int getname();
static inline int newid();

void login()  // Log a player in
{
    // Initialise player variables
    // iverb = iadj1 = inoun1 = iadj2 = inoun2 = actor = -1;
    int i;
    int fails = -1;

    // Force re-initialisation of the random seed
    // The parent already kicked it way back for us,
    // so we make use of that (rand) and the other random
    // factor, *our* process id
    srand(rand() + getpid());

    for (int tries = 3; fails == -1 && tries; tries--) {
        me->reset();          // Reset all player features
        fails = -1;           // Set default flags, etc
        if (getname() == -1)  // Get player's name
            continue;

        // if (!findpers())         // Existing persona?
        //    fails = newid();    // No - can we/should we create this?
        // else fails = getpasswd(); // otherwise check the password

        fails = 0;
        break;
    }

    if (fails == -1)  // Not a success
    {
        tx(message(FAILEDLOGIN), '\n');
        if (debug)
            syslog(LOG_INFO, "didn't bother logging in");
        exit(0);
    }

    // Determine what rank we are. By working down through the
    // ranks, we ensure the player gets the highest rank given
    // their current standing. This also means that if a player
    // fails to complete a given task, they can't get up to a
    // higher rank.
    // Some people may find this behaviour undesirable -- if a
    // game has it's scorings changed, players ranks get dropped.
    myRank = RankIdx::top_rank();
    for (i = data->ranks - 1; i >= 0; i--, myRank--) {
        if (me->score >= myRank->score)
        // && taskcnt(Af) >= myRank->tasks
        {
            me->set_rank(i);
            break;
        }
    }

    // If we have enough score for the next level, we obviously haven't
    // met the neccesary task requirements.
    if (myRank < RankIdx::top_rank() && me->score >= (myRank + 1)->score) {
        if (debug)
            syslog(LOG_INFO,
                   "%s has %ld points for rank %ld but not %ld",
                   me->_name,
                   me->score,
                   myRank->number(),
                   (myRank + 1)->number());
        tx(message(NOTASK), '\n');
    }

    me->state = PLAYING;  // We're in the game now
    me->std_flags = 0;    // XXX: really?
    me->flags = 0;        // XXX: really?
    me->add_name();       // Add my name into the userbase

    me->plays++;        // I'm playing again
    if (me->plays > 1)  // Hey - I've seen you before!
        tx(message(WELCOMEBAK));

    // Try and find a start location
    // XXX: Surely this should be a 'RoomIdx::' function?
    class Room *start_room = data->anterm;
    if (start_room == NULL) {
        long which_start = rand() % data->start_rooms;
        class RoomIdx Iteration;
        class Room *first = NULL;
        class Room *cur = Iteration.current();
        while (!start_room && cur) {
            if (cur->flags & STARTL && which_start-- == 0) {
                start_room = cur;
                break;
            } else if (!first && !(cur->std_flags & bob_DEATH))
                first = cur;
            cur = Iteration.next();
        }
        if (!start_room && first)
            start_room = first;
        else if (!start_room && !(start_room = RoomIdx::locate("start"))) {  // Last ditch effort
            tx(">> No start room. Can't enter the game.\n");
            syslog(LOG_WARNING, "no start rooms in game");
            exit(0);
        }
    }

    // XXX: These should be covered in Player::reset() ???
    // XYZ: But possibly not because we don't want to clober
    // XYZ: all the other variables, do we?
    me->wield = -1;
    me->helping = 0;
    me->helped = 0;
    me->following = 0;
    me->followed = 0;
    me->fighting = 0;
    me->hadlight = 0;
    me->light = 0;
    me->sctg = 0;

    if (me->flags & ufANSI) {
        ans("1m");
        tx(message(ANSION));
        ans("0m");
    }

    // calcdext();
    // me->last_session = him.last_session;
    // save_me();
    // txs(*(errtxt-6),vername);
    // if ((i = isverb("!start")) != -1)
    //    lang_proc(i,0);

    // Make sure we're flagged as not having visited any rooms
    unsigned long clr_bit = ~me->bitmask;
    for (i = 0; i < data->rooms; i++)
        data->roombase[i].visitor_bf &= clr_bit;

    // We found a start location, so we're OK
    if (me->plays > 1)
        tx(message(WELCOMEBAK), '\n');
    else
        tx(message(YOUBEGIN), '\n');
    announce(ALLBUT(slot), COMMENCED);
    me->locations = 1;
    start_room->enter();
}

static inline int getname()  // Get the player's name
{
    char *p = new_name;

    prompt(WHATNAME);
    fetch_input(new_name, NAMEL);
    txc('\n');
    if (!new_name[0])
        return -1;
    while (*p) {          // Validate the input string
        if (isspace(*p))  // Replace spaces with hyphens
        {
            *(p++) = '-';
            continue;
        }
        if (!isalnum(*(p++)) || *p == '_') {  // No non-alphanum characters
            // XXX: Hardwired message
            tx("^GInvalid character name - try another.\n");
            return -1;
        }
    }
    if (strlen(new_name) < 3) {  // Must be at least 3 characters long
        tx(message(LENWRONG), '\n');
        return -1;
    }

    // If they enter a word that we already know, it means
    // something in the game is already using the word.
    vocid_t name_id = is_word(new_name);
    if (name_id != -1) {
        // If it's another player, tell the other player someone
        // just tried to use their name, and reject this login.
        if (class Player *other = PlayerIdx::locate(name_id)) {
            announce(to_MASK(other->number()), LOGINASME);
            strcpy(me->_name, new_name);
            tx(message(ALREADYIN), '\n');
        } else  // Otherwise it's a game 'object'
            tx(message(NAME_USED), '\n');
        return -1;
    }

    // This name choice is fine and valid, accept the login
    strcpy(me->_name, new_name);

    return 0;
}

static inline int newid()  // Create a new user
{
    me->reset();
    strcpy(me->_name, new_name);

    prompt(CREATE);  // Create a new user?
    fetch_input(input, 3);
    if (toupper(*input) != 'Y')
        return -1;

    do  // Choose gender
    {
        prompt(WHATGENDER);
        fetch_input(input, 2);
        *input = toupper(*input);
        if (*input != 'M' && *input != 'F')
            tx(message(GENDINVALID), '\n');
    } while (*input != 'M' && *input != 'F');
    me->sex = (*input == 'M') ? MALE : FEMALE;

    prompt(ENTERPASSWD);  // Ask the user for a password
    fetch_input(input, -20);
    if (!*input)
        return -1;
    strcpy(me->passwd, input);

    // Capitalise the name properly
    me->_name[0] = toupper(me->_name[0]);
    for (u_int i = 1; i < strlen(me->_name); i++) {
        me->_name[i] = (me->_name[i - 1] == ' ') ? toupper(me->_name[i]) : tolower(me->_name[i]);
    }

    //    flagbits();
    me->rec = -1;
    ShowFile("scenario.text");  // Show the introduction
    pressret();
    tx(message(YOUBEGIN), '\n');  // Explain who they start as
    txc('\n');                    // spare, blank line

    sem_lock(sem_PLYR_FILE);  // Lock the player-data file
    //    him.name[0]=0;		// Find first free record
    //    {
    //    struct _PLAYER mx;
    //    mx=*me;
    //    if(findpers())
    //        me->rec--;
    //    *me=mx;
    //    }
    sem_unlock(sem_PLYR_FILE);
    me->last_session = 0;
    return 0;
}

#ifdef NEVER
int getpasswd(void)  // Existing user - verify password
{
    // Give the user three attempts at entering the right password.
    for (i = 0; i < 3; i++) {
        txprintf("\nTry #%d -- ", i + 1);
        prompt(ENTERPASSWD);
        fetch_input(input, -20);
        if (!*input)
            return -1;
        if (strcmp(input, me->passwd) == 0)
            break;
    }

    if (i == 3)  // We used up all 3 attempts
    {
        tx(message(TRIESOUT));  // Update bad try count
        me->tries++;
        // save_me();
        exit(0);
    }

    if (me->tries > 0) {
        ans("1m");
        txc(0x7);
        txc('\n');
        txn(acp(FAILEDTRIES), me->tries);
        txc('\n');
        ans("0m");
    }

    me->tries = 0;
    return 0;
}
#endif /* NEVER */
