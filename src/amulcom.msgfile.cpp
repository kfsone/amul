#include <h/amul.alog.h>
#include <h/amul.defs.h>
#include <h/amul.enum.h>
#include <h/amul.type.h>
#include <h/amul.xtra.h>

#include "amulcom.ctxlog.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"

#include <stdexcept>
#include <string>

using IDCheckFn = bool(*)(std::string&);

static error_t
consumeMessageFile(FILE *fp, const char *prefix, IDCheckFn checkerFn) noexcept
{
    if (!nextc(false))
        return ENOENT;

    while (!feof(fp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(fp);
        if (!p)
            continue;

        p = getWordAfter(prefix, p);

        ScopeContext ctx { Word };

        std::string word { Word };
        if (word.length() < 2 || word.length() > IDL) {
            alog(AL_ERROR, "Invalid %s ID: %s", prefix, Word);
            skipblock();
        } else if (checkerFn && !checkerFn(word)) {
            skipblock();
        } else if (!checkerFn && Word[0] == '$') {
            alog(AL_ERROR, "Invalid ID (can't begin with '$'): %s", Word);
            skipblock();
        }

        if (error_t err = TextStringFromFile(Word, fp, nullptr); err != 0) {
            return err;
        }
    }

    return 0;
}

static bool
check_smsg(std::string& token)
{
    if (token[0] != '$') {
        alog(AL_ERROR, "Invalid system message ID (must start with '$'): %s", token.c_str());
        return false;
    }
    int messageID = [&]() {
        try {
           return std::stoi(token.substr(1));
        } catch (const std::invalid_argument& e) {
            return 0;
        }
    } ();
    if (messageID < 1 || messageID > NSMSGS){
        alog(AL_ERROR, "Invalid sysmsg ID (expected $1-$%d): %s", NSMSGS, token.c_str());
        return false;
    }
    return true;
}

void
smsg_proc()
{
    ScopeContext ctx{"smsg"};

    (void)consumeMessageFile(ifp, "msgid=", check_smsg);
    for (size_t i = 1; i <= NSMSGS; ++i) {
        std::string msgId = "$" + std::to_string(i);
        if (LookupTextString(msgId.c_str()) != 0) {
            alog(AL_ERROR, "System Message %s is missing.", msgId.c_str());
            checkErrorCount();
        }
    }
}

void
umsg_proc()
{
    ScopeContext ctx{"umsg"};

    error_t err = consumeMessageFile(ifp, "msgid=", nullptr);
    if (err == ENOENT) {
        /// TODO: Tell the user
        return;
    }
}

void
obds_proc()
{
    ScopeContext ctx{"objdesc"};

    error_t err = consumeMessageFile(ifp, "desc=", nullptr);
    if (err == ENOENT) {
        alog(AL_INFO, "No long object descriptions");
        return;
    }
}

