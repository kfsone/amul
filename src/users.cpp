// For dealing with logins/logout/accounts etc.

#include "users.h"

#include <cstring>
#include <optional>
#include <string>
using namespace std::string_literals;

#include "amul.defs.h"
#include "amul.msgs.h"
#include "amul.stct.h"
#include "amulinc.h"
#include "amullib.h"
#include "client.io.h"
#include "exceptions.h"
#include "filemapping.h"
#include "game.h"
#include "message.common.h"
#include "parser.context.h"
#include "spinlock.h"
#include "stringmanip.h"
#include "users.h"

extern thread_local char t_inputBuffer[1024];

SpinLock s_characterListLock;

error_t
LoadPlayers(string_view filepath)
{
    FileMapping<Character> file{ filepath };
    if (error_t err = file.Open(); err != 0)
        return err;
    SpinGuard lock(s_characterListLock);
    g_game.m_characters.clear();
    g_game.m_characters.reserve(file.size());
    for (auto &it : file) {
        g_game.m_characters.push_back(it);
        g_game.m_characterIndex.emplace(it.name, g_game.m_characters.back());
    }
    return 0;
}

std::optional<const Character *>
lookupPlayer(std::string name)
{
    SpinGuard lock(s_characterListLock);
    /// ISSUE: Need a lock to look this up, you should need a ref count on the entry.
    auto it = g_game.m_characterIndex.find(name);
    return (it != g_game.m_characterIndex.cend()) ? &(it->second) : std::optional<Character *>();
}

void
GetLineLength()
{
    Printf("\nEnter screen width [%d characters]: ", t_character->llen);
    GetInput(t_inputBuffer, 4);
    if (t_inputBuffer[0] != 0)
        t_character->llen = atoi(t_inputBuffer);
    Printf("Screen width set to %d characters.\n", t_character->llen);
}

void
GetScreenLength()
{
    Print("\n(Choose '0' to disable MORE? prompting)");
    Printf("\nEnter screen length [%d lines]: ", t_character->slen);
    GetInput(t_inputBuffer, 4);
    if (t_inputBuffer[0] != 0)
        t_character->slen = atoi(t_inputBuffer);
    if (t_character->slen == 0)
        Print("Paging disabled.\n");
    else
        Printf("Screen length set to %d lines.\n", t_character->slen);
}

void
GetRedoCharacter()
{
#ifdef CHARACTER_CODE
    char rchar = t_character->rchar;
    t_character->rchar = 0;
    sprintf(t_inputBuffer, "currently \"%c\"", rchar);
    sprintf(str, "\nEnter %s%s[%s]: ", "redo-character", " ", t_inputBuffer);
    Print(str);
    GetInput(str, 2);
    if (str[0] != 0)
        t_character->rchar = str[0];
    else
        t_character->rchar = rchar;
    if (t_character->rchar == '/' || isspace(str[0])) {
        Print("Invalid redo-character (how do you expect to do / commands?)\n");
        t_character->rchar = rchar;
        return;
    }
    sprintf(t_inputBuffer, "\"%c\"", t_character->rchar);
    sprintf(str, "%s set to %s.\n", "redo-character", t_inputBuffer);
    Print(str);
#else
    Print("-- Not implemented\n");
#endif
}

void
GetCRLFUse()
{
#ifdef CHARACTER_CODE
    Print("Follow CR with a Line Feed? ");
    if (t_character->flags & ufCRLF)
        Print("[Y/n]: ");
    else
        Print("[y/N]: ");
    GetInput(str, 2);
    if (toupper(str[0]) == 'Y' || toupper(str[0]) == 'N') {
        if (toupper(str[0]) == 'Y')
            t_character->flags |= ufCRLF;
        else
            t_character->flags &= ~ufCRLF;
    }
#else
    Print("-- Not implemented\n");
#endif
}

