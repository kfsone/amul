#include <cstring>
#include <ctime>

#include "game.h"
#include "amul.stringmanip.h"
#include "typedefs.h"
#include "client.io.h"
#include "parser.context.h"

extern long scaled(long value, flag_t flags);
extern void fwait(long n);
extern uint32_t GetResetCountdown();
extern bool CanSee(slotid_t viewer, slotid_t subject);

extern thread_local slotid_t t_slotId;

// Other players won't always know your identity. For example, if you're invisible
// and you shout, the game might want to obscure your identity and make you a distance voice
/// TODO: Document them better
static thread_local char t_mxx[64], t_mxy[64];

static thread_local char t_textScratch[4096];

string_view
Trim(string_view *view) noexcept
{
    while (!view->empty() && view->front() == ' ')
        view->remove_prefix(1);
    while (!view->empty() && view->back() == ' ')
        view->remove_suffix(1);
    return *view;
}

std::string
Meld(string_view prefix, string_view content, string_view suffix)
{
    Trim(&prefix);
	Trim(&content);
	Trim(&suffix);
    std::string result { prefix };
    if (!result.empty() && !content.empty()) {
        result += " ";
        result += content;
    } else {
        result.assign(content);
    }
    if (!suffix.empty()) {
        if (!result.empty() && !ispunct(suffix.front()))
            result += " ";
    }
    result += suffix;
    return result;
}

std::string
MakeTitle(slotid_t slotId)
{
    auto &player = GetCharacter(slotId);
    auto &avatar = GetAvatar(slotId);
    const auto title = player.gender == 0 ? GetRank(player.rank).male : GetRank(player.rank).female;

    return Meld(avatar.pre, title, avatar.post);
}

void
PutRankInto(char *into) noexcept
{
    PutARankInto(into, t_slotId);
}

void
PutARankInto(char *into, slotid_t slot) noexcept
{
    auto &player = GetCharacter(slot);
    auto &avatar = GetAvatar(slot);

    if (avatar.pre[0] != 0) {
        const char *src = avatar.pre;
        while (*src != 0)
            *(into++) = *(src++);
        *(into++) = ' ';
    }
    char *src = (player.gender == 0) ? GetRank(player.rank).male : GetRank(player.rank).female;
    while (*src != 0)
        *(into++) = *(src++);
    if (avatar.post[0] != 0) {
        src = avatar.post;
        *(into++) = ' ';
        while (*src != 0)
            *(into++) = *(src++);
    }
    *into = 0;
}

// Translates the characters of an escape sequence into "s".
// E.g, if processing "@me" you pass a pointer to "me" and get the
// name of the player back.

