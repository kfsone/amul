#include "amulcom.strings.h"
#include "amulcom.h"
#include "modules.h"

#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.hash.h>
#include <h/amul.test.h>
#include <h/amul.xtra.h>

static FILE *          stringFP;
static struct HashMap *stringIDs;  // string name -> {stypes and position in file}

struct StringIDEntry {
    enum StringType types;
    stringid_t      offset;
};

static inline bool
isNewlineSuppressor(char c)
{
    return c == '{';
}

stringid_t
getStringID(size_t length)
{
    uint64_t offset = ftell(stringFP);
    uint64_t maxOffset = 1ULL << 32;
    if (offset >= maxOffset || offset + length >= maxOffset || offset + length < offset)
        alog(AL_FATAL, "Text data exceeds 4GB");
    return (stringid_t)(offset & 0xffffffff);
}

error_t
testLabelEntry(const char *label, enum StringType stype, struct StringIDEntry *entryp)
{
    REQUIRE(label && entryp);
    error_t err = LookupStrHashValue(stringIDs, label, (uint64_t *)entryp);
    if (err != 0 && err != ENOENT)
        return err;
    // If we found it, check the types
    return (!stype || (entryp->types & stype)) ? EEXIST : ENOENT;
}

error_t
initStringModule(struct Module *module)
{
    REQUIRE(!stringFP);
    return 0;
}

error_t
startStringModule(struct Module *module)
{
    error_t err = NewHashMap(1024, &stringIDs);
    if (err != 0)
        alog(AL_FATAL, "Unable to create string table");

    stringFP = OpenGameFile(stringTextFile, "w");  // Note: text mode, translate \r please
    REQUIRE(stringFP);

    return 0;
}

error_t
closeStringModule(struct Module *module, error_t err)
{
    CloseFile(&stringFP);
    CloseHashMap(&stringIDs);

    return 0;
}

error_t
InitStrings()
{
    NewModule(false, MOD_STRINGS, NULL, startStringModule, closeStringModule, NULL, NULL);
    return 0;
}

#define check_write_str(op, buffer, length, fp)                                                    \
    if (fwrite(buffer, 1, (length), fp) != (length))                                               \
    alog(AL_FATAL, "Unable to write %s", op)

error_t
AddTextString(const char *start, const char *end, bool isLine, stringid_t *idp)
{
    REQUIRE(start && end && idp);

    if (end > start && isNewlineSuppressor(*(end - 1))) {
        --end;
        isLine = false;
    }

    stringid_t id = getStringID(end - start);

    check_write_str("text string", start, end - start, stringFP);
    if (isLine)
        check_write_str("newline", "\n", 2, stringFP);
    else
        check_write_str("eos", "", 1, stringFP);

    *idp = id;

    return 0;
}

// Consume a paragraph of a file, undoing leading indents and copying text
error_t
TextStringFromFile(const char *label, FILE *fp, enum StringType stype, stringid_t *idp)
{
    REQUIRE(fp);

    // string's id will be the current position, so snag it now.
    stringid_t id = getStringID(0);

    struct StringIDEntry entry;
    error_t              err = testLabelEntry(label, stype, &entry);
    if (err != ENOENT)
        return err;

    char indent = 0;
    char line[2048];

    // Consume lines from the file until we reach a paragraph break
    while (!feof(fp)) {
        char *p = fgets(line, sizeof(line), fp);
        if (p == NULL)
            continue;
        const char *end = p;
        if (*p == '\n' && stype != STRING_FILE)
            break;
        if (*p != '\n') {
            if (!indent && *p) {
                indent = *p;
            }
            if (isspace(*p) && *p == indent) {
                p++;
            }
            end = strstop(p, '\n');
            if (end > p && *(end - 1) == '{')
                --end;
        }
        if (*end == '\n')
            ++end;
        check_write_str("text line", p, end - p, stringFP);
    }
    check_write_str("eos", "", 1, stringFP);

    if (idp)
        *idp = entry.offset;

    return 0;
}

error_t
RegisterTextString(
        const char *label, const char *start, const char *end, bool isLine, enum StringType stype,
        stringid_t *idp)
{
    error_t err;

    REQUIRE(start && end && idp);
    REQUIRE(stringFP && stringIDs);

    // Has this label already been registered?
    struct StringIDEntry entry;
    err = testLabelEntry(label, stype, &entry);
    if (err != ENOENT)
        return err;

    err = AddTextString(start, end, isLine, idp);
    if (err != 0)
        return err;

    entry.types |= stype;
    entry.offset = *idp;

    return AddStrToHash(stringIDs, label, *(uint64_t *)&entry);
}

error_t
LookupTextString(const char *label, enum StringType stype, stringid_t *idp)
{
    REQUIRE(label && idp);

    struct StringIDEntry entry;

    error_t err = testLabelEntry(label, stype, &entry);
    if (err != EEXIST)
        return err;

    *idp = entry.offset;

    return 0;
}

size_t
GetStringCount()
{
    return GetMapSize(stringIDs);
}

char *
StrCopy(char *into, size_t intoSize, const char *start, const char *end)
{
    const size_t copylen = end - start;
    if (copylen + 1 > intoSize) {
        return NULL;
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
        return NULL;
    }
    const char *intoEnd = into + copylen;
    while (into < intoEnd) {
        *(into++) = tolower(*(start++));
    }
    into[copylen] = 0;
    return into + copylen;
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
