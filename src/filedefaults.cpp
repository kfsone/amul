#include <array>
#include <cstdio>
#include <string>
#include <string_view>

#include "filesystem.h"
#include "logging.h"

constexpr string_view DefaultTitleTxt = R""""(; Default "title.txt" file for AMUL.

; Specifies a name for this game.
name=\"Your MUD\"

; Specifies how many minutes a game runs before the world resets.
resettime=90

; What rank must a player be to be able to see invisible players.
seeinvis=3

; What rank must a player be to be able to see "super invisible" players.
seesuperinvis=5

; 'supergo' lets you teleport to a room by a '*' followed by the name of
; a room. This controls what rank players can use the command at, if ever.
sgorank=1

; Alter the value of objects based on players' ranks. Rank 1 gets 100%,
; but a player at the highest rank gets rankscale%.
rankscale=100

; Alter the value of objects based on how early in the game we are. This is
; to make it so that the more time there is left in a session, the less
; you reward player for using a swamp mechanism for points.
;
; Or: objects effectively get their peak value in the final minutes of a game.
timescale=100

; Everything following [title] is used as the game's splash screen.
[title]
This is an AMUL MUD. Enjoy.
)"""";

constexpr string_view DefaultRanksTxt = R""""(; Default "ranks.txt" file for AMUL.

; You can create as many ranks as you like. Players gain ranks by accumulating score and, if implemented,
; completing tasks/achievements.

; Each rank has a male and female version of the name, or '=' in the female line to re-use the male name.
; A rank line contains:
;   male-name female-name score-required strength stamina dexterity wisdom experience magic max-weight max-objects kill-value task-required prompt
; max-weight is assumed to be in grammes (I'm european)

;male           female       score  str sta dex wis exp mag   maxwt maxobj kval task  prompt 
adventurer      adventuress      0   50  50  50  50   0   1   20000      8   10    0  "* "
hero            heroine        100   75  75  75  75  50   5   50000     10   25    0  "* "
legend          =             1000  100 100 100 100 100  25  100000     15   50    0  "~~* "
)"""";

constexpr string_view DefaultUmsgTxt = R""""(; Default 'umsg.txt' file for AMUL.

; Any messages/responses you will re-use frequently or that require more than one line can be
; assigned a name here.

msgid=hint
Welcome to my MUD - a Multi User Dungeon. Use directional verbs (north, northeast, up, down...) or
their abbreviations (n, ne, ...) to move around the world.
)"""";

constexpr string_view DefaultObDescsTxt = R""""(; Default 'obdescs.txt' file for AMUL.

; You can assign names to object descriptions here in-case you need multi-line descriptions or
; want to ensure descriptions are shared between certain objects.

; msgid=torch_lit
; There is a torch on the floor, shining brightly.
;
; msgid=torch_dark
; There is an unlit torch on the floor.
)"""";

constexpr string_view DefaultNPCsTxt = R""""(; Default 'npcs.txt' file for AMUL.

; Non-player characters (NPCs or 'mobiles') comprise two parts in AMUL: A "persona" which outlines
; the behavior of the NPCs, and objects to which they are attached. This is the file where the
; NPC persona is defined.

;;; TODO: Default
)"""";

constexpr string_view DefaultSynsTxt = R""""(; Default 'syns.txt' file for AMUL.

; Define synonyms (aliases) for words.

; Layout is simply: <verb | object>  <synonym [... synonym]>
north       n
northeast   ne
east        e
southeast   se
south       s
southwest   sw
west        w
northwest   nw
)"""";

constexpr string_view DefaultLangTxt = R""""(; Default 'lang.txt' file for AMUL.
;;; TODO: Rename 'verbs.txt'

; This is where you declare verbs. You can tell the game which verbs are primarily intended
; for travelling with a "travel=<verb ... verb>" line.

travel=north northeast east southeast south southwest west northwest up down in out

; lang.txt is where we associate patterns of user input with the actions we want those to be
; interpreted as. We declare verbs here and break them up into "syntax=" lines that match
; upto 5 input "token slots" for type and/or value.

verb=quit
syntax=none     ; user typed "quit" with no parameters
    reply "If you're sure..."
    quit        ; this will ask them to confirm they are sure
; 
)"""";

