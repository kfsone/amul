#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

#include <string>

// Process ROOMS.TXT
void
room_proc()
{
    char c;
    char lastc, *p, *p2;
    int  n;

    rooms = 0;
    nextc(true);  // Skip any headings etc

    fopenw(Resources::Compiled::roomData());
    fopenw(Resources::Compiled::roomDesc());

    do {
        p = block;
        while ((c = fgetc(ifp)) != EOF && !isspace(c))
            *(p++) = c;
        *p = 0;  // Set null byte
        if (block[0] == 0)
            break;

        const char *id = skiplead("room=", block);
        auto        len = strlen(id);
        if (len < 3) {
            GetLogger().errorf("Invalid room id (too short): %s", id);
        } else if (len >= sizeof(room.id)) {
            GetLogger().errorf("Invalid room id (too long): %s", id);
        }
        strncpy(room.id, id, sizeof(room.id));

        // Do the flags
        room.flags = 0;
        room.tabptr = -1;
        std::string cemeteryId{};
        if (!isEol(c)) {
            fgets(block, 1024, ifp);
            p = block;
            n = -1;
            do {
                while (isspace(*p) && *p != 0)
                    p++;
                if (*p == 0)
                    continue;
                p2 = p;
                while (!isspace(*p2) && *p2 != 0)
                    p2++;
                *p2 = 0;
                if (n == 0)  // Get dmove param
                {
                    cemeteryId = p;
                    dmoves++;
                    p = p2 + 1;
                    n = -1;
                    continue;
                }
                if ((n = isrflag(p)) == -1) {
                    GetLogger().errorf(
                            "Room: %s: Invalid room flag: %s", room.id, p);
                }
                n -= NRNULL;
                if (n >= 0)
                    room.flags = (room.flags | bitset(n));
                p = p2 + 1;
            } while (*p != 0);
        }

        lastc = '\n';
        fseek(ofp2, 0, 1);
        room.desptr = ftell(ofp2);
        n = 0;
        if (!cemeteryId.empty())
            fwrite(cemeteryId.c_str(), IDL, 1,
                   ofp2);  // save dmove	///ISSUE: IDL+1?
        while ((c = fgetc(ifp)) != EOF && !(isEol(c) && isEol(lastc))) {
            if (isEol(lastc) && c == '\t')
                continue;
            fputc((lastc = c), ofp2);
            n++;
        };
        fputc(0, ofp2);
        fwritesafe(room, ofp1);
        rooms++;
        nextc(false);
    } while (c != EOF);
    close_ofps();
}
