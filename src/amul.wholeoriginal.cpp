//       ####         ###     ### ##     ## ####
//      ##  ##         ###   ###  ##     ##  ##            Amiga
//     ##    ##        #########  ##     ##  ##            Multi
//     ##    ##        #########  ##     ##  ##            User
//     ########  ----  ## ### ##  ##     ##  ##            adventure
//     ##    ##        ##     ##  ##     ##  ##            Language
//    ####  ####      ####   ####  #######  #########
//
//           ****       AMUL.C.......Adventure System      ****
//           ****               Main Program!              ****
//
// Copyright (C) KingFisher Software / Oliver Smith, 1990-2019.

#include <functional>
#include <sstream>
#include <vector>

#include "amul.actions.h"
#include "amul.acts.h"
#include "amul.algorithms.h"
#include "amul.cons.h"  // Predefined Constants etc
#include "amul.demonactions.h"
#include "amul.msgs.h"
#include "amul.stct.h"
#include "amul.stringmanip.h"
#include "amul.vars.h"     // all INTERNAL variables
#include "amul.version.h"  // Version info etc.
#include "amulinc.h"       // Main Include file
#include "amullib.h"
#include "client.io.h"
#include "exceptions.h"
#include "filesystem.h"
#include "game.h"
#include "logging.h"
#include "message.common.h"
#include "message.execdemon.h"
#include "message.execfn.h"
#include "msgports.h"
#include "objflag.h"
#include "parser.context.h"
#include "playerflag.h"
#include "roomflag.h"
#include "spellid.h"
#include "statid.h"
#include "stringmanip.h"
#include "system.h"
#include "typedefs.h"
#include "users.h"

extern thread_local std::atomic_bool t_terminate;

thread_local FILE *ifp, *ofp1, *afp;
thread_local bool debug;
thread_local char llen;

thread_local char t_inputBuffer[1024];  // 1k input buffer

// Client specific variables
thread_local char MyFlag;              // What type of client am I
thread_local char spc[200];            // Output string scratch pad
thread_local bool failed;              // Current parse failed/aborted
thread_local bool t_forced;            // current action was forced on us
thread_local bool t_follow;            // current action is result of a follow
thread_local char inc, died;           // For parsers use
thread_local char autoexits;           // General flags
thread_local uint32_t *rescnt;         // Reset counter from AMan
thread_local short int donev, skip;    // No. of vb's/TT's done
thread_local char exeunt, more, link;  // If we've linked yet
thread_local long ml, donet;           // Maximum lines
thread_local bool t_inIocheck;         // to prevent recursion

/// TODO: Eliminate
thread_local TravelLine tt, *ttabp;
thread_local Verb *vbptr;
thread_local Syntax *syntaxp;
thread_local char dmove[IDL + 1];

// The persistent image of a given character.
thread_local Character *t_character;
// The active presence of a character currently online.
thread_local Avatar *t_avatar;

void iocheck();
static void REALiocheck();
bool lang_proc(verbid_t verbId, char e);
// static bool parser();
// static void grab();
// static void lockusr(slotid_t user);
void cure(slotid_t subject, SpellID spell);
void executeAction(const VMLine &vmLine);
bool cond(const VMLine &line, bool lastResult);
bool lit(roomid_t roomNo);
void sendex(slotid_t slot, int d, int p1 = 0, int p2 = 0, int p3 = 0, int p4 = 0);
void interact(stringid_t msg, slotid_t slotId, int d, const char *text = nullptr);
bool canSeeObject(objid_t obj, slotid_t who);
void afix(StatID stat, slotid_t plyr);
void list_what(roomid_t roomNo, bool visible);
void calcdext();
void loseobj(objid_t objId);
void adrop(objid_t objId, roomid_t roomId, bool f);

using NamedStringMap = std::unordered_map<string_view, string_view>;
const NamedStringMap s_namedStrings = {};

string_view
GetNamedString(string_view name, string_view defaultValue) noexcept
{
    if (auto it = s_namedStrings.find(name); it != s_namedStrings.end()) {
        return it->second;
    }
    return defaultValue;
}

template<typename MsgType, typename... Args>
void
SendMsg(Args &&... args)
{
    t_managerPort->Put(make_unique<MsgType>(forward<Args>(args)...));
}

void
memfail(const char *s)
{
    Printf("** Unable to allocate memory for %s! **\n", s);
    throw EndThread{};
}

// NAME
//  scaled -- rehash object value using time-scaling!
//
// SYNOPSIS
//  Actual = scaled( Value )
//
//  int scaled( int );
//
// FUNCTION
//  Recalculates an object value based on AMUL Time and Rank scaling.
//  Rather than a straight forward scaling basd on RScale and TScale,
//  you have to calculate what percentage of each of these to use! A
//  player of rank 1 in the last minute of game play will find objects
//  worth their full value. The formula is quite simple, but doing %ages
//  in 'C' always looks messy.
//
// INPUTS
//  Value  - the value to be scaled
//
// RESULT
//  Actual - the actual CURRENT value
//
// NOTES
//  Note: rscale is based on %age of total ranks achieved and tscale is
//        based on %age of game-time remaining.
//
long
scaled(long value, flag_t flags)
{
    if (!(flags & SF_SCALED))
        return value;

    // The longer the game runs, the more valuable objects should become.
    auto gameDuration = std::max(double(*rescnt), double(g_game.gameDuration_m) * 60.);
    auto timeFactor = double(*rescnt) / gameDuration;
    auto timeAdjust = timeFactor * double(g_game.timeScale) / 100.;

    // The higher a player's rank, the less points they can get purely from items.
    auto maxRank = double(g_game.MaxRank());
    auto rankFactor = double(t_character->rank) / maxRank;
    auto rankAdjust = rankFactor * double(g_game.rankScale) / 100.;

    auto adjusted = double(value) * timeAdjust;
    adjusted -= adjusted * rankAdjust;

    return long(adjusted);
}

void
fopenr(const char *filename)
{
    if (ifp != nullptr)
        fclose(ifp);
    std::string filepath{};
    safe_gamedir_joiner(filename);
    if ((ifp = fopen(filepath.c_str(), "rb")) == nullptr) {
        Print("File error");
        LogError("Couldn't open '", filepath, "' for reading.");
        throw EndThread{};
    }
}

void
fopena(const char *filename)
{
    if (afp != nullptr)
        fclose(afp);
    std::string filepath{};
    safe_gamedir_joiner(filename);
    if ((afp = fopen(filepath.c_str(), "rb+")) == nullptr) {
        Print("File error");
        LogError("Unable to open file '", filepath, "' for appending");
        throw EndThread{};
    }
}

void
sendResetText()
{
    fopenr("reset.txt");
    //  Print the Reset Text
    char text[1024]{};
    for (;;) {
        size_t bytes = fread(text, 1, sizeof(text), ifp);
        if (bytes == 0)
            break;
        text[bytes] = 0;
        Print(text);
    }
    fclose(ifp);
    ifp = nullptr;
    Printf("\n%s is resetting ... Saving at %ld.\n\nPlease call back later.\n\n",
           g_game.gameName,
           t_character->score);
    pressret();
}

static bool
IsPlayerClient() noexcept
{
    return t_slotId < MAXU;
}

void
MsgResetting::Dispatch()
{
    t_avatar->helping = t_avatar->helped = t_avatar->following = t_avatar->followed = -1;
    if (IsPlayerClient()) {
        Print(RESETSTART);
        sendResetText();
    }
    link = 0;
    throw EndThread{};
}

// Assorted message handlers.
void
MsgExecDemon::Dispatch()
{
    /// TODO: MESSAGE_CODE: t_inIoCheck = false;
    /// TODO: MESSAGE_CODE: create a context
    LogDebug("Executing demon: ", GetVerb(m_param.m_verb).id);
    lang_proc(m_param.m_verb, 0);
    /// TODO: MESSAGE_CODE: pop the context
    /// TODO: MESSAGE_CODEt_inIocheck = true;
}

void
MsgSummoned::Dispatch()
{
    if (m_param == t_avatar->room) {
        m_param = WNONE;
        return;
    }
    Print(BEENSUMND);
    if (lit(t_avatar->room) && !(t_avatar->flags & PFINVIS) && !(t_avatar->flags & PFSINVIS))
        action(GetString(SUMVANISH), AOTHERS);
    /// TODO: If a player with a light is summoned into a dark room, we want the
    /// other players to see just "it's light now", so we should send the arrival
    /// message *before* we light the room.
    Action::MovePlayerTo(m_param);
    if (lit(t_avatar->room) && !(t_avatar->flags & PFINVIS) && !(t_avatar->flags & PFSINVIS))
        action(GetString(SUMARRIVE), AOTHERS);
}

void
DisturbSleep()
{
    if (t_avatar->flags & PFASLEEP) {
        cure(t_slotId, SSLEEP);
        Print(IWOKEN);
        if (!IamINVIS && !(t_avatar->flags & PFSINVIS))
            action(GetString(WOKEN), AOTHERS);
    }
}

void
MsgForced::Dispatch()
{
    DisturbSleep();
    strcpy(t_inputBuffer, m_param.c_str());
    Printf("--+ You are forced to \"%s\" +--\n", t_inputBuffer);
    t_forced = true;
}

void
MsgFollow::Dispatch()
{
    strcpy(t_inputBuffer, m_param.c_str());
    /// TODO: If you can't see "m_sender", it should print someone else.
    Printf("You follow %s %s...\n", GetCharacter(m_sender).name, t_inputBuffer);
    t_follow = t_forced = true;
}

void
MsgExecuteVmop::Dispatch()
{
    m_param.notCondition = false;
    m_param.condition.m_op = 0;
    executeAction(m_param);
}

void
MsgPrintText::Dispatch()
{
    if (IsPlayerClient())
        Printf("%s\n", m_param.c_str());
}

void
REALiocheck()
{
    // int i;
    // long d, f;
    // char *pt;
    // oparg_t p[MAX_VMOP_PARAMS];

    for (;;) {
        auto msg = t_replyPort->Get();
        if (!msg)
            return;
        /// TODO: MESSAGE_CODE: This used to lockusr
        SendMsg<MsgSetBusy>(true);
        t_inIocheck = true;
        t_addCR = true;
        msg.get()->Dispatch();
        t_inIocheck = false;
        SendMsg<MsgSetBusy>(false);
        ReplyMsg(move(msg));
    }
}

void
iocheck()
{
    if (!t_inIocheck) {
        REALiocheck();
        t_addCR = false;
    }
}

/// TODO: This is not ok
void
fwait(long n)
{
    if (n < 1)
        n = 1;
    for (int i = 0; i < 7; i++) {
        Delay(n * 7);
        iocheck();
    }
}

std::string
MakeInventory(slotid_t slotId, bool stateEmpty)
{
    /// TODO: Attach a linked-list of inventory to each player.
    using namespace std::string_literals;
    std::vector<std::string> items{};
    if (GetAvatar(slotId).numobj > 0) {
        for (const auto &object : g_game.m_objects) {
            if (object.Owner() == slotId && object.IsVisibleTo(t_slotId)) {
                if (object.flags & OF_INVIS)
                    items.emplace_back("("s + object.id + ")"s);
                else
                    items.emplace_back(object.id);
            }
        }
    }
    if (items.empty()) {
        if (stateEmpty)
            return std::string{ GetNamedString("carrying.nothing", "empty handed.") };
        return ""s;
    }

    string_view prefix{};
    if (slotId == t_slotId)
        prefix = GetNamedString("carrying.you-are", "You are carrying");
    else
        prefix = GetNamedString("carrying.carrying", "carrying");
    auto suffix = GetNamedString("carrying.suffix", ".");
    return Meld(prefix, JoinStrings(items.begin(), items.end(), ", ", true), suffix);
}

// This must abide FULLY by INVIS & INVIS2... Should we make it so that visible players
// can't see an invisible players entry, they can just see that they're here?

void
whohere()
{
    if (!lit(t_avatar->room))
        return;
    if ((GetRoom(t_avatar->room).flags & HIDE) == HIDE &&
        t_character->rank != g_game.MaxRank()) {
        Print(WHO_HIDE);
        return;
    }
    char text[256];
    for (slotid_t i = 0; i < MAXU; i++) {
        if (i != t_slotId && CanSee(t_slotId, i) && !(GetAvatar(i).flags & PFMOVING)) {
            snprintf(text,
                     sizeof(text),
                     GetString(ISHERE),
                     GetCharacter(i).name,
                     MakeTitle(i).c_str());
            std::vector<string_view> info{ text };
            if (GetAvatar(i).flags & PFSITTING)
                info.emplace_back(GetNamedString("who.sitting", "sitting down"));
            if (GetAvatar(i).flags & PFLYING)
                info.emplace_back(GetNamedString("who.lying", "lying down"));
            if (GetAvatar(i).flags & PFASLEEP)
                info.emplace_back(GetNamedString("who.asleep", "asleep"));
            info.emplace_back(MakeInventory(i, false));
            auto line = JoinStrings(info.begin(), info.end(), ", ", true);
            if (!ispunct(line.back()))
                line += ".";
            line += "\n";
            Print(line);
        }
    }
}

// NAME
//  isValidNumber -- mathematically compare two numbers, including <> and =.
//
// SYNOPSIS
//  ret = isValidNumber( Number, Value )
//  d0            d0      d1
//
//  BOOLEAN isValidNumber( long, long );
//
// FUNCTION
//  Quantifies and/or equates the value of two numbers.
//
// INPUTS
//  Number - Real integer numeric value.
//  Value  - Integer value with optional quantifier (MORE/LESS) ie
//           '<' or '>'.
//
// RESULT
//  ret    - TRUE if Number equates with Value (ie 5 IS <10)
//
// EXAMPLE
//  isValidNumber(10, ( LESS & 20 ));   Returns TRUE.
// Lang.Txt:
//  numb ?brick >%wall
//
// NOTES
//  Remember to process the values using ACTUAL before passing them,
//  thus: Numb(GetConcreteValue(n1), GetConcreteValue(n2));
//
bool
isValidNumber(long x, long n)
{
    if (n == x) {
        return true;
    }
    if ((n & MORE) == MORE) {
        n = n - MORE;
        if (n > x)
            return true;
    }
    if ((n & LESS) == LESS) {
        n = n - LESS;
        if (n < x)
            return true;
    }
    return false;
}

// NAME
//  ado -- AMUL's equivalent of GOSUB
//
// SYNOPSIS
//  ado( Verb )
//
//  void ado( int );
//
// FUNCTION
//  Causes the parser to process <verb> before continuing the current
//  parse. This works similiarily to GOSUB in Basic, and allows the user
//  to write sub-routine verbs.
//
// INPUTS
//  Verb - the verb to be processed.
//
// EXAMPLE
//  ado("\"travel");    / * Visits the "travel verb. * /
//
// SEE ALSO
//  Action::TreatAsVerb

