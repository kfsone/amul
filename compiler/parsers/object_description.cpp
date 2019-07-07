#include "amulcom.includes.h"

#include <string>
#include <unordered_map>

using namespace AMUL::Logging;
using namespace Compiler;

static std::unordered_map<std::string, int> obdescIDs{};

int
getObjDescID(const char *name)
{
    if (stricmp(name, "none") == 0) {
        return OBJ_DESC_NONE;
    }
    std::string id = name;
    std::transform(id.cbegin(), id.cend(), id.begin(), ::tolower);
    if (auto it = obdescIDs.find(id); it != obdescIDs.end()) {
        return it->second;
	}
    return -1;
}

void
obds_proc()
{
    char c, lastc;

    obdes = 0;
    if (nextc(0) == -1) {
        GetLogger().infof("No entries in object-description file");
        return;
    }

    fopenw(Resources::Compiled::objDesc());
    do {
        fgets(block, 1024, ifp);
        tidy(block);
        getWordAfter("desc=", block);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            GetLogger().errorf("Invalid object description ID: %s: bad length", Word);
            skipblock();
            continue;
        }
        if (stricmp(Word, "none") == 0) {
            GetLogger().errorf("Invalid object description ID: %s: reserved name", Word);
            skipblock();
            continue;
		}
        std::string id = Word;
        std::transform(id.cbegin(), id.cend(), id.begin(), ::tolower);
        if (obdescIDs.find(id) != obdescIDs.end()) {
            GetLogger().errorf("Multiple definitions of obj. description id: %s", Word);
            skipblock();
            continue;
        }
        obdescIDs[id] = obdes++;

        lastc = '\n';
        while ((c = fgetc(ifp)) != EOF && !(c == '\n' && lastc == '\n')) {
            if ((lastc == EOF || lastc == '\n') && c == 9)
                continue;
            fputc((lastc = c), ofp1);
        };
        fputc(0, ofp1);
        nextc(0);
    } while (!feof(ifp));

    GetContext().terminateOnErrors();

    close_ofps();
}