void
ChooseAnsiMode()
{
    for (;;) {
        Print(ASK4ANSI);
        GetInput(t_inputBuffer, 2);
        char c = toupper(t_inputBuffer[0]);
        switch (c) {
            case 'Y':
                t_character->flags |= ufANSI;
                return;
            case 'N':
                t_character->flags &= ~ufANSI;
                return;
            case '?':
                Print("\033[1mANSI Codes\033[0m\n");
                Print("If the above text stands out in bold, your client supports ANSI control "
                      "codes. If you saw something like X[1mANSIX[0m' above, then your client does "
                      "not.\n\n");
                Print("The game will use ANSI codes to insert some color into the otherwise plain "
                      "text.\n");
                break;
            default:
                Print("Didn't understand that. Choose y, n or press ? for help.\n");
        }
    }
}

// Get the users flag bits
void
flagbits()
{
    t_character->llen = DLLEN;
    t_character->slen = DSLEN;
    t_character->rchar = DRCHAR;

    Printf("Default settings are:\n\nScreen width = %ld, Lines = %ld, Redo = '%c', Flags = "
           "ANSI: "
           "%s, Add LF: %s\n\nChange settings (y/N): ",
           (t_character->llen),
           (t_character->slen),
           t_character->rchar,
           (t_character->flags & ufANSI) ? "On" : "Off",
           (t_character->flags & ufCRLF) ? "On" : "Off");
    GetInput(t_inputBuffer, 2);
    if (toupper(*t_inputBuffer) != 'Y')
        return;

#ifndef CHARACTER_CODE
    Print("-- NOTE: Partially Implemented\n");
#endif

    GetCRLFUse();
    ChooseAnsiMode();
    GetLineLength();
    GetScreenLength();
    GetRedoCharacter();

#ifdef CHARACTER_CODE
    save_me();
#endif
}

std::string
getPlayerName()
{
    for (;;) {
        LnPrint(WHATNAME);
        SafeInput(t_inputBuffer);
        Printc('\n');
        if (*t_inputBuffer == 0)
            throw EndThread{};
        if (strlen(t_inputBuffer) < 3 || strlen(t_inputBuffer) > NAMEL) {
            Print(LENWRONG);
            continue;
        }

        bool invalid = false;
        for (char *p = t_inputBuffer; *p != 0; ++p) {
            if (!isalpha(*p)) {
                invalid = true;
                break;
            }
            *p = tolower(*p);
        }
        if (invalid) {
            Print(GetNamedString("login.illegal-name", "Invalid name, only latin/ascii characters allowed.\n"));
            continue;
        }

        char *p = t_inputBuffer;
        auto [wt, slotId] = GetTokenType(&p);
        if (wt != WNONE && wt != WPLAYER) {
            Print(NAME_USED);
            continue;
        }
        if (wt == WPLAYER && slotId != t_slotId) {
            PrintSlot(slotId, GetString(LOGINASME));
            Print(NAME_USED);  /// TODO: Was already in, re-use.
            continue;
        }

        return t_inputBuffer;
    }
}

int
chooseGender()
{
    for (;;) {
        LnPrint(WHATGENDER);
        GetInput(t_inputBuffer, 2);
        switch (toupper(t_inputBuffer[0])) {
            case 'M':
                return 0;
            case 'F':
                return 1;
            default:
                LnPrint(GENDINVALID);
        }
    }
    return WNONE;
}

void
choosePassword()
{
    for (;;) {
        LnPrint(ENTERPASSWD);
        GetInput(t_inputBuffer, sizeof(t_character->passwd));
        if (strlen(t_inputBuffer) < 4) {
            LnPrint(PASLENWRONG);
            continue;
        }
        break;
    }
    strcpy(t_character->passwd, t_inputBuffer);
}

bool
createCharacter(const std::string &name)
{
    strcpy(t_character->name, name.c_str());

    Print(CREATE);

    GetInput(t_inputBuffer, 2);
    if (toupper(t_inputBuffer[0]) != 'Y')
        return false;

    const auto &firstRank = GetRank(0);
    t_character->score = 0;
    t_character->plays = 1;
    t_character->strength = firstRank.strength;
    t_character->stamina = firstRank.stamina;
    t_character->dext = firstRank.dext;
    t_character->wisdom = firstRank.wisdom;
    t_character->experience = firstRank.experience;
    t_character->magicpts = firstRank.magicpts;
    t_character->tasks = 0;
    t_character->tries = t_character->gender = t_character->rank = 0;
    t_character->rdmode = RDRC;
    t_character->llen = DLLEN;
    t_character->slen = DSLEN;

    t_character->gender = chooseGender();

    choosePassword();

    flagbits();

#ifdef CHARACTER_CODE
    save_me();
    Printc('\n');

    /* Send them the scenario */
    ShowFile("scenario");
#endif

    LnPrint(YOUBEGIN);
    Printc('\n');

    return true;
}