#ifdef LANG_CODE
void
ado(verbid_t verb)
{
    // context snapshot
    long old_ml = ml;
    long old_donet = donet;
    long old_verb = iverb;
    iverb = verb;
    long old_ttv = tt.verb;
    long old_rm = t_avatar->room;
    TravelLine *old_ttabp = ttabp;
    Verb *ovbptr = vbptr;
    Syntax *osyntaxp = syntaxp;

    lang_proc(verb, 1);

    iverb = old_verb;

    if (failed == true || t_forced != 0 || died != 0 || exeunt != 0) {
        donet = ml + 1;
        ml = -1;
        failed = true;
        return;
    }

    donet = old_donet;
    ml = old_ml;
    tt.verb = old_ttv;
    roomtab = &GetRoom(old_rm);
    ttabp = old_ttabp;
    vbptr = ovbptr;
    syntaxp = osyntaxp;
}
#endif

// NAME
//  add_obj -- Add an object to a players inventory (movement)
//
// SYNOPSIS
//  add_obj( Player )
//
//  void add_obj( int );
//
// FUNCTION
//  Updates the players statistics and settings to indicate addition
//  of an object to his inventory. The objects location IS set to
//  indicate the player (you don't need to move it!). The object must
//  be pointed to by global variable objtab.
//
// INPUTS
//  Player - number of the player to give it to.
//  objtab - must point to the objects structure.
//
// NOTES
//  If objtab does NOT point to the right object, things will go astray!
//
// SEE ALSO
//  rem_obj
void
add_obj(objid_t objId, int to)
{
    auto &object = GetObject(objId);
    object.rooms[0] = -(5 + to);  // It now belongs to the player
    GetAvatar(to).numobj++;
    GetAvatar(to).weight += object.State().weight;
    const auto &rank = GetRank(GetCharacter(to).rank);
    GetAvatar(to).strength -= (rank.strength * object.State().weight) / rank.maxweight;
    if (object.State().flags & SF_LIT)
        GetAvatar(to).light++;
}

// NAME
//  rem_obj -- Remove an object from a players inventory (no move).
//
// SYNOPSIS
//  rem_obj( Player, Noun )
//
//  void rem_obj( int, int );
//
// FUNCTION
//  Removes an object from the players inventory without changing the
//  location of the object. Simply all flags pertaining to the posession
//  of the object or requiring it (eg decreased strength, or if its
//  wielded) are cleared.
//
// INPUTS
//  Player - who's inventory its in.
//  Noun   - the object number.
//
// NOTES
//  The calling function MUST change the objects location, otherwise
//  the player will effectively still own the object.
//
// SEE ALSO
//  add_obj()
void
rem_obj(objid_t objId, slotid_t to)
{
    const auto &obj = GetObject(objId);
    const auto &rank = GetRank(GetCharacter(to).rank);

    GetAvatar(to).numobj--;
    GetAvatar(to).weight -= obj.State().weight;
    GetAvatar(to).strength += (rank.strength * obj.State().weight) / rank.maxweight;
    if (obj.State().flags & SF_LIT)
        GetAvatar(to).light--;
    if (t_avatar->wield == objId)
        t_avatar->wield = -1;  // = Don't let me wield it
}

slotid_t
owner(objid_t objId) noexcept
{
    return GetObject(objId).Owner();
}

objid_t
AmCarrying(objid_t objId) noexcept
{
    if (t_avatar->numobj == 0)
        return -1;
    if (owner(objId) == t_slotId)
        return objId;
    return -1;
}

bool
IsObjectIn(objid_t objId, roomid_t roomNo)
{
    const auto &obj = GetObject(objId);
    for (int i = 0; i < obj.nrooms; i++) {
        if (obj.Room(i) == roomNo)
            return true;
    }
    return false;
}

char
CHAEtype(objid_t obj) noexcept
{
    if (AmCarrying(obj) != -1)
        return 'C';
    if (IsObjectIn(obj, t_avatar->room))
        return 'H';
    int i = owner(obj);
    if (i != -1 && GetAvatar(i).room == t_avatar->room)
        return 'A';
    return 'E';
}

objid_t
scanNouns(objid_t start, const char *precedence, int pos, bool matchState, const char *str, int adj)
{
    objid_t last = -1;
    for (objid_t i = start; i < objid_t(g_game.numObjects); i++) {
        const auto &object = GetObject(i);
        if ((object.flags & OF_COUNTER) || (adj != -1 && object.adj != adj))
            continue;
        if (!Match(object.id, str) || CHAEtype(i) != precedence[pos])
            continue;
        // If state doesn't matter or states match, we'll try it
        if (precedence[0] == -1 || matchState == false || object.state == precedence[0]) {
            if (adj == object.adj)
                return i;
            else
                last = i;
        } else if (last == -1)
            last = i;
    }
    return last;
}

// Due to the complexity of a multi-ocurance/state environment, I gave up
// trying to do a 'sort', storing the last, nearest object, and went for
// an eight pass seek-and-return parse. This may be damned slow with a
// few thousand objects, but if you only have a thousand, it'll be OK!

