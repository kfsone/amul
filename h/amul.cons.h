/*
			  ****    AMUL.CONS.H.....Adventure Compiler    ****
			  ****                constants                 ****

*/

#ifdef COMPILER
#	define NRFLAGS 15
#	define NRNULL 1
const char *rflag[NRFLAGS] = /* Remember to update amul.defs.h */
	{
		"light",  "dmove",  "startloc", "randobjs",  "dark",	 "small",	"death",
		"nolook", "silent", "hide",		"sanctuary", "hideaway", "peaceful", "noexits",
};

#	define NOFLAGS 8
const char *obflags1[NOFLAGS] = {
	"opens", "scenery", "counter", "flamable", "shines", "fire", "invis", "smell",
};

#	define NOPARMS 5
const char *obparms[NOPARMS] = {
	"adj=", "start=", "holds=", "put=", "mobile=",
};

#	define NSFLAGS 7
const char *obflags2[NSFLAGS] = {
	"lit", "open", "closed", "weapon", "opaque", "scaled", "alive",
};
#endif

#ifndef AMAN
#	define NPUTS 4
const char *obputs[NPUTS] = {
	"In",
	"On",
	"Behind",
	"Under",
};

#	define NPREP 6
const char *prep[NPREP] = {
	"in", "on", "behind", "under", "from", "with",
};
#endif

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
#define NCONDS 64
const char ncop[NCONDS] = /* No. of params for each condition */
	{0, 0, 0, 0, 0, 1, 1, 2, 1, 1, 1, 1, 0, 0, 0, 2, 1, 0, 0, 1, 1, 0,
	 1, 1, 0, 2, 2, 2, 1, 1, 0, 2, 1, 2, 1, 1, 1, 2, 0, 0, 1, 0, 0, 0,
	 3, 1, 1, 1, 1, 1, 1, 1, 0, 0, 2, 1, 1, 1, 1, 1, 2, 3, 2, 2};
#ifdef COMPILER
const char *conds[NCONDS] = {
	"&",		  "-",			"else",		"always",	"light",	"ishere",   "myrank",
	"state",	  "mysex",		"lastverb", "lastdir",   "lastroom", "asleep",   "sitting",
	"lying",	  "rand",		"rdmode",   "onlyuser",  "alone",	"inroom",   "opens",
	"gotnothing", "carrying",   "nearto",   "hidden",	"cangive",  "infl",	 "inflicted",
	"sameroom",   "someonehas", "toprank",  "gota",		 "active",   "timer",	"burns",
	"container",  "empty",		"objsin",   "->",		 "&>",		 "helping",  "gothelp",
	"givinghelp", "else>",		"stat",		"objinv",	"fighting", "taskdone", "cansee",
	"visibleto",  "noun1",		"noun2",	"autoexits", "debug",	"full",	 "time",
	"dec",		  "inc",		"lit",		"fire",		 "health",   "magic",	"spell",
	"in"};

const char tcop[NCONDS][3] = /* Type of Parameters */
	{NONE,
	 NONE,
	 NONE,
	 NONE,
	 NONE,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NUM,
	 0,
	 0,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 CAP_GENDER,
	 0,
	 0,
	 CAP_VERB,
	 0,
	 0,
	 CAP_VERB,
	 0,
	 0,
	 CAP_ROOM,
	 0,
	 0,
	 NONE,
	 NONE,
	 NONE,
	 CAP_NUM,
	 CAP_NUM,
	 0,
	 -2,
	 0,
	 0,
	 NONE,
	 NONE,
	 CAP_ROOM,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 CAP_NOUN,
	 CAP_PLAYER,
	 0,
	 CAP_PLAYER,
	 -3,
	 0,
	 CAP_PLAYER,
	 -3,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 CAP_DAEMON_ID,
	 0,
	 0,
	 CAP_DAEMON_ID,
	 CAP_NUM,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 NONE,
	 NONE,
	 CAP_PLAYER,
	 0,
	 0,
	 NONE,
	 NONE,
	 NONE,
	 -4,
	 CAP_PLAYER,
	 CAP_NUM,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 CAP_NUM,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 NONE,
	 -4,
	 CAP_PLAYER,
	 0,
	 CAP_NUM,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_PLAYER,
	 CAP_NUM,
	 0,
	 CAP_NUM,
	 CAP_NUM,
	 CAP_NUM,
	 CAP_PLAYER,
	 CAP_NUM,
	 0,
	 CAP_ROOM,
	 CAP_NOUN,
	 0};