bool
checkPassword(string_view password)
{
    for (int i = 0; i < 4; i++) {
        if (i == 3) {
            Print(TRIESOUT); /* Update bad try count */
            t_character->tries++;
#if defined(CHARACTER_CODE)
            save_me();
#endif
            throw EndThread{};
        }

        if (i > 0)
            Printf("Attempt #%d -- ", i + 1);
        Print(ENTERPASSWD);

        /// TODO: CHARACTER_CODE: Hash the password instead of storing it as text
        SafeInput(t_inputBuffer);
        if (password == t_inputBuffer)
            break;
    }

    if (t_character->tries > 0) {
        Ansify("1m\n");
        Printf(GetString(FAILEDTRIES), t_character->tries);
        Ansify("0m\n");
    }

    t_character->tries = 0;

    return true;
}

void
PlayerLogin()
{
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = actor = WNONE;
    last_him = last_her = -1;
    strcpy(t_avatar->arr, GetString(ARRIVED));
    strcpy(t_avatar->dep, GetString(LEFT));
    t_character->rchar = 0;
    t_avatar->rec = -1;
    t_avatar->flags = 0;

    // Print the title screen.
    Print(GetString(0));  /// TODO: Should give this a name

    bool ok = false;
    while (!ok) {
        auto name = getPlayerName();

        if (auto entry = lookupPlayer(name); entry)
            ok = checkPassword(entry.value()->passwd);
        else
            ok = createCharacter(name);
    }

#ifdef MESSAGE_CODE
    /* Inform AMAN that we have begun! */
    SendIt(MLOGGED, 0, t_character->name);
    for (int i = g_game.numRanks - 1; i >= 0; i--)
        if (t_character->score >= GetRank(i).score) {
            t_character->rank = i;
            break;
        }
    refresh();
#endif

    if (t_character->plays != 1)
        Print(WELCOMEBAK);

    /* Work out the players start location */
    lroom = WNONE;

    if (!g_game.m_startRooms.empty()) {
        auto startRoomNo = RandomInt(0, g_game.m_startRooms.size() - 1);
        t_avatar->room = g_game.m_startRooms[startRoomNo];
    } else {
        t_avatar->room = 0;
    }

    t_avatar->wield = WNONE;
    t_avatar->helping = WNONE;
    t_avatar->helped = WNONE;
    t_avatar->following = WNONE;
    t_avatar->followed = WNONE;
    t_avatar->fighting = WNONE;
    t_avatar->numobj = 0;
    t_avatar->weight = 0;
    t_avatar->hadlight = 0;
    t_avatar->light = 0;
    t_avatar->flags = 0;
    t_avatar->sessionScore = 0;
    Ansify("1m");
    if (t_character->flags & ufANSI) {
        Ansify("1m");
        Print(ANSION);
        Ansify("0m");
    }
    t_character->strength = GetRank(t_character->rank).strength;
    t_character->stamina = GetRank(t_character->rank).stamina;
    t_character->magicpts = GetRank(t_character->rank).magicpts;

#ifdef CHARACTER_CODE
    if ((i = IsVerb("\"start")) != -1)
        lang_proc(i, 0);
#endif
    action(GetString(COMMENCED), AOTHERS);
    look(t_avatar->room, (RoomDescMode) t_character->rdmode);
}

void
SaveCharacter()
{
#ifdef CHARACTER_CODE
    if (t_character->score < 0)
        t_character->score = 0;
    fopena("Players Data");
    fseek(afp, t_avatar->rec * sizeof(*t_character), 0);
    fwrite(t_character->name, sizeof(*t_character), 1, afp);
    fclose(afp);
    afp = nullptr;
#else
    Print("-- Character saving not implemented\n");
#endif
}

