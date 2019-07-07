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

using namespace AMUL::Logging;
using namespace Compiler;

void registerMessageId(std::string id);

void
smsg_proc()
{
    const char *s;

    smsgs = 0;

    if (nextc(0) == -1)
        return;  // Nothing to process!
    fopenw(Resources::Compiled::umsgIndex());
    fopenw(Resources::Compiled::umsgData());

    blkget(&datal, &data, 0L);
    s = data;

    do {
        do
            s = sgetl(s, block);
        while (isCommentChar(block[0]));

        tidy(block);
        if (block[0] == 0)
            continue;
        getWordAfter("msgid=", block);
        if (Word[0] == 0)
            break;

        if (Word[0] != '$') {
            printf("\x07\n\n!! Invalid SysMsg ID, '%s'. SysMsgs MUST begin with a '$'!\n", Word);
            quit();
        }
        if (atoi(Word + 1) != smsgs + 1) {
            printf("\x07\n\n!! Message %s out of sequence!\n\n", Word);
            quit();
        }
        if (smsgs >= NSMSGS) {
            printf("\x07\n\n!! Too many System Messages, only require %ld!\n\n", NSMSGS);
            quit();
        }

        registerMessageId(Word);

        umsgoff_t pos = ftell(ofp2);
        fwrite(&pos, sizeof(pos), 1, ofp1);

        do {
            while (isCommentChar(*s))
                s = skipline(s);
            if (isLineEnding(*s)) {
                s = "";  // to break the while loops
                break;
            }
            if (*s == 9)
                s++;
            if (*s == 13) {
                block[0] = 13;
                continue;
            }
            s = sgetl(s, block);
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

        ++smsgs;  // Now copy the text across
    } while (*s != 0);
    close_ofps();

    OS::Free(data, datal);
}
