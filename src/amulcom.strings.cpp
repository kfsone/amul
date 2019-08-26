#include <algorithm>
#include <cctype>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "h/amul.cons.h"
#include "h/amul.gcfg.h"
#include "h/amul.test.h"
#include "h/amul.xtra.h"
#include "h/amulcom.h"
#include "h/amulcom.strings.h"
#include "h/logging.h"
#include "h/modules.h"

// Offsets of strings by string id.
using StringMap = std::unordered_map<std::string, stringid_t>;

static StringMap stringMap;

std::string &
StringLower(std::string &str)
{
    std::transform(
            str.cbegin(), str.cend(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

static inline bool
isNewlineSuppressor(char c)
{
    return c == '{';
}

static stringid_t
nextStringId() noexcept
{
    return stringid_t(g_game.m_stringIndex.size());
}

static bool /*ok*/
registerStringLabel(std::string_view label_)
{
    std::string label{ label_ };
    StringLower(label);
    auto [it, ins] = stringMap.insert(std::make_pair(label, nextStringId()));
    if (!ins)
        return false;
    LogDebug("registered ", label, " as ", nextStringId());
    return true;
}

error_t
AddTextString(std::string_view text, bool isLine, stringid_t *idp)
{
    REQUIRE(idp);

    while (!text.empty() && isEol(text.back()))
        text.remove_suffix(1);

    if (!text.empty() && isNewlineSuppressor(text.back())) {
        text.remove_suffix(1);
        isLine = false;
    }

    *idp = nextStringId();

    g_game.m_stringIndex.push_back(GetStringBytes());

    g_game.m_strings.insert(g_game.m_strings.end(), text.cbegin(), text.cend());
    if (isLine)
        g_game.m_strings.push_back('\n');
    g_game.m_strings.push_back('\0');

    return 0;
}

// Consume a paragraph of a file, undoing leading indents and copying text
error_t
TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEof)
{
    REQUIRE(fp);
    REQUIRE(!label || *label);

    // string's id will be the current position, so snag it now.
    stringid_t id{ nextStringId() };

    if (label && !registerStringLabel(label)) {
        return EEXIST;
    }

    char indent = 0;
    char line[2048];

    // Consume lines from the file until we reach a paragraph break
    static std::string text;
    text.clear();
    while (!feof(fp)) {
        char *p = fgets(line, sizeof(line), fp);
        if (p == nullptr)
            continue;
        if (isEol(*p) && !toEof)
            break;
        const char *end = p;
        if (!isEol(*p)) {
            if (!indent && *p) {
                indent = *p;
            }
            if (isspace(*p) && *p == indent) {
                p++;
            }
            end = strstop(p, '\n');
            if (end > p && *(end - 1) == '\r')
                --end;
            if (end > p && *(end - 1) == '{')
                --end;
        }
        text += end;
        if (isEol(*p)) {
            text += '\n';
        }
    }
    text += '\0';

    g_game.m_stringIndex.push_back(GetStringBytes());
    g_game.m_strings.insert(g_game.m_strings.end(), text.cbegin(), text.cend());

    if (idp)
        *idp = id;

    return 0;
}

error_t
RegisterTextString(std::string_view label, std::string_view text, bool isLine, stringid_t *idp)
{
    REQUIRE(!label.empty());

    stringid_t id{ 0 };
    if (!idp) {
        idp = &id;
    }
    *idp = nextStringId();
    if (!registerStringLabel(label)) {
        return EEXIST;
    }

    error_t err = AddTextString(label, isLine, idp);
    if (err != 0)
        return err;

    return 0;
}

error_t
LookupTextString(std::string_view label, stringid_t *idp)
{
    REQUIRE(!label.empty());
    std::string labelId{ label };
    StringLower(labelId);
    auto it = stringMap.find(labelId);
    if (it == stringMap.end())
        return ENOENT;
    if (idp)
        *idp = it->second;
    return 0;
}

size_t
GetStringCount()
{
    return g_game.m_stringIndex.size();
}

size_t
GetStringBytes()
{
    return g_game.m_strings.size();
}

char *
StrCopy(char *into, size_t intoSize, const char *start, const char *end)
{
    const size_t copylen = end - start;
    if (copylen + 1 > intoSize) {
        return nullptr;
    }
    memcpy(into, start, copylen);
    into[copylen] = 0;
    return into + copylen;
}

char *
WordCopy(char *into, size_t intoSize, const char *start, const char *end)
{
    const size_t copylen = end - start;
    if (copylen + 1 > intoSize) {
        return nullptr;
    }
    const char *intoEnd = into + copylen;
    while (into < intoEnd) {
        *(into++) = tolower(*(start++));
    }
    *into = 0;
    return into;
}

void
ZeroPad(char *string, size_t stringSize)
{
    char *end = string + stringSize;
    while (string < end && *string)
        ++string;
    while (string < end)
        *(string++) = 0;
}

error_t
initStringModule(Module *module)
{
    return 0;
}

error_t
startStringModule(Module *module)
{
    REQUIRE(stringMap.empty());

    return 0;
}

error_t
closeStringModule(Module *module, error_t err)
{
    LogDebug("Strings: ", GetStringCount(), ", Bytes: ", GetStringBytes());

    return 0;
}

error_t
InitStrings()
{
    NewModule(MOD_STRINGS, nullptr, startStringModule, closeStringModule, nullptr, nullptr);
    return 0;
}