constexpr string_view DefaultRoomsTxt = R""""(; Default 'rooms.txt' file for AMUL.

; Rooms are defined in blocks separated by (wholly) blank lines.
;
;  [room=]<room short name>  [<flags>]
;  [short description]
;  [long description]

; A simple room with lighting and in which players will start.
room=beginning startloc light
	Beginning of path.
	You're standing at the beginning of an exciting adventure with heroes and villains,
	puzzles and mysteries. Someone just has to create it. The path leads ahead to the north.

room=adventure light
	On the path.
	Congratulations! You have started your adventure with AMUL. Enjoy creating your game(s)!

; A room for top-rank players / GMs
room=destiny light hide hideaway noexits nolook
	Halls of Destiny.
	This place is unreachable to all but the most powerful heroes, and you.

; A place to move objects out of play -- players aren't supposed to be able to get here.
; Note that it does not have a description.
room=secretstore dark nolook hide hideaway noexits nogo death
)"""";

constexpr string_view DefaultTravelTxt = R""""(; Default 'travel.txt' file for AMUL.

room=beginning
verbs=north out
	adventure		; moves to the room 'adventure'

room=adventure
verbs=south in
	beginning
verbs=southwest west northwest north northeast east southeast up down out
	error "The game hasn't been written yet..."
)"""";


constexpr string_view DefaultObjectsTxt = R""""(; Default 'objects.txt' file for AMUL.

; Objects are things which may have some form of presence in one or more rooms. Obviously
; tangible items like weapons or items are "objects" but so would things like doors and
; intangible items like the weather.

; 'rain' is something players can't pick up (scenery) but the game will understand the
; word as something they can interact with. It has no description in the first (State 0)
; state, but setting it to state 1 will cause it to be described as "It is raining".
; Note that it is in two locations
object=rain scenery
rooms=beginning adventure
	desc=""               ; state 0: no-description
	desc="It is raining"  ; state 1: describe actively

; A simple torch players can use to create light. It is dark in state 0 but
; illuminated in state 1. Because this is not a scenery object, each state has
; values for: weight, value, durability and mele dmg
object=torch adj=silver shines
rooms=adventure
	weight=10 value=20 health=50 dmg=5 desc="On the floor is an unlit, silver torch."
	weight=10 value=25 health=50 dmg=5 desc="On the floor shines a silver torch." lit
)"""";

