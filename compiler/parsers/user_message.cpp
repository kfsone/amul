// user message definition parser

#include "amulcom.includes.h"

#include <string>
#include <unordered_map>

using namespace AMUL::Logging;
using namespace Compiler;

static std::unordered_map<std::string, int32_t> umsgIDs;

void
registerMessageId(std::string id)
{
    if (id[0] != '$')
        std::transform(id.cbegin(), id.cend(), id.begin(), ::tolower);
    int32_t msgNo = (int32_t)umsgIDs.size();
    umsgIDs[id] = msgNo;
}

bool
umsg_proc()
{
    umsgs = 0;
    fopena(Resources::Compiled::umsgIndex());
    ofp1 = afp;
    afp = NULL;
    fseek(ofp1, 0, SEEK_END);
    fopena(Resources::Compiled::umsgData());
    ofp2 = afp;
    afp = NULL;
    fseek(ofp2, 0, SEEK_END);
    if (nextc(0) == -1) {
        close_ofps();
        return false;
    }  // None to process

	Buffer      umsgBuffer{0};
    const char *s = static_cast<const char*>(umsgBuffer.m_data);

    do {
        do
            s = extractLine(s, block);
        while (isCommentChar(block[0]) && *s != 0);
        if (*s == 0)
            break;
        tidy(block);
        if (block[0] == 0)
            continue;
        getWordAfter("msgid=", block);
        if (Word[0] == 0)
            continue;

        if (Word[0] == '$') {
            GetLogger().errorf("Invalid ID: %s: '$' is reserved for System Messages", Word);
            skipblock();
            continue;
        }
        if (strlen(Word) > IDL) {
            GetLogger().errorf("Invalid ID: %s: too long", Word);
            skipblock();
            continue;
        }

        // write index
        umsgoff_t fpos = ftell(ofp2);
        fwrite(&fpos, sizeof(fpos), 1, ofp1);

        registerMessageId(Word);

        do {
            while (isCommentChar(*s))
                s = skipline(s);
            if (isLineBreak(*s)) {
                s = "";  // break loop
                break;
            }
            if (*s == 9)
                s++;
            if (*s == 13) {
                block[0] = 13;
                continue;
            }
            s = extractLine(s, block);
            if (block[0] == 0)
                break;
            umsgoff_t umsglen = strlen(block);
            if (block[umsglen - 1] == '{')
                block[--umsglen] = 0;
            else
                strcat(block + (umsglen++) - 1, "\n");
            fwrite(block, 1, umsglen, ofp2);
        } while (*s != 0 && block[0] != 0);

        fputc(0, ofp2);
        umsgs++;
    } while (*s != 0);
    close_ofps();

	umsgBuffer.free();

    GetContext().terminateOnErrors();

    return true;
}

// Check FP for umsg id!
int
isumsg(const char *str)
{
    if (*str == '$') {
        int i = atoi(str + 1);
        if (i < 1 || i > NSMSGS) {
            GetLogger().errorf("Invalid System Message ID: %s", str);
            return -1;
        }
        return i - 1;
    }
    if (umsgs == 0)
        return -1;
    std::string id = str;
    std::transform(id.cbegin(), id.cend(), id.begin(), ::tolower);
    if (auto match = umsgIDs.find(id); match != umsgIDs.end())
        return match->second;
    return -1;
}

int
msgline(const char *s)
{
    int32_t pos;
    char    c;
    FILE *  fp = afp;
    afp = NULL;
    fopena(Resources::Compiled::umsgData());
    fseek(afp, 0, SEEK_END);
    pos = ftell(afp);

    fwrite(s, strlen(s) - 1, 1, afp);
    if ((c = *(s + strlen(s) - 1)) != '{') {
        fputc(c, afp);
        fputc('\n', afp);
    }
    fputc(0, afp);
    fopena(Resources::Compiled::umsgIndex());
    fseek(afp, 0, SEEK_END);
    fwrite((char *)&pos, sizeof(int32_t), 1, afp);
    fclose(afp);
    afp = fp;
    return NSMSGS + (umsgs++);
}

int
ttumsgchk(const char *s)
{
    s = skiplead("msgid=", s);
    s = skiplead("msgtext=", s);
    s = skipspc(s);
    if (*s == '\"' || *s == '\'')
        return msgline(s + 1);
    return isumsg(s);
}
