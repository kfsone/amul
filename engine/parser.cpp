// The AMUL parser and VT processor

#include "amulinc.h"

void grab()  // Copy INPUT to BLOCK, taking one sentence at a time, etc
{
    char *s, *d, *p, c;  // Source & Destination

    s = input;
    more = 10;
    forced = 0;
    exeunt = 0;
    failed = NO;
    died = 0;
    donet = -1;
    ml = 0;

    do {
        d = block;
        block[0] = 0;
    loop:
        *d = 0;
        while (isspace(*s))
            s++;  // Skip spaces
        if (*s == 0) {
            *(s + 1) = 0;
            goto proc;
        }
    quotes:
        if (block[0] != 0)
            *(d++) = ' ';  // Force spacing
        if (*s == '\'' || *s == '\"') {
            c = *(d++) = *(s++);        // Store which
            while (*s != c && *s != 0)  // Find match or \0
                *(d++) = *(s++);
            if (*s == 0)
                *(s + 1) = 0;
            *(d++) = *(s++);
            *d = 0;
            if (*s != 0)
                s++;  // Skip " or ' at end
            goto loop;
        }

        p = d;
        while (*s != 0 && !isspace(*s) && *s != '!' && *s != ';' && *s != ',' && *s != '.' &&
               *s != '\"' && *s != '\'')
            *(d++) = *(s++);
        *d = 0;
        *(d + 1) = 0;
        if (stricmp(p, "then") == 0 || stricmp(p, "and") == 0) {
            *p = 0;
            goto proc;
        }
        if (*s == '\'' || *s == '\"')
            goto quotes;
        if (isspace(*s))
            goto loop;
    proc:
        if (*s != 0 && *s != '\'' && *s != '\"')
            s++;
        if (block[0] == 0)
            continue;
        // Print the prompt & the line, if not first text
        if (more != 10) {
            ans("3m");
            tx((rktab + me->rank)->prompt);
            txs("%s\n", block);
            ans("0;37m");
        }
        if (parser() == -1)
            return;
    } while (*s != 0 && more == 0 && exeunt == 0 && forced == 0 && failed == NO && died == 0);
    iocheck();
}

void
parser()
{
    char *p;
    int   x, om;

    om = more;  // We need to know if this is phrase one in a mo...
    more = 0;
    if (strlen(block) == 0)
        return;

phrase:
    wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = TC_NONE;
    iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = -1;
    p = block + strlen(block) - 1;
    while (p != block && isspace(*p))
        *(p--) = 0;
    if (me->rank >= (minsgo - 1) && (x = isroom(block)) != -1) {
        if (visible() == YES)
            action(acp(SGOVANISH), AOTHERS);
        StopFollow();
        LoseFollower();
        sys(SGO);
        moveto(x);
        if (visible() == YES)
            action(acp(SGOAPPEAR), AOTHERS);
        return;
    }
    iocheck();
    if (forced != 0 || exeunt != 0 || died != 0)
        return;
    p = block;
    if (*p == '\"' || *p == '\'') {
        char *p2;
        if (*(p + 1) == 0)
            return;
        if ((iverb = isverb("\"speech")) == -1) {
            sys(CANTDO);
            return -1;
        }
        p2 = p + 1;
    loop:
        while (*p != *p2 && *p2 != 0)
            p2++;
        *(p2 + 1) = *(p2) = 0;
        inoun1 = (int32_t)p + 1;
        wtype[2] = TC_TEXT;
        goto skip;
    }
    if ((word = isaverb(&p)) == -1) {
        char *bp;
        /* If not a verb, then check for nouns. If they typed
            > get NounA, NounB, NounC  then allow it.
             If they typed
            > kiss PlayerA, NounA
             don't allow it.
        */

        bp = p;
        if (om == 10 || (x = type(&bp)) == -2) {
            sys(INVALIDVERB);
            more = 1;
            return -1;
        }
        word = iverb;
    }
    x = TC_VERB;
    vbptr = vbtab + word;
    if ((me2->flags & PFASLEEP) && !(vbptr->flags & VB_DREAM)) {
        tx("You can't do anything until you wake up!\n");
        failed = YES;
        return -1;
    }
    if (!(vbptr->flags & VB_TRAVEL)) {
        if ((x = isverb("\"travel")) != -1) {
            vbptr = vbtab + word;
            if (lang_proc(x, 1) == 0)
                return;
        }
        vbptr = vbtab + word;
    }
    if (iverb >= 0)
        lverb = iverb;
    iverb = word;
    vbptr = vbtab + iverb;
    wtype[0] = TC_VERB;

    // adjectives are optional, so assume next word is a noun
l1:
    if (*p == 0)
        goto ended;
    wtype[2] = type(&p);
    inoun1 = word;
    if (wtype[2] == TC_NOUN)
        it = inoun1;
    if (wtype[2] == TC_ADJ) {
        if (wtype[1] != -1) {
            sys(NONOUN);
            return -1;
        }
        wtype[1] = TC_ADJ;
        iadj1 = inoun1;
        wtype[2] = -1;
        inoun1 = -1;
        goto l1;
    }
    if (wtype[2] == TC_PREP) {
        if (wtype[3] != -1) {
            sys(WORDMIX);
            return -1;
        }
        wtype[3] = TC_PREP;
        iprep = inoun1;
        wtype[2] = -1;
        inoun1 = -1;
    }
l2:
    if (*p == 0)
        goto ended;
    wtype[5] = type(&p);
    inoun2 = word;
    if (wtype[5] == TC_NOUN)
        it = inoun2;
    if (wtype[5] == TC_PREP) {
        if (wtype[3] != -1) {
            sys(WORDMIX);
            return -1;
        }
        wtype[3] = TC_PREP;
        iprep = inoun2;
        wtype[5] = -1;
        inoun2 = -1;
        goto l2;
    }
    if (wtype[5] == TC_ADJ) {
        if (wtype[4] != -1) {
            sys(NONOUN);
            return -1;
        }
        wtype[4] = TC_ADJ;
        iadj2 = inoun2;
        wtype[5] = -1;
        inoun2 = -1;
        goto l2;
    }
ended:
    overb = iverb;
    vbptr = vbtab + iverb;
skip:
    iocheck();
    if (forced != 0 || exeunt != 0 || died != 0 || failed != NO)
        return;
    return lang_proc(iverb, 0);
}

