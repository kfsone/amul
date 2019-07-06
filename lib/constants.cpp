#include "h/amul.incs.h"
#include "h/amul.vars.h"

struct ACTUAL actual[NACTUALS] = {"verb", IWORD + IVERB, TC_VERB, "adj", IWORD + IADJ1, TC_ADJ, "adj1", IWORD + IADJ1,
        TC_ADJ, "noun", IWORD + INOUN1, TC_NOUN, "noun1", IWORD + INOUN1, TC_NOUN, "player", IWORD + INOUN1, TC_PLAYER,
        "player1", IWORD + INOUN1, TC_PLAYER, "text", IWORD + INOUN1, TC_TEXT, "text1", IWORD + INOUN1, TC_TEXT, "room",
        IWORD + INOUN1, TC_ROOM, "room1", IWORD + INOUN1, TC_ROOM, "number", IWORD + INOUN1, WC_NUMBER, "number1",
        IWORD + INOUN1, WC_NUMBER, "prep", IWORD + IPREP, TC_PREP, "adj2", IWORD + IADJ2, TC_ADJ, "noun2",
        IWORD + INOUN2, TC_NOUN, "player2", IWORD + INOUN2, TC_PLAYER, "text2", IWORD + INOUN2, TC_TEXT, "room2",
        IWORD + INOUN2, TC_ROOM, "locate", MEPRM + LOCATE, TC_NOUN, "me", MEPRM + SELF, TC_PLAYER, "here", MEPRM + HERE,
        TC_ROOM, "myrank", MEPRM + RANK, WC_NUMBER, "friend", MEPRM + FRIEND, TC_PLAYER, "helper", MEPRM + HELPER,
        TC_PLAYER, "enemy", MEPRM + ENEMY, TC_PLAYER, "weapon", MEPRM + WEAPON, TC_NOUN, "myscore", MEPRM + SCORE,
        WC_NUMBER, "mysctg", MEPRM + SCTG, WC_NUMBER, "mystr", MEPRM + STR, WC_NUMBER, "lastroom", MEPRM + LASTROOM,
        TC_ROOM, "lastdir", MEPRM + LASTDIR, TC_VERB, "lastverb", MEPRM + LASTVB, TC_VERB};

/*
    Current language/travel conditions:   (x=players name or ME)

    always (or -) 	-	do this regardless
    &		-	if the last condition was true...
    ->		-	Always ..., then endparse
    &>		-	And ..., then endparse
    else		-       if the last condition was false...
    else>		-	Else ..., then endparse
    light		-	only if room has light source
    ishere o	-	check object is here
    myrank n	-	if my rank is n
    state o n	-	check object state
    mysex M|F	-       if my sex is male or female. You can put
                the whole word, AMULCOM only checks the
                first letter.
    lastverb v	-       if last verb was v
    lastdir	 v	-	if last travel verb was v
    lastroom r	-       if last room was r
    asleep		-	if players is sleeping
    sitting		-	if player is sitting down.
    lying		-	if player is lying down.
    rand n1 n2	-	if random(0<=n2<=rand) == n2
                n2 can be ># or <#
    rdmode <mode>	-	tests current RD mode... <see actions>
    onlyuser	-	checks if only player on-line
    alone		-	checks if the player is alone in the room
    inroom room	-	checks which rooms player is in
    opens o		-	If object is openable.
    burns o		-	If object is flamable.
    gotnothing	-	If player is carrying nothing.
    carrying o	-	If player is carrying object.
    nearto o	-	If object is carried by player or in room.
    hidden		-	If others can see me
    cangive o x	-	If player (x) can manage object... 'give'
                because you want to know if the game can
                GIVE the object to him... See?
    infl p s	-
    inflicted p s	-	If player <p> is inflicted by spell <s>
    sameroom  p	-	If your in the same room as player <p>
    someonehas o	-	If obj <o> is being carried.
    toprank		-	If your the top rank.
    gota o s	-	If you are carrying an 'o' in state #s.
    active <d>	-	Check if a daemon is active
    timer <d> <n>	-	Check if a daemon has n seconds left.
                (e.g <10 or >10)
    container <o>	-	True if object is a container
    empty <o>	-	True if object is empty
    objsin o n	-	If there are [<|>] n objects in object.
    helping x	-	If you are helping fred
    givinghelp	-	If we are helping _anyone_
    gothelp		-	If someone is helping you
    stat <st> x <n>	-	If attribute of player x st > n.
    objinv <o>	-	Checks if objects is invisible.
    fighting <p>	-	if player is fighting.
    taskdone <t>	-	if players done the task.
    cansee <p>	-	If I can see player
    visibleto <p>	-	If I am visible to that player
    noun1 <o>	-	Compares noun1 with object
    noun2 <o>	-	Compares noun2 with object
    autoexits	-	True if autoexits is on
    debug		-	True if debug mode is on
    full st x	-	If player's stat is at maximum
    time <numb>	-	Evaluates time (in seconds) remaining till reset
    dec o		-	Decrement state of object & test for fail
    inc o		-	Increment & test
    lit o		-	Test if object is lit
    fire o		-	If object has the 'fire' flag.
    health x <numb>	-	Health in % of player x
    magic lv po %	-	Checks for cast spell: Level Points & % chance.
                (% is chance for level lvl. Toprank = 100%)
    spell x	%	-	Checks player x defence  %. Used with magic.
    in <room> <obj>	-	Checks if object is in room

*/
// No. of params for each condition
extern const uint8_t ncop[NCONDS] = {0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 1, 0, 0, 0, 2, 1, 0, 0, 1, 1, 0, 1, 1, 0, 2, 2, 2,
        1, 1, 0, 2, 1, 2, 1, 1, 1, 2, 0, 0, 1, 0, 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 1, 1, 1, 1, 1, 2, 3, 2, 2};

