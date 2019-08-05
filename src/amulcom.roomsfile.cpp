#include <algorithm>
#include <cctype>
#include <cstring>
#include <deque>
#include <map>
#include <string>

#include <h/amul.alog.h>
#include <h/amul.cons.h>
#include <h/amul.gcfg.h>
#include <h/amul.lcst.h>
#include <h/amul.type.h>
#include <h/amul.xtra.h>
#include <h/room_struct.h>

#include "amulcom.h"
#include "amulcom.ctxlog.h"
#include "amulcom.fileprocessing.h"
#include "amulcom.strings.h"

extern bool checkDmoves;

extern GameConfig g_gameConfig;

// Process ROOMS.TXT
void
room_proc()
{
    // Seek to the next non-whitespace, non-comment character. The 'true' will
    // make it an error not to find one.
    nextc(true);

    do {
        // Terminate if we've reached the error limit.
        checkErrorCount();

        // Seek to the next useful character (we'll already be there first time)
        if (!nextc(false))
            break;

        // Fetch all the text from here to the next double-eol (end of paragraph),
        // convert tabs in it to spaces, etc.
        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        // Copy the text at 'p' into 'Word', unless it is prefixed with "room=",
        // in which case first skip the prefix.
        p = getWordAfter("room=", p);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            *p = 0;
            alog(AL_ERROR, "Invalid ID (length): %s", p);
            skipblock();
            continue;
        }
        ScopeContext roomCtx { Word };

        // Because 'room' is a global, we have to clear it out.
        Room room {};
        AssignWord(room.id);

        if (RoomIdx::Lookup(room.id)) {
            ctxLog(AL_ERROR, "Re-definition of room");
            skipblock();
            continue;
        }

        // If there are additional words on the line, they are room flags.
        for (;;) {
            p = skipspc(p);
            if (!*p)
                break;
            p = getword(p);
            char *dmove = Word;
            if (canSkipLead("dmove", &dmove)) {
                if (!room.dmove.empty()) {
                    ctxLog(AL_ERROR, "multiple dmove flags");
                } else {
                    AssignWord(room.dmove);
                }
                continue;
            }
            int no = isRoomFlag(Word);
            if (no == -1) {
                ctxLog(AL_ERROR, "Invalid room flag: ", Word);
                continue;
            }
            if (room.flags & bitset(no)) {
                ctxLog(AL_WARN, "Duplicate room flag: %s", Word);
            }
            room.flags |= bitset(no);
        }

        // Everything else in the current block is the room description.
        error_t err = TextStringFromFile(nullptr, ifp, &room.descid);
        if (err != 0) {
            ctxLog(AL_ERROR, "Unable to write description");
            skipblock();
            continue;
        }
    } while (!feof(ifp));
}

void
checkdmoves()
{
    if (!checkDmoves)
        return;
    for (auto&& room : RoomIdx::Rooms()) {
        if (!room.dmove.empty()) {
            if (auto it = g_roomIndex.find(room.id); it == g_roomIndex.end()) {
                ScopeContext ctx { room.id };
                ctxLog(AL_ERROR, "unrecognized dmove destination: ", room.dmove);
                checkErrorCount();
            }
        }
    }
}
