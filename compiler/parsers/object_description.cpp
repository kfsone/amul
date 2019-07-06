#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

void
obds_proc()
{
    char c, lastc;

    obdes = 0;
    OS::CreateFile(Resources::Compiled::objDesc());
    if (nextc(0) == -1) {
        GetLogger().infof("No entries in object-description file");
        return;
    }

    fopenw("ODIDs.tmp");
    fopenw(Resources::Compiled::objDesc());
    do {
        fgets(block, 1024, ifp);
        tidy(block);
        striplead("desc=", block);
        getword(block);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            GetLogger().errorf("Invalid object ID: %s: bad length", Word);
            skipblock();
            continue;
        }
        strcpy(objdes.id, Word);
        fseek(ofp2, 0, 2);
        objdes.descrip = ftell(ofp2);
        fwrite(objdes.id, sizeof(objdes), 1, ofp1);
        lastc = '\n';
        while ((c = fgetc(ifp)) != EOF && !(c == '\n' && lastc == '\n')) {
            if ((lastc == EOF || lastc == '\n') && c == 9)
                continue;
            fputc((lastc = c), ofp2);
        };
        fputc(0, ofp2);
        obdes++;
        nextc(0);
    } while (c != EOF);

    GetContext().terminateOnErrors();

    close_ofps();
}