int
isnoun(const char *s, adjid_t adj, const char *precedence)
{
    objid_t start = IsANoun(s);
    if (GetObject(start).adj == adj && CHAEtype(start) == precedence[1] &&
        GetObject(start).state == precedence[0])
        return start;

    bool done_e = 0;
    int lsuc = -1, lobj = -1, x = -1;

    for (int pass = 1; pass < 5; pass++) {
        /* At this point, we out to try BOTH phases, however, in the
            second phase, try for the word. Next 'pass', if there is
            no suitable item, drop back to that from the previous... */
        if (precedence[pass] == 'X')
            return lobj;
        if (precedence[0] == -1) {
            if ((x = scanNouns(start, precedence, pass, false, s, adj)) != -1) {
                if (adj != -1 || GetObject(x).adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lsuc = 0;
                lobj = x;
            } else if (lsuc == 0)
                return lobj;
        } else {
            // Did we get a match?
            if ((x = scanNouns(start, precedence, pass, true, s, adj)) != -1) {
                // If adjectives match
                if (adj != -1 || GetObject(x).adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lobj = x;
                lsuc = 0;
                continue;
            }
            if ((x = scanNouns(start, precedence, pass, false, s, adj)) != -1) {
                if (adj != -1 || GetObject(x).adj == adj) {
                    lobj = x;
                    lsuc = 0;
                    continue;
                }  // Must have matched
                if (lsuc == 0)
                    return lobj;
            }
            if (lsuc == 0)
                return lobj;
        };
        if (precedence[pass] == 'E')
            done_e = true;
    }
    if (!done_e)
        return scanNouns(0,
                         "\xff"
                         "E",
                         0,
                         false,
                         s,
                         adj);
    else
        return lobj;
}

// NAME
//  asyntax -- set new noun1 & noun2, using slot labels too!
//
// SYNOPSIS
//  asyntax( Noun1, Noun2 )
//
//  void asyntax( ulong, ulong );
//
// FUNCTION
//  Alters the content of inoun1 and inoun2 along with the word-type
//  slots, thus altering the current input syntaxes. The actual value
//  of Noun1 and Noun2 is calculated by asyntax, so effectives and
//  slot labels should be passed RAW. This allows a call something
//  like: asyntax( IWORD + INOUN2, IWORD + INOUN1); which is the
//  equivalent of "- syntaxes noun2 noun1".
//
// INPUTS
//  Noun1 - unprocessed item for noun1.
//  Noun2 - unprocessed item for noun2.
//
// EXAMPLE
//  asyntax(*(tt.pptr+ncop[tt.condition.m_op]), *(tt.pptr+ncop[tt.condition.m_op]+1));
//
// NOTES
//  If noun1 or noun2 are not REAL values, they will be processed here.
//  Passing a REAL value will assume the passed item was a noun. If you
//  use asyntax(TP1, TP2) you could EASILY be passing a player number!
//
// SEE ALSO
//  ado, Action::TreatAsVerb

constexpr auto expandNounToken = [](const amulid_t id, uint32_t precedenceIndex) noexcept
{
    if (id == WNONE) {
        return pair{ amulid_t{ WNONE }, WType{ WNONE } };
    }
    if (id & IWORD) {
        switch (id & ~IWORD) {
            case IVERB:
                return pair{ amulid_t{ iverb }, wtype[0] };
            case IADJ1:
                return pair{ amulid_t{ iadj1 }, wtype[1] };
            case INOUN1:
                return pair{ amulid_t{ inoun1 }, wtype[2] };
            case IPREP:
                return pair{ amulid_t{ iprep }, wtype[3] };
            case IADJ2:
                return pair{ amulid_t{ iadj2 }, wtype[4] };
            case INOUN2:
                return pair{ amulid_t{ inoun2 }, wtype[5] };
            default:
                return pair{ amulid_t{ inoun1 }, wtype[2] };
        }
    }
    auto noun =
            isnoun(GetObject(id).id, GetObject(id).adj, GetVerb(iverb).precedence[precedenceIndex]);
    return pair{ amulid_t{ noun }, WNOUN };
};

void
asyntax(amulid_t n1, amulid_t n2)
{
    inc = 0;
    auto [in1, wt1] = expandNounToken(n1, 0);
    auto [in2, wt2] = expandNounToken(n2, 1);

    inoun1 = in1, wtype[2] = wt1;
    inoun2 = in2, wtype[5] = wt2;
    ml = -(iverb + 1);
}

// NAME
//  iocopy -- process and copy a string (restricted length).
//
// SYNOPSIS
//  iocopy( Dest, Source, MaxLen )
//
//  void iocopy( int8_t, int8_t, int );
//
// FUNCTION
//  Puts the source string through processString, causing any escape characters
//  to be processed, and then copies the output to another string.
//
// INPUTS
//  Dest   - The target string
//  Source - The input string (unprocessed)
//  MaxLen - Maximum number of characters to be copied.
//
// RESULT
//  Dest   - Remains unchanged.
//  Source - Contains the processed string, upto MaxLen bytes long.
//
// NOTES
//  MaxLen does NOT include the nul byte, always allow for this.
//
// SEE ALSO
//  frame/IOBits.C:processString()
void
qcopy(char *dst, const char *src, size_t size) noexcept
{
    CriticalSection cs{};
    for (size_t i = 0; *src && i < size && *src != '\n'; i++)
        *(dst++) = *(src++);
    *dst = 0;
}

void
iocopy(char *dest, const char *source, size_t size)
{
    auto text = ProcessString(source);
    qcopy(dest, text, size);
}

// NAME
//  DoThis -- Tell another player to follow me or perform an action.
//
// SYNOPSIS
//  DoThis( Player, Command, Type )
//
//  void DoThis( int, char*, int );
//
// FUNCTION
//  Forces another player to perfom an action. Type tells the other end
//  how to cope with this. Used for FORCE and FOLLOW.
//
// INPUTS
//  Player  - Number of the player to tell.
//  Command - Pointer to the text to be processed.
//  Type    - 0 to FORCE player, 1 to make player FOLLOW.
//
// EXAMPLE
//  DoThis( TP1, "quit", 0 );   /-* Force player to quit *-/
//  DoThis( TP1, "east", 1 );   /-* Force them to follow you east *-/
//
void
DoThis(slotid_t slotId, const char *cmd, short int type)
{
#ifdef MESSAGE_CODE
    messageToClient(slotId, MFORCE, [=](Message *msg) {
        msg->m_data = type;
        msg->m_ptrs[0] = strdup(cmd);
    });
#endif
}

// Intended to handle "connection lost" from a client.

[[noreturn]] void
kquit(const char *error)
{
    /// TODO: Log error
    (void) error;
    action("@me just dropped carrier.\n", AOTHERS);
    throw EndThread{};
}

[[noreturn]] void
quit()
{
    Print("\n" AMUL_VERS " exiting.\n\n");

    throw EndThread{};
}

// NAME
//  ShowFile -- Send file to user (add extension)
//
// SYNOPSIS
//  ShowFile( FileName )
//
//  void ShowFile( int8_t );
//
// FUNCTION
//  Locates the file (experiments with extensions and paths) and
//  displays it to the user. If there is insufficient memory or
//  it is unable to find the file, it informs the user and takes
//  apropriate action.
//
// INPUTS
//  FileName - ASCIZ string containing the file name. First try
//         to open file assumes the file is in the adventure
//         directory with the extension .TXT
//
// EXAMPLE
//  ShowFile("Scenario");
//  ShowFile("Ram:Scenario.Txt");
//
// NOTES
//  Tries: <AdvPath>/<File Name>.TXT
//         <AdvPath>/<File Name>
//         <File Name>.TXT
//         <File Name>

void
ShowFile(const char *filename)
{
    std::string filepath{};
    safe_gamedir_joiner(filename);
    FILE *fp = fopen(filepath.c_str(), "rb");
    if (fp == nullptr) {
        Printf("\n--+ Please inform the dungeon master that file %s is missing.\n\n", filename);
        return;
    }

    /// TODO: Map the file.
    fseek(fp, 0, 2L);
    auto fsize = ftell(fp);
    fseek(fp, 0, 0L);
    char *p = (char *) AllocateMem(fsize + 2);
    if (p == nullptr) {
        fclose(fp);
        Print("\n--+ \x07System memory too low, exiting! +--\n");
        t_forced = true;
        exeunt = 1;
        kquit("out of memory!\n");
    }
    fread(p, fsize, 1, ifp);
    fclose(fp);
    Print(p);
    ReleaseMem(&p);
    pressret();
}

// NAME
//  showin -- Display the contents of an object.
//
// SYNOPSIS
//  showin( Object, verbose )
//
//  void showin( int, bool );
//
// FUNCTION
//  Displays the contents of an object, modified depending on the
//  objects 'putto' flag. Mode determines whether output is given when
//  the contents of the object cannot be seen or there it is empty.
//
// INPUTS
//  Object -- the object's id number.
//  verbose   -- YES to force display of contents, or to inform the player
//        if the object is empty.
//        NO not to list the contents of the object if it is opaque,
//        and not to display anything when it is empty.

std::string
showin(objid_t objId, bool verbose)
{
    const auto &object = GetObject(objId);
    if ((object.flags & SF_OPAQUE) && !verbose) {
        return "\n";
    }
    std::string result{};
    if (verbose) {
        if (object.putto == 0)
            result = "The {adj} {noun} contains ";
        else {
            result = "{prep} the {adj} {noun} you find: ";
            ReplaceAll(result, "{prep}", prep[object.putto]);
        }
        ReplaceAll(result, "{adj}", object.adj != -1 ? GetAdjective(object.adj).word : "");
        ReplaceAll(result, "{noun}", object.id);
    }
    if (object.inside <= 0) {
        if (!result.empty())
            result += "nothing.\n";
    } else {
        const roomid_t containerId = -(INS + objId);
        bool first{ true };
        int itemsLeft = object.inside;
        for (objid_t child = 0; child < objid_t(g_game.numObjects) && itemsLeft > 0; child++) {
            if (!IsObjectIn(child, containerId))
                continue;
            if (!first)
                result += ", ";
            result += GetObject(child).id;
            first = false;
            itemsLeft--;
        }
        result += "\n";
    }

    return result;
}

// NAME
//  isStatFull -- Check if players property is at full
//
// SYNOPSIS
//  isStatFull( Stat, Player )
//
//  BOOLEAN isStatFull( int, int );
//
// FUNCTION
//  Tests to see if a players 'stat' is at full power and returns a
//  TRUE or FALSE result (YES or NO).
//
// INPUTS
//  stat   -- a players stat number (see h/AMUL.Defs.H)
//  player -- number of the player to check
//
// RESULT
//  YES if it is
//  NO  if it isn't

bool
isStatFull(int st, int p)
{
    const auto &character = GetCharacter(p);
    const auto &stats = GetAvatar(p);
    switch (st) {
        case STSCORE:
            return false;
        case STSCTG:
            return false;
        case STSTR:
            if (stats.strength < character.strength)
                return false;
            break;
        case STDEX:
            if (stats.dext < character.dext)
                return false;
            break;
        case STSTAM:
            if (stats.stamina < character.stamina)
                return false;
            break;
        case STWIS:
            if (stats.wisdom < character.wisdom)
                return false;
            break;
        case STMAGIC:
            if (stats.magicpts < character.magicpts)
                return false;
            break;
        case STEXP:
            if (character.experience < GetRank(character.rank).experience)
                return false;
            break;
    }
    return true;
}

// twho - who to notify
void
lighting(slotid_t player, AnnounceType twho)
{
    /// TODO: This should be a per-room level check done as an event.
    if (GetAvatar(player).light == GetAvatar(player).hadlight ||
        !(GetRoom(GetAvatar(player).room).flags & DARK))
        return;
    if (GetAvatar(player).light <= 0) {
        if (GetAvatar(player).hadlight <= 0)
            return;
        GetAvatar(player).hadlight = GetAvatar(player).light = 0;
        if (!lit(GetAvatar(player).room))
            action(GetString(NOWTOODARK), twho);
    } else {
        if (GetAvatar(player).hadlight != 0)
            return;
        if (!lit(GetAvatar(player).room))
            action(GetString(NOWLIGHT), twho);
        GetAvatar(player).hadlight = GetAvatar(player).light;
    }
}

void
asetstate(objid_t objId, int stat)
{
    auto &object = GetObject(objId);
    auto owner = object.Owner();
    const bool wasLit = lit(object.Room(0));
    // Remove from owners inventory
    objid_t weapon = -1;
    if (owner != -1) {
        weapon = GetAvatar(owner).wield;
        rem_obj(objId, owner);
    }
    const bool isLit = object.State().flags & SF_LIT;
    object.state = stat;
    if (object.flags & OF_SHOWFIRE) {
        if (!isLit)
            object.State().flags &= ~SF_LIT;
        else
            object.State().flags |= SF_LIT;
        if (owner != -1)
            add_obj(objId, owner);
        return;  // Don't need to check for change of lights!
    }

    if (owner != -1) {
        add_obj(objId, owner);  // And put it back again
        // = Should check to see if its too heavy now
        lighting(owner, AHERE);
        GetAvatar(owner).wield = weapon;
    }

    const bool nowLit = lit(object.Room(0));
    if (nowLit == wasLit)
        return;
    if (!nowLit) {
        actionfrom(objId, GetString(NOWTOODARK));
        Print(NOWTOODARK);
    } else {
        actionfrom(objId, GetString(NOWLIGHT));
        Print(NOWLIGHT);
    }
}

void
describe_room(roomid_t roomNo, RoomDescMode mode) noexcept
{
    const auto &room = GetRoom(roomNo);
    Print(GetString(room.shortDesc));
    if (t_character->rank == g_game.MaxRank())
        Printf(" (%s)", room.id);
    Printc('\n');
    if (room.longDesc != -1 && (mode == RDVB || !g_game.m_visited[t_slotId][roomNo])) {
        Print(GetString(room.longDesc));
        Printc(' ');
        if (mode != RDVB)
            g_game.m_visited[t_slotId][roomNo] = true;
    }
}

void
awhere(objid_t objId)
{
    bool found{ false };
    for (objid_t i = 0; i < objid_t(g_game.numObjects); i++) {
        if (stricmp(GetObject(objId).id, GetObject(i).id) == 0) {
            if (!canSeeObject(i, t_slotId))
                continue;
            if (int j = owner(i); j != -1) {
                if (lit(GetAvatar(j).room)) {
                    if (j != t_slotId) {
                        Print("You see ");
                        Ansify("1m");
                        Print(GetCharacter(j).name);
                        Ansify("0;37m");
                        Print(".\n");
                    } else
                        Print("There is one in your possesion.\n");
                    found = true;
                }
                continue;
            }
            auto *rp = GetObject(i).rooms;
            for (int j = 0; j < GetObject(i).nrooms; j++) {
                if (rp[j] == -1)
                    continue;
                if (rp[j] >= 0) {
                    if (rp[j] == -1 || !lit(rp[j]))
                        continue;
                    describe_room(rp[j], RDBF);
                } else {
                    int k = -(INS + rp[j]);
                    Printf("There is one %s something known as %s!\n",
                           obputs[GetObject(k).putto],
                           GetObject(k).id);
                }
                found = true;
            }
        }
    }
    if (!found)
        Print(SPELLFAIL);
}

objid_t
getLocationOf(objid_t objId)
{
    const auto &object = GetObject(objId);
    roomid_t room = object.Room(0);
    if (room >= -1)
        return room;
    slotid_t own = object.Owner();
    if (own != -1)
        return GetAvatar(own).room;
    // Else its in a container
    return -1;
}

void
osflag(const objid_t objId, int flag)
{
    bool wasLit = false;
    slotid_t own = owner(objId);
    if (own == -1)
        wasLit = lit(getLocationOf(objId));
    else
        rem_obj(objId, own);

    auto &object = GetObject(objId);
    object.State().flags = flag;
    if (own != -1) {
        add_obj(objId, own);
        lighting(own, AHERE);
        return;
    }

    if (lit(getLocationOf(objId)) != wasLit) {
        if (wasLit) {
            actionfrom(objId, GetString(NOWTOODARK));
            Print(NOWTOODARK);
        } else {
            actionfrom(objId, GetString(NOWLIGHT));
            Print(NOWLIGHT);
        }
    }
}

// Do I own a 'obj' in state 'stat'?
bool
gotin(objid_t objId, int st)
{
    const auto &obj = GetObject(objId);
    for (const auto &container : g_game.m_objects) {
        if (st != -1 && container.state != st)
            continue;
        if (container.Owner() != t_slotId)
            continue;
        if (stricmp(obj.id, container.id) == 0)
            return true;
    }
    return false;
}

// The next two commands are ACTIONS but work like conditions

bool
nearto(objid_t obj)
{
    if (!canSeeObject(obj, t_slotId))
        return false;
    if (IsObjectIn(obj, t_avatar->room))
        return true;
    if (AmCarrying(obj) != -1)
        return true;
    return false;
}

// Check player is near object, else msg + endparse
bool
achecknear(objid_t objId)
{
    if (!nearto(objId)) {
        Printf("You can't see the %s!\n", GetObject(objId).id);
        donet = ml + 1;
        return false;
    }
    inc = 0;
    return true;
}

// Check the player can 'get' the object
int
acheckget(objid_t objId)
{
    if (AmCarrying(objId) != -1) {
        Print("You've already got it!\n");
        donet = ml + 1;
        return -1;
    }
    if (!achecknear(objId))
        return -1;
    else
        inc = 1;
    auto &obj = GetObject(objId);
    if ((obj.flags & OF_SCENERY) || (obj.State().flags & SF_ALIVE) || obj.nrooms != 1) {
        Print("You can't move that!\n");
        donet = ml + 1;
        return -1;
    }
    const auto &rank = GetRank(t_character->rank);
    if (rank.numobj <= 0) {
        Print("You can't manage it!\n");
        donet = ml + 1;
        return -1;
    }
    if (obj.State().weight > rank.maxweight) {
        Print("You aren't strong enough to lift that!\n");
        donet = ml + 1;
        return -1;
    }
    if (t_avatar->numobj + 1 > rank.numobj) {
        Print("You can't carry any more! You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    if (obj.State().weight + t_avatar->weight > rank.maxweight) {
        Print("It's too heavy. You'll have to drop something else first.\n");
        donet = ml + 1;
        return -1;
    }
    inc = 0;
    return 0;
}

void
look_here(roomid_t roomNo, RoomDescMode f)
{
    // Can I see?
    if (t_avatar->flags & PFBLIND) {
        list_what(roomNo, false);
        return;
    }
    // Can I see in here?
    if (!lit(t_avatar->room)) {
        g_game.m_visited[t_slotId][roomNo] = false;
        Print(TOODARK);
    } else {
        describe_room(roomNo, f);
        list_what(roomNo, false);
    }

    if ((GetRoom(roomNo).flags & DEATH) && t_character->rank != g_game.MaxRank()) {
        if (dmove[0] == 0)
            strcpy(dmove, GetRoom(lroom).id);
        akillme();
        return;
    } else
        whohere();
    Printc('\n');
}

void
descobj(objid_t objId)
{
    const auto &obj = GetObject(objId);
    const auto &state = obj.State();
    if (state.description < 0)
        return;
    /// TODO: Replace %s use with {}
    std::string str = GetString(state.description);
    if ((obj.flags & OF_SHOWFIRE) && (state.flags & SF_LIT)) {
        if (str.back() == '\n' || str.back() == '{')
            str.pop_back();
        str += " The {adj} {noun} is on fire.\n";
    }
    ReplaceAll(str, "{adj}", obj.adj >= -1 ? GetAdjective(obj.adj).word : "");
    ReplaceAll(str, "{noun}", obj.id);
    if (obj.contains <= 0) {
        Print(str.c_str());  /// SV: Direct
        return;
    }
    if (str.back() == '\n' || str.back() == '{')
        str.pop_back();

    str += " ";
    str += showin(objId, false);

    Print(str.c_str());  /// SV: Direct
}

void
list_what(roomid_t roomNo, bool visible)
{
    if (!lit(t_avatar->room))
        return Print(TOOMAKE);
    if (t_avatar->flags & PFBLIND)
        Print(YOURBLIND);
    const bool isHideaway = (GetRoom(roomNo).flags & HIDEWY);
    const bool isTopRank = t_character->rank == g_game.MaxRank();
    if (isHideaway && visible && !isTopRank) {
        Print(NOWTSPECIAL);  // Wizards can see in hideaways!
        return;
    }
    bool found{ false };
    for (objid_t o = 0; o < objid_t(g_game.numObjects); o++)  // All objects
    {
        // Only let the right people see the object
        if (!canSeeObject(o, t_slotId))
            continue;
        const auto &obj = GetObject(o);
        if (isHideaway && (!visible || !isTopRank) && !(obj.flags & OF_SCENERY))
            continue;
        if (!lit(t_avatar->room) && !(obj.flags & OF_SMELL))
            continue;
        for (int orIdx = 0; orIdx < obj.nrooms; orIdx++) {
            if (obj.Room(orIdx) == roomNo && obj.State().description >= 0) {
                found = true;
                if (obj.flags & OF_INVIS)
                    Ansify("3m");
                descobj(o);
                if (obj.flags & OF_INVIS)
                    Ansify("0m");
                break;
            }
        }
    }
    if (!found && visible)
        Print(NOWTSPECIAL);
}

void
inflict(slotid_t subject, SpellID spellId)
{
    auto &pstate = GetAvatar(subject);
    if (pstate.state != PLAYING)
        return;
    switch (spellId) {
        case SGLOW:
            if (!(pstate.flags & PFGLOW)) {
                pstate.flags |= PFGLOW;
                pstate.light++;
            }
            break;
        case SINVIS:
            pstate.flags |= PFINVIS;
            break;
        case SDEAF:
            pstate.flags |= PFDEAF;
            break;
        case SBLIND:
            pstate.flags |= PFBLIND;
            break;
        case SCRIPPLE:
            pstate.flags |= PFCRIP;
            break;
        case SDUMB:
            pstate.flags |= PFDUMB;
            break;
        case SSLEEP:
            pstate.flags |= PFASLEEP;
            break;
        case SSINVIS:
            pstate.flags |= PFSINVIS;
            break;
    }
    calcdext();
    lighting(subject, AHERE);
}

void
cure(slotid_t subject, SpellID spell)
{
    auto &pstate = GetAvatar(subject);
    if (pstate.state != PLAYING)
        return;
    switch (spell) {
        case SGLOW:
            if (pstate.flags & PFGLOW) {
                pstate.flags &= ~PFGLOW;
                pstate.light--;
            }
            break;
        case SINVIS:
            pstate.flags &= ~PFINVIS;
            break;
        case SDEAF:
            pstate.flags &= ~PFDEAF;
            break;
        case SBLIND:
            pstate.flags &= ~PFBLIND;
            break;
        case SCRIPPLE:
            pstate.flags &= ~PFCRIP;
            break;
        case SDUMB:
            pstate.flags &= ~PFDUMB;
            break;
        case SSLEEP:
            pstate.flags &= ~PFASLEEP;
            break;
        case SSINVIS:
            pstate.flags &= ~PFSINVIS;
            break;
    }
    calcdext();
    lighting(subject, AHERE);
}

void
asummon(slotid_t plyr)
{
    if (GetAvatar(plyr).room == t_avatar->room) {
        Printf(GetString(CANTSUMN), GetCharacter(plyr).name);
        return;
    }
#ifdef MESSAGE_CODE
    interact(MSUMMONED, plyr, t_avatar->room);
#endif
}

void
adestroy(objid_t objId)
{
    CriticalSection cs{};
    loseobj(objId);
    auto &object = GetObject(objId);
    for (int i = 0; i < object.nrooms; i++)
        object.rooms[i] = -1;
    object.flags |= OF_ZONKED;
}

void
arecover(objid_t objId)
{
    auto &object = GetObject(objId);
    if (object.State().flags & SF_LIT)
        t_avatar->light++;
    for (int i = 0; i < object.nrooms; i++)
        object.rooms[i] = t_avatar->room;
    object.flags &= ~OF_ZONKED;
    lighting(t_slotId, AHERE);
}

// Refresh the player's stats
void
refresh()
{
    const auto &rank = GetRank(t_character->rank);
    if (t_character->strength <= 0)
        t_character->strength = rank.strength;
    t_avatar->strength = t_character->strength;
    if (t_character->stamina <= 0)
        t_character->stamina = rank.stamina;
    t_avatar->stamina = t_character->stamina;
    if (t_character->dext <= 0)
        t_character->dext = rank.dext;
    t_avatar->dext = t_character->dext;
    t_avatar->dextadj = 0;
    if (t_character->wisdom <= 0)
        t_character->wisdom = rank.wisdom;
    t_avatar->wisdom = t_character->wisdom;
    if (t_character->experience <= 0)
        t_character->experience = rank.experience;
    if (t_character->magicpts <= 0)
        t_character->magicpts = rank.magicpts;
    t_avatar->magicpts = t_character->magicpts;
    calcdext();
}

#ifdef CHARACTER_CODE
void
zapme()
{
    {
        CriticalSection cs{};
        new (t_character) g_game.m_players[t_slotId];
    }
    Action::CancelHelp();
    SaveCharacter();
}
#endif

void
send(objid_t objId, roomid_t to)
{
    bool wasLit = lit(to);
    loseobj(objId);
    auto &object = GetObject(objId);
    for (int i = 0; i < object.nrooms; i++)
        object.rooms[i] = to;
    if (lit(to) != wasLit)
        actionin(to, GetString(NOWLIGHT));
}

constexpr auto MsgToPlayer = [](slotid_t slotId, auto &&msg) {
    if (auto playerPort = FindPort(slotId); playerPort) {
        playerPort->Put(move(msg));
        return true;
    }
    return false;
};

// Change player's gender
void
ChangeGender(slotid_t slotId)
{
    if (slotId == t_slotId) {
        t_character->gender = 1 - t_character->gender;
        Print(CHANGESEX);
    } else {
        MsgToPlayer(slotId, make_unique<MsgExecuteFn>([]() {
                        t_character->gender = 1 - t_character->gender;
                        Print(CHANGESEX);
                    }));
    }
}

// Fixed to allow increase/decrease
void
newrank(slotid_t slotId, rankid_t newRankNo)
{
    const auto &oldRank = GetRank(t_character->rank);
    const auto &newRank = GetRank(newRankNo);

    // If the rank has qualifying tasks, make sure the player has them.
    if (newRank.tasks) {
        const auto taskMask = 1 << (newRank.tasks - 1);
        if ((t_character->tasks & taskMask) == 0) {
            Print(NOTASK);
            return;
        }
    }

    t_character->rank = newRankNo;  /// TODO: Sync
    Print(MADERANK);

    // Update Current Line Stats
    t_avatar->strength += newRank.strength - oldRank.strength;
    if (t_avatar->strength > newRank.strength)
        t_avatar->strength = newRank.strength;
    t_avatar->stamina += newRank.stamina - oldRank.stamina;
    if (t_avatar->stamina > newRank.stamina)
        t_avatar->stamina = newRank.stamina;
    t_avatar->wisdom += newRank.wisdom - oldRank.wisdom;
    if (t_avatar->wisdom > newRank.wisdom)
        t_avatar->wisdom = newRank.wisdom;
    t_character->experience += newRank.experience - oldRank.experience;
    if (t_character->experience > newRank.experience)
        t_character->experience = newRank.experience;
    t_avatar->magicpts += newRank.magicpts - oldRank.magicpts;
    if (t_avatar->magicpts > newRank.magicpts)
        t_avatar->magicpts = newRank.magicpts;
    calcdext();

    // Update File Stats
    t_character->strength = newRank.strength;
    t_character->stamina = newRank.stamina;
    t_character->wisdom = newRank.wisdom;
    t_character->dext = newRank.dext;
    t_character->experience += newRank.experience - oldRank.experience;
    t_character->magicpts = newRank.magicpts;

    if (newRankNo == g_game.MaxRank()) {
        Print(TOPRANK);
#ifdef MESSAGE_CODE
        SendIt(MMADEWIZ, 0, t_character->name);
#endif
    }
}

void
asub(int howmuch, StatID stat, slotid_t plyr)
{
    if (howmuch < 0)
        return asub(-howmuch, stat, plyr);
    if (howmuch == 0)
        return;
    if (plyr == t_slotId) {
        switch (stat) {
            case STSCORE:
                t_character->score -= howmuch;
                t_avatar->sessionScore -= howmuch;
                if (t_character->score < 0)
                    t_character->score = 0;
                Ansify("1m");
                PrintSlot(plyr, "(%ld)\n", t_character->score);
                Ansify("0;37m");
                for (int r = 0; r < g_game.MaxRank(); r++) {
                    if (t_character->score >= GetRank(r + 1).score)
                        continue;
                    if (t_character->rank == r)
                        break;
                    newrank(plyr, r);
                }
                break;
            case STSTR:
                t_avatar->strength -= howmuch;
                if (t_avatar->strength < 0)
                    t_avatar->strength = 0;
                break;
            case STSTAM:
                t_avatar->stamina -= howmuch;
                if (t_avatar->stamina < 0)
                    t_avatar->stamina = 0;
                Printf("\n<STAM: %ld/%ld>\n", t_avatar->stamina, t_character->stamina);
                if (t_avatar->stamina <= 0) {
                    akillme();
                    died = 1;
                }
                break;
            case STDEX:
                t_avatar->dextadj -= howmuch;
                break;
            case STWIS:
                t_avatar->wisdom -= howmuch;
                if (t_avatar->wisdom < 0)
                    t_avatar->wisdom = 0;
                break;
            case STEXP:
                t_character->experience -= howmuch;
                if (t_character->experience < 0)
                    t_character->experience = 0;
                break;
            case STMAGIC:
                t_avatar->magicpts -= howmuch;
                if (t_avatar->magicpts < 0)
                    t_avatar->magicpts = 0;
                break;
            default:
                LogError("asub got invalid stat: ", stat);
        }
    } else {
#ifdef MESSAGE_CODE
        sendex(plyr, ASUB, howmuch, stat, plyr);  // Tell them to clear up!
#endif
    }
}

void
aadd(int howmuch, StatID stat, slotid_t plyr)
{
    if (howmuch < 0)
        return asub(-howmuch, stat, plyr);
    if (howmuch == 0)
        return;
    if (plyr == t_slotId) {
        switch (stat) {
            case STSCORE:
                t_character->score += howmuch;
                t_avatar->sessionScore += howmuch;
                Ansify("1m");
                PrintSlot(plyr, "(%d)\n", t_character->score);
                Ansify("m");
                for (int r = g_game.MaxRank(); r >= 0; r--) {
                    if (t_character->score >= GetRank(r).score) {
                        if (t_character->rank == r)
                            break;
                        newrank(plyr, r);
                        break;
                    }
                }
                break;
            case STSTR:
                t_avatar->strength += howmuch;
                break;
            case STSTAM:
                t_avatar->stamina += howmuch;
                Printf("<STAM: %d/%d>\n", t_avatar->stamina, t_character->stamina);
                break;
            case STDEX:
                t_avatar->dextadj += howmuch;
                break;
            case STEXP:
                t_character->experience += howmuch;
                break;
            case STWIS:
                t_avatar->wisdom += howmuch;
                break;
            case STMAGIC:
                t_avatar->magicpts += howmuch;
                break;
            default:
                LogError("aadd got invalid stat: ", stat);
        }
    }
#ifdef MESSAGE_CODE
    else {
        sendex(plyr, AADD, howmuch, stat, plyr);  // Tell them to clear up!
    }
#endif
}

void
afix(StatID stat, slotid_t plyr)
{
    if (plyr == t_slotId) {
        const auto &rank = GetRank(t_character->rank);
        switch (stat) {
            case STSTR:
                t_avatar->strength =
                        (rank.strength * rank.maxweight - t_avatar->weight) / rank.maxweight;
                break;
            case STSTAM:
                t_avatar->stamina = rank.stamina;
                break;
            case STDEX:
                t_avatar->dextadj = 0;
                calcdext();
                break;
            case STWIS:
                t_avatar->wisdom = rank.wisdom;
                break;
            case STEXP:
                t_character->experience = rank.experience;
                break;
            case STMAGIC:
                t_avatar->magicpts = rank.magicpts;
                break;
            default:
                LogError("afix got invalid stat: ", stat);
        }
    } else {
#ifdef MESSAGE_CODE
        sendex(plyr, AFIX, stat, plyr, 0);  // Tell them to clear up!
#endif
    }
}

// Loud noises/events
void
announce(const char *s, int towho)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is deaf, ignore him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & PFDEAF))
            continue;
        /*
           The next line says:
            if this is another player, and he's in another room,
            and the room is a silent room, ignore him.
        */
        if (i != t_slotId && GetAvatar(i).room != t_avatar->room &&  // --v
            (GetRoom(GetAvatar(i).room).flags & SILENT))
            continue;
        int x = 0;
        switch (towho) {
            case AALL:
            case AEVERY1:
                x = 1;
                break;
            case AGLOBAL:
                if (i != t_slotId)
                    x = 1;
                break;
            case AOTHERS:
                if (i == t_slotId)
                    break;
            case AHERE:
                if (GetAvatar(i).room == t_avatar->room)
                    x = 1;
                break;
            case AOUTSIDE:
                if (GetAvatar(i).room != t_avatar->room)
                    x = 1;
                break;
        }
        if (x == 1) {
            SetMxxMxy(NOISE, i);
            PrintSlot(i, s);
        }
    }
}