#endif

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
#define NACTS 96
const char nacp[NACTS] = /* No. of params for each action */
	{0, 0, 1, 2, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 2, 2, 1, 1, 0,
	 1, 1, 0, 0, 0, 0, 0, 1, 0, 2, 1, 0, 2, 2, 2, 2, 2, 1, 2, 2, 2, 1, 3, 3,
	 1, 1, 1, 1, 2, 1, 1, 1, 2, 2, 1, 2, 1, 0, 2, 1, 1, 1, 0, 1, 0, 1, 1, 0,
	 0, 0, 1, 0, 2, 2, 2, 3, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 0, 2, 2, 2, 2};
#ifdef COMPILER
const char *acts[NACTS] = {
	"quit",		  "save",		 "score",		 "state",	  "look",		"what",
	"where",	  "who",		 "treatas",		 "message",	"skip",		"endparse",
	"killme",	 "finishparse", "abortparse",   "failparse",  "wait",		"bleep",
	"whereami",   "send",		 "announce",	 "get",		   "drop",		"invent",
	"reply",	  "changesex",   "sleep",		 "wake",	   "sit",		"stand",
	"lie",		  "rdmode",		 "reset",		 "action",	 "move",		"travel",
	"announceto", "actionto",	"announcefrom", "actionfrom", "tell",		"addval",
	"give",		  "inflict",	 "cure",		 "summon",	 "add",		"sub",
	"checknear",  "checkget",	"destroy",		 "recover",	"start",		"cancel",
	"begin",	  "showtimer",   "objannounce",  "objaction",  "contents",  "force",
	"help",		  "stophelp",	"fix",			 "objinvis",   "objvis",	"fight",
	"flee",		  "log",		 "combat",		 "wield",	  "follow",	"lose",
	"stopfollow", "exits",		 "settask",		 "showtasks",  "syntax",	"setpre",
	"setpost",	"senddaemon",  "do",			 "interact",   "autoexits", "setarr",
	"setdep",	 "respond",	 "error",		 "burn",	   "douse",		"inc",
	"dec",		  "toprank",	 "deduct",		 "damage",	 "repair",	"gstart"};