/*
    Current language/travel actions:

    quit		-	Leave the adventure
    save		-	save players current status
    look		-	describe current room.
    what		-	list 'what' is in the current room
    score <type>	-	display players current status
                type can be either brief or verbose.
    state		-	set objects state
    where		-	explain 'where' an object is
    who <type>	-	list who is playing
                type can be either brief or verbose.
    treatas	<verb>	-	Process in same way as <verb>
    finishparse	-	Disregards rest of users input.
    abortparse	-	Stops parsing this phrase. See note *1
    failparse	-	Stops parsing this phrase. See note *2
    killme		-	Kill the player. (Reincarnatable)
    endparse	-	Stops parsing this phrase.
    wait <n>	-	Waits for <n> seconds
    bleep <n>	-	Waits 1 scnd prints '.' and repeats n times.
    get <noun>	-	Take an object
    drop <noun>	-	Drop the damned thing
    invent		-	List what you are carrying!
    whereami	-	Tell me which room I am in!
    message <umsg>	-	Send a USER message to the user
    replyPort <umsg>	-	 " " " " " " " " " " " " " " "
    respond <umsg>	-	Same, but also executes an ENDPARSE
    error <umsg>	-	Same, but also executes a FAILPARSE
    announce x m	-	Send noise msg to "x" group of users:
                   global   - everyone (-me)
                   everyone - ALL players
                   outside  - everyone outside room (-me)
                   here     - everyone in this room
                   others   - everyone in this room (-me)
    action x m	-	Used to send movement/comment messages.
    send o r	-	Send object to room
    changesex p	-	Toggle players sex between m/f
    sleep/wake	-	Puts player to sleep/wakes player up.
    sit/stand	-	Makes player sit down/Stand up.
    lie		-	Make the player lie down.
    rdmode <mode>	-	Set room descriptions mode...
                   Brief   = only give short descriptions
                   verbose = ALWAYS give LONG descriptions
                   rmcnt   = RooMCouNT. Long description is
                 (default)  given first time player enters
                            the room.
                 Note: Look ALWAYS gives a long description.
    reset		-	Resets the game
    move r		-	move player to room. no messages sent
                  (to allow teleportation etc).
    travel		-	jump to the travel table
    announceto r m	-	Announce to a specific room (m=umsg)
    actionto r m	-	Action in a specific room
    announcefrom o m-}	sends message to all players near or holding
    actionfrom o m	-}	'o' _EXCEPT_ yourself.
    objannounce o m	-	Same as above, but includes yourself.
    objaction o m	-	ditto, but sends a 'quiet' message.
    tell p t	-	Tell player to piss of home.
    addval o	-	Add value of object to score
    give o x	-	give object to player
    inflict x s	-	Inflict player with spell
    cure x s	-	Remove spell (s) from player (x)
    summon p	-	summon player <p>
    add n st p	-	Adds <n> points to player <p> stat <st>
    sub n st p	-	Minus <n> points to player <p> stat <st>
    fix st p	-	Fixes stat <st> to player <p> minimum level
    checknear o	-	Check nearto object, else complain & endparse
    checkget o	-	Check object for getting. else complain & enparse
    destroy o	-	Destroy an object.
    recover o	-	Recover a destroyed object.
    start d	n	-	Start a private daemon in n seconds time...
    gstart d n	-	Start a global daemon in n seconds time...
    cancel d	-	Cancel a daemon
    begin d		-	Force a daemon to happen now
    showtimer d	-	Displays the time left on a daemon
    contents o	-	Shows the contents of an object
    force x <str>	-	Force player to do something!
    help x		-	Assist player (only one at a time)
    stophelp	-	Stop helping whoever we are helping
    objinvis o	-	Make an object invisible
    objvis o	-	Make an object visible
    fight <p>	-	Start fight routine
    flee		-	End fight routine
    log <text>	-	Write the <text> to the aman log file.
    combat		-	Does the to hit and damage stuff.
    wield o		-	Use an object for fighting.
    follow x	-	Follow player.
    lose 		-	Stop player following you.
    stopfollow 	-	Stop following player.
    exits		-	Shows exits in location.
    settask		-	Sets a players task.
    showtasks	-	Shows which tasks player has completed.
    syntax n1 n2	-	Sets new noun1 & noun2
    setpre x text	-	Sets a players pre-rank description
    setpost x text	-	Sets a players post-rank description
    setarr x text	-	Sets a players 'arrival' string
    setdep x text	-	Sets a players depature string
    do verb		-	Calls 'verb' as a subroutine
    interact x	-	Sets current interactor
    senddaemon x d n	Sends daemon (d) to player (x) in N seconds
    autoexits on|off-	enables/disables auto-exits
    burn o		-	Sets an object alight
    douse o		-	Extinguishes an object
    inc o		-	Same as condition but no return
    dec o		-	Same as condition but no return
    toprank		-	Make the player the toprank.
    deduct p n	-	Reduces player p score by n percent.
    damage o n	-	Inflict n points of damage on object o.
    repair o n	-	Repair n points of damage on object o.


Note: <umsg> can be replaced by a text string in quotes.
*/
// No. of params for each action
extern const uint8_t nacp[NACTS] = {0, 0, 1, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 2, 2, 1, 1, 0, 1, 1, 0, 0,
        0, 0, 0, 1, 0, 2, 1, 0, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 3, 3, 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 1, 2, 1, 0, 2, 1, 1,
        1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 2, 2, 2, 3, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2};

const char *obputs[NPUTS] = {
        "In",
        "On",
        "Behind",
        "Under",
};

const char *prep[NPREP] = {
        "in",
        "on",
        "behind",
        "under",
        "from",
        "with",
};