/****** AMUL3.C/SlashCommands ******************************************
 *
 *   NAME
 *	SlashCommands -- process slash commands.
 *
 *   SYNOPSIS
 *	SlashCommands( Command )
 *
 *	void SlashCommands( int8_t );
 *
 *   FUNCTION
 *	Processes an internal-command string pointed to by Command. Options
 *	available are listed below.
 *
 *   INPUTS
 *	Command - points to an ASCIZ string containing a command sequence.
 *		available commands are:
 *			?		Displays available commands.
 *			p [password]	Change users password
 *			lf [on|off]	Toggles linefeeds on/off
 *			ar [on|off]	Toggles auto-redo on/off
 *			r <redo char>	Changes users redo-char
 *			mN <macro>	Changes macro #N (n=1-4)
 *			ansi [on|off]	Toggles ANSI on/off for the user
 *			llen <line len>	Changes users line-length
 *			plen <page len>	Changes users page-length
 *
 ******************************************************************************
 *
 */

void
SlashCommands(std::string cmd)
{
    if (cmd.empty() || cmd[0] == '?') {
        Print("AMULEd v0.5 - All commands prefixed with a \"/\"\n\n");
        Print(" /?\tDisplays this list\n");
        Print(" /p\tChange password\n");
        Print(" /lf\tToggle linefeeds on/off\n");
        Print(" /ar\tToggle auto-redo on/off\n");
        Print(" /r\tSet redo-character\n");
        Print(" /mN\tSet macro #N (n=1-4)\n");
        Print(" /q\tQuit\n");
        Print(" /an\tToggle ANSI on/off\n");
        Print(" /x\tSet line-length\n");
        Print(" /y\tSet page-length\n\n");
        return;
    }

    StringLower(cmd);

    if (cmd[0] == 'q') {
        throw EndThread{ "User /q'd" };
    }

    if (cmd[0] == 'r') {
        GetRedoCharacter();
        SaveCharacter();
        return;
    }
    if (cmd[0] == 'x') {
        GetLineLength();
        SaveCharacter();
        return;
    }
    if (cmd[0] == 'y') {
        GetScreenLength();
        SaveCharacter();
        return;
    }
    if (cmd[0] == 'l') {
        t_character->flags = t_character->flags ^ ufCRLF;
        Printf("LineFeed follows carriage return %sABLED.\n",
               (t_character->flags & ufCRLF) ? "EN" : "DIS");
        SaveCharacter();
        return;
    }
    if (cmd[0] == 'a') {
        t_character->flags = t_character->flags ^ ufANSI;
        Ansify("1m");
        Printf("ANSI control codes now %sABLED.\n", (t_character->flags & ufANSI) ? "EN" : "DIS");
        Ansify("0m");
        SaveCharacter();
        return;
    }

    if (cmd[0] == 'p') {
#ifdef CHARACTER_CODE
        Print("Enter old password  : ");
        GetInput(t_inputBuffer, 250);
        if (t_inputBuffer[0] == 0) {
            Print("Cancelled.\n");
            return;
        }
        if (stricmp(t_inputBuffer, t_character->passwd) != 0) {
            Print("Invalid password.\n");
            return;
        }
        Print("Enter new password  : ");
        char entry1[32], entry2[32];
        GetInput(entry1, sizeof(entry1));
        if (t_inputBuffer[0] == 0) {
            Print("Cancelled.\n");
            return;
        }
        if (stricmp(t_inputBuffer, t_character->passwd) == 0) {
            Print("Password unchanged.\n");
            return;
        }
        Print("Confirm new password: ");
        GetInput(entry2, sizeof(entry2));
        if (stricmp(entry1, entry2) != 0) {
            Print("Passwords did not match.\n");
            return;
        }
        strcpy(t_character->passwd, entry1);
        Print("Password changed.\n");
        save_me();
#else
        Print("-- Not implemented\n");
#endif
        return;
    }

    Print("Invalid internal command. Type /? for list of commands.\n");
}
