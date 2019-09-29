// Things that shouldn't change all that often.

#include <cstring>
#include <random>

#include "amul.cons.h"
#include "amul.stct.h"
#include "amullib.h"
#include "game.h"
#include "typedefs.h"

static auto &
getRandomEngine() noexcept
{
    static thread_local std::random_device t_randDev;
    static thread_local std::default_random_engine t_randEngine{ t_randDev() };
    return t_randEngine;
}

int32_t
RandomInt(int32_t lower, int32_t upper) noexcept
{
    upper = std::max(lower, upper);
    std::uniform_int_distribution<int32_t> distrib(lower, upper);
    return distrib(getRandomEngine());
}

bool
Match(string_view lhs, string_view rhs) noexcept
{
    // If the right-hand string is shorter, then lhs can't match it.
    if (rhs.length() < lhs.length())
        return false;
    // If the left-hand string is shorter
    if (lhs.length() < rhs.length() && rhs[lhs.length()] != ' ')
        return false;

    for (size_t i = 0; i < lhs.length(); ++i) {
        char lhc = tolower(lhs[i]), rhc = tolower(rhs[i]);
        if (lhc == rhc)
            continue;
        if (lhc == ' ' && rhc == '_')
            continue;
        if (lhc == '_' && rhc == ' ')
            continue;
        return false;
    }
    return true;
}

bool
IsSynonym(const char *s) noexcept
{
    for (auto &syn : g_game.m_synonyms) {
        if (Match(s, syn.word))
            return true;
    }
    return false;
}

static pair<size_t /*length*/, amulid_t /*aliasTo*/>
findSynonym(const char *s, bool (*predicate)(const Synonym &)) noexcept
{
    for (auto &syn : g_game.m_synonyms) {
        if (predicate(syn) && Match(s, syn.word))
            return make_pair(strlen(syn.word), syn.aliases);
    }
    return make_pair(0, WNONE);
}

pair<size_t /*length*/, objid_t /*aliasTo*/>
IsNounSynonym(const char *s) noexcept
{
    return pair<size_t, objid_t>(findSynonym(
            s, [](const Synonym &syn) noexcept { return syn.type == SYN_NOUN; }));
}

pair<size_t /*length*/, verbid_t /*aliasTo*/>
IsVerbSynonym(const char *s) noexcept
{
    return pair<size_t, verbid_t>(findSynonym(
            s, [](const Synonym &syn) noexcept { return syn.type == SYN_VERB; }));
}

verbid_t
IsVerb(string_view verb) noexcept
{
    // Note: We currently allow verbs with spaces in by using '_' in them.
    for (auto it = g_game.m_verbs.cbegin(); it != g_game.m_verbs.cend(); ++it) {
        if (Match(verb, it->id))
            return int(std::distance(g_game.m_verbs.cbegin(), it));
    }

    return WNONE;
}

prepid_t
IsPreposition(const char *s) noexcept
{
    for (prepid_t i = 0; i < NPREP; i++)
        if (Match(s, prep[i]))
            return i;
    return WNONE;
}

// Find the end of a quoted string literal.
char *
FindEndQuote(char *text) noexcept
{
    char quote = *(text++);
    // Look for a follow up quote that isn't escaped
    while (*text) {
        const char c = *(text++);
        if (c == quote || c == '\n')
            break;
        if (c == '\\' && *text != 0)
            ++text;
    }
    return text;
}
