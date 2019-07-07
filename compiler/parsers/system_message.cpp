// system message parser
//
// Note:
//
// System messages are enumerated so they must be listed in order and must
// all be defined.
//
// AMUL didn't include defaults because that would have used up more memory/space.

#include "amulcom.includes.h"

#include <string>
#include <unordered_set>

using namespace AMUL::Logging;
using namespace Compiler;

void registerMessageId(std::string id);

void
smsg_proc()
{
    std::unordered_set<int> registered{};
    const char *            s;

    if (nextc(false) == -1) {
        GetLogger().fatalf("Empty system messages file.");
    }

    fopenw(Resources::Compiled::umsgIndex());
    fopenw(Resources::Compiled::umsgData());

    Buffer smsgBuffer{0};
    s = static_cast<const char *>(smsgBuffer.m_data);

    do {
        GetContext().checkErrorCount();

        do {
            s = extractLine(s, block);
        } while (isCommentChar(block[0]));
        if (block[0] == 0)
            continue;

        tidy(block);
        if (block[0] == 0)
            continue;
        getWordAfter("msgid=", block);
        if (Word[0] == 0)
            break;

        bool valid = true;

        if (Word[0] != '$') {
            GetLogger().errorf("Invalid sysmsg id (must start with '$'): %s", Word);
            valid = false;
        } else {
            auto msgId = atoi(Word + 1);
            if (msgId < 1 || msgId > NSMSGS) {
                GetLogger().errorf("Invalid sysmsg id (out of range): %s", Word);
                valid = false;
            } else if (registered.find(msgId) != registered.end()) {
                GetLogger().errorf("Duplicate definition of system message: %s", Word);
                valid = false;
            }
            if (valid)
                registered.insert(msgId);
        }

        if (valid) {
            registerMessageId(Word);
        }

        umsgoff_t pos = ftell(ofp2);
        fwritesafe(pos, ofp1);

        do {
            while (isCommentChar(*s))
                s = skipline(s);
            if (isLineEnding(*s)) {
                break;
            }
            if (*s == '\t')
                s++;
            s = extractLine(s, block);
            if (block[0] == 0)
                break;
            pos = (int32_t)strlen(block);
            if (block[pos - 1] == '{')
                block[--pos] = 0;
            else
                strcat(block + (pos++) - 1, "\n");
            fwrite(block, 1, pos, ofp2);
        } while (*s != 0 && block[0] != 0);

        fputc(0, ofp2);
    } while (*s != 0);
    close_ofps();

    smsgBuffer.free();

    // Validate that we have all the system messages
    if (registered.size() != NSMSGS) {
        for (int i = 1; i < NSMSGS; ++i) {
            if (registered.find(i) == registered.end()) {
                GetLogger().fatalf(
                        "System Message $%d is missing (total of %u missing)\n", i, NSMSGS - registered.size());
            }
        }
    }

    GetContext().terminateOnErrors();
}
