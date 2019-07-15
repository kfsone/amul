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

static void
getBlock(const char *linetype, std::function<void(const char *prefix, const char *value)> callback)
{
    for (;;) {
        if (!fgets(block, 1000, ifp)) {
            GetLogger().fatalf("Invalid title.txt: Missing '%s' line", linetype);
        }

        repspc(block);
        stripNewline(block);
        auto p = skipspc(block);
        if (!*p || isCommentChar(*p))
            continue;

        if (!skiplead(linetype, &p)) {
            GetLogger().fatalf("Invalid title.txt: Expected '%s' got: %s", linetype, p);
        }

        callback(linetype, p);

        break;
    }
}

void
getBlockNo(const char *prefix, int *into)
{
    getBlock(prefix, [into](const char *prefix, const char *value) {
        *into = getNo(prefix, value);
    });
}

void
title_proc()
{
    nextc(true);

    getBlock("name=", [](const char *prefix, const char *value) {
        strncpy(adname, value, sizeof(adname));
        if (strlen(value) > sizeof(adname) - 1) {
            GetLogger().warnf("Game name too long: truncated to %s", adname);
        }
    });

    getBlockNo("gametime=", &mins);
    if (mins < 15) {
        mins = 15;
        GetLogger().warnf("gametime= too short: falling back to %d minutes", mins);
    }

    getBlockNo("invisible=", &invis);

    getBlockNo("min sgo=", &minsgo);

    // Get the Scaleing lines.
    getBlockNo("rankscale=", &rscale);
    getBlockNo("timescale=", &tscale);

    // Read the rest of the text into "titleText";
    while (freadsafe(block, ifp) > 0) {
        titleText += block;
    }

    GetContext().terminateOnErrors();
}
