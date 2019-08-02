#include "amulcom.strings.h"
#include "amulcom.h"
#include "modules.h"

#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.hash.h>
#include <h/amul.test.h>
#include <h/amul.xtra.h>

static FILE *   stringFP;
static HashMap *stringIDs;  // string name -> {stypes and position in file}

struct StringIDEntry {
    uint32_t   stringTypes;  // masked values
    stringid_t offset;
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
        afatal("Text data exceeds 4GB");
    return (stringid_t)(offset & 0xffffffff);
}

error_t
testLabelEntry(const char *label, StringType stype, StringIDEntry *entryp)
{
    REQUIRE(label && entryp);
    error_t err = LookupStrHashValue(stringIDs, label, (uint64_t *)entryp);
    if (err != 0)
        return err;
    // we found it, check the types
    return (!stype || (entryp->stringTypes & stype)) ? EEXIST : ENOENT;
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
    error_t err = NewHashMap(1024, &stringIDs);
    if (err != 0)
        afatal("Unable to create string table");

    stringFP = OpenGameFile(stringTextFile, "w");  // Note: text mode, translate \r please
    REQUIRE(stringFP);

    return 0;
}

error_t
closeStringModule(Module *module, error_t err)
{
    alog(AL_DEBUG, "Strings: cap %" PRIu64 ", size %" PRIu64, stringIDs->capacity, stringIDs->size);
    for (size_t i = 0; i < stringIDs->capacity; ++i) {
        const HashBucket *bucket = stringIDs->buckets[i];
        if (bucket)
        	alog(AL_DEBUG, "bucket #%04" PRIu64 ": capacity: %04" PRIu64, i, bucket->capacity);
        }
    }
    CloseFile(&stringFP);
    CloseHashMap(&stringIDs);

    return 0;
}

error_t
InitStrings()
{
    NewModule(MOD_STRINGS, NULL, startStringModule, closeStringModule, NULL, NULL);
    return 0;
}

#define check_write_str(op, buffer, length, fp)                                                    \
    do {                                                                                           \
        if (fwrite(buffer, 1, (length), fp) != (length))                                           \
            afatal("Unable to write %s", op);                                                      \
    } while (0)

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
    if (isLine) {
        check_write_str("newline", "\n", 2, stringFP);
    } else {
        check_write_str("eos", "", 1, stringFP);
    }

    *idp = id;

    return 0;
}

// Consume a paragraph of a file, undoing leading indents and copying text
error_t
TextStringFromFile(const char *label, FILE *fp, StringType stype, stringid_t *idp)
{
    REQUIRE(fp);

    // string's id will be the current position, so snag it now.
    stringid_t id = getStringID(0);

    if (label) {
        StringIDEntry entry;
        error_t       err = testLabelEntry(label, stype, &entry);
        if (err != ENOENT)
            return err;
        entry.stringTypes |= stype;
        err = AddStrToHash(stringIDs, label, *(uint64_t *)&entry);
        entry.offset = id;
    }

    char indent = 0;
    char line[2048];

    // Consume lines from the file until we reach a paragraph break
    while (!feof(fp)) {
        char *p = fgets(line, sizeof(line), fp);
        if (p == NULL)
            continue;
        if (isEol(*p) && stype != STRING_FILE)
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
        if (isEol(*end))
            ++end;
        check_write_str("text line", p, end - p, stringFP);
    }
    check_write_str("eos", "", 1, stringFP);

    if (idp)
        *idp = id;

    return 0;
}

error_t
RegisterTextString(
        const char *label, const char *start, const char *end, bool isLine, StringType stype,
        stringid_t *idp)
{
    REQUIRE(start && end && idp);
    REQUIRE(stringFP && stringIDs);

    // Has this label already been registered?
    StringIDEntry entry;
    error_t       err = testLabelEntry(label, stype, &entry);
    if (err != ENOENT)
        return err;

    err = AddTextString(start, end, isLine, idp);
    if (err != 0)
        return err;

    entry.stringTypes |= stype;
    entry.offset = *idp;

    alog(AL_DEBUG, "register %s as %" PRIu64, label, entry);
    return AddStrToHash(stringIDs, label, *(uint64_t *)&entry);
}

error_t
LookupTextString(const char *label, StringType stype, stringid_t *idp)
{
    REQUIRE(label && idp);

    StringIDEntry entry;

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

size_t
GetStringBytes()
{
	return ftell(stringFP);
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