bool
ProcessEscape(const char *escape, char *s)
{
    const char c = tolower(*(escape + 1));
    switch (tolower(*escape)) {
        case 'm':
            switch (c) {
                case 'e':
                    strcpy(s, t_character->name);
                    return true;
                case '!':
                    sprintf(s, "%-21s", t_character->name);
                    return true;
                case 'r':
                    PutRankInto(s);
                    return true;
                case 'f':
                    if (t_avatar->following == -1)
                        strcpy(s, "no-one");
                    else
                        strcpy(s, GetCharacter(t_avatar->following).name);
                    return true;
                case 'g':
                    sprintf(s, "%d", int(t_avatar->magicpts));
                    return true;
                default:
                    return false;
            }
        case 'g':
            switch (c) {
                case 'n':
                    strcpy(s, (t_character->gender == 0) ? "male" : "female");
                    return true;
                case 'e':
                    strcpy(s, (t_character->gender == 0) ? "he" : "she");
                    return true;
                case 'o':
                    strcpy(s, (t_character->gender == 0) ? "his" : "her");
                    return true;
                case 'h':
                    strcpy(s, (t_character->gender == 0) ? "him" : "her");
                    return true;
                case 'p':
                    sprintf(s, "%d", int(t_character->plays));
                    return true;
                default:
                    return false;
            }
        case 's':
            if (c == 'c') {
                sprintf(s, "%d", int(t_character->score));
                return true;
            }
            if (c == 'g') {
                sprintf(s, "%d", int(t_avatar->sessionScore));
                return true;
            }
            if (c == 'r') {
                sprintf(s, "%d", int(t_avatar->strength));
                return true;
            }
            if (c == 't') {
                sprintf(s, "%d", int(t_avatar->stamina));
                return true;
            }
            return false;
        case 'v':
            if (c == 'b') {
                strcpy(s, GetVerb(overb).id);
                return true;
            }
            if (c == 'e') {
                strcpy(s, GetVerb(iverb).id);
                return true;
            }
            if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
                const auto &state = GetObject(inoun1).State();
                sprintf(s, "%ld", scaled(state.value, state.flags));
                return true;
            }
            if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
                const auto &state = GetObject(inoun2).State();
                sprintf(s, "%ld", scaled(state.value, state.flags));
                return true;
            }
            return false;
        case 'w':
            if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
                const auto &state = GetObject(inoun1).State();
                sprintf(s, "%dg", state.weight);
                return true;
            }
            if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
                const auto &state = GetObject(inoun2).State();
                sprintf(s, "%dg", state.weight);
                return true;
            }
            if (c == 'i') {
                sprintf(s, "%d", t_avatar->wisdom);
                return true;
            }
            return false;
        case 'n':
            if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
                strcpy(s, GetObject(inoun1).id);
                return true;
            }
            if (c == '1' && wtype[2] == WTEXT) {
                strcpy(s, reinterpret_cast<const char*>(inoun1));
                return true;
            }
            if (c == '1' && inoun1 >= 0 && wtype[2] == WPLAYER) {
                strcpy(s, GetCharacter(inoun1).name);
                return true;
            }
            if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
                strcpy(s, GetObject(inoun2).id);
                return true;
            }
            if (c == '2' && wtype[5] == WTEXT) {
                strcpy(s, reinterpret_cast<const char*>(inoun2));
                return true;
            }
            if (c == '2' && inoun2 >= 0 && wtype[5] == WPLAYER) {
                strcpy(s, GetCharacter(inoun2).name);
                return true;
            }
            strcpy(s, "something");
            return true;
        case 'e':
            if (c == 'x') {
                sprintf(s, "%d", t_character->experience);
                return true;
            }
            return false;
        case 'l':
            if (c == 'r') {
                strcpy(s, g_game.lastResetTime);
                return true;
            }
            if (c == 's') {
                strcpy(s, g_game.lastStartupTime);
                return true;
            }
            if (c == 'c') {
                strcpy(s, g_game.lastCompileTime);
                return true;
            }
            return false;
        case 'p':
            if (c == 'l') {
                strcpy(s, GetCharacter(t_avatar->fighting).name);
                return true;
            }
            if (c == 'w') {
                strcpy(s, t_character->passwd);
                return true;
            }
            if (isdigit(c)) {
                fwait(c - '0');
                return true;
            }
            return false;
        case 'r':
            if (c == 'e') {
                FormatTimeInterval(s, GetResetCountdown());
                return true;
            }
            return false;
        case 'h': /* The person helping you */
			///TODO: There is the possibility of more than one
            if (c == 'e' && t_avatar->helped != WNONE) {
                strcpy(s, GetCharacter(t_avatar->helped).name);
                return true;
            }
            return false;
        case 'f': /* <friend> - person you are helping */
            if (c == 'r' && t_avatar->helping != WNONE) {
                strcpy(s, GetCharacter(t_avatar->helping).name);
                return true;
            }
			///TODO: There is the possibility of more than one.
            if (c == 'm' && t_avatar->followed != WNONE) {
                strcpy(s, GetCharacter(t_avatar->followed).name);
                return true;
            }
            strcpy(s, "no-one");
            return true;
        case 'o':
            if (c == '1' && t_avatar->wield != WNONE) {
                strcpy(s, GetObject(t_avatar->wield).id);
                return true;
            }
            if (c == '2') {
                if (auto &fighting = GetAvatar(t_avatar->fighting); fighting.wield != -1) {
                    strcpy(s, GetObject(fighting.wield).id);
                    return true;
                }
            }
            strcpy(s, "bare hands");
            return true;
        case 'x':
            if (c == 'x')
                strcpy(s, t_mxx);
            if (c == 'y')
                strcpy(s, t_mxy);
            return true;
        default:
            return false;
    }
}