// Loud noises/events
void
announcein(roomid_t toroom, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is deaf, ignore him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & PFDEAF) ||
            GetAvatar(i).room != toroom)
            continue;
        SetMxxMxy(NOISE, i);
        PrintSlot(i, s);
    }
}

// Loud noises/events
void
announcefrom(objid_t obj, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is deaf or can see me, ignore him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & PFDEAF) ||
            GetAvatar(i).room == t_avatar->room)
            continue;
        // Check if the player is NEAR to someone carrying the object
        int o = owner(obj);
        if (o != -1 && GetAvatar(o).room != GetAvatar(i).room)
            continue;
        if (o == -1 && !IsObjectIn(obj, GetAvatar(o).room))
            continue;
        SetMxxMxy(NOISE, i);
        PrintSlot(i, s);
    }
}

// Loud noises/events (via an object)
void
objannounce(objid_t obj, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is deaf or can see me, ignore him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & PFDEAF))
            continue;
        // Check if the player is NEAR to someone carrying the object
        int o = owner(obj);
        if (o != -1 && GetAvatar(o).room != GetAvatar(i).room)
            continue;
        if (o == -1 && !IsObjectIn(obj, GetAvatar(o).room))
            continue;
        SetMxxMxy(NOISE, i);
        PrintSlot(i, s);
    }
}

// Quiet actions/notices
void
action(const char *s, slotid_t towho)
{
    for (slotid_t i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || (GetAvatar(i).state < 2) ||
            (GetAvatar(i).flags & (PFBLIND + PFASLEEP)) != 0)
            continue;
        int x = 0;
        switch (towho) {
            case AALL:
            case AEVERY1:
                x = 1;
                break;
            case AGLOBAL:
                if (i != t_slotId)
                    x = 1;
                break;
            case AOTHERS:
                if (i == t_slotId)
                    break;
            case AHERE:
                if (GetAvatar(i).room == t_avatar->room && CanSee(i, t_slotId))
                    x = 1;
                break;
            case AOUTSIDE:
                if (GetAvatar(i).room != t_avatar->room)
                    x = 1;
                break;
        }
        if (x == 1) {
            SetMxxMxy(ACTION, i);
            PrintSlot(i, s);
        }
    }
}

// Quiet actions/notices
void
actionin(roomid_t toroom, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || (GetAvatar(i).state < PLAYING) ||
            (GetAvatar(i).flags & (PFBLIND + PFASLEEP)) || GetAvatar(i).room != toroom)
            continue;
        SetMxxMxy(ACTION, i);
        PrintSlot(i, s);
    }
}

// Quiet actions/notices
void
actionfrom(objid_t obj, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & (PFBLIND + PFASLEEP)) ||
            GetAvatar(i).room == t_avatar->room)
            continue;
        // Check if the player is NEAR to someone carrying the object
        int o = owner(obj);
        if (o != -1)
            if (GetAvatar(o).room != GetAvatar(i).room)
                continue;
        if (o == -1 && !IsObjectIn(obj, GetAvatar(i).room))
            continue;
        SetMxxMxy(ACTION, i);
        PrintSlot(i, s);
    }
}

// Quiet actions/notices
void
objaction(objid_t obj, const char *s)
{
    for (int i = 0; i < MAXU; i++) {
        // If the player is asleep, or blind, skip him
        if (actor == i || (GetAvatar(i).state < 2) || (GetAvatar(i).flags & (PFBLIND + PFASLEEP)))
            continue;
        // Check if the player is NEAR to someone carrying the object
        int o = owner(obj);
        if (o != -1)
            if (GetAvatar(o).room != GetAvatar(i).room)
                continue;
        if (o == -1 && !IsObjectIn(obj, GetAvatar(i).room))
            continue;
        SetMxxMxy(ACTION, i);
        PrintSlot(i, s);
    }
}

void
ableep(int n)
{
    for (int i = 0; i < n; i++) {
        Print(". ");
        fwait(1);
    }
    Printc('\n');
}

// Remove object from its owners inventory
void
loseobj(objid_t objId)
{
    if (slotid_t own = owner(objId); own != -1) {
        auto &object = GetObject(objId);
        for (int i = 0; i < object.nrooms; i++)
            object.rooms[i] = -1;
        rem_obj(objId, own);
        lighting(own, AHERE);
    }
}

void
aforce(slotid_t slotId, const char *cmd)
{
    DoThis(slotId, cmd, 0);
}

#ifdef MESSAGE_CODE
void
afight(int plyr)
{
    if (plyr == t_slotId)
        return;
    if (GetRoom(t_avatar->room).flags & PEACEFUL) {
        Print(NOFIGHT);
        return;
    }
    if (GetAvatar(plyr).fighting == t_slotId) {
        Printf("You are already fighting %s!\n", GetCharacter(plyr).name);
        donet = ml + 1;
        return;
    }
    if (GetAvatar(plyr).fighting != -1) {
        Printf("%s is already in a fight!\n", GetCharacter(plyr).name);
        donet = ml + 1;
        return;
    }
    CriticalSection cs{};
    auto &avatar = GetAvatar(plyr);
    avatar.flags |= PFFIGHT;
    t_avatar->flags |= PFFIGHT | PFATTACKER;
    avatar.fighting = t_slotId;
    t_avatar->fighting = plyr;
}
#endif

