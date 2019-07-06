// user message definition parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

struct UMSG {
    char    id[IDL + 1];
    int32_t fpos;
};

bool
umsg_proc()
{
    UMSG umsg{};

    umsgs = 0;
    OS::CreateFile("umsg.tmp");
    fopena(Resources::Compiled::umsgIndex());
    ofp1 = afp;
    afp = NULL;
    fseek(ofp1, 0, 2L);
    fopena(Resources::Compiled::umsgData());
    ofp2 = afp;
    afp = NULL;
    fseek(ofp2, 0, 2L);
    fopena("umsg.tmp");
    if (nextc(0) == -1) {
        close_ofps();
        return false;
    }  // None to process
    blkget(&datal, &data, 0L);

    const char *s = data;

    do {
    loop:
        do
            s = sgetl(s, block);
        while (isCommentChar(block[0]) && *s != 0);
        if (*s == 0)
            break;
        tidy(block);
        if (block[0] == 0)
            goto loop;
        striplead("msgid=", block);
        getword(block);
        if (Word[0] == 0)
            goto loop;

        if (Word[0] == '$') {
            GetLogger().errorf("Invalid ID: %s: '$' is reserved for System Messages", Word);
            skipblock();
            goto loop;
        }
        if (strlen(Word) > IDL) {
            GetLogger().errorf("Invalid ID: %s: too long", Word);
            skipblock();
            goto loop;
        }
        umsgs++;  // Now copy the text across
        strcpy(umsg.id, Word);
        umsg.fpos = ftell(ofp2);
        fwrite(umsg.id, sizeof(umsg), 1, afp);
        fwrite((char *)&umsg.fpos, 4, 1, ofp1);
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
            s = sgetl(s, block);
            if (block[0] == 0)
                break;
            umsg.fpos = (int32_t)strlen(block);
            if (block[umsg.fpos - 1] == '{')
                block[--umsg.fpos] = 0;
            else
                strcat(block + (umsg.fpos++) - 1, "\n");
            fwrite(block, 1, umsg.fpos, ofp2);
        } while (*s != 0 && block[0] != 0);
        fputc(0, ofp2);
    } while (*s != 0);
    close_ofps();

    OS::Free(data, datal);

    GetContext().terminateOnErrors();

    return true;
}

// Check FP for umsg id!
int
isumsg(const char *s, FILE *fp)
{
    int i;

    if (*s == '$') {
        i = atoi(s + 1);
        if (i < 1 || i > NSMSGS) {
            GetLogger().errorf("Invalid System Message ID: %s", s);
            return -1;
        }
        return i - 1;
    }
    if (umsgs == 0)
        return -1;
    fseek(fp, 0, 0L);  // Rewind file
    for (i = 0; i < umsgs; i++) {
        UMSG umsg;
        fread(umsg.id, sizeof(umsg), 1, fp);
        if (stricmp(umsg.id, s) == NULL)
            return i + NSMSGS;
    }
    return -1;
}

int
msgline(const char *s)
{
    int32_t pos;
    char    c;
    FILE* fp = afp;
    afp = NULL;
    fopena(Resources::Compiled::umsgData());
    fseek(afp, 0, 2L);
    pos = ftell(afp);

    fwrite(s, strlen(s) - 1, 1, afp);
    if ((c = *(s + strlen(s) - 1)) != '{') {
        fputc(c, afp);
        fputc('\n', afp);
    }
    fputc(0, afp);
    fopena(Resources::Compiled::umsgIndex());
    fseek(afp, 0, 2L);
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
    return chkumsg(s);
}

int
chkumsg(const char *s)
{
    int   r;
    FILE *fp;

    if (*s != '$' && umsgs == 0)
        return -1;

    if ((fp = fopen("umsg.tmp", "rb+")) == NULL) {
        GetLogger().fatalf("Unable to access umsg.tmp");
    }
    r = isumsg(s, fp);
    fclose(fp);
    return r;
}

