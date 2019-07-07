// travel table parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

void
trav_proc()
{
    int      strip, lines, nvbs, i, ntt, t, r;
    const char *   p;
    int32_t *l;

    nextc(1);  // Move to first text
    fopenw(Resources::Compiled::travelTable());
    fopenw(Resources::Compiled::travelParams());
    fopena(Resources::Compiled::roomData());
    ntt = t = 0;

    do {
    loop1:
        GetContext().checkErrorCount();

        fgets(block, 1000, ifp);
        if (feof(ifp))
            continue;
        tidy(block);
        if (isCommentChar(block[0]) || block[0] == 0)
            goto loop1;
        getWordAfter("room=", block);
        if ((rmn = isroom(Word)) == -1) {
            GetLogger().errorf("Invalid room ID: %s", Word);
            skipblock();
            goto loop1;
        }
        if (roomtab->tabptr != -1) {
            GetLogger().errorf("Redefinition of room '%s' in travel table.", roomtab->id);
            skipblock();
            goto loop1;
        }
    vbloop:
        do
            fgets(block, 1000, ifp);
        while (isCommentChar(block[0]));
        if (block[0] == 0 || block[0] == '\n') {
            // Only complain if room is not a death room
            if ((roomtab->flags & RF_LETHAL) != RF_LETHAL)
                GetLogger().warnf("Room '%s' has no travel table entries.", roomtab->id);
            roomtab->tabptr = -2;
            ntt++;
            continue;
        }
        tidy(block);
        if (!striplead("verb=", block) && !striplead("verbs=", block)) {
            GetLogger().errorf("Room: %s: Expected a verb[s]= entry.", roomtab->id);
            goto vbloop;
        }
        lines = 0;
        verb.id[0] = 0;
        roomtab->tabptr = t;
        roomtab->ttlines = 0;
    vbproc:  // Process verb list
        nvbs = 0;
        tt.pptr = (int32_t *)-1;
        l = (int32_t *)temp;
        p = block;
        // Break verb list down to verb no.s
        do {
            p = getword(p);
            if (Word[0] == 0)
                break;
            if ((*l = is_verb(Word)) == -1) {
                GetLogger().errorf("Room: %s: invalid verb: %s", roomtab->id, Word);
            }
            l++;
            nvbs++;
        } while (Word[0] != 0);
        if (nvbs == 0) {
            printf("Room \"%s\" has empty verb[s]= line!\n", roomtab->id);
            quit();
        }
        // Now process each instruction line
        do {
        xloop:
            strip = 0;
            r = -1;
            block[0] = 0;
            fgets(block, 1000, ifp);
            if (feof(ifp))
                break;
            if (block[0] == 0 || block[0] == '\n') {
                strip = -1;
                continue;
            }
            tidy(block);
            if (isCommentChar(block[0]) || block[0] == 0)
                goto xloop;
            if (striplead("verb=", block) || striplead("verbs=", block)) {
                strip = 1;
                break;
            }
            p = precon(block);  // Strip pre-condition opts
        notloop:
            p = getword(p);
            if (strcmp(Word, ALWAYSEP) == NULL) {
                tt.condition = CALWAYS;
                tt.action = -(1 + AENDPARSE);
                goto write;
            }
            if (strcmp(Word, "not") == NULL || strcmp(Word, "!") == NULL) {
                r = -1 * r;
                goto notloop;
            }
        notlp2:
            if (Word[0] == '!') {
                strcpy(Word, Word + 1);
                r = -1 * r;
                goto notlp2;
            }
            if ((tt.condition = iscond(Word)) == -1) {
                tt.condition = CALWAYS;
                if ((tt.action = isroom(Word)) != -1)
                    goto write;
                if ((tt.action = isact(Word)) == -1) {
                    GetLogger().errorf("Room: %s: invalid travel table entry: %s", roomtab->id, Word);
                    goto xloop;
                }
                goto gotohere;
            }
            p = skipspc(p);
            if ((p = chkcparms(p, tt.condition, ofp2)) == NULL) {
                GetContext().addError();
                goto next;
            }
            if (r == 1)
                tt.condition = -1 - tt.condition;
            if (*p == 0) {
                GetLogger().errorf("Room: %s: entry with missing action", roomtab->id);
                goto xloop;
            }
            p = preact(p);
            p = getword(p);
            if ((tt.action = isroom(Word)) != -1)
                goto write;
            if ((tt.action = isact(Word)) == -1) {
                GetLogger().errorf("Room: %s: Invalid action in travel table: %s", roomtab->id, Word);
                goto xloop;
            }
        gotohere:
            if (tt.action == ATRAVEL) {
                GetLogger().errorf("Room: %s: Tried to use 'TRAVEL' action from travel table.", roomtab->id);
                goto xloop;
            }
            p = skipspc(p);
            if ((p = chkaparms(p, tt.action, ofp2)) == NULL) {
                GetContext().addError();
                goto next;
            }
            tt.action = 0 - (tt.action + 1);
        write:
            roomtab = rmtab + rmn;
            l = (int32_t *)temp;
            for (i = 0; i < nvbs; i++) {
                if (i < nvbs - 1)
                    tt.pptr = (int32_t *)-2;
                else
                    tt.pptr = (int32_t *)-1;
                tt.verb = *(l++);
                fwrite((char *)&tt.verb, sizeof(tt), 1, ofp1);
                roomtab->ttlines++;
                t++;
                ttents++;
            }
            lines++;
        next:
            strip = 0;
        } while (strip == 0 && !feof(ifp));
        if (strip == 1 && !feof(ifp))
            goto vbproc;
        nextc(0);
        ntt++;
    } while (!feof(ifp));

    // If there have have been no errors, we're not in quiet mode, and the number of
    // travel table entries doesn't match the number of rooms, then report on which
    // room is missing it's travel table entry.
    if (GetContext().errorCount() == 0 && ntt != rooms && !GetLogger().m_quiet) {
        roomtab = rmtab;
        for (i = 0; i < rooms; i++, roomtab++)
            if (roomtab->tabptr == -1 && (roomtab->flags & RF_LETHAL) != RF_LETHAL)
                GetLogger().warnf("Room: %s: no travel table entry.", roomtab->id);
    }

    GetContext().terminateOnErrors();

    ttroomupdate();
    close_ofps();
}