void
finishfight(slotid_t plyr)
{
    GetAvatar(plyr).flags &= ~(PFFIGHT | PFATTACKER);
    GetAvatar(plyr).fighting = -1;
}

void
clearfight()
{
    CriticalSection cs{};
    if (t_avatar->fighting != -1 && t_avatar->fighting != t_slotId) {
        finishfight(t_avatar->fighting);
    }
    finishfight(t_slotId);
}

#ifdef MESSAGE_CODE
void
acombat()
{
    /* Check this out for Stats:
    To hit:  Str=40 Exp=50 Dext=10
    Defence: Dext=70 Exp=20 Str=10
    No hits: Players level different by 2 double attacks etc.
    Damage:  Str / 10 + weapon.     <--- made this random!

    == Should ALSO go on how crippled a player is... A cripple can't
    strike out at another player! Also, blindness should affect your
    attack/defence. */

    int aatt, adef, adam, datt, ddef, str;
    int oldstr, minpksl;

    calcdext();

    if (t_avatar->fighting == t_slotId || t_avatar->fighting == -1 || t_avatar->state < PLAYING ||
        t_avatar->stamina <= 0) {
        donet = ml + 1;  // End parse
        finishfight(t_slotId);
        return;
    }

    auto &player = GetCharacter(t_avatar->fighting);
    auto &avatar = GetAvatar(t_avatar->fighting);
    minpksl = GetRank(player.rank).minpksl;

    if (avatar.state < PLAYING || avatar.room != t_avatar->room || avatar.stamina <= 0) {
        donet = ml + 1;
        finishfight(t_slotId);
        return;
    }

    str = 20 * t_avatar->strength;
    if (t_avatar->wield != -1) {
        str += GetObject(t_avatar->wield).State().damage;
    }

    const auto &maxRank = GetTopRank();
    if (t_character->dext == 0)
        aatt = 5;
    else
        aatt = (50 * t_character->experience) / maxRank.experience +
               (40 * t_avatar->strength) / maxRank.strength + (10 * t_avatar->dext) / maxRank.dext;

    if (t_character->dext == 0)
        adef = 5;
    else
        adef = (5 * t_character->experience) / maxRank.experience +
               (15 * t_avatar->strength) / maxRank.strength + (80 * t_avatar->dext) / maxRank.dext;

    /*  if(t_avatar->flags & PFCRIP)  { aatt = aatt / 5; adef = adef / 10; }
        if(t_avatar->flags & PFBLIND) { aatt = aatt / 2; adef = adef / 4;  } */

    str = 20 * avatar.strength;
    if (avatar.wield != -1) {
        str += GetObject(avatar.wield).State().damage;
    }

    if (player.dext == 0)
        datt = 5;
    else
        datt = (50 * player.experience) / maxRank.experience +
               (40 * avatar.strength) / maxRank.strength + (10 * avatar.dext) / maxRank.dext;

    if (player.dext == 0)
        ddef = 5;
    else
        ddef = (5 * player.experience) / maxRank.experience +
               (15 * avatar.strength) / maxRank.strength + (80 * avatar.dext) / maxRank.dext;

    /*  if(avatar.flags & PFCRIP)  { datt = datt / 5; ddef = ddef / 10; }
        if(avatar.flags & PFBLIND) { datt = datt / 2; ddef = ddef / 4;  } */

    oldstr = avatar.stamina;
    adam = -1;
    if (RandomInt(0, 100) < aatt || (ddef <= 0 && RandomInt(0, 100) < 50)) {
        if (RandomInt(0, 100) < ddef) {
            if (avatar.wield != -1) {
                Print(WBLOCK);
                PrintSlot(t_avatar->fighting, GetString(WDEFEND));
            } else {
                Print(BLOCK);
                PrintSlot(t_avatar->fighting, GetString(DEFEND));
            }
            //      if((i=isverb("\"block"))!=-1) lang_proc(i,0);
        } else {
            if (t_avatar->wield != -1) {
                Print(WATTACK);
                const auto &weapon = GetObject(t_avatar->wield);
                adam = (t_avatar->strength / 10) + 1 + RandomInt(0, weapon.State().damage);
                PrintSlot(t_avatar->fighting, GetString(WHIT));
            } else {
                adam = (t_avatar->strength / 10) + 1;
                Print(ATTACK);
                PrintSlot(t_avatar->fighting, GetString(AMHIT));
            }
            asub(adam, STSTAM, t_avatar->fighting);
            //      if((i=isverb("\"hit"))!=-1) lang_proc(i,0);
        }
    } else {
        Print(MISSED);
        PrintSlot(t_avatar->fighting, GetString(HEMISSED));
        //  if((i=isverb("\"miss"))!=-1) lang_proc(i,0);
    }
    oldstr -= adam;
    if ((t_avatar->flags & PFATTACKER) && oldstr > 0) {
        /*      if(t_avatar->helped != WNONE &&
         * GetAvatar(t_avatar->helped).room==t_avatar->room) Well?
         */

        sendex(t_avatar->fighting, ACOMBAT, -1, 0, 0);  // Tell them to clear up!
    }
    if (oldstr <= 0) {
        donet = ml + 1;  // End parse
        Print("You have defeated @pl!\n");
        aadd(minpksl, STSCORE, t_slotId);
        finishfight(t_slotId);
    }
}
#endif

void
exits()
{
    const auto &room = GetRoom(t_avatar->room);
    if (room.ttlines <= 0) {
        Print("There are no visible exits.\n");
        return;
    }

#ifdef PARSER_CODE
    /// TODO: make a list of travel verbs, duh.
    for (auto verbIt = g_game.m_verbs.cbegin(); verbIt != g_game.m_verbs.cend(); ++verbIt) {
        // Only look at verbs flagged for travel
        if (verbIt->flags & VB_TRAVEL)
            continue;

        bool lastResult = true;  // track return value of conditions
        int verbId = int(std::distance(g_game.m_verbs.cbegin(), verbIt));
        int skipping = 0;
        for (auto ttent = room.ptr; ttent < room.ptr + room.ttlines; ++ttent) {
            if (skipping) {
                --skipping;
                continue;
            }
            if (ttent->verb != verbId)
                continue;
            lastResult = cond(*ttent, lastResult);
            if (!lastResult)
                continue;
            vmopid_t action = ttent->action.m_op;
            switch (action) {
                case AGOTO_ROOM:
                    const auto roomId = ttent->action.m_args[0];
                    const auto &destination = GetRoom(roomId);
                    Printf("%s ", verbIt->id);
                    if (destination.flags & DEATH)
                        Print(CERTDEATH);
                    else
                        describe_room(roomId, RDBF);
                    break;
                case AKILLME:
                    Printf("%s: It's difficult to tell...\n", verbIt->id);
                case AENDPARSE:
                case AFAILPARSE:
                case AABORTPARSE:
                case ARESPOND:
                    return;
                case ASKIP:
                    skipping += ttent->action.m_args[0];
            }
            if (ttent->condition.m_op == CANTEP || ttent->condition.m_op == CALTEP ||
                ttent->condition.m_op == CELTEP)
                break;
        }
    }
#endif
}

#ifdef MESSAGE_CODE
void
follow(slotid_t slotId, char *cmd)
{
    lockusr(slotId);
    auto msg = make_unique<AMessage>();
    msg->mn_Length = sizeof(*msg.get());
    msg->m_sender = t_slotId;
    msg->mn_ReplyPort = repbk;
    msg->type = MFORCE;
    msg->data = 1;
    msg->opaque = cmd;
    GetAvatar(slotId).m_replyPort->Put(move(msg));
    GetAvatar(slotId).IOlock = -1;
}
#endif

void
log(char *s)
{
    std::string str = ProcessString(s);
    for (auto it = str.find('\r'); it != str.npos;) {
        str.erase(it, 1);
    }
    for (auto it = str.find('\n'); it != str.npos;) {
        str.erase(it, 1);
    }
    t_managerPort->Put(make_unique<MsgLog>(str));
}

void
akillme()
{
    Action::FailParse();
    Action::StopFollowing();
    Action::LoseFollower();
    Action::CancelHelp();
    if (t_avatar->fighting != -1)
        clearfight();
    iocheck();
    t_character->plays = -1;
    t_forced = true;
    exeunt = 1;
    t_avatar->state = LOGGING;
    Ansify("1m");
    Print(DIED);
    Ansify("0;37m");
}

void
show_tasks(int p)
{
    std::stringstream output{};
    output << "Tasks completed by " << (p != t_slotId ? GetCharacter(p).name : "you") << ": ";
    if (t_character->tasks == 0)
        output << "None";
    else {
        int added = 0;
        for (int i = 0; i < 32; i++) {
            if (GetCharacter(p).tasks & (1 << i)) {
                if (added++ > 0)
                    output << ", ";
                output << (i + 1);
            }
        }
    }
    output << ".\n";
    Print(output.str().c_str());
}

#ifdef PARSER_CODE
// Drop everything to a room
void
dropall(roomid_t torm)
{
    for (objid_t i = 0; i < g_game.numObjects && t_avatar->numobj > 0; i++) {
        if (owner(i) == t_slotId)
            adrop(i, torm, false);
    }
    t_avatar->numobj = 0;
}
#endif

void
ascore(int type)
{
    calcdext();

    const auto &rank = GetRank(t_character->rank);
    if (type == TYPEV) {
        Print("Recorded details:        " AMUL_VERS "\n\n");
        Print("Name: @m! Sex  : @gn     Played   : @gp times\n");
        Print("Rank: @mr  Score: @sc points This game: @sg points\n");
        Printf("Strength: @sr/%ld. Stamina: @st/%ld. Dexterity %ld/%ld.\n",
               rank.strength,
               rank.stamina,
               t_avatar->dext,
               rank.dext);
        Printf("Magic Points: @mg/%ld. Wisdom:  @wi.\n", rank.magicpts);

        Printf("\nCurrent Info:\nObjects Carried: %ld/%ld,  Weight Carried: %ld/%ldg\n",
               t_avatar->numobj,
               rank.numobj,
               t_avatar->weight,
               rank.maxweight);
        Print("Following: @mf.  ");
        if (t_avatar->helping != WNONE)
            Print("Helping: @fr.  ");
        if (t_avatar->helped != WNONE)  /// TODO: Possibility of more than one
            Print("Helped by: @he.");
        if (t_avatar->helping != WNONE || t_avatar->helped != WNONE)
            Printc('\n');
        // = Current weapon
        if (t_avatar->wield != WNONE)
            Printf("Currently wielding: %s.\n", GetObject(t_avatar->wield).id);
        show_tasks(t_slotId);
    } else {
        Print("Score: @sc. ");
        Printf("Strength: @sr/%ld. Stamina: @st/%ld. Dexterity: %ld/%ld. Magic: @mg/%ld\n",
               rank.strength,
               rank.stamina,
               t_avatar->dext,
               rank.dext,
               rank.magicpts);
    }
}

void
calcdext()
{
    const auto &rank = GetRank(t_character->rank);
    t_avatar->dext = rank.dext;

    if (t_avatar->flags & PFSITTING)
        t_avatar->dext = t_avatar->dext / 2;
    else if (t_avatar->flags & PFLYING)
        t_avatar->dext = t_avatar->dext / 3;
    if (!lit(t_avatar->room) || (t_avatar->flags & PFBLIND))
        t_avatar->dext = t_avatar->dext / 5;

    t_avatar->dext -=
            ((t_avatar->dext / 10) -
             (((t_avatar->dext / 10) * (rank.maxweight - (t_avatar->weight))) / rank.maxweight));

    if (t_avatar->flags & PFINVIS)
        t_avatar->dext += (t_avatar->dext / 3);
    if (t_avatar->flags & PFSINVIS)
        t_avatar->dext += (t_avatar->dext / 2);
    if (t_character->flags & PFCRIP)
        t_avatar->dext = 0;
    t_avatar->dext += t_avatar->dextadj;
}

void
PromoteToTopRank()
{
    for (const auto &rank : g_game.m_ranks) {
        // turn the task into a bit
        t_character->tasks |= 1 << (rank.tasks - 1);
    }
    aadd(GetTopRank().score - t_character->score + 1, STSCORE, t_slotId);
}

void
damage(objid_t objId, int howmuch)
{
    auto &object = GetObject(objId);
    if (object.flags & OF_SCENERY)
        return;
    object.State().strength -= howmuch;
    if (object.State().strength >= 0)
        return;
    if (object.flags & OF_SHOWFIRE) {
        Printf("The %s has burnt away.", object.id);
        loseobj(objId);
        return;
    }
    Printf("The %s has just broken.", object.id);
    loseobj(objId);
}

void
repair(objid_t objId, int howmuch)
{
    auto &object = GetObject(objId);
    if (object.flags & OF_SCENERY)
        return;
    object.State().strength += howmuch;
}

void
look(roomid_t roomNo, RoomDescMode mode)
{
    /* Some complex stuff here!
      if f==0 (rdmode=RoomCount) and we have been here before,
        look(brief)
      if f==0 (rdmode=RoomCount) and this is our first visit,
        look(verbose)
      if f==0 visit the room
                                   */
    bool visited = false;
    if (mode == RDRC) {
        visited = g_game.m_visited[t_slotId][roomNo];
        mode = visited ? RDBF : RDVB;
    }
    // If we're not showing you the brief description and you haven't
    // been here, then you are now considered as having visited it.
    if (mode != RDBF && !visited)
        g_game.m_visited[t_slotId][roomNo] = true;

    look_here(roomNo, mode);
}

// Add object to players inventory
void
agive(objid_t objId, int to)
{
    auto &object = GetObject(objId);
    if ((object.flags & OF_SCENERY) || (object.State().flags & SF_ALIVE) || object.nrooms != 1)
        return;
    slotid_t own = owner(objId);
    if (own != -1)
        rem_obj(objId, own);
    const roomid_t prevRm = object.rooms[0];
    add_obj(objId, to);

    /*== The lighting conditions for transfering an object between a
         variable source and destination are complex! See below!    */
    if (object.State().flags & SF_LIT) {
        if (own == -1)  // = Did I just pick and was it from here?
        {
            if (prevRm == GetAvatar(own).room)
                return;
        } else {  // = If I got it from someone, and he is near me...
            if (GetAvatar(own).room == GetAvatar(to).room)
                return;
            lighting(own, AHERE);  // = Else check his lights!
        }
        lighting(to, AHERE);
    }
}