int
lang_proc(int v, char e)
{
    int i, j, l, m, d;

    forced = 0;
    exeunt = 0;
    failed = NO;
    died = 0;
    donet = 0;
    ml = 0;
    d = -2;
    tt.verb = -1;
    vbptr = vbtab + v;
caloop:
    for (i = 0; i < (vbtab + v)->ents; i++) {
        m = 0;
        stptr = vbptr->ptr + i;
        donet = 0;
        ml = stptr->ents;
        if (stptr->wtype[2] != TC_ANY)
            for (j = 0; j < 5 && m == 0; j++) {
                if (stptr->wtype[j] == TC_ANY && (j == 0 || j == 3 || wtype[j + 1] != TC_NONE))
                    continue;
                if (stptr->wtype[j] != wtype[j + 1]) {
                    m = 1;
                    continue;
                }
                // We have a match, now see if its the same word!
                if (stptr->slot[j] == TC_ANY)
                    continue;
                switch (j) {
                case 0:
                    if (iadj1 != stptr->slot[j])
                        m = 1;
                    break;
                case 1:
                    if (stptr->slot[j] == TC_NONE && inoun1 == TC_NONE)
                        break;
                    if (stptr->wtype[j] == TC_PLAYER && inoun1 == Af && stptr->slot[j] == -3)
                        break;
                    if (stptr->wtype[j] == TC_TEXT &&
                        stricmp((char *)inoun1, umsgp + *(umsgip + stptr->slot[j])) == NULL)
                        break;
                    if (stptr->wtype[j] == TC_NOUN &&
                        stricmp((obtab + inoun1)->id, (obtab + stptr->slot[j])->id) == NULL)
                        break;
                    if (inoun1 != stptr->slot[j])
                        m = 1;
                    break;
                case 2:
                    if (iprep != stptr->slot[j])
                        m = 1;
                    break;
                case 3:
                    if (iadj2 != stptr->slot[j])
                        m = 1;
                    break;
                case 4:
                    if (stptr->slot[j] == TC_NONE && inoun2 == TC_NONE)
                        break;
                    if (stptr->wtype[j] == TC_PLAYER && inoun2 == Af && stptr->slot[j] == -3)
                        break;
                    if (stptr->wtype[j] == TC_TEXT &&
                        stricmp((char *)inoun2, umsgp + *(umsgip + stptr->slot[j])) == NULL)
                        break;
                    if (stptr->wtype[j] == TC_NOUN &&
                        stricmp((obtab + inoun2)->id, (obtab + stptr->slot[j])->id) == NULL)
                        break;
                    if (inoun2 != stptr->slot[j])
                        m = 1;
                    break;
                }
            }
        if (m != 0)
            goto after;
        l = -1;
        d = -1;
        for (donet = 0; donet < ml; donet++) {
            stptr = vbptr->ptr + i;
            vtabp = stptr->ptr + donet;
            tt.action = vtabp->action;
            tt.pptr = vtabp->pptr;
            if (skip != 0) {
                skip--;
                continue;
            }
            if ((l = cond(vtabp->condition, l)) == -1)
                continue;
            inc = 1;
            act(tt.action, vtabp->pptr);
            if (inc == 1)
                d = 0;
            if (ml < -1) {
                d = lang_proc(iverb, e);
                donet = ml + 1;
                break;
            }
            if (ml < 0 || failed != NO || forced != 0 || died != 0 || exeunt != 0)
                break;
        }
        if (failed != NO || forced != 0 || died != 0 || exeunt != 0)
            break;
    after:
        if (donet > ml)
            break;
    }
    if (d > -1)
        return 0;  // If we processed something...

    vbptr = vbtab + v;
    if (!(vbptr->flags & VB_TRAVEL)) {
        int iv;
        iv = iverb;
        iverb = v;
        if (ttproc() == 0)
            return 0;
        else
            d = -1;
        iverb = iv;
    }
    if (d == -2 && e == 0)
        sys(ALMOST);
    if (d == -1 && e == 0) {
        if (!(vbptr->flags & VB_TRAVEL))
            sys(CANTGO);
        else
            sys(CANTDO);
    }
    return -1;
}
