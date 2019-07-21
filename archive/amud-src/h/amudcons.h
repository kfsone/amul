/*
              ****    AMUD.CONS.H.....Adventure Compiler    ****
              ****                constants                 ****

									   */

#ifdef COMPILER
#define	NRFLAGS		17
#define NRNULL		1
char 	*rflag[NRFLAGS]={	/* Remember to update amud.defs.h */
	"light","dmove","startloc","randobjs","dark","small","death","nolook",
	"silent","hide","sanctuary","hideaway","peaceful","noexits","anteroom",
	"nogo"
};

#define	NOFLAGS		8
char	*obflags1[NOFLAGS]={
	"opens","scenery","counter","flamable","shines","fire","invis","smell"
};

#define	NOPARMS		5
char	*obparms[NOPARMS]={
	"adj=","start=","holds=","put=","mobile="
};

#define	NSFLAGS		7
char	*obflags2[NSFLAGS]={
	"lit","open","closed","weapon","opaque","scaled","alive"
};
#endif

#ifndef AMAN
#define	NPUTS		4
char	*obputs[NPUTS]={
	"in","on","behind","under"
};

#define NPREP		6
char	*prep[NPREP]={
	"in","on","behind","under","from","with"
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
				the whole word, AMCOM only checks the
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
	exists <obj>	-	Checks object hasn't been zonked etc.

*/
#define NCONDS		65
char	ncop[NCONDS]={		/* No. of params for each condition */
	0,		0,		0,		0,
	0,		1,		1,		2,
	1,		1,		1,		1,
	0,		0,		0,		2,
	1,		0,		0,		1,
	1,		0,		1,		1,
	0,		2,		2,		2,
	1,		1,		0,		2,
	1,		2,		1,		1,
	1,		2,		0,		0,
	1,		0,		0,		0,
	3,		1,		1,		1,
	1,		1,		1,		1,
	0,		0,		2,		1,
	1,		1,		1,		1,
	2,		3,		2,		2,
	1
};
#ifdef COMPILER
char	*conds[NCONDS]={
	"&",		"-",		"else",		"always",
	"light",	"ishere",	"myrank",	"state",
	"mysex",	"lastverb",	"lastdir",	"lastroom",
	"asleep",	"sitting",	"lying",	"rand",
	"rdmode",	"onlyuser",	"alone",	"inroom",
	"opens",	"gotnothing",	"carrying",	"nearto",
	"hidden",	"cangive",	"infl",		"inflicted",
	"sameroom",	"someonehas",	"toprank",	"gota",
	"active",	"timer",	"burns",	"container",
	"empty",	"objsin",	"->",		"&>",
	"helping",	"gothelp",	"givinghelp",	"else>",
	"stat",		"objinv",	"fighting",	"taskdone",
	"cansee",	"visibleto",	"noun1",	"noun2",
	"autoexits",	"debug",	"full",		"time",
	"dec",		"inc",		"lit",		"fire",
	"health",	"magic",	"spell",	"in",
	"exists"
};
char	tcop[NCONDS][3]={	/* Type of Parameters */
	NONE, 		NONE, 		NONE, 		NONE,
	NONE,		PNOUN,0,0,	PNUM,0,0,	PNOUN,PNUM,0,
	PSEX,0,0, 	PVERB,0,0,	PVERB,0,0,	PROOM,0,0,
	NONE,		NONE,		NONE,		PNUM,PNUM,0,
	-2,0,0,		NONE,		NONE,		PROOM,0,0,
	PNOUN,0,0,	NONE,		PNOUN,0,0,	PNOUN,0,0,
	NONE,		PNOUN,PPLAYER,0,PPLAYER,-3,0,	PPLAYER,-3,0,
	PPLAYER,0,0,	PNOUN,0,0,	NONE,		PNOUN,PNUM,0,
	PDAEMON,0,0,	PDAEMON,PNUM,0,	PNOUN,0,0,	PNOUN,0,0,
	PNOUN,0,0,	PNOUN,PNUM,0,	NONE,		NONE,
	PPLAYER,0,0,	NONE,		NONE,		NONE,
	-4,PPLAYER,PNUM,PNOUN,0,0,	PPLAYER,0,0,	PNUM,0,0,
	PPLAYER,0,0,	PPLAYER,0,0,	PNOUN,0,0,	PNOUN,0,0,
	NONE,		NONE,		-4,PPLAYER,0,	PNUM,0,0,
	PNOUN,0,0,	PNOUN,0,0,	PNOUN,0,0,	PNOUN,0,0,
	PPLAYER,PNUM,0,	PNUM,PNUM,PNUM,	PPLAYER,PNUM,0,	PROOM,PNOUN,0,
	PNOUN,0,0
};
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
	finish		-	Disregards rest of users input.
	abort		-	Stops parsing this phrase. See note *1
	fail		-	Stops parsing this phrase. See note *2
	killme		-	Kill the player. (Reincarnatable)
	end		-	Stops parsing this phrase.
	wait <n>	-	Waits for <n> seconds
	get <noun>	-	Take an object
	drop <noun>	-	Drop the damned thing
	invent		-	List what you are carrying!
	whereami	-	Tell me which room I am in!
	print <umsg>	-	Send a USER message to the user
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
	tell p t	-	Tell player something.
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
	maketop n	-	Blast to TopRank <n=Fail% for min rank>
	deduct p n	-	Reduces player p score by n percent.
	damage o n	-	Inflict n points of damage on object o.
	repair o n	-	Repair n points of damage on object o.
	blast o t1 t2	-	"Blast object".cansee get t1, cant get t2.
	provoke o n	-	Provoke mobile. add n to fight%.
	randomgo to	-	To is 'start' or 'any'.


Note: <umsg> can be replaced by a text string in quotes.
*/
#define	NACTS		99
#ifndef FRAME
char	nacp[NACTS]={		/* No. of params for each action */
	0,		0,		1,		2,
	0,		0,		1,		1,
	1,		1,		1,		0,
	0,		0,		0,		0,
	1,		0,		0,		2,
	2,		1,		1,		0,
	1,		1,		0,		0,
	0,		0,		0,		1,
	0,		2,		1,		0,
	2,		2,		2,		2,
	2,		1,		2,		2,
	2,		1,		3,		3,
	1,		1,		1,		1,
	2,		1,		1,		1,
	2,		2,		1,		2,
	1,		0,		2,		1,
	1,		1,		0,		1,
	0,		1,		1,		0,
	0,		0,		1,		0,
	2,		2,		2,		3,
	1,		1,		1,		2,
	2,		1,		1,		1,
	1,		1,		1,		1,
	2,		2,		2,		2,
	3,		2,		1
};
#endif
#ifdef COMPILER
char	*acts[NACTS]={
	"quit",		"save",		"score",	"state",
	"look",		"what",		"where",	"who",
	"treatas",	"print",	"skip",		"end",
	"killme",	"finish",	"abort",	"fail",
	"wait",		",",		"whereami",	"send",
	"announce",	"get",		"drop",		"invent",
	",",		"changesex",	",",		",",
	"sit",		"stand",	"lie",		"rdmode",
	"reset",	"action",	"move",		"travel",
	"announceto",	"actionto",	"announcefrom",	"actionfrom",
	"tell",		",",		"give",		"inflict",
	"cure",		"summon",	"add",		"sub",
	"checknear",	"checkget",	"destroy",	"recover",
	"start",	"cancel",	"begin",	"showtimer",
	"objannounce",	"objaction",	"contents",	"force",
	"help",		"stophelp",	"fix",		"objinvis",
	"objvis",	"fight",	"flee",		"log",
	"combat",	"wield",	"follow",	"lose",
	"stopfollow",	"exits",	"settask",	"showtasks",
	"syntax",	"setpre",	"setpost",	"senddaemon",
	"do",		"interact",	"autoexits",	"setarr",
	"setdep",	"respond",	"error",	"burn",
	"douse",	"inc",		"dec",		"maketop",
	"deduct",	"damage",	"repair",	"gstart",
	"blast",	"provoke",	"randomgo"
};
char	tacp[NACTS][3]={	/* Type of params... */
	NONE,		NONE,		-5,0,0,		PNOUN,PNUM,0,
	NONE,		NONE,		PNOUN,0,0,	-5,0,0,
	PVERB,0,0,	PUMSG,0,0,	PNUM,0,0,	NONE,
	NONE,		NONE,		NONE,		NONE,
	PNUM,0,0,	NONE,		NONE,		PNOUN,PROOM,0,
	-1,PUMSG,0,	PNOUN,0,0,	PNOUN,0,0,	NONE,
	NONE,		PPLAYER,0,0,	NONE,		NONE,
	NONE,		NONE,		NONE,		-2,0,0,
	NONE,		-1,PUMSG,0,	PROOM,0,0,	NONE,
	PROOM,PUMSG,0,	PROOM,PUMSG,0,	PNOUN,PUMSG,0,	PNOUN,PUMSG,0,
	PPLAYER,PUMSG,0,PNOUN,0,0,	PNOUN,PPLAYER,0,PPLAYER,-3,0,
	PPLAYER,-3,0,	PPLAYER,0,0,	PNUM,-4,PPLAYER,PNUM,-4,PPLAYER,
	PNOUN,0,0,	PNOUN,0,0,	PNOUN,0,0,	PNOUN,0,0,
	PDAEMON,PNUM,0,	PDAEMON,0,0,	PDAEMON,0,0,	PDAEMON,0,0,
	PNOUN,PUMSG,0,	PNOUN,PUMSG,0,	PNOUN,0,0,	PPLAYER,PUMSG,0,
	PPLAYER,0,0,	NONE,		-4,PPLAYER,0,	PNOUN,0,0,
	PNOUN,0,0,	PPLAYER,0,0,	NONE,		PUMSG,0,0,
	NONE,		PNOUN,0,0,	PPLAYER,0,0,	NONE,
	NONE,		NONE,		PNUM,0,0,	NONE,
	PREAL,PREAL,0,	PPLAYER,PUMSG,0,PPLAYER,PUMSG,0,PPLAYER,PDAEMON,PNUM,
	PVERB,0,0,	PPLAYER,0,0,	-6,0,0,		PPLAYER,PUMSG,0,
	PPLAYER,PUMSG,0,PUMSG,0,0,	PUMSG,0,0,	PNOUN,0,0,
	PNOUN,0,0,	PNOUN,0,0,	PNOUN,0,0,	PNUM,0,0,
	PPLAYER,PNUM,0,	PNOUN,PNUM,0,	PNOUN,PNUM,0,	PDAEMON,PNUM,0,
	PNOUN,PUMSG,PUMSG,PMOBILE,PNUM,0,-7,0,0
};
#endif

#ifdef COMPILER
#define nsynts 11
char *syntax[nsynts]={
	"none","any","noun","adj","player","room","syn","text","verb","class","number"
};
short int syntl[nsynts]={	/* Length of --^ */
	4,3,4,3,6,4,3,4,4,5,6
};
#endif

char	advfn[]   ="Main.CMP";			/* Game profile	*/
char	rooms2fn[]="Rd.CMP";			/* Descriptions	*/
#ifndef COMPILER
char	plyrfn[]  ="PlayerData";		/* User Details	*/
#endif
#ifndef FRAME
char	rooms1fn[]="R1.CMP";			/* Room blocks	*/
char	ranksfn[] ="R2.CMP";			/* Rank details	*/
char	ttfn[]    ="T1.CMP";			/* T.T. Entries	*/
char	ttpfn[]   ="T2.CMP";			/* TT Paramtrs	*/
char	lang1fn[] ="V1.CMP";			/* Verb blocks	*/
char	lang2fn[] ="V2.CMP";			/* Slot Tables	*/
char	lang3fn[] ="V3.CMP";			/* Verb Table	*/
char	lang4fn[] ="V4.CMP";			/* VT Parms	*/
char	synsfn[]  ="S1.CMP";			/* Synonyms	*/
char	synsifn[] ="S2.CMP";			/* Syns Index	*/
char	ntabfn[]  ="N1.CMP";			/* Noun table	*/
char	obdsfn[]  ="O1.CMP";			/* Object descs	*/
char	objsfn[]  ="O2.CMP";			/* Detail!!!	*/
char	objrmsfn[]="O3.CMP";			/* Object rooms	*/
char	statfn[]  ="O4.CMP";			/* Obj. States	*/
char	adjfn[]   ="A1.CMP";			/* Adjectives	*/
char	umsgifn[] ="U1.CMP";			/* UMsgs Index	*/
char	umsgfn[]  ="U2.CMP";			/* Umsg text	*/
char	mobfn[]   ="M1.CMP";			/* Mobile data	*/
#endif