// Drop the object (to a room)
void
adrop(objid_t objId, roomid_t roomId, bool /*f*/)
{
    auto &object = GetObject(objId);
    object.rooms[0] = roomId;
    rem_obj(objId, t_slotId);
    lighting(t_slotId, AHERE);

    // If the room IS a 'swamp', give em points
    if (GetRoom(GetAvatar(t_slotId).room).flags & SANCTRY) {
        // Only give points if player hasn't quit.
        if (exeunt == 0)
            aadd(scaled(object.State().value, object.State().flags), STSCORE, t_slotId);
        loseobj(objId);
    }
}

// Is my room lit?
bool
lit(roomid_t roomNo)
{
    const auto &room = GetRoom(roomNo);
    if (!(room.flags & DARK))
        return true;

    CriticalSection cs{};
    for (size_t i = 0; i < MAXU; i++) {
        if (GetAvatar(i).room == roomNo && GetAvatar(i).hadlight) {
            return true;
        }
    }
    for (objid_t i = 0; i < objid_t(g_game.numObjects); i++) {
        const auto &object = GetObject(i);
        if ((object.State().flags & SF_LIT) != SF_LIT)
            continue;
        for (int locNo = 0; locNo < object.nrooms; ++locNo) {
            if (object.Room(locNo) == roomNo) {
                return true;
            }
        }
    }
    return false;
}

// Can others in this room see me?
bool
isVisible()
{
    if (!lit(t_avatar->room))
        return false;
    if (IamINVIS || IamSINVIS)
        return false;
    return true;
}

// If player could manage to carry object
bool
canGive(objid_t objId, slotid_t plyr)
{
    auto &object = GetObject(objId);
    auto &player = GetCharacter(plyr);
    auto &rank = GetRank(player.rank);
    if (GetAvatar(plyr).weight + object.State().weight > rank.maxweight)
        return false;
    if (GetAvatar(plyr).numobj + 1 > rank.numobj)
        return false;
    if ((object.flags & OF_SCENERY) || (object.flags & OF_COUNTER) || object.nrooms != 1)
        return false;
    return true;
}

verbid_t
isaverb(char **s)
{
    verbid_t ret = IsVerb(*s);
    if (ret != WNONE) {
        (*s) += strlen(GetVerb(ret).id);
        return ret;
    }
    if (auto [slen, csyn] = IsVerbSynonym(*s); slen != 0) {
        *s += slen;
        return csyn;
    }
    return WNONE;
}

bool
isInflicted(slotid_t plyr, SpellID spell)
{
    const auto &avatar = GetAvatar(plyr);
    switch (spell) {
        case SGLOW:
            if (avatar.flags & PFGLOW)
                return true;
            break;
        case SINVIS:
            if (avatar.flags & PFINVIS)
                return true;
            break;
        case SDEAF:
            if (avatar.flags & PFDEAF)
                return true;
            break;
        case SBLIND:
            if (avatar.flags & PFBLIND)
                return true;
            break;
        case SCRIPPLE:
            if (avatar.flags & PFCRIP)
                return true;
            break;
        case SDUMB:
            if (avatar.flags & PFDUMB)
                return true;
            break;
        case SSLEEP:
            if (avatar.flags & PFASLEEP)
                return true;
            break;
        case SSINVIS:
            if (avatar.flags & PFSINVIS)
                return true;
            break;
    }
    return false;
}

bool
testStat(slotid_t plyr, StatID st, int value)
{
    switch (st) {
        case STSTR:
            return isValidNumber(GetAvatar(plyr).strength, value);
        case STSTAM:
            return isValidNumber(GetAvatar(plyr).stamina, value);
        case STEXP:
            return isValidNumber(GetCharacter(plyr).experience, value);
        case STWIS:
            return isValidNumber(GetAvatar(plyr).wisdom, value);
        case STDEX:
            return isValidNumber(GetAvatar(plyr).dext, value);
        case STMAGIC:
            return isValidNumber(GetAvatar(plyr).magicpts, value);
        case STSCTG:
            return isValidNumber(GetAvatar(plyr).sessionScore, value);
        default:
            return false;
    }
}

// If <player1> can see <player2>
bool
CanSee(slotid_t viewer, slotid_t subject) noexcept
{
    // You can't see YOURSELF, and check for various other things...
    if (*GetCharacter(subject).name == 0 || viewer == subject)
        return false;
    if (GetAvatar(subject).state != PLAYING)
        return false;
    // If different rooms, or current room is dark
    if (pROOM(viewer) != pROOM(subject))
        return false;
    // If subject is Super Invis, he can't be seen!
    if (GetAvatar(subject).flags & PFSINVIS)
        return false;
    // If player is blind, obviously can't see subject!
    if (GetAvatar(viewer).flags & PFBLIND)
        return false;
    if (!lit(pROOM(viewer)))
        return false;
    // If you are in a 'hide' room and he isn't a wizard
    if (pRANK(viewer) == g_game.MaxRank())
        return true;
    if (GetRoom(pROOM(viewer)).flags & HIDE)
        return false;
    // If he isn't invisible
    if (!isPINVIS(subject))
        return true;
    // Otherwise
    if (isPINVIS(viewer) && pRANK(viewer) >= g_game.seeInvisRank - 1)
        return true;
    if (pRANK(viewer) >= g_game.seeSuperInvisRank - 1)
        return true;
    return false;
}

bool
canSeeObject(objid_t obj, slotid_t who)
{
    return GetObject(obj).IsVisibleTo(who);
}

bool
castWillSucceed(int rnk, int points, int chance)
{
    if (t_character->rank < rnk - 1) {
        Print(LOWLEVEL);
        return false;
    }

    if (t_avatar->magicpts < points) {
        Print(NOMAGIC);
        return false;
    }

    if (t_character->rank < rnk)
        chance = ((t_character->rank) + 1 - rnk) * ((100 - chance) / rankid_t(g_game.numRanks - rnk)) +
                 chance;
    if (RandomInt(0, 100) < chance) {
        Print(SPELLFAIL);
        return false;
    }
    t_character->magicpts -= points;
    return true;
}

// GetID.C -- Get user name & details. Create record for new users.
#ifdef MESSAGE_CODE
void
lockusr(slotid_t user)
{
    long t, d, p;
    do {
        t = amul->type;
        d = amul->data;
        p = (long) amul->opaque;
        SendIt(MLOCK, user, nullptr);
        if (amul->data != user && !t_inIocheck) {
            iocheck();
            amul->data = -1;
        }
    } while (amul->data != user);
    amul->type = t;
    amul->data = d;
    amul->opaque = (char *) p;
}

void
messageToClient(slotid_t slotId, uint32_t msgType, std::function<void(Message *)> populater)
{
    auto &avatar = GetAvatar(slotId);
    if (avatar.state < PLAYING)
        return;
    lockusr(slotId);
    auto msg = make_unique<Message>(repbk, msgType);
    populater(msg.get());
    msg->m_from = t_slotId;

    avatar.m_replyPort->Put(move(msg));
    avatar.IOlock = -1;
}

void
interact(int msgType, slotid_t slotId, int data, const char *text)
{
    messageToClient(slotId, msgType, [=](Message *msg) {
        if (msgType == MMESSAGE)
            strcat(GetAvatar(slotId).m_outputBuffer, text);
        msg->m_data = data;
    });
}

void
sendex(slotid_t slotId, int d, int p1, int p2, int p3, int p4)
{
    if (GetAvatar(slotId).state < PLAYING)
        return;
    lockusr(slotId);
    if ((intam = (struct AMessage *) AllocateMem(sizeof(*amul))) == nullptr)
        memfail("comms port");
    intam->mn_Length = sizeof(*amul);
    intam->m_sender = t_slotId;
    intam->mn_ReplyPort = repbk;
    intam->type = MEXECUTE;
    intam->data = ~d;
    intam->p1 = p1;
    intam->p2 = p2;
    intam->p3 = p3;
    intam->p4 = p4;
    GetAvatar(slotId).IOlock = -1;
    PutMsg(GetAvatar(slotId).m_replyPort, intam);
}
#endif

// The AMUL parser and VT processor

void
actionSuperGo(roomid_t roomId)
{
    // Announce departure.
    if (isVisible())
        action(GetString(SGOVANISH), AOTHERS);

    // Can't be followed.
    Action::StopFollowing();
    Action::LoseFollower();

    // Tell me it's happening.
    Print(SGO);

    // Relocate
    Action::MovePlayerTo(roomId);

    // Announce arrival
    if (isVisible())
        action(GetString(SGOAPPEAR), AOTHERS);
}

optional<bool>
parseSuperGo(string_view text)
{
    if (text.front() == '*' && t_character->rank >= g_game.superGoRank) {
        text.remove_prefix(1);
        if (auto roomId = IsRoomName(text); roomId != WNONE) {
            if (!(GetRoom(roomId).flags & NOGORM)) {
                actionSuperGo(roomId);
                return true;
            }
            Print(GetNamedString("supergo.nogorm", "You cannot travel to that place.\n"));
        } else {
            Print(GetNamedString("supergo.unknown",
                                 "I didn't recognize the room name in that super-go command.\n"));
        }
        Action::FailParse();
        return false;
    }
    return {};
}

optional<bool>
parseSpeech(string_view text)
{
    const char quote = text.front();
    if (quote != '\"' && quote != '\'') {
        return {};
    }

    // Ignore an empty string.
    if (text.length() == 1) {
        Print("...\"?");
        return true;
    }

    // Look up the speech verb once.
    static const verbid_t speechVerb = IsVerb("\"speech");
    if (speechVerb == WNONE) {
        Print(CANTDO);
        return true;
    }

    // Skip the leading quote.
    text.remove_prefix(1);

    // No special processing - don't even try to remove a trailing close quote
    iverb = speechVerb;
    wtype[2] = WTEXT;
    inoun1 = amulid_t(text.data());

    lang_proc(iverb, 0);

    return true;
}

bool
parser(string_view text)
{
    auto oldMore = more;  // We need to know if this is phrase one in a mo...

    // Kill any parse that was already in progress.
    if (iverb > WNONE)
        lverb = iverb;

    Action::AbortParse();
    failed = false;
    actor = WNONE;

    Trim(&text);
    if (text.empty())
        return true;

    using namespace std;
    fill(begin(wtype), end(wtype), WNONE);

    if (auto result = parseSuperGo(text); result)
        return result.value();

    if (auto result = parseSpeech(text); result)
        return result.value();

    // Did they type a verb synonym?
    char *p = const_cast<char *>(text.data());
    auto word = isaverb(&p);
    if (word == WNONE) {
#ifdef PARSER_CODE
        char *bp;
        /*== If not a verb, then check for g_game.numObjects. If they typed
            > get NounA, NounB, NounC  then allow it.
                If they typed
            > kiss PlayerA, NounA
                don't allow it.
        */

        bp = p;
        if (oldMore == 10 || (x = getTokenType(&bp)) == -2) {
            Print(INVALIDVERB);
            more = 1;
            return false;
        }
        word = iverb;
#endif
        Print(INVALIDVERB);
        Action::AbortParse();
        return false;
    }

    auto &verb = GetVerb(word);
    if ((t_avatar->flags & PFASLEEP) && !(verb.flags & VB_DREAM)) {
        Print(GetNamedString("parse.sleeping", "You can't do anything until you wake up!\n"));
        Action::AbortParse();
        return false;
    }
    if (verb.flags & VB_TRAVEL) {
        // Allows the user to intercept travel attempts.
        static const verbid_t travelVerb = IsVerb("\"travel");
        if (travelVerb != WNONE) {
            if (!lang_proc(travelVerb, 1))
                return false;
        }
    }

    iverb = word;
    vbptr = &GetVerb(iverb);
    wtype[0] = WVERB;

    // adjectives are optional, so assume next word is a noun
l1:
    if (*p == 0)
        goto ended;

    static constexpr auto tokenize = [](char *&ptr, WType &wt, auto &into) {
        auto result = GetTokenType(&ptr);
        wt = result.first, into = result.second;
    };

    tokenize(p, wtype[2], inoun1);

    if (wtype[2] == WNOUN)
        last_it = inoun1;

    if (wtype[2] == WADJ) {
        if (wtype[1] != WNONE) {
            Print(NONOUN);
            Action::FailParse();
            return false;
        }
        wtype[1] = WADJ;
        iadj1 = inoun1;
        wtype[2] = WNONE;
        inoun1 = WNONE;
        goto l1;
    }
    if (wtype[2] == WPREP) {
        if (wtype[3] != WNONE) {
            Print(WORDMIX);
            Action::FailParse();
            return false;
        }
        wtype[3] = WPREP;
        iprep = inoun1;
        wtype[2] = WNONE;
        inoun1 = WNONE;
    }
l2:
    if (*p == 0)
        goto ended;

    tokenize(p, wtype[5], inoun2);

    if (wtype[5] == WNOUN)
        last_it = inoun2;
    if (wtype[5] == WPREP) {
        if (wtype[3] != WNONE) {
            Print(WORDMIX);
            Action::FailParse();
            return false;
        }
        wtype[3] = WPREP;
        iprep = inoun2;
        wtype[5] = WNONE;
        inoun2 = WNONE;
        goto l2;
    }
    if (wtype[5] == WADJ) {
        if (wtype[4] != WNONE) {
            Print(NONOUN);
            Action::FailParse();
            return false;
        }
        wtype[4] = WADJ;
        iadj2 = inoun2;
        wtype[5] = WNONE;
        inoun2 = WNONE;
        goto l2;
    }
ended:
    overb = iverb;
    vbptr = &GetVerb(iverb);

    iocheck();
    if (t_forced || exeunt != 0 || died != 0 || failed == true)
        return true;
    return lang_proc(iverb, 0);
}

// Copy INPUT to BLOCK, taking one sentence at a time, etc
bool
parseSentences()
{
    string_view input = t_inputBuffer;
    Trim(&input);

    more = 10;
    t_forced = false;
    exeunt = 0;
    failed = false;
    died = 0;
    donet = -1;
    ml = 0;

#ifdef PARSER_CODE
    do {
        while (*s != 0 && !isspace(*s) && *s != '!' && *s != ';' && *s != ',' && *s != '.' &&
               *s != '\"' && *s != '\'')
            *(d++) = *(s++);
        *d = 0;
        *(d + 1) = 0;
        if (stricmp(p, "then") == 0 || stricmp(p, "and") == 0) {
            *p = 0;
            goto proc;
        }
        if (*s == '\'' || *s == '\"')
            goto quotes;
        if (isspace(*s))
            continue;
    proc:
        if (*s != 0 && *s != '\'' && *s != '\"')
            s++;
        if (block[0] == 0)
            continue;
        // Print the prompt & the line, if not first text
        if (more != 10) {
            Ansify("3m");
            Print(GetRank(t_character->rank).prompt);
            Print(input);
            Ansify("m");
        }
        if (parser(input) == false)
            return false;
    } while (*s != 0 && more == 0 && exeunt == 0 && !t_forced && !failed && died == 0);
#else
    if (parser(input) == false)
        return false;
#endif
    return true;
}

