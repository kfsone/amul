#include "amulcom.strings.h"
#include "amulcom.h"
#include "modules.h"

#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.test.h>
#include <h/amul.xtra.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <unordered_map>

using StringMap = std::unordered_map<std::string, stringid_t>;

static FILE *     stringFP;
static stringid_t stringBytes;
static StringMap  stringMap;

const std::string &
StringLower(std::string &str)
{
	std::transform(str.cbegin(), str.cend(), str.begin(),
			[](unsigned char c) { return std::tolower(c); });
	return str;
}

static inline bool
isNewlineSuppressor(char c)
{
    return c == '{';
}

static bool /*ok*/
registerStringLabel(std::string label)
{
	StringLower(label);
	auto [it, ins] = stringMap.insert(std::make_pair(label, stringBytes));
	if (!ins)
		return false;
	alog(AL_DEBUG, "registered %s as %u", label.c_str(), stringBytes);
	return true;
}

#define check_write_str(op, buffer, length, fp)                                                    \
    do {                                                                                           \
        if (fwrite(buffer, 1, (length), fp) != (length))                                           \
            afatal("Unable to write %s", op);                                                      \
		stringBytes += (length);                                                                     \
    } while (0)

error_t
AddTextString(const char *start, const char *end, bool isLine, stringid_t *idp)
{
    REQUIRE(start && end && idp);

    while (end > start && isEol(*(end - 1)))
        --end;

    if (end > start && isNewlineSuppressor(*(end - 1))) {
        --end;
        isLine = false;
    }

	*idp = stringBytes;

    check_write_str("text string", start, end - start, stringFP);
    if (isLine) {
        check_write_str("newline", "\n", 2, stringFP);
    } else {
        check_write_str("eos", "", 1, stringFP);
    }

    return 0;
}

// Consume a paragraph of a file, undoing leading indents and copying text
error_t
TextStringFromFile(const char *label, FILE *fp, stringid_t *idp, bool toEof)
{
    REQUIRE(fp);
    REQUIRE(!label || *label);

    // string's id will be the current position, so snag it now.
    stringid_t id { stringBytes };

    if (label && !registerStringLabel(label)) {
		return EEXIST;
	}

    char indent = 0;
    char line[2048];

    // Consume lines from the file until we reach a paragraph break
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
        check_write_str("text line", p, end - p, stringFP);
        if (isEol(*p)) {
            check_write_str("eol", "\n", 1, stringFP);
        }
    }
    check_write_str("eos", "", 1, stringFP);

    if (idp)
        *idp = id;

    return 0;
}

error_t
RegisterTextString(
        const char *label, const char *start, const char *end, bool isLine, stringid_t *idp)
{
    REQUIRE(start && end);
    REQUIRE(stringFP);
	REQUIRE(label && *label);

    stringid_t id { 0 };
    if (!idp) {
        idp = &id;
    }
	*idp = stringBytes;
	if (!registerStringLabel(label)) {
		return EEXIST;
	}

    error_t err = AddTextString(start, end, isLine, idp);
    if (err != 0)
        return err;

    alog(AL_DEBUG, "register %s as %u", label, *idp);

    return 0;
}

error_t
LookupTextString(const char *label, stringid_t *idp)
{
    REQUIRE(label);
    std::string labelId { label };
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
    return stringMap.size();
}

size_t
GetStringBytes()
{
    return stringBytes;
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
    REQUIRE(!stringFP);
    return 0;
}

error_t
startStringModule(Module *module)
{
	REQUIRE(stringMap.empty());
    stringFP = OpenGameFile(stringTextFile, "w");  // Note: text mode, translate \r please
    REQUIRE(stringFP);

    return 0;
}

error_t
closeStringModule(Module *module, error_t err)
{
	alog(AL_DEBUG, "Strings: %zu, Bytes: %u", stringMap.size(), stringBytes);
    CloseFile(&stringFP);
	stringMap.clear();

    return 0;
}

error_t
InitStrings()
{
    NewModule(MOD_STRINGS, nullptr, startStringModule, closeStringModule, nullptr, nullptr);
    return 0;
}
