/*
 * actions.cpp -- Defines the action table
 */

#include "includes.hpp"
#include "structs.hpp"

/*
 * Current language/travel actions:
 *
 * quit         -       Leave the adventure
 * save         -       save players current status
 * look         -       describe current room.
 * what         -       list 'what' is in the current room
 * score <type> -       display players current status
 * type can be either brief or verbose.
 * state                -       set objects state
 * where                -       explain 'where' an object is
 * who <type>   -       list who is playing
 * type can be either brief or verbose.
 * treatas      <verb>  -       Process in same way as <verb>
 * finishparse  -       Disregards rest of users input.
 * abortparse   -       Stops parsing this phrase. See note *1
 * failparse    -       Stops parsing this phrase. See note *2
 * killme               -       Kill the player. (Reincarnatable)
 * endparse     -       Stops parsing this phrase.
 * wait <n>     -       Waits for <n> seconds
 * get <noun>   -       Take an object
 * drop <noun>  -       Drop the damned thing
 * invent               -       List what you are carrying!
 * whereami     -       Tell me which room I am in!
 * print <umsg> -       Send a USER message to the user
 * respond <umsg>       -       Same, but also executes an ENDPARSE
 * error <umsg> -       Same, but also executes a FAILPARSE
 * announce x m -       Send noise msg to "x" group of users:
 * global   - everyone (-me)
 * everyone - ALL players
 * outside  - everyone outside room (-me)
 * here     - everyone in this room
 * others   - everyone in this room (-me)
 * action x m   -       Used to send movement/comment messages.
 * send o r     -       Send object to room
 * changesex p  -       Toggle players sex between m/f
 * sleep/wake   -       Puts player to sleep/wakes player up.
 * sit/stand    -       Makes player sit down/Stand up.
 * lie          -       Make the player lie down.
 * rdmode <mode>        -       Set room descriptions mode...
 * Brief   = only give short descriptions
 * verbose = ALWAYS give LONG descriptions
 * rmcnt   = RooMCouNT. Long description is
 * (default)  given first time player enters
 * the room.
 * Note: Look ALWAYS gives a long description.
 * reset                -       Resets the game
 * move r               -       move player to room. no messages sent
 * (to allow teleportation etc).
 * travel               -       jump to the travel table
 * announceto r m       -       Announce to a specific room (m=umsg)
 * actionto r m -       Action in a specific room
 * announcefrom o m-}   sends message to all players near or holding
 * actionfrom o m       -}      'o' _EXCEPT_ yourself.
 * objannounce o m      -       Same as above, but includes yourself.
 * objaction o m        -       ditto, but sends a 'quiet' message.
 * tell p t     -       Tell player something.
 * addval o     -       Add value of object to score
 * give o x     -       give object to player
 * inflict x s  -       Inflict player with spell
 * cure x s     -       Remove spell (s) from player (x)
 * summon p     -       summon player <p>
 * add n st p   -       Adds <n> points to player <p> stat <st>
 * sub n st p   -       Minus <n> points to player <p> stat <st>
 * fix st p     -       Fixes stat <st> to player <p> minimum level
 * checknear o  -       Check nearto object, else complain & endparse
 * checkget o   -       Check object for getting. else complain & enparse
 * destroy o    -       Destroy an object.
 * recover o    -       Recover a destroyed object.
 * start d      n       -       Start a private daemon in n seconds time...
 * gstart d n   -       Start a global daemon in n seconds time...
 * cancel d     -       Cancel a daemon
 * begin d              -       Force a daemon to happen now
 * showtimer d  -       Displays the time left on a daemon
 * contents o   -       Shows the contents of an object
 * force x <str>        -       Force player to do something!
 * help x               -       Assist player (only one at a time)
 * stophelp     -       Stop helping whoever we are helping
 * objinvis o   -       Make an object invisible
 * objvis o     -       Make an object visible
 * fight x              -       Start fight routine
 * combat               -       Does the hit and damage stuff.
 * hit x                -       Takes a single turn at combat
 * flee         -       End fight routine
 * log <text>   -       Write the <text> to the aman log file.
 * wield o              -       Use an object for fighting.
 * follow x     -       Follow player.
 * lose                 -       Stop player following you.
 * stopfollow   -       Stop following player.
 * exits                -       Shows exits in location.
 * settask              -       Sets a players task.
 * showtasks    -       Shows which tasks player has completed.
 * syntax n1 n2 -       Sets new noun1 & noun2
 * setpre x text        -       Sets a players pre-rank description
 * setpost x text       -       Sets a players post-rank description
 * setarr x text        -       Sets a players 'arrival' string
 * setdep x text        -       Sets a players depature string
 * do verb              -       Calls 'verb' as a subroutine
 * interact x   -       Sets current interactor
 * senddaemon x d n     Sends daemon (d) to player (x) in N seconds
 * autoexits on|off-    enables/disables auto-exits
 * burn o               -       Sets an object alight
 * douse o              -       Extinguishes an object
 * inc o                -       Same as condition but no return
 * dec o                -       Same as condition but no return
 * maketop n    -       Blast to TopRank <n=Fail% for min rank>
 * deduct p n   -       Reduces player p score by n percent.
 * damage o n   -       Inflict n points of damage on object o.
 * repair o n   -       Repair n points of damage on object o.
 * blast o t1 t2        -       "Blast object".cansee get t1, cant get t2.
 * provoke o n  -       Provoke mobile. add n to fight%.
 * randomgo to  -       To is 'start' or 'any'.
 * put o1 o2    -       Put object 1 into object 2
 *
 *
 * Note: <umsg> can be replaced by a text string in quotes.
 */