int
ttproc()
{
    exeunt = died = donet = skip = 0;
    failed = false;
    auto &room = GetRoom(t_avatar->room);
    tt.verb = iverb;
    if (room.ttlines == 0) {
        Print(CANTGO);
        return 0;
    }

    int dun = -1;
    ml = room.ttlines;
    const auto *travels = room.ptr;
    while (donet < ml && !exeunt && !died) {
        int match{ -1 };
        for (int i = donet; i < ml; i++) {
            const auto &travel = travels[i];
            donet++;
            if (skip != 0) {
                skip--;
                continue;
            }
            if (travel.verb == iverb && cond(travel, true)) {
                match = i;
                break;
            }
        }
        skip = 0;
        // Special case for testing spells
        if (travels->condition.m_op == CSPELL && match == -1)
            return 0;
        if (match == -1)
            return dun;
        inc = 1;
        executeAction(travels[match]);
        if (inc == 1)
            dun = 0;
        if (ml < -1) {
            ml = room.ttlines;  /// TODO: PARSER_CODE: This was roomtab->, could it have changed?
            donet = 0;
        }
    }
    return 0;
}

bool
lang_proc(verbid_t verbId, char e)
{
    t_forced = false;
    exeunt = 0;
    failed = false;
    died = 0;
    donet = 0;
    ml = 0;
    int d = -2;

    tt.verb = -1;
    auto &verb = GetVerb(verbId);

    for (int i = 0; i < GetVerb(verbId).ents; i++) {
        syntaxp = verb.ptr + i;
        donet = 0;
        ml = syntaxp->ents;
        int m = 0;
        if (syntaxp->wtype[2] != WANY) {
            for (int j = 0; j < 5 && m == 0; j++) {
                if (syntaxp->wtype[j] == WANY && (j == 0 || j == 3 || wtype[j + 1] != WNONE))
                    continue;
                if (syntaxp->wtype[j] != wtype[j + 1]) {
                    m = 1;
                    continue;
                }
                // We have a match, now see if its the same word!
                if (syntaxp->slot[j] == WANY)
                    continue;
                switch (j) {
                    case 0:
                        if (iadj1 != syntaxp->slot[j])
                            m = 1;
                        break;
                    case 1:
                        if (syntaxp->slot[j] == WNONE && inoun1 == WNONE)
                            break;
                        if (syntaxp->wtype[j] == WPLAYER && inoun1 == t_slotId &&
                            syntaxp->slot[j] == -3)
                            break;
                        if (syntaxp->wtype[j] == WTEXT &&
                            stricmp((const char *) inoun1, GetString(syntaxp->slot[j])) == 0)
                            break;
                        if (syntaxp->wtype[j] == WNOUN &&
                            stricmp(GetObject(inoun1).id, GetObject(syntaxp->slot[j]).id) == 0)
                            break;
                        if (inoun1 != syntaxp->slot[j])
                            m = 1;
                        break;
                    case 2:
                        if (iprep != syntaxp->slot[j])
                            m = 1;
                        break;
                    case 3:
                        if (iadj2 != syntaxp->slot[j])
                            m = 1;
                        break;
                    case 4:
                        if (syntaxp->slot[j] == WNONE && inoun2 == WNONE)
                            break;
                        if (syntaxp->wtype[j] == WPLAYER && inoun2 == t_slotId &&
                            syntaxp->slot[j] == -3)
                            break;
                        if (syntaxp->wtype[j] == WTEXT &&
                            stricmp((const char *) inoun2, GetString(syntaxp->slot[j])) == 0)
                            break;
                        if (syntaxp->wtype[j] == WNOUN &&
                            stricmp(GetObject(inoun2).id, GetObject(syntaxp->slot[j]).id) == 0)
                            break;
                        if (inoun2 != syntaxp->slot[j])
                            m = 1;
                        break;
                }
            }
            if (m != 0)
                goto after;
        }
        {
            bool lastResult = true;
            d = -1;
            for (donet = 0; donet < ml; donet++) {
                syntaxp = vbptr->ptr + i;
                const auto &vmLine = syntaxp->ptr[donet];
                if (skip != 0) {
                    skip--;
                    continue;
                }
                if ((lastResult = cond(vmLine, lastResult)) == false)
                    continue;
                inc = 1;
                executeAction(vmLine);
                if (inc == 1)
                    d = 0;
                if (ml < -1) {
                    d = lang_proc(iverb, e);
                    donet = ml + 1;
                    break;
                }
                if (ml < 0 || failed == true || t_forced || died != 0 || exeunt != 0)
                    break;
            }
            if (failed == true || t_forced || died != 0 || exeunt != 0)
                break;
        }
    after:
        if (donet > ml)
            break;
    }
    if (d > -1)
        return true;  // If we processed something...

    // Incase iverb changed
    vbptr = &GetVerb(iverb);
    if (vbptr->flags & VB_TRAVEL) {
        auto savedVerb = iverb;
        iverb = verbId;
        if (ttproc() == 0)
            return true;
        else
            d = -1;
        iverb = savedVerb;
    }

    if (d == -2 && e == 0)
        Print(ALMOST);
    if (d == -1 && e == 0) {
        if (vbptr->flags & VB_TRAVEL)
            Print(CANTGO);
        else
            Print(CANTDO);
    }
    return false;
}

// Phrase/sentence processing

void
ActionGoToRoom(roomid_t roomId)
{
    auto &room = GetRoom(roomId);

    t_needCR = false;
    if (t_follow)
        Action::StopFollowing();

    CriticalSection cs{};
    // Stop anyone who was following us.
    if (room.flags & SMALL) {
        if (HasAny(
                    g_game.m_avatars, [=](auto &avatar) noexcept {
                        return avatar.room == roomId;
                    })) {
            cs.Release();
            actionin(roomId, GetString(NOROOMIN));
            Action::LoseFollower();
            return;
        }
    }

    t_avatar->flags = t_avatar->flags | PFMOVING;  // As of now I'm out of here.

    if (isVisible()) {
        cs.Release();
        action(t_avatar->dep, AOTHERS);
        cs.Acquire();
    }

    ldir = iverb;
    lroom = t_avatar->room;
    auto oldLight = t_avatar->light;
    t_avatar->light = 0;
    cs.Release();
    lighting(t_slotId, AOTHERS);
    t_avatar->room = roomId;
    t_avatar->light = oldLight;
    t_avatar->hadlight = 0;
    lighting(t_slotId, AOTHERS);
    if (isVisible())
        action(t_avatar->arr, AOTHERS);
    t_avatar->flags &= ~PFMOVING;
    if (t_avatar->followed > -1 && t_avatar->followed != t_slotId && (!IamINVIS) && (!IamSINVIS)) {
        /* If we didn't just execute a travel verb, we've lost them.
            If the other player hasn't caught up with us, lose them! */
        if ((GetVerb(overb).flags & VB_TRAVEL) || GetAvatar(t_avatar->followed).room != lroom ||
            (GetAvatar(t_avatar->followed).flags & PFMOVING))
            Action::LoseFollower();
        else {
            DoThis(t_avatar->followed, GetVerb(overb).id, 1);
        }
    } else
        t_avatar->followed = -1;
    look(t_avatar->room, t_character->rdmode);
    if (exeunt != 0 || died != 0)
        return;
    if (autoexits != 0)
        exits();
}

