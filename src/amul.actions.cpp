#include <array>

#include "amul.actions.h"
#include "amulinc.h"

#include "amul.acts.h"
#include "amul.algorithms.h"
#include "amul.msgs.h"
#include "amul.stringmanip.h"
#include "client.io.h"
#include "game.h"
#include "parser.context.h"
#include "playerflag.h"
#include "spinlock.h"
#include "typedefs.h"
#include "users.h"

using namespace std::string_literals;

namespace Action
{

////////////////////////////////////////////////////////////////////////////////
// NAME
// 	aabortparse -- ends the current parse and any additional phrases that
//  were part of the same parse. e.g.
//   west, north, south
//  abortparse during 'west' would stop north and south being parsed.
//
// SEE ALSO
// 	aabortparse, afailparse, aendparse
//
void
AbortParse()
{
    wtype[0] = wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = WNONE;
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = last_him = last_her = last_it = -1;
    EndParse();
    more = 0;
}

////////////////////////////////////////////////////////////////////////////////
// Cancel giving or receiving help.
//
void
CancelHelp()
{
    static auto notReceiving = GetNamedString("help.end-recv", "@me is no-longer helping you.");
    static auto notHelping = GetNamedString("help.end-give", "You are no-longer helping @me.");

    if (t_avatar->helping != WNONE) {
        PrintSlot(t_avatar->helping, notReceiving.data());
        GetAvatar(t_avatar->helping).helped &= ~(1 << t_slotId);
        t_avatar->helping = WNONE;
    }

    for (slotid_t i = 0; i < MAXU; i++) {
        if (GetAvatar(i).helping == t_slotId) {
            PrintSlot(i, notHelping.data());
            GetAvatar(i).helping = WNONE;
        }
    }
    t_avatar->helped = WNONE;
}

////////////////////////////////////////////////////////////////////////////////
//
void
DescribeInventory()
{
    Print(MakeInventory(t_slotId, true) + "\n");
}

////////////////////////////////////////////////////////////////////////////////
// Ends the current parse.
//
void
EndParse()
{
    donet = ml + 1;
}

////////////////////////////////////////////////////////////////////////////////
// 	Prevents continued parseing of the current phrase. Any
// 	other phrases will be parsed, but this won't. Used for
// 	looped-parses caused by such as 'All' and noun-classes.
//
// SEE ALSO
// 	AbortParse, FinishParse, EndParse
//
void
FailParse()
{
    EndParse();
    donet = ml + 1;
    ml = -1;
    failed = true;
}

////////////////////////////////////////////////////////////////////////////////
// NAME
// 	afinishparse -- unused
//
// SYNOPSIS
//
// FUNCTION
//
// SEE ALSO
// 	aabortparse, afailparse, aendparse
//
void
FinishParse()
{
    // unclear what this was supposed to do, I think it's actually
    // just an empty placeholder.
    // EndParse(); ?
}

////////////////////////////////////////////////////////////////////////////////
// NAME
// 	LoseFollower -- Get rid of the person following us.
//
// SYNOPSIS
// 	void LoseFollower( void );
//
// FUNCTION
// 	Unhooks the player who WAS following us (if there was one) and
// 	tells them they can no-longer follow us.
//
// SEE ALSO
// 	StopFollowing(), Follow(), DoThis()
//
void
LoseFollower()
{
    if (t_avatar->followed == -1)
        return;
    GetAvatar(t_avatar->followed).following = -1;  // Unhook them
    static const auto str = GetNamedString("follow.stop", "You are no-longer following @mf.\n");
    PrintSlot(t_avatar->followed, str.data());
    t_avatar->followed = -1;  // Unhook me
}

////////////////////////////////////////////////////////////////////////////////
// Move player to room, testing ligthing!
void
MovePlayerTo(roomid_t roomId)
{
    // Set the players current lighting to NONE and then test lighting for
    // the room. Then move the player and restore his lighting; test again.
    StopFollowing();  // interrupts any follow we are performing.

    // Mark the player as being in-transit.
    /// TODO: MESSAGE_CODE
    t_avatar->flags = t_avatar->flags | PFMOVING;

    // Remember where we were previously.
    lroom = t_avatar->room;

    auto i = t_avatar->light;
    t_avatar->light = 0;

    lighting(t_slotId, AOTHERS);

    t_avatar->room = roomId;
    t_avatar->light = i;
    t_avatar->hadlight = 0;
    lighting(t_slotId, AOTHERS);
    look(t_avatar->room, t_character->rdmode);
    t_avatar->flags &= ~PFMOVING;
}

////////////////////////////////////////////////////////////////////////////////
// Print something that might be contain an 'iword' value.
//
void
PrintText(amulid_t id)
{
    if (id & IWORD) {
        // Replace with no. of a users word
        switch (id ^ IWORD) {
            case INOUN2:
                Print("@n2\n");
				break;
            default:
                Print("@n1\n");
				break;
        }
        return;
    }

    id = GetConcreteValue(id);
    if (id == -1 || id == -2 || id > amulid_t(g_game.numStrings))
        return;
    auto text = GetString(id);
    Print(text);
}
////////////////////////////////////////////////////////////////////////////////
// Confirm the player wants to quit and then log them out
//
void
QuitPlayer()
{
    if (Prompt(REALLYQUIT, "yn") != 'y')
        return;

    /// TODO: CLEANUP: We should have a general cleanup function.
    CancelHelp();
    exeunt = 1;
    SaveCharacter();
    donet = ml + 1;
}

////////////////////////////////////////////////////////////////////////////////
// Save the player's character
//
void
SavePlayerCharacter()
{
    SaveCharacter();
    Printf(GetString(SAVED), t_character->score);
}

////////////////////////////////////////////////////////////////////////////////
// 	Masks a player as currently being 'interacted' with. This currently
// 	only affects messageing, ie ACTION and ANNOUNCE (and their
// 	derivatives) don't reach the other player. For example, if you
// 	give something to someone nearby, and want to tell the OTHER
// 	players in the room. In the future this may be used as a mini-locking
// 	system, for example to prevent a player leaving a room halfway through
// 	someone giving him something, or as someone attacks him.
//
// 	PLEASE use this whenever you ARE interacting with a player, and
// 	you have a few lines of Lang.Txt to go... In future this will
// 	prevent ALL sorts of things, eg the player logging out three-quarters
// 	of the way through your action.
//
void
SetInteractingWith(slotid_t who)
{
    actor = -1;
    if (GetAvatar(who).state == PLAYING)
        actor = who;
}

////////////////////////////////////////////////////////////////////////////////
//
void
SetPlayerFlags(flag_t flagsOn, flag_t flagsOff)
{
    t_avatar->flags = (t_avatar->flags | flagsOn) & ~flagsOff;
}

////////////////////////////////////////////////////////////////////////////////
//
void
SetRoomDescMode(RoomDescMode newMode)
{
    t_character->rdmode = newMode;
    static std::array<string_view, 3> modes{
        GetNamedString("rdmode.rc", "Room-Counting mode selected.\n"),
        GetNamedString("rdmode.vb", "Verbose mode selected.\n"),
        GetNamedString("rdmode.bf", "Brief mode selected.\n")
    };
    Print(modes[newMode]);
}

////////////////////////////////////////////////////////////////////////////////
// NAME
// 	StopFollowing -- Stop being a follower.
//
// SYNOPSIS
// 	void StopFollowing( void );
//
// FUNCTION
// 	If the current verb (overb) is a Travel verb and we were following
// 	the we can no-longer follow them.
//
// SEE ALSO
// 	LoseFollower(), Follow(), DoThis()
//
void
StopFollowing()
{
    if (t_follow || t_avatar->following == -1 || GetVerb(overb).flags & VB_TRAVEL) {
        return;
    }
    if (GetAvatar(t_avatar->following).state != PLAYING ||
        GetAvatar(t_avatar->following).followed != t_slotId) {
        t_avatar->following = -1;
        return;
    }
    GetAvatar(t_avatar->following).followed = -1;
    static const auto str = GetNamedString("follow.stop", "You are no-longer following @mf.\n");
    Print(str);
    t_avatar->following = -1;
}

////////////////////////////////////////////////////////////////////////////////
// Switches the current parse to a new verb, leaving other slots alone.
//
// INPUTS
//  NewVerb - the number of the new verb to be processed.
//
// RESULT
//  vbptr   - points to new verbs table entry.
//  iverb   - set to NewVerb
//  ml      - set to -(1+verb) to reset the current loop
//
// SEE ALSO
//  amul2.c/asyntax ado
//
void
TreatAsVerb(verbid_t verbId)
{
    donet = 0;
    inc = 0;
#ifdef PARSER_CODE
    if (tt.verb == -1) {
        vbptr = &GetVerb(verbId);
        ml = ~verbId;
    }
#endif
    iverb = verbId;
    iocheck();
}

////////////////////////////////////////////////////////////////////////////////
// Produce a list of players online
//
void
Who(Verbosity verbosity)
{
    std::vector<std::string> players{};

    ForEachPlayer([&](auto slotId, auto &player, auto &avatar) {
        if (!CanSee(t_slotId, slotId))
            return;
        if (avatar.flags & PFSINVIS)
            players.emplace_back("("s + player.name + ")"s);
        else
            players.emplace_back(player.name);
        if (verbosity == TYPEV)
            players.back() +=
                    Meld(GetNamedString("who.the", "the"), MakeTitle(slotId), GetString(ISPLAYING));
    });
    if (players.empty()) {
        Print(std::string{ GetNamedString("who.justyou", "Just you.") } + "\n");
        return;
    }
    auto joiner = verbosity == TYPEV ? ".\n" : ", ";
    const bool useOxfordAnd = verbosity == TYPEB;
    if (verbosity == TYPEB)
        players.emplace_back(GetNamedString("who.you", "you"));
    std::string result = JoinStrings(players.cbegin(), players.cend(), joiner, useOxfordAnd);
    Print(result);
}

}  // namespace Action