ARGS action[ACTIONS] = {
    { "quit", 0, { NONE } },
    { "save", 0, { NONE } },
    { "score", 1, { -5, 0, 0 } },
    { "state", 2, { PNOUN, PNUM, 0 } },
    { "look", 0, { NONE } },
    { "what", 0, { NONE } },
    { "where", 1, { PNOUN, 0, 0 } },
    { "who", 1, { -5, 0, 0 } },
    { "treatas", 1, { PVERB, 0, 0 } },
    { "print", 1, { PUMSG, 0, 0 } },
    { "skip", 1, { PNUM, 0, 0 } },
    { "end", 0, { NONE } },
    { "killme", 0, { NONE } },
    { "finish", 0, { NONE } },
    { "abort", 0, { NONE } },
    { "fail", 0, { NONE } },
    { "wait", 1, { PNUM, 0, 0 } },
    { "hit", 1, { PPLAYER, 0, 0 } },
    { "whereami", 0, { NONE } },
    { "send", 2, { PNOUN, PROOM, 0 } },
    { "announce", 2, { -1, PUMSG, 0 } },
    { "get", 1, { PNOUN, 0, 0 } },
    { "drop", 1, { PNOUN, 0, 0 } },
    { "invent", 0, { NONE } },
    { "randomgo", 1, { -7, 0, 0 } },
    { "changesex", 1, { PPLAYER, 0, 0 } },
    { "provoke", 2, { PMOBILE, PNUM, 0 } },
    { "blast", 3, { PNOUN, PUMSG, PUMSG } },
    { "sit", 0, { NONE } },
    { "stand", 0, { NONE } },
    { "lie", 0, { NONE } },
    { "rdmode", 1, { -2, 0, 0 } },
    { "reset", 0, { NONE } },
    { "action", 2, { -1, PUMSG, 0 } },
    { "move", 1, { PROOM, 0, 0 } },
    { "travel", 0, { NONE } },
    { "announceto", 2, { PROOM, PUMSG, 0 } },
    { "actionto", 2, { PROOM, PUMSG, 0 } },
    { "announcefrom", 2, { PNOUN, PUMSG, 0 } },
    { "actionfrom", 2, { PNOUN, PUMSG, 0 } },
    { "tell", 2, { PPLAYER, PUMSG, 0 } },
    { "put", 2, { PNOUN, PNOUN, 0 } },
    { "give", 2, { PNOUN, PPLAYER, 0 } },
    { "inflict", 2, { PPLAYER, -3, 0 } },
    { "cure", 2, { PPLAYER, -3, 0 } },
    { "summon", 1, { PPLAYER, 0, 0 } },
    { "add", 3, { PNUM, -4, PPLAYER } },
    { "sub", 3, { PNUM, -4, PPLAYER } },
    { "checknear", 1, { PNOUN, 0, 0 } },
    { "checkget", 1, { PNOUN, 0, 0 } },
    { "destroy", 1, { PNOUN, 0, 0 } },
    { "recover", 1, { PNOUN, 0, 0 } },
    { "start", 2, { PDAEMON, PNUM, 0 } },
    { "cancel", 1, { PDAEMON, 0, 0 } },
    { "begin", 1, { PDAEMON, 0, 0 } },
    { "showtimer", 1, { PDAEMON, 0, 0 } },
    { "objannounce", 2, { PNOUN, PUMSG, 0 } },
    { "objaction", 2, { PNOUN, PUMSG, 0 } },
    { "contents", 1, { PNOUN, 0, 0 } },
    { "force", 2, { PPLAYER, PUMSG, 0 } },
    { "help", 1, { PPLAYER, 0, 0 } },
    { "stophelp", 0, { NONE } },
    { "fix", 2, { -4, PPLAYER, 0 } },
    { "objinvis", 1, { PNOUN, 0, 0 } },
    { "objvis", 1, { PNOUN, 0, 0 } },
    { "fight", 1, { PPLAYER, 0, 0 } },
    { "flee", 0, { NONE } },
    { "log", 1, { PUMSG, 0, 0 } },
    { "combat", 0, { NONE } },
    { "wield", 1, { PNOUN, 0, 0 } },
    { "follow", 1, { PPLAYER, 0, 0 } },
    { "lose", 0, { NONE } },
    { "stopfollow", 0, { NONE } },
    { "exits", 0, { NONE } },
    { "settask", 1, { PNUM, 0, 0 } },
    { "showtasks", 0, { NONE } },
    { "syntax", 2, { PREAL, PREAL, 0 } },
    { "setpre", 2, { PPLAYER, PUMSG, 0 } },
    { "setpost", 2, { PPLAYER, PUMSG, 0 } },
    { "senddaemon", 3, { PPLAYER, PDAEMON, PNUM } },
    { "do", 1, { PVERB, 0, 0 } },
    { "interact", 1, { PPLAYER, 0, 0 } },
    { "autoexits", 1, { -6, 0, 0 } },
    { "setarr", 2, { PPLAYER, PUMSG, 0 } },
    { "setdep", 2, { PPLAYER, PUMSG, 0 } },
    { "respond", 1, { PUMSG, 0, 0 } },
    { "error", 1, { PUMSG, 0, 0 } },
    { "burn", 1, { PNOUN, 0, 0 } },
    { "douse", 1, { PNOUN, 0, 0 } },
    { "inc", 1, { PNOUN, 0, 0 } },
    { "dec", 1, { PNOUN, 0, 0 } },
    { "maketop", 1, { PNUM, 0, 0 } },
    { "deduct", 2, { PPLAYER, PNUM, 0 } },
    { "damage", 2, { PNOUN, PNUM, 0 } },
    { "repair", 2, { PNOUN, PNUM, 0 } },
    { "gstart", 2, { PDAEMON, PNUM, 0 } },
    { "extend", 1, { PNUM, 0, 0 } },
};