const char tacp[NACTS][3] = /* Type of params... */
	{NONE,
	 NONE,
	 -5,
	 0,
	 0,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 NONE,
	 NONE,
	 CAP_NOUN,
	 0,
	 0,
	 -5,
	 0,
	 0,
	 CAP_VERB,
	 0,
	 0,
	 CAP_UMSG,
	 0,
	 0,
	 CAP_NUM,
	 0,
	 0,
	 NONE,
	 NONE,
	 NONE,
	 NONE,
	 NONE,
	 CAP_NUM,
	 0,
	 0,
	 CAP_NUM,
	 0,
	 0,
	 NONE,
	 CAP_NOUN,
	 CAP_ROOM,
	 0,
	 -1,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 CAP_UMSG,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 NONE,
	 NONE,
	 NONE,
	 NONE,
	 NONE,
	 -2,
	 0,
	 0,
	 NONE,
	 -1,
	 CAP_UMSG,
	 0,
	 CAP_ROOM,
	 0,
	 0,
	 NONE,
	 CAP_ROOM,
	 CAP_UMSG,
	 0,
	 CAP_ROOM,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 CAP_UMSG,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 CAP_PLAYER,
	 0,
	 CAP_PLAYER,
	 -3,
	 0,
	 CAP_PLAYER,
	 -3,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 CAP_NUM,
	 -4,
	 CAP_PLAYER,
	 CAP_NUM,
	 -4,
	 CAP_PLAYER,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_DAEMON_ID,
	 CAP_NUM,
	 0,
	 CAP_DAEMON_ID,
	 0,
	 0,
	 CAP_DAEMON_ID,
	 0,
	 0,
	 CAP_DAEMON_ID,
	 0,
	 0,
	 CAP_NOUN,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 CAP_UMSG,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 NONE,
	 -4,
	 CAP_PLAYER,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 NONE,
	 CAP_UMSG,
	 0,
	 0,
	 NONE,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 NONE,
	 NONE,
	 NONE,
	 CAP_NUM,
	 0,
	 0,
	 NONE,
	 CAP_REAL,
	 CAP_REAL,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_PLAYER,
	 CAP_DAEMON_ID,
	 CAP_NUM,
	 CAP_VERB,
	 0,
	 0,
	 CAP_PLAYER,
	 0,
	 0,
	 -6,
	 0,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_PLAYER,
	 CAP_UMSG,
	 0,
	 CAP_UMSG,
	 0,
	 0,
	 CAP_UMSG,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 CAP_NOUN,
	 0,
	 0,
	 NONE,
	 CAP_PLAYER,
	 CAP_NUM,
	 0,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 CAP_NOUN,
	 CAP_NUM,
	 0,
	 CAP_DAEMON_ID,
	 CAP_NUM,
	 0};
#endif

#ifdef COMPILER
#	define nsynts 12
const char *	syntax[nsynts] = {"none", "any", "noun", "adj",  "prep",  "player",
							  "room", "syn", "text", "verb", "class", "number"};
const short int syntl[nsynts] = /* Length of --^ */
	{4, 3, 4, 3, 4, 6, 4, 3, 4, 4, 5, 6};
#endif

const char advfn[] = "Prof.CMP";   /* Game profile	*/
const char rooms2fn[] = "RDs.CMP"; /* Descriptions	*/
#ifndef COMPILER
const char managerName[] = "AMUL Manager Port"; /* MU driver port */
const char plyrfn[] = "Players Data";			/* User Details	*/
#endif

const char rooms1fn[] = "Rms.CMP"; /* Room blocks	*/
const char ranksfn[] = "Rks.CMP";  /* Rank details	*/
const char ttfn[] = "TT.CMP";	  /* T.T. Entries	*/
const char ttpfn[] = "TTP.CMP";	/* TT Paramtrs	*/
const char lang1fn[] = "Vbs.CMP";  /* Verb blocks	*/
const char lang2fn[] = "ST.CMP";   /* Slot Tables	*/
const char lang3fn[] = "VT.CMP";   /* Verb Table	*/
const char lang4fn[] = "VTP.CMP";  /* VT Parms	*/
const char synsfn[] = "Syns.CMP";  /* Synonyms	*/
const char synsifn[] = "SynI.CMP"; /* Syns Index	*/
const char ntabfn[] = "Ntb.CMP";   /* Noun table	*/
const char obdsfn[] = "Ods.CMP";   /* Object descs	*/
const char objsfn[] = "Obs.CMP";   /* Detail!!!	*/
const char objrmsfn[] = "Ors.CMP"; /* Object rooms	*/
const char statfn[] = "Oss.CMP";   /* Obj. States	*/
const char adjfn[] = "Adjs.CMP";   /* Adjectives	*/
const char umsgifn[] = "UMI.CMP";  /* UMsgs Index	*/
const char umsgfn[] = "UMT.CMP";   /* Umsg text	*/
const char mobfn[] = "Mbs.CMP";	/* Mobile data	*/
const char mobcmdfn[] = "Mcm.CMP"; /* Mob cmds	*/
