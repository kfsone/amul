#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"
#include "logging.h"

#include <h/amul.type.h>
#include <h/amul.xtra.h>
#include <h/amul.gcfg.h>

#include <cstring>
#include <cstdio>
#include <string>

extern FILE *ifp;
extern GameConfig g_gameConfig;
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
    char block[1024];
    for (;;) {
        if (!fgets(block, sizeof(block), ifp)) {
            LogFatal("Invalid title.txt: Missing '", linetype, "' line");
        }

        repspc(block);
        stripNewline(block);
        char *p = skipspc(block);
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
    static int blockValue { 0 };
    // TODO: This is awful, use a context.
    getBlock(prefix, [](const char *prefix, const char *value) { blockValue = getNo(prefix, value); });
    *into = blockValue;
}

void
_getAdventureName(const char *prefix, const char *value)
{
    strncpy(g_gameConfig.gameName, value, sizeof(g_gameConfig.gameName));
    if (strlen(value) > sizeof(g_gameConfig.gameName) - 1)
        LogWarn("Game name too long, truncated to: ", g_gameConfig.gameName);
}

void
title_proc()
{
    nextc(true);

    getBlock("name=", _getAdventureName);
    getBlockNo("resettime=", &g_gameConfig.gameDuration_m);
    if (g_gameConfig.gameDuration_m < 15) {
        g_gameConfig.gameDuration_m = 15;
        LogWarn("resettime= too short: falling back to ", g_gameConfig.gameDuration_m, " minutes");
    }

    getBlockNo("seeinvis=", &g_gameConfig.seeInvisRank);
    getBlockNo("seesuperinvis=", &g_gameConfig.seeSuperInvisRank);

    getBlockNo("sgorank=", &g_gameConfig.superGoRank);

    getBlockNo("rankscale=", &g_gameConfig.rankScale);
    getBlockNo("timescale=", &g_gameConfig.timeScale);

    while (nextc(false)) {
        char *p = getTidyBlock(ifp);
        if (!p || strncmp(p, "[title]", 7) != 0)
            continue;
        if (TextStringFromFile("$title", ifp, nullptr, true) != 0) {
            LogFatal("Could not write splash text to message file");
        }
        return;
    }

    LogWarn("Missing [title] in Title.Txt");
    std::string defaultTitle = compilerVersion + "\n\n";
    RegisterTextString("$title", defaultTitle, true, nullptr);
}

