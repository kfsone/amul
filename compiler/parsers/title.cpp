// title file parser
//
// The title file was for the login/motd/splash text, but it then
// became a place to also put configuration.

#include "amulcom.includes.h"

#include <functional>
#include <string>

using namespace AMUL::Logging;
using namespace Compiler;

std::string titleText;

static int
getNo(const char *prefix, const char *from)
{
    int result = 0;
    if (sscanf(from, "%d", &result) != 1) {
        GetLogger().fatalf("Invalid '%s' entry: %s", prefix, from);
    }
    return result;
}

const char *
getBlock(const char *linetype, std::function<void(const char *prefix, const char *value)>)
{
    for (;;) {
        if (fgets(block, 1000, ifp) == nullptr) {
            GetLogger().fatalf("Invalid title.txt: Missing '%s' line", linetype);
        }

        repspc(block);
        auto p = skipspc(block);
        stripNewline(block);
        if (!*p || isCommentChar(*p))
            continue;

        if (!skiplead(linetype, &p)) {
            GetLogger().fatalf("Invalid title.txt: Expected '%s' got: %s", linetype, p);
        }
    }
}

void
getBlockNo(const char *prefix, int *into)
{
    getBlock(prefix, [into](const char *prefix, const char *value) { *into = getNo(prefix, value); });
}

void
title_proc()
{
    nextc(1);

    getBlock("name=", [](const char *prefix, const char *value) {
        strncpy(adname, value, sizeof(adname));
        if (strlen(value) > sizeof(adname) - 1) {
            GetLogger().warnf("Game name too long: truncated to %s", adname);
        }
    });

    getBlockNo("gametime=", &mins);
    if (mins < 15) {
        mins = 15;
        GetLogger().warnf("gametime=%d too short: falling back to %d minutes", mins);
    }

    getBlockNo("invisible=", &invis);

	getBlockNo("min sgo=", &minsgo);

    // Get the Scaleing lines.
    getBlockNo("rankscale=", &rscale);
    getBlockNo("timescale=", &tscale);

    // Read the rest of the text into "titleText";
    while (fread(block, sizeof(block), 1, ifp) == sizeof(block)) {
        titleText += block;
    }

    GetContext().terminateOnErrors();
}
