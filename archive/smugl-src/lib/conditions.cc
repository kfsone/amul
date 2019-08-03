/*
 * conditions.cpp -- The condition table
 */

static const char rcsid[] = "$Id: conditions.cc,v 1.4 1997/05/22 02:21:14 oliver Exp $";

#include "includes.hpp"
#include "structs.hpp"

/*
 * Current language/travel conditions:   (x=players name or ME)
 *
 * always (or -)        -       do this regardless
 * &            -       if the last condition was true...
 * ->           -       Always ..., then endparse
 * &>           -       And ..., then endparse
 * else         -       if the last condition was false...
 * else>                -       Else ..., then endparse
 * light                -       only if room has light source
 * ishere o     -       check object is here
 * myrank n     -       if my rank is n
 * state o n    -       check object state
 * mysex M|F    -       if my sex is male or female. You can put
 * the whole word, AMULCOM only checks the
 * first letter.
 * lastverb v   -       if last verb was v
 * lastdir      v       -       if last travel verb was v
 * lastroom r   -       if last room was r
 * asleep               -       if players is sleeping
 * sitting              -       if player is sitting down.
 * lying                -       if player is lying down.
 * rand n1 n2   -       if random(0<=n2<=rand) == n2
 * n2 can be ># or <#
 * rdmode <mode>        -       tests current RD mode... <see actions>
 * onlyuser     -       checks if only player on-line
 * alone                -       checks if the player is alone in the room
 * inroom room  -       checks which rooms player is in
 * opens o              -       If object is openable.
 * burns o              -       If object is flamable.
 * gotnothing   -       If player is carrying nothing.
 * carrying o   -       If player is carrying object.
 * nearto o     -       If object is carried by player or in room.
 * hidden               -       If others can see me
 * cangive o x  -       If player (x) can manage object... 'give'
 * because you want to know if the game can
 * GIVE the object to him... See?
 * infl p s     -
 * inflicted p s        -       If player <p> is inflicted by spell <s>
 * sameroom  p  -       If your in the same room as player <p>
 * someonehas o -       If obj <o> is being carried.
 * toprank              -       If your the top rank.
 * gota o s     -       If you are carrying an 'o' in state #s.
 * active <d>   -       Check if a daemon is active
 * timer <d> <n>        -       Check if a daemon has n seconds left.
 * (e.g <10 or >10)
 * container <o>        -       True if object is a container
 * empty <o>    -       True if object is empty
 * objsin o n   -       If there are [<|>] n objects in object.
 * helping x    -       If you are helping fred
 * givinghelp   -       If we are helping _anyone_
 * gothelp              -       If someone is helping you
 * stat <st> x <n>      -       If attribute of player x st > n.
 * objinv <o>   -       Checks if objects is invisible.
 * fighting <p> -       if player is fighting.
 * taskdone <t> -       if players done the task.
 * cansee <p>   -       If I can see player
 * visibleto <p>        -       If I am visible to that player
 * noun1 <o>    -       Compares noun1 with object
 * noun2 <o>    -       Compares noun2 with object
 * autoexits    -       True if autoexits is on
 * debug                -       True if debug mode is on
 * full st x    -       If player's stat is at maximum
 * time <numb>  -       Evaluates time (in seconds) remaining till reset
 * dec o                -       Decrement state of object & test for fail
 * inc o                -       Increment & test
 * lit o                -       Test if object is lit
 * fire o               -       If object has the 'fire' flag.
 * health x <numb>      -       Health in % of player x
 * magic lv po %        -       Checks for cast spell: Level Points & % chance.
 * (% is chance for level lvl. Toprank = 100%)
 * spell x      %       -       Checks player x defence  %. Used with magic.
 * in <room> <obj>      -       Checks if object is in room
 * exists <obj> -       Checks object hasn't been zonked etc.
 * canput o1 o2 -       Can I put <o1> into <o1>
 *
 */