void
executeAction(const VMLine &vmline)
{
    vmopid_t actionOp = vmline.action.m_op;
    const auto &args = vmline.action.m_args;
    switch (actionOp) {
        case AGOTO_ROOM:
            ActionGoToRoom(args[0]);
            break;

        case ASAVE:
            Action::SavePlayerCharacter();
            break;
        case AWHO:
            Action::Who(Verbosity(args[0]));
            break;
        case ATREATAS:
            Action::TreatAsVerb(args[0]);
            return;

        case AQUIT:
            Action::QuitPlayer();
            Action::EndParse();
            break;

        case AENDPARSE:
            Action::EndParse();
            break;

        case AERROR:
            // error prints a message and fails the parse
            Action::PrintText(args[0]);
            Action::FailParse();
            break;

        case ARESPOND:
            Action::PrintText(args[0]);
            Action::EndParse();
            break;

        case AREPLY:
        case AMESSAGE:
            Action::PrintText(args[0]);
            break;

        case AFAILPARSE:
            Action::FailParse();
            break;
        case AFINISHPARSE:
            Action::FinishParse();
            break;
        case AABORTPARSE:
            Action::AbortParse();
            break;

        case AWHEREAMI:
            Printf("Current room is known as \"%s\".\n", GetRoom(t_avatar->room).id);
            break;

        case AINTERACT:
            Action::SetInteractingWith(args[0]);
            break;

        case AMOVE:
            Action::MovePlayerTo(args[0]);
            break;

        case ARDMODE:  // room description mode
            Action::SetRoomDescMode(RoomDescMode(args[0]));
            break;

        case ASLEEP:
            Action::SetPlayerFlags(PFASLEEP, 0);
            break;

        case AWAKE:
            Action::SetPlayerFlags(0, PFASLEEP);
            break;

        case ASIT:
            Action::SetPlayerFlags(PFSITTING, PFLYING);
            break;

        case ASTAND:
            Action::SetPlayerFlags(0, PFSITTING | PFLYING);
            break;

        case ALIE:
            Action::SetPlayerFlags(PFLYING, PFSITTING);
            break;

        case AINVENT:
            Action::DescribeInventory();
            break;

        case ALOSE:
            Action::LoseFollower();
            break;
        case ASTOPFOLLOW:
            Action::StopFollowing();
            break;

        // Skip forward a number of lines
        case ASKIP:
            skip += args[0];
            break;

        // uncoverted
        case ASCORE:
            ascore(args[0]);
            break;
        case ASETSTATE:
            asetstate(args[0], args[1]);
            break;
        case ALOOK:
            look(t_avatar->room, RDVB);
            break;
        case AWHAT:
            list_what(t_avatar->room, true);
            break;
        case AWHERE:
            awhere(args[0]);
            break;
        case ATRAVEL: {
            auto donetSave = donet;
            auto mlSave = ml;
            if (ttproc() == 0)
                donet = ml = mlSave;
            else
                donet = donetSave, ml = mlSave;
            break;
        }
        case AKILLME:
            akillme();
            Action::EndParse();
            break;
        case AWAIT:
            fwait(args[0]);
            break;
        case ABLEEP:
            ableep(args[0]);
            break;

        case ASEND:
            send(args[0], args[1]);
            break;

        case AANOUN:
            /// TODO: PARSER_CODE: Make sure we actually get a string
            announce((const char *) GetConcreteValue(args[1]), args[0]);
            break;

#ifdef MESSAGE_CODE
        case ACHANGESEX:
            achange(args[0]);
            break;
#endif

#ifdef PARSER_CODE
        case ARESET:
            SendIt(MRESET, 0, nullptr);
            break;  // Tell AMAN that we want a reset!
#endif

        case AACTION:
            /// TODO: PARSER_CODE: Check that's what we actually get
            action((const char *) GetConcreteValue(args[1]), args[0]);
            break;

        case AMSGIN:
            announcein(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case AACTIN:
            actionin(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case AMSGFROM:
            announcefrom(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case AACTFROM:
            actionfrom(args[0], (const char *) GetConcreteValue(args[1]));
            break;

        case ATELL:
            if (!(GetAvatar(args[0]).flags & PFDEAF)) {
                SetMxxMxy(NOISE, args[0]);
                /// TODO: PARSER_CODE: FIX
                PrintSlot((slotid_t) args[0], (const char *) GetConcreteValue(args[1]));
            }
            break;

        case AADDVAL:
            aadd(scaled(GetObject(args[0]).State().value, GetObject(args[0]).State().flags),
                 STSCORE,
                 t_slotId);
            break;

        case AGET:
            agive(args[0], t_slotId);
            break;

        case ADROP:
            adrop(args[0], t_avatar->room, true);
            break;

        case AGIVE:
            agive(args[0], args[1]);
            break;
        case AINFLICT:
            inflict(args[0], SpellID(args[1]));
            break;
        case ACURE:
            cure(args[0], SpellID(args[1]));
            break;
        case ASUMMON:
            asummon(args[0]);
            break;
        case AADD:
            aadd(args[0], StatID(args[1]), args[2]);
            break;
        case ASUB:
            asub(args[0], StatID(args[1]), args[2]);
            break;
        case ACHECKNEAR:
            achecknear(args[0]);
            break;
        case ACHECKGET:
            acheckget(args[0]);
            break;
        case ADESTROY:
            adestroy(args[0]);
            break;
        case ARECOVER:
            arecover(args[0]);
            break;
        case ASTART:
            Action::Demon::Schedule(args[0], args[1], false);
            break;
        case AGSTART:
            Action::Demon::Schedule(args[0], args[1], true);
            break;
        case ACANCEL:
            Action::Demon::Cancel(args[0]);
            break;
        case ABEGIN:
            Action::Demon::ForceExecute(args[0]);
            break;
        case ASHOWTIMER:
            Action::Demon::Status(args[0]);
            break;
        case AOBJAN:
            objannounce(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case AOBJACT:
            objaction(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case ACONTENTS:
            Print(showin(args[0], true));
            break;
        case AFORCE:
            aforce(args[0], (const char *) GetConcreteValue(args[1]));
            break;
        case AHELP:
            t_avatar->helping = args[0];
            GetAvatar(args[0]).helped = t_slotId;
            break;
        case ASTOPHELP:
            Action::CancelHelp();
            break;
        case AFIX:
            afix((StatID) args[0], args[1]);
            break;
        case AOBJINVIS:
            GetObject(args[0]).flags = GetObject(args[0]).flags | OF_INVIS;
            break;
        case AOBJSHOW:
            GetObject(args[0]).flags = GetObject(args[0]).flags & (-1 - OF_INVIS);
            break;
        case AEXITS:
            exits();
            break;
        case ATASK:
            t_character->tasks = t_character->tasks | (1 << (args[0] - 1));
            break;
        case ASHOWTASK:
            show_tasks(t_slotId);
            break;
        case ASETPRE:
            iocopy(GetAvatar(args[0]).pre, (const char *) GetConcreteValue(args[1]), 79);
            break;
        case ASETPOST:
            iocopy(GetAvatar(args[0]).post, (const char *) GetConcreteValue(args[1]), 79);
            break;
        case ASETARR:
            qcopy(GetAvatar(args[0]).arr, (const char *) GetConcreteValue(args[1]), 79);
            strcat(GetAvatar(args[0]).arr, "\n");
            break;
        case ASETDEP:
            qcopy(GetAvatar(args[0]).dep, (const char *) GetConcreteValue(args[1]), 79);
            strcat(GetAvatar(args[0]).dep, "\n");
            break;

        case AAUTOEXITS:
            /// TODO: PARSER_CODE: Make this a character setting
            autoexits = (char) args[0];
            break;

        case ABURN:
            osflag(args[0], GetObject(args[0]).State().flags | SF_LIT);
            break;
        case ADOUSE:
            osflag(args[0], GetObject(args[0]).State().flags & ~SF_LIT);
            break;
        case AINC:
            if (GetObject(args[0]).state != (GetObject(args[0]).nstates - 1))
                asetstate(args[0], GetObject(args[0]).state + 1);
            break;
        case ADEC:
            if (GetObject(args[0]).state != 0)
                asetstate(args[0], GetObject(args[0]).state - 1);
            break;
        case ATOPRANK:
            PromoteToTopRank();
            break;

#ifdef PARSER_CODE
        case AFIGHT:
            afight(args[0]);
            break;
        case AFLEE:
            dropall(GetAvatar(t_avatar->fighting).room);
            clearfight();
            break;
        case ALOG:
            log(GetConcreteValue(args[0]));
            break;
        case ACOMBAT:
            acombat();
            break;

        case AWIELD:
            /// GAH: This might no-longer be legal!
            t_avatar->wield = args[0];
            break;
        /* - */ case AFOLLOW:
            GetAvatar(args[0]).followed = t_avatar->unum;
            t_avatar->following = args[0];
            break;
        case ASYNTAX:
            asyntax(*(tt.pptr + ncop[tt.condition.m_op]), *(tt.pptr + ncop[tt.condition.m_op] + 1));
            break;
        case ASENDDAEMON:
            dsend(args[0], args[1], args[2]);
            break;
        case ADO:
            ado(args[0]);
            break;
        case ADEDUCT:
            deduct(args[0], args[1]);
            break;
        case ADAMAGE:
            damage(args[0], args[1]);
            break;
        case AREPAIR:
            repair(args[0], args[1]);
            break;
#endif

        default:
            Printf("** Internal error - illegal action %ld!\n", actionOp);
    }
    if (tt.condition.m_op == CANTEP || tt.condition.m_op == CALTEP || tt.condition.m_op == CELTEP)
        donet = ml + 1;
}

bool
IsINoun(objid_t objid, WType iWtype, objid_t iNoun) noexcept
{
    if (objid == WNONE || iWtype != WNOUN)
        return false;
    return stricmp(GetObject(objid).id, GetObject(iNoun).id) == 0;
}

// Execute a condition on me
bool
cond(const VMLine &line, bool lastResult)
{
    // when notCondition is false, then we return false on failure
    const auto &condition = line.condition;
    bool result = true;

    // Do the conditions
    switch (condition.m_op) {
        case CALTEP:
        case CSTAR:
        case CALWAYS:
            break;
        case CANTEP:
        case CAND:
            result = lastResult;
            break;
        case CELTEP:
        case CELSE:
            result = !lastResult;
            break;
        case CLIGHT:
            result = lit(t_avatar->room);
            break;
        case CISHERE:
            result = IsObjectIn(CA1, t_avatar->room);
            break;
        case CMYRANK:
            result = isValidNumber(t_character->rank + 1, CA1);
            break;
        case CSTATE:
            result =
                    !(GetObject(CA1).flags & OF_ZONKED) && isValidNumber(GetObject(CA1).state, CA2);
            break;
        case CMYSEX:
            result = t_character->gender == CA1;
            break;
        case CLASTVB:
            result = lverb == CA1;
            break;
        case CLASTDIR:
            result = ldir == CA1;
            break;
        case CLASTROOM:
            result = lroom == CA1;
            break;
        case CASLEEP:
            result = (t_avatar->flags & PFASLEEP);
            break;
        case CSITTING:
            result = (t_avatar->flags & PFSITTING);
            break;
        case CLYING:
            result = (t_avatar->flags & PFLYING);
            break;
        case CRAND:
            result = isValidNumber(RandomInt(0, CA1), CA2);
            break;
        case CRDMODE:
            result = t_character->rdmode == RoomDescMode(CA1);
            break;
        case CONLYUSER:
            for (int i = 0; i < MAXU; i++) {
                if (i != t_slotId && GetAvatar(i).state == PLAYING) {
                    result = false;
                    break;
                }
            }
            break;
        case CALONE:
            for (int i = 0; i < MAXU; i++) {
                if (GetAvatar(i).room == t_avatar->room && i != t_slotId) {
                    result = false;
                    break;
                }
            }
            break;
        case CINROOM:
            result = t_avatar->room == CA1;
            break;
        case COPENS:
            result = GetObject(CA1).flags & OF_OPENS;
            break;
        case CGOTNOWT:
            result = t_avatar->numobj == 0;
            break;
        case CCARRYING:
            result = gotin(CA1, -1);
            break;
        case CNEARTO:
            result = nearto(CA1);
            break;
        case CHIDDEN:
            result = !isVisible();
            break;
        case CCANGIVE:
            result = canGive(CA1, CA2);
            break;
        case CINFL:
        case CINFLICTED:
            result = isInflicted(CA1, (SpellID) CA2);
            break;
        case CSAMEROOM:
            result = GetAvatar(CA1).room == t_avatar->room;
            break;
        case CTOPRANK:
            result = t_character->rank == g_game.MaxRank();
            break;
        case CSOMEONEHAS:
            result = GetObject(CA1).IsOwned();
            break;
        case CGOTA:
            result = gotin(CA1, CA2);
            break;
#ifdef PARSER_CODE
        case CACTIVE:
            SendIt(MCHECKD, CA1, nullptr);
            if (amul->data == -1)
                ret = -1;
            break;
        case CTIMER:
            SendIt(MCHECKD, CA1, nullptr);
            if (amul->data == -1 || !isValidNumber(amul->p1, CA2))
                ret = -1;
            break;
#endif
        case CBURNS:
            result = GetObject(CA1).flags & OF_FLAMABLE;
            break;
        case CCONTAINER:
            result = GetObject(CA1).contains > 0;
            break;
        case CEMPTY:
            result = GetObject(CA1).inside == 0;
            break;
        case COBJSIN:
            result = isValidNumber(GetObject(CA1).inside, CA2);
            break;
        case CHELPING:
            result = t_avatar->helping == CA1;
            break;
        case CGOTHELP:
            result = t_avatar->helped != -1;
            break;
        case CANYHELP:
            result = t_avatar->helping > 0;
            break;
        case CSTAT:
            result = testStat(CA2, (StatID) CA1, CA3);
            break;
        case COBJINV:
            result = isOINVIS(CA1);
            break;
        case CFIGHTING:
            result = GetAvatar(CA1).flags & PFFIGHT;
            break;
        case CTASKSET:
            result = (t_character->tasks & (1 << (CA1 - 1)));
            break;
        case CCANSEE:
            result = CanSee(t_slotId, CA1);
            break;
        case CVISIBLETO:
            result = CanSee(CA1, t_slotId);
            break;
        case CNOUN1:
            result = IsINoun(CA1, wtype[2], inoun1);
            break;
        case CNOUN2:
            result = IsINoun(CA1, wtype[5], inoun2);
            break;
        case CAUTOEXITS:
            result = autoexits != 0;
            break;
        case CDEBUG:
            result = debug;
            break;
        case CFULL:
            result = isStatFull(CA1, CA2);
            break;
        case CTIME:
            result = isValidNumber(*rescnt, CA1);
            break;
        case CDEC:
            if (GetObject(CA1).state == 0)
                result = false;
            else
                asetstate(CA1, GetObject(CA1).state - 1);
            break;
        case CINC:
            if (GetObject(CA1).state >= (GetObject(CA1).nstates - 1))
                result = false;
            else
                asetstate(CA1, GetObject(CA1).state + 1);
            break;
        case CLIT:
            result = (GetObject(CA1).State().flags & SF_LIT);
            break;
        case CFIRE:
            result = GetObject(CA1).flags & OF_SHOWFIRE;
            break;
        case CHEALTH:
            result = isValidNumber(
                    (GetAvatar(CA1).stamina * 100) / (GetRank(GetCharacter(CA1).rank).stamina),
                    CA2);
            break;
        case CMAGIC:
            result = castWillSucceed(CA1, CA2, CA3);
            break;
        case CSPELL:
            result = isValidNumber(GetAvatar(CA1).wisdom, RandomInt(0, CA2));
            break;
        case CIN:
            result = IsObjectIn(CA2, CA1);
            break;
        default:
            result = false;
    }

    if (line.notCondition)
        result = !result;
    return result;
}

amulid_t
GetConcreteValue(amulid_t srcValue)
{
    // Rand 0 is a simple rand(N)
    if (srcValue & RAND0)
        return RandomInt(0, srcValue ^ RAND0);
    // Rand 1 is N +/- .5N
    if (srcValue & RAND1) {
        auto nval = srcValue ^ RAND1;
        return nval / 2 + RandomInt(0, nval);
    }
    if (srcValue & PRANK)
        return pRANK(GetConcreteValue(srcValue & ~PRANK));

    if ((srcValue & (OBVAL + OBDAM + OBWHT)) != 0) {
        int x;
        x = GetConcreteValue(srcValue & ~(OBVAL + OBDAM + OBWHT));
        switch (srcValue & (OBVAL + OBDAM + OBWHT)) {
            case OBVAL:
                return (int) scaled(GetObject(x).State().value, GetObject(x).State().flags);
            case OBWHT:
                return (int) GetObject(x).State().weight;
            case OBDAM:
                return (int) GetObject(x).State().strength;
            case OBLOC:
                return (int) getLocationOf(x);
        }
        return -1;
    }
    if ((srcValue & IWORD) == IWORD) {
        // Replace with no. of a users word
        switch (srcValue & (-1 - IWORD)) {
            case IVERB:
                return iverb;
            case IADJ1:
                return iadj1;
            case INOUN1:
                return inoun1;
            case IPREP:
                return iprep;
            case IADJ2:
                return iadj2;
            case INOUN2:
                return inoun2;
        }
        return -1;
    }
    if ((srcValue & MEPRM) == MEPRM) {
        // Replace with details of self
        switch (srcValue & (-1 - MEPRM)) {
            case LOCATE:
                return -1;  // Not implemented
            case SELF:
                return (int) t_slotId;
            case HERE:
                return (int) t_avatar->room;
            case RANK:
                return (int) t_character->rank;
            case FRIEND:
                return (int) t_avatar->helping;
            case HELPER:
                return (int) t_avatar->helped;
            case ENEMY:
                return (int) t_avatar->fighting;
            case WEAPON:
                return (int) t_avatar->wield;
            case SCORE:
                return (int) t_character->score;
            case SCTG:
                return (int) t_avatar->sessionScore;
            case STR:
                return (int) t_avatar->strength;
            case LASTROOM:
                return (int) lroom;
            case LASTDIR:
                return (int) ldir;
            case LASTVB:
                return (int) lverb;
        }
        return -1;
    }
    return srcValue;
}

#ifdef MESSAGE_CODE
void
deduct(int plyr, int howmuch)
{
    if (howmuch < 0)
        return;
    if (plyr == t_slotId) {
        int amount = t_character->score * howmuch / 100;
        asub(amount, STSCORE, t_slotId);
    } else
        sendex(plyr, ADEDUCT, plyr, howmuch, 0, 0);  // Tell them to clear up!
}
#endif  // MESSAGE_CODE

error_t
PlayerClientUp()
{
    std::cout << "player client started" << std::endl;

    /// TODO: Should really get a lock before doing this or something. Maybe you should
    /// *start* locked and
    // your "ready to go" signal should be your first unlock?
    t_avatar = &GetAvatar(t_slotId);
    t_character = &GetCharacter(t_slotId);
    t_avatar->slotId = t_slotId;

    link = 1;

    t_avatar->ioType = t_iosup;
    t_avatar->m_outputBuffer[0] = '\0';

    return 0;
}

void
PlayerClientDown()
{
    std::cout << "player client finished" << std::endl;

    // Let the manager know we're done
    if (t_managerPort)
        t_managerPort->Put(make_unique<MsgDisconnectClient>());

    t_replyPort->Close();

    CloseFile(&ofp1);
    CloseFile(&ifp);
    CloseFile(&afp);
}

// Original AMUL entry point
void
amul_main()
{
    if (t_slotId < 0 || t_slotId >= MAXU) {
        LogFatal("reached amul_main without a valid t_slotId: ", t_slotId);
    }

#ifdef MESSAGE_CODE
    /// TODO: Tell the SERVER we're unlocked.
#endif
    t_inIocheck = false;
    t_avatar->IOlock = -1;
    SendMsg<MsgSetBusy>(false);

    // Clear room flags, and send scenario
    g_game.m_visited[t_slotId].clear();
    g_game.m_visited[t_slotId].resize(g_game.m_rooms.size());

    PlayerLogin();

    last_him = last_her = last_it = -1;

    do {
        died = 0;
        actor = -1;
        t_follow = false;
        t_needCR = false;
        t_addCR = false;
        if (last_him != -1 && GetAvatar(last_him).state != PLAYING)
            last_him = -1;
        if (last_her != -1 && GetAvatar(last_her).state != PLAYING)
            last_her = -1;
        iocheck();
        Print(GetRank(t_character->rank).prompt);
        t_needCR = true;
        SafeInput(t_inputBuffer);
        if (exeunt != 0)
            break;
        if (stricmp(t_inputBuffer, "help") == 0) {
            Print(HELP);
            continue;
        }
        if (t_inputBuffer[0] == '/') {
            SlashCommands(t_inputBuffer + 1);
            continue;
        }
        if (stricmp(t_inputBuffer, "***debug") == 0) {
            debug = !debug;
            continue;
        }
        if (t_inputBuffer[0] == 0)
            continue;
    gloop:
        failed = false;
        t_forced = false;
        died = 0;
        exeunt = 0;
        if (!parseSentences())
            continue;
        iocheck();
        if (t_forced && died == 0 && exeunt == 0)
            goto gloop;
    } while (exeunt == 0 && died == 0);

    if (died == 0)
        action(GetString(EXITED), AGLOBAL);
    else
        action(GetString(HEDIED), AGLOBAL);
    t_forced = false;
    exeunt = 0;
    died = 0;
    if (t_character->plays == 1)
        Print(BYEPASSWD);

#ifdef MESSAGE_CODE
    if (dmove[0] != 0)
        dropall(isRoomName(dmove));
    else
        dropall(t_avatar->room);
    LoseFollower();  // Lose who ever is following us.
#endif
}
