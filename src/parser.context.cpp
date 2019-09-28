#include "parser.context.h"

#include <cctype>
#include <cstring>
#include <optional>
#include <string_view>
#include <unordered_set>

#include "game.h"
#include "typedefs.h"
#include "amul.xtra.h"
#include "amulinc.h"
#include "amullib.h"
#include "enumerate.h"
#include "objflag.h"
#include "stringmanip.h"

extern int isnoun(const char *s, adjid_t adj, const char *pat);

thread_local verbid_t iverb, overb;
thread_local adjid_t iadj1, iadj2;
thread_local amulid_t inoun1, inoun2;
thread_local prepid_t iprep;
thread_local verbid_t lverb, ldir;
thread_local roomid_t lroom;
thread_local WType wtype[6];
thread_local amulid_t last_it;
thread_local slotid_t actor, last_him, last_her; /* People we talked about */

static bool
skipNoiseWords(std::string &token, char **s)
{
    static std::unordered_set<std::string> noiseWords{
        // Words we want to ignore in the parse
        "a", "an", "at", "the", "to", "for", "from", "is", "of", "using", "with", "for", "from"
    };
    if (auto it = noiseWords.find(token); it != noiseWords.end()) {
        s += token.length();
        return true;
    }
    return false;
}

// New-style <optional>lookups
constexpr auto EnumerateAdjective = [](auto &&candidate) noexcept
{
    return Enumerate(
            g_game.m_adjectives, [&](auto &adjective) noexcept {
                return candidate == adjective.word;
            });
};

constexpr auto EnumerateRoom = [](auto &&candidate) noexcept
{
    return Enumerate(
            g_game.m_rooms, [&](auto &room) noexcept { return candidate == room.id; });
};

constexpr auto EnumerateNoun = [](auto &&candidate) noexcept
{
    return Enumerate(
            g_game.m_objects, [&](auto &object) noexcept {
                return !(object.flags & OF_COUNTER) && candidate == object.id;
            });
};

// Old-style value or -1 lookups
adjid_t
IsAdjective(string_view name) noexcept
{
    return EnumerateAdjective(name).value_or(WNONE);
}

roomid_t
IsRoomName(string_view name) noexcept
{
    return EnumerateRoom(name).value_or(WNONE);
}

objid_t
IsANoun(string_view name) noexcept
{
    return EnumerateNoun(name).value_or(WNONE);
}

/// TODO: Convert to string_view.
/// TODO: Convert to returning a Token.
std::pair<WType, amulid_t>
GetTokenType(char **s)
{
strip:
    char *p = *s;
    while (isspace(*p))
        p++;
    *s = p;
    if (*p == 0) {
        return {WNONE, WNONE};
    } /* none */

    if (*p == '\'' || *p == '\"') {
        auto text = reinterpret_cast<amulid_t>(p + 1);
        *s = FindEndQuote(p);
        *((*s) - 1) = 0;  /// TODO: replace with string_view
        return {WTEXT, text};
    }

    auto endWord = strstop(p, ' ');
    std::string tokenWord{ p, size_t(endWord - p) };
    StringLower(tokenWord);

    // Check for a LIVE player's name BEFORE checking for white words!
    {
        static auto isPlayer = [&](auto &player) noexcept { return tokenWord == player.name; };
        if (auto slotId = Enumerate(g_game.m_players, isPlayer); slotId.value_or(WNONE) != WNONE) {
            *s += tokenWord.length();
            return make_pair(WPLAYER, slotId.value());
        }
    }

	if (skipNoiseWords(tokenWord, s))
		goto strip;

	constexpr auto returnPair = [](WType wt, amulid_t id) noexcept {
		return id != WNONE ? make_pair(wt, id) : make_pair(WNONE, WNONE);
	};
	auto lastRef = [&](WType wt, amulid_t id) noexcept {
		*s += tokenWord.length();
		return returnPair(wt, id);
	};

    if (tokenWord == "it")
		return lastRef(WNOUN, last_it);
	if (tokenWord == "her")
		return lastRef(WPLAYER, last_her);
	if (tokenWord == "him")
        return lastRef(WPLAYER, last_him);
	if (tokenWord == "me" || tokenWord == "myself")
        return lastRef(WPLAYER, t_slotId);

    /* inoun/precedence is related to object chae patterns */
    if (auto [slen, csyn] = IsNounSynonym(p); slen != 0) {
        *s += slen;
        if (inoun1 == WNONE && inoun2 == WNONE) {
            return { WNOUN, csyn };
        }
        auto adj = inoun1 == WNONE ? iadj2 : iadj2;
        auto precedence = GetVerb(iverb).precedence[inoun1 == WNONE ? 0 : 1];
        auto word = isnoun(GetObject(csyn).id, adj, precedence);
		return returnPair(WNOUN, word);
    }
	if (auto [slen, verbId] = IsVerbSynonym(p); slen != 0) {
		*s += slen;
		return { WVERB, verbId };
	}

    if (inoun1 == WNONE && inoun2 == WNONE) {
		if (auto word = IsANoun(tokenWord); word != WNONE) {
            *s += tokenWord.length();
            return { WNOUN, word };
        }
    } else {
        auto adj = inoun1 == WNONE ? iadj2 : iadj2;
        auto precedence = GetVerb(iverb).precedence[inoun1 == WNONE ? 0 : 1];
        if (auto word = isnoun(*s, adj, precedence); word != WNONE) {
            *s += tokenWord.length();
            return { WNOUN, word };
        }
    }
    if (auto word = IsAdjective(tokenWord); word != WNONE) {
        *s += tokenWord.length();
        return { WADJ, word };
    }
    if (auto word = IsPreposition(*s); word != WNONE) {
        *s += tokenWord.length();
		///TODO: Store the preposition
        goto strip;
    }
    if (auto word = IsVerb(*s); word != WNONE) {
        *s += strlen(GetVerb(word).id);
        return { WVERB, word };
    }

    // Last ditch: does this match /any/ player?
    if (auto it = g_game.m_characterIndex.find(tokenWord); it != g_game.m_characterIndex.cend()) {
        return { WPLAYER, WNONE };
    }

    return { WNONE, WNONE }; /* Unknown */
}
