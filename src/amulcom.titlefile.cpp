#include <cstdio>
#include <cstring>
#include <string>

#include "game.h"
#include "typedefs.h"
#include "amul.xtra.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"
#include "amulcom.version.h"
#include "logging.h"
#include "svparse.h"

extern thread_local FILE *ifp;
extern std::string compilerVersion;

static int
getNo(const char *prefix, const char *from)
{
    int result = 0;
    if (sscanf(from, "%d", &result) != 1) {
        LogFatal("Invalid '", prefix, "' entry: ", from);
    }
    return result;
}

void
stripNewline(char *text)
{
    size_t len = strlen(text);
    while (len-- > 0 && isEol(text[len])) {
        text[len] = 0;
    }
}

static void
getBlock(const char *linetype, void (*callback)(const char *, const char *))
{
    char scratch[1024];
    for (;;) {
        if (!fgets(scratch, sizeof(scratch), ifp)) {
            LogFatal("Invalid title.txt: Missing '", linetype, "' line");
        }

        repspc(scratch);
        stripNewline(scratch);
        char *p = skipspc(scratch);
        if (!*p || isCommentChar(*p))
            continue;

        if (!canSkipLead(linetype, &p)) {
            LogFatal("Invalid title.txt: Expected '", linetype, "' got: ", p);
        }

        callback(linetype, p);
        break;
    }
}

static void
getBlockNo(const char *prefix, int *into)
{
	std::cout << "prefix = " << prefix << ", into = " << into << "\n";
    static int blockValue{ 0 };
    // TODO: This is awful, use a context.
    getBlock(prefix, [](const char *pfx, const char *value) { blockValue = getNo(pfx, value); });
    *into = blockValue;
}

void
_getAdventureName(string_view prefix, string_view value)
{
    RemovePrefix(value, prefix);
    ssize_t overflow = value.size() - (sizeof(g_game.gameName) - 1);
    if (overflow > 0) {
        value.remove_suffix(overflow);
        LogWarn("Game name too long, truncated to: ", value);
    }
    strcpy(g_game.gameName, value.data());
}

void
title_proc(const std::string &/*filepath*/)
{
    nextc(true);

    getBlock("name=", [](auto pfx, auto val) { _getAdventureName(pfx, val); });
    getBlockNo("resettime=", &g_game.gameDuration_m);
    if (g_game.gameDuration_m < 15) {
        g_game.gameDuration_m = 15;
        LogWarn("resettime= too short: falling back to ", g_game.gameDuration_m, " minutes");
    }

    getBlockNo("seeinvis=", &g_game.seeInvisRank);
    getBlockNo("seesuperinvis=", &g_game.seeSuperInvisRank);

    getBlockNo("sgorank=", &g_game.superGoRank);

    getBlockNo("rankscale=", &g_game.rankScale);
    getBlockNo("timescale=", &g_game.timeScale);

    while (nextc(false)) {
        char *p = getTidiedLineToScratch(ifp);
        if (!p || strncmp(p, "[title]", 7) != 0)
            continue;
        if (TextStringFromFile("$title", ifp, nullptr, true) != 0) {
            LogFatal("Could not write splash text to message file");
        }
        return;
    }

    LogWarn("Missing [title] in title.txt");
    std::string defaultTitle = AMULCOM_VSTRING "\n\n";
    RegisterTextString("$title", defaultTitle, true, nullptr);
}