// Consumes "@.." codes in strings and converts the text
char *
ProcessString(const char *src)
{
    /// TODO: search for an '@' sequence and, if none is found, return the source string.
    char *p = t_textScratch;
    for (;;) {
        if (*src == 0) {
            *p = 0;
            break;
        }
        if ((*p = *(src++)) == '@' && ProcessEscape(src, p)) {
            p += strlen(p);
            src += 2;
        } else {
            ++p;
        }
    }
    return t_textScratch;
}

// Get current time/date as string
const char *
GetTimeStr(time_t timenow) noexcept
{
    if (timenow == 0)
        timenow = time(nullptr);
    static thread_local char str[64];
#if defined(_MSC_VER)
    error_t err = ctime_s(str, sizeof(str), &timenow);
    if (err)
        return "{invalid.time}";
#else
    ctime_r(&timenow, str);
#endif

    char *text = str + 4;            // Skip the three letter day and a space
    *(text + strlen(text) - 1) = 0;  // Strip cr/lf

    return text;
}

void
FormatTimeInterval(char *into, uint32_t seconds) noexcept
{
    if (seconds >= 3600) /* More than an hour */
    {
        int x = seconds / 3600;       /* Hours */
        int y = seconds - (x * 3600); /* Minutes & seconds */
        if (y < 60)                   /* Upto 1 minute? */
        {
            sprintf(into,
                    "%d %s, %d %s",
                    x,
                    (x > 1) ? "hours" : "hour",
                    y,
                    (y > 1) ? "seconds" : "second");
            return;
        }
        y = y / 60;
        sprintf(into,
                "%d %s and %d %s",
                x,
                (x > 1) ? "hours" : "hour",
                y,
                (y > 1) ? "minutes" : "minute");
        return;
    }
    int x = seconds / 60;
    int y = seconds - (x * 60);
    if (x == 0 && y == 0)
        strcpy(into, "now");
    if (x != 0 && y == 0)
        sprintf(into, "%d %s", x, (x > 1) ? "minutes" : "minute");
    if (x == 0 && y != 0)
        sprintf(into, "%d %s", y, (y > 1) ? "seconds" : "second");
    if (x != 0 && y != 0)
        sprintf(into,
                "%d %s and %d %s",
                x,
                (x > 1) ? "minutes" : "minute",
                y,
                (y > 1) ? "seconds" : "second");
}

/* Set @xx and @xy corresponding to a specific player */

void
SetMxxMxy(BroadcastType style, slotid_t slotid) noexcept
{
    // For cases where you definitely know the identity
    if (slotid == t_slotId || CanSee(slotid, t_slotId)) {
        strcpy(t_mxx, ProcessString("@me"));
        strcpy(t_mxy, ProcessString("@me the @mr"));
        return;
    }

    if (pROOM(slotid) == t_avatar->room) {
        switch (style) {
            case ACTION:
            case EVENT:
            case TEXTS:
                strcpy(t_mxx, "Someone nearby");
                strcpy(t_mxy, "Someone nearby");
                return;
            case NOISE:
                strcpy(t_mxx, "Someone nearby");
                strcpy(t_mxy, ProcessString("A @gn voice nearby"));
                return;
        }
    }

    /* They aren't in the same room */
    switch (style) {
        case ACTION:
        case EVENT:
            strcpy(t_mxx, "Someone");
            if (t_character->rank == g_game.MaxRank())
                strcpy(t_mxy, "Someone very powerful");
            else
                strcpy(t_mxy, "Someone");
            return;
        case TEXTS:
            strcpy(t_mxx, ProcessString("@me"));
            if (t_character->rank == g_game.MaxRank())
                strcpy(t_mxy, ProcessString("@me the @mr"));
            else
                strcpy(t_mxy, t_mxx);
            return;
        case NOISE:
            strcpy(t_mxx, "Someone");
            if (t_character->rank == g_game.MaxRank())
                strcpy(t_mxy, ProcessString("A powerful @gn voice somewhere in the distance"));
            else
                strcpy(t_mxy, ProcessString("A @gn voice in the distance"));
            return;
    }
}
