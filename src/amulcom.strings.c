#include "amulcom.strings.h"
#include "amulcom.h"

#include "h/amul.alog.h"
#include "h/amul.hash.h"
#include "h/amul.test.h"
#include "h/amul.xtra.h"

static const char *strings_file = "strings.amulc";

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
    if (offset + length >= maxOffset)
        alog(AL_FATAL, "Text data exceeds 4GB");
    return (stringid_t)(offset & 0xffffffff);
}

error_t
testLabelEntry(const char *label, enum StringType stype, struct StringIDEntry *entryp)
{
    REQUIRE(label && entryp);
    error_t err = LookupStrHashValue(stringIDs, label, entryp);
    if (err != 0 && err != ENOENT)
        return err;
    // If we found it, check the types
    return (!stype || (entryp->types & stype)) ? EEXIST : ENOENT;
}

error_t
InitStrings()
{
    REQUIRE(stringFP == NULL);

    error_t err = NewHashMap(1024, &stringIDs);
    if (err != 0)
        alog(AL_FATAL, "Unable to create string table");

    stringFP = OpenGameFile(strings_file, "r");  // Note: text mode, translate \r please
}

void
CloseStrings()
{
    CloseFile(&stringFP);
    CloseHashMap(&stringIDs);
}

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
    error_t err;
    REQUIRE(fp);

    // string's id will be the current position, so snag it now.
    stringid_t id = getStringID(0);

    struct StringIDEntry entry;
    err = testLabelEntry(label, stype, &entry);
    if (err != ENOENT)
        return err;

    char indent = 0;
    char line[2048];

    // Consume lines from the file until we reach a paragraph break
    while (!feof(fp)) {
        char *p = fgets(line, sizeof(line), fp);
        if (p == NULL)
            break;
        if (*p == '\n')
            break;
        if (isspace(*p)) {
			if (!indent || (indent && *p == indent)) {
                indent = *(p++);
			}
		}
        const char *end = strstop(p, '\n');
        if (end > p && *(end - 1) == '{')
            --end;
        check_write_str("text line", p, end - p, fp);
    }
    check_write_str("eol", "", 1, fp);

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

    return AddStrToHash(stringIDs, label, &entry);
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
