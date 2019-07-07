// synonyms parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

void
syn_proc()
{
    const char *t;
    const char *s;
    short int   no;
    short int   x;

    syns = 0;
    if (nextc(0) == -1)
        return;
    fopenw(Resources::Compiled::synonymData());
    fopenw(Resources::Compiled::synonymIndex());

	Buffer synBuffer{0L};
    s = static_cast<const char*>(synBuffer.m_data);

    do {
        do
            s = extractLine(s, block);
        while (isCommentChar(block[0]));

        tidy(block);
        if (block[0] == 0)
            continue;
        t = getword(block);
        t = skipspc(t);

        if ((no = isnoun(Word)) < 0) {
            if ((x = is_verb(Word)) == -1) {
                GetLogger().errorf("Invalid verb/noun: %s", Word);
                continue;
            }
            no = -(2 + x);
        }

        while (*t != 0) {
            t = getword(t);
            if (Word[0] == 0)
                break;
            fwrite((char *)&no, 1, sizeof(short int), ofp2);
            fprintf(ofp1, "%s%c", Word, 0);
            syns++;
        }
    } while (*s != 0);
    close_ofps();

	synBuffer.free();

    GetContext().terminateOnErrors();
}