struct ARGS cond[CONDITIONS] = {
    { "&", 0, { NONE } },
    { "-", 0, { NONE } },
    { "else", 0, { NONE } },
    { "always", 0, { NONE } },
    { "light", 0, { NONE } },
    { "ishere", 1, { PNOUN, 0, 0 } },
    { "myrank", 1, { PNUM, 0, 0 } },
    { "state", 2, { PNOUN, PNUM, 0 } },
    { "mysex", 1, { PSEX, 0, 0 } },
    { "lastverb", 1, { PVERB, 0, 0 } },
    { "lastdir", 1, { PVERB, 0, 0 } },
    { "lastroom", 1, { PROOM, 0, 0 } },
    { "asleep", 0, { NONE } },
    { "sitting", 0, { NONE } },
    { "lying", 0, { NONE } },
    { "rand", 2, { PNUM, PNUM } },
    { "rdmode", 1, { -2, 0, 0 } },
    { "onlyuser", 0, { NONE } },
    { "alone", 0, { NONE } },
    { "inroom", 1, { PROOM, 0, 0 } },
    { "opens", 1, { PNOUN, 0, 0 } },
    { "gotnothing", 0, { NONE } },
    { "carrying", 1, { PNOUN, 0, 0 } },
    { "nearto", 1, { PNOUN, 0, 0 } },
    { "hidden", 0, { NONE } },
    { "cangive", 2, { PNOUN, PPLAYER, 0 } },
    { "infl", 2, { PPLAYER, -3, 0 } },
    { "inflicted", 2, { PPLAYER, -3, 0 } },
    { "sameroom", 1, { PPLAYER, 0, 0 } },
    { "someonehas", 1, { PNOUN, 0, 0 } },
    { "toprank", 0, { NONE } },
    { "gota", 2, { PNOUN, PNUM, 0 } },
    { "active", 1, { PDAEMON, 0, 0 } },
    { "timer", 2, { PDAEMON, PNUM, 0 } },
    { "burns", 1, { PNOUN, 0, 0 } },
    { "container", 1, { PNOUN, 0, 0 } },
    { "empty", 1, { PNOUN, 0, 0 } },
    { "objsin", 2, { PNOUN, PNUM, 0 } },
    { "->", 0, { NONE } },
    { "&>", 0, { NONE } },
    { "helping", 1, { PPLAYER, 0, 0 } },
    { "gothelp", 0, { NONE } },
    { "givinghelp", 0, { NONE } },
    { "else>", 0, { NONE } },
    { "stat", 3, { -4, PPLAYER, PNUM } },
    { "objinv", 1, { PNOUN, 0, 0 } },
    { "fighting", 1, { PPLAYER, 0, 0 } },
    { "taskdone", 1, { PNUM, 0, 0 } },
    { "cansee", 1, { PPLAYER, 0, 0 } },
    { "visibleto", 1, { PPLAYER, 0, 0 } },
    { "noun1", 1, { PNOUN, 0, 0 } },
    { "noun2", 1, { PNOUN, 0, 0 } },
    { "autoexits", 0, { NONE } },
    { "debug", 0, { NONE } },
    { "full", 2, { -4, PPLAYER, 0 } },
    { "time", 1, { PNUM, 0, 0 } },
    { "dec", 1, { PNOUN, 0, 0 } },
    { "inc", 1, { PNOUN, 0, 0 } },
    { "lit", 1, { PNOUN, 0, 0 } },
    { "fire", 1, { PNOUN, 0, 0 } },
    { "health", 2, { PPLAYER, PNUM, 0 } },
    { "magic", 3, { PNUM, PNUM, PNUM } },
    { "spell", 2, { PPLAYER, PNUM, 0 } },
    { "in", 2, { PROOM, PNOUN, 0 } },
    { "exists", 1, { PNOUN, 0, 0 } },
    { "canput", 2, { PNOUN, PNOUN, 0 } },
};