constexpr string_view DefaultSysMsgTxt = R""""(; Default 'sysmsg.txt' file for AMUL.

; This file lets the game developer define strings for all of the built-in
; instances where the game needs to send text to the players.
;
; For legacy reasons, each System Message is named "$1" thru "$75".
;
; If the first line of the body of a message is indented, the same indent
; will be removed from subsequent lines.


$1
	Can't enter game: reset in progress.

$2
	Can't enter game: too many players.

;;;TODO: Use '\' instead of '{'
; When a message ends with '{' no newline is appended.
$3
	Press ENTER: {

$4
	Enter character name: {

$5
	Invalid name, please choose another.

$6
	That name is reserved, please choose another.

$7
	Someone just attempted to log in as you (@me).

$8
	"@me" is already in the game.

$9
	"@me" will be a new character, is that correct (Y/N)? {

$10
	Will "@me" be (M)ale or (F)emale? {

$11
	Please enter either M or F.

$12
	Password: {

$13
	Password must be bettern 3 and 8 characters long.

$14
	(If you're not sure of the answer to this, choose N)
	Do you want the game to send ANSI codes (Y/N)? {

$15
	@me the @mr has been created, welcome to the game.

$16
	Too many login failures, disconnecting.

$17
	Note: %u failed login attempts since you last played.

$18
	Welcome back, @me the @mr! You have now played @gp times.

$19
	ANSI decoration enabled. If you see lots of "[31m"s etc, your client does not support ANSI.

$20
	@me the @mr has joined the game.

$21
	AMUL Test Game help
 
	Movement commands: N, S, E, W, NE, SE, SW, NW, U, D, IN, O.	

$22  ; Say goodbye to a first time player (I recommend you remind them of their password)
	Farewell, @me. Your password is "@pw". 

$23
	@me logged out.

$24
 
	You have died...
 

$25
	@me has died.

$26
	Your spell fizzled.

$27			; when listing other players online (who/verbose)
	 is playing{

$28
	DING: You are now @me the @mr.

$29
	Saving at %u points.

$30
	Confirm quit [y/N]: {

$31
	It is now too dark to see.

$32
	It is now light enough to see.

$33
	It is too dark to see.

$34			; When the player tries to 'WHAT' in a dark room.
	It is too dark to make anything out.

$35			; When WHAT sees nothing in a hideaway.
	You can see nothing special here.

$36			; How to describe another player in the room (first %s = name, second %s = rank)
	%s the %s is here{

$37			; When the reset happens.
	--|> Something magical is happening <|--

$38			; Parser unable to interpret input
	I don't understand that.

$39			; You tried to move in a direction you can't.
	You can't go @vb from here.

$40			; Something you tried to but can't do.
	You can't do that.

$41			; You've just done a supergo
	Loading ... Please wait.

$42			; When a player leaves (supergo)
	@me suddenly isn't here anymore.

$43			; When a player arrives (supergo)
	@me appeared suddenly, out of nowhere.

$44			; First word was understood, but not a verb.
	I didn't understand the way you put that.

$45			; Understood the verb, but not the rest
	I don't understand what you're referring to.

$46			; Understood the verb, but no matching syntax lines
	Understood the words, but not the meaning.

$47			; You've just been summoned
	--|> You have been magically summoned <|--

$48			; When a player leaves (summoned)
	@me suddenly isn't here anymore.

$49			; When a player arrives (summoned)
	@me appears suddenly, out of nowhere.

$50
	@me has just woken up.

$51
	You are suddenly awake!

$52			; Fail to summon a player from this location.
	%s is already here.

$53			; Player exited room by travel.
	@me has just left.

$54			; Player entered room by travel.
	@me has just arrived.

$55
	There is not enough room for two in there. 

$56
	Someone else just tried to enter.

$57			; Gender changed by admin
	You have been turned into a @gn by a spell.

; * NOTE
;
;   The text in the fight-sequence where the opponent DEFENDS against a
;   blow is sent BY THE ATTACKING player. Therefore you should use
;
;	You defend against an attack from @me
;   not
;	You defend against an attack from @pl
;
; *NW = No Weapon
; *WP = Weapon

$58			; YOU hit @pl				*NW
	You strike @pl with your bare hands.

$59			; tell @pl he blocked YOUR attack	*NW
	You manage to parry a blow from @xx.

$60			; YOU hit @pl				*WP
	You score a direct hit on @pl with your @o1.

$61			; YOU WOULD have hit, but @pl blocked	*WP
	Using your @o2 you manage to block a feeble attack from @xx.

$62			; tell @pl YOU struck			*NW
	@xx manages to score a hefty blow on you!

$63			; @pl blocked YOUR attack		*NW
	@pl easily blocks a clumsy lunge from you.

$64			; tell @pl YOU struck			*WP
	@xx strikes you with @go @o1!

$65			; @pl blocked your attack		*WP
	@pl easily blocks a feeble attack from you with @go @g2.

$66			; You miss - dimwit			*NW
	You attack @pl but miss.

$67			; tell @pl you missed			*NW
	@xx attacks you but misses.

$68			; Wizard message (when a user makes it, that is)
	Congratulations! You have become @me the @mr! Permadeath no-longer applies to this character, and you will have access to many new powers.

$69			; Scored enough points but not enough tasks for new rank
	You have not completed the required task(s) to reach the next level.

$70
	You can see nothing.

$71			;;;TODO: WTF is this?
	{

$72
	You can make nothing out.

$73
	Fighting is forbidden here.

$74
	You aren't powerful to cast that spell, you'll have to wait until you reach a higher rank.

$75
	You don't have enough magical power to cast the spell.
)"""";

std::array<pair<string_view, string_view>, 11> DefaultFiles{
    make_pair("title.txt", DefaultTitleTxt),
    make_pair("ranks.txt", DefaultRanksTxt),
    make_pair("sysmsg.txt", DefaultSysMsgTxt),
    make_pair("umsg.txt", DefaultUmsgTxt),
    make_pair("obdescs.txt", DefaultObDescsTxt),
    make_pair("npcs.txt", DefaultNPCsTxt),
    make_pair("rooms.txt", DefaultRoomsTxt),
    make_pair("objects.txt", DefaultObjectsTxt),
    make_pair("travel.txt", DefaultTravelTxt),
    make_pair("lang.txt", DefaultLangTxt),
    make_pair("syns.txt", DefaultSynsTxt),
};

void
CreateDefaultFiles()
{
    for (auto &pair : DefaultFiles) {
        std::string filepath{};
        safe_gamedir_joiner(pair.first);
        if (FILE *fp = fopen(filepath.c_str(), "r"); fp != nullptr) {
            fclose(fp);
            continue;
        }
        LogInfo("Creating default ", pair.first);
        FILE *fp = fopen(filepath.c_str(), "w");
        if (fp == nullptr) {
            LogFatal("Cannot create default ", pair.first);
        }
        fwrite(pair.second.data(), sizeof(char), pair.second.length(), fp);
        fclose(fp);
    }
}

