#include <stdexcept>
#include <string>

#include "amul.defs.h"
#include "amul.enum.h"
#include "typedefs.h"
#include "amul.xtra.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"
#include "filesystem.h"
#include "filesystem.inl.h"
#include "logging.h"
#include "sourcefile.h"
#include "svparse.h"

using IDCheckFn = bool (*)(string_view);

static error_t
consumeMessageFile(const std::string &filepath, string_view prefix, IDCheckFn checkerFn) noexcept
{
    SourceFile src{ filepath };
    if (error_t err = src.Open(); err != 0) {
        LogFatal("Aborting due to file error: ", filepath);
    }

    std::string text{};
    text.reserve(4096);

    while (!src.Eof()) {
        CheckErrorCount();

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
check_smsg(string_view token)
{
    if (token[0] != '$') {
        LogError("Invalid system message ID (must start with '$'): ", token);
        return false;
    }
    int messageID = [&]() {
        try {
            return std::stoi(token.data() + 1);
        } catch (const std::invalid_argument &) {
            return 0;
        }
    }();
    if (messageID < 1 || messageID > NSMSGS) {
        LogError("Invalid sysmsg ID (expected $1-$", NSMSGS, "): ", token);
        return false;
    }
    return true;
}

void
smsg_proc(const std::string &filepath)
{
    /// TODO: Name should be constexpr
    (void) consumeMessageFile(filepath, "msgid=", check_smsg);
    for (size_t i = 1; i <= NSMSGS; ++i) {
        std::string msgId = "$" + std::to_string(i);
        if (LookupTextString(msgId.c_str()) != 0) {
            LogError("System Message ", msgId, " is missing.");
            CheckErrorCount();
        }
    }
}

void
umsg_proc(const std::string &filepath)
{
    /// TODO: Name should be constexpr
    error_t err = consumeMessageFile(filepath, "msgid=", nullptr);
    if (err == ENOENT) {
        LogInfo("No user defined messages");
        return;
    }
}

void
obds_proc(const std::string &filepath)
{
    error_t err = consumeMessageFile(filepath, "desc=", nullptr);
    if (err == ENOENT) {
        LogInfo("No long object descriptions");
        return;
    }
}