#include <h/amul.defs.h>
#include <h/amul.enum.h>
#include <h/amul.type.h>
#include <h/amul.xtra.h>

#include "amulcom.strings.h"
#include "amulcom.fileprocessing.h"
#include "filesystem.h"
#include "filesystem.inl.h"
#include "logging.h"
#include "sourcefile.h"
#include "svparse.h"

#include <stdexcept>
#include <string>

using IDCheckFn = bool(*)(std::string_view);

static error_t
consumeMessageFile(std::string_view name, std::string_view prefix, IDCheckFn checkerFn) noexcept
{
    char filepath[MAX_PATH_LENGTH];
    MakeTextFileName(std::string{name}, filepath);

    SourceFile src{filepath};
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath);
    }

    std::string text {};
    text.reserve(4096);
    
    while (!src.Eof()) {
        checkErrorCount();

        if (!src.GetIDLine(prefix))
            continue;

        // first line of each message block is the identifier/label, which may have
        // an optional prefix, e.g. msgid=somelabel
        auto label = src.line.front();

        if (checkerFn && !checkerFn(label)) {
            /*nothing*/
        } else if (!checkerFn && label[0] == '$') {
            LogError(filepath, ":", src.lineNo, ": Invalid ID (can't begin with '$'): ", label);
        } else if (src.line.size() > 1) {
            LogError(filepath, ":", src.lineNo, ": Expected end-of-line, got: ", src.line[1]);
        } else if (LookupTextString(label) != ENOENT) {
            LogError(filepath, ":", src.lineNo, ": Message ID already in use: ", label);
        } else {
            text.clear();
            src.GetLines(text);
            RegisterTextString(label, text, false, nullptr);
            continue;
        }

        src.SkipBlock();
    }

    return 0;
}

static bool
check_smsg(std::string_view token)
{
    if (token[0] != '$') {
        LogError("Invalid system message ID (must start with '$'): ", token);
        return false;
    }
    int messageID = [&]() {
        try {
           return std::stoi(token.data() + 1);
        } catch (const std::invalid_argument& e) {
            return 0;
        }
    } ();
    if (messageID < 1 || messageID > NSMSGS){
        LogError("Invalid sysmsg ID (expected $1-$", NSMSGS, "): ", token);
        return false;
    }
    return true;
}

void
smsg_proc()
{
    ///TODO: Name should be constexpr
    (void)consumeMessageFile("SysMsg", "msgid=", check_smsg);
    for (size_t i = 1; i <= NSMSGS; ++i) {
        std::string msgId = "$" + std::to_string(i);
        if (LookupTextString(msgId.c_str()) != 0) {
            LogError("System Message ", msgId, " is missing.");
            checkErrorCount();
        }
    }
}

void
umsg_proc()
{
    ///TODO: Name should be constexpr
    error_t err = consumeMessageFile("UMsg", "msgid=", nullptr);
    if (err == ENOENT) {
        LogInfo("No long user messages");
        return;
    }
}

void
obds_proc()
{
    error_t err = consumeMessageFile("ObDescs", "desc=", nullptr);
    if (err == ENOENT) {
        LogInfo("No long object descriptions");
        return;
    }
}
