/* Process LANG.TXT */
void
lang_proc()
{
    char lastc;
    /* n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2) */
    int n, cs, s, r;

    nextc(true);
    fopenw(verbDataFile);
    CloseOutFiles();
    fopena(verbDataFile);
    ofp1 = afp;
    afp = nullptr;
    fopenw(verbSlotFile);
    fopenw(verbTableFile);
    fopenw(verbParamFile);

    vbtab = (_VERB_STRUCT *)AllocateMem(filesize() + 128 * sizeof(g_verb));
    vbptr = vbtab;

    size_t of2p = ftell(ofp2);
    size_t of3p = ftell(ofp3);
    FPos = ftell(ofp4);

    while (!feof(ifp) && nextc(false)) {
        checkErrorCount();

        char *p = getTidyBlock(ifp);
        if (!p)
            continue;

        // check for 'travel' line which allows specification of travel verbs

        if (canSkipLead("travel=", &p)) {
            registerTravelVerbs(p);
            continue;
        }

        p = getWordAfter("verb=", p);
        if (!Word[0]) {
            alog(AL_ERROR, "verb= line without a verb?");
            skipblock();
            continue;
        }

        if (strlen(Word) > IDL) {
            alog(AL_ERROR, "Invalid verb ID (too long): %s", Word);
            skipblock();
            continue;
        }

        memset(&g_verb, 0, sizeof(g_verb));
        strncpy(g_verb.id, Word, sizeof(g_verb.id));

        ++g_gameConfig.numVerbs;
        alog(AL_DEBUG, "verb#%04d:%s", g_gameConfig.numVerbs, g_verb.id);

        getVerbFlags(&g_verb, p);

        p = getTidyBlock(ifp);
        if (!p)
            alog(AL_ERROR, "Unexpected end of file during verb: %s", g_verb.id);
        if (!*p || isEol(*p)) {
            if (g_verb.ents == 0 && (g_verb.flags & VB_TRAVEL)) {
                alog(AL_WARN, "Verb has no entries: %s", g_verb.id);
            }
            goto write;
        }

        if (!canSkipLead("syntax=", &p)) {
            vbprob("no syntax= line", block);
            skipblock();
            continue;
        }

        /* Syntax line loop */
synloop:

    _SLOTTAB vbslot { WNONE };

    p = skiplead("verb", p);
    char *qualifier = getword(p);
    qualifier = skipspc(qualifier);

    /* If syntax line is 'syntax=verb any' or 'syntax=none' */
    if (*qualifier == 0 && strcmp("any", Word) == 0) {
        vbslot = _SLOTTAB { WANY };
        goto endsynt;
    }
    if (*qualifier == 0 && strcmp("none", Word) == 0) {
        goto endsynt;
    }

    sp2: /* Syntax line processing */
        p = skipspc(p);
        if (consumeComment(p) || *p == '|')
            goto endsynt;

        p = getword(p);
        if (Word[0] == 0)
            goto endsynt;

        if ((n = iswtype(Word)) == -3) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid phrase on syntax line: %s", Word);
            vbprob(tmp, block);
            goto commands;
        }
        if (Word[0] == 0) {
            s = WANY;
            goto skipeq;
        }

        /* First of all, eliminate illegal combinations */
        if (n == WNONE || n == WANY) { /* you cannot say none=fred any=fred etc */
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Tried to define %s on syntax line", syntax[n]);
            vbprob(tmp, block);
            goto endsynt;
        }
        if (n == WPLAYER && strcmp(Word, "me") != 0 && strcmp(Word, "myself") != 0) {
            vbprob("Tried to specify player other than self", block);
            goto endsynt;
        }

        /* Now check that the 'tag' is the correct type of word */

        s = -1;
        switch (n) {
        case WADJ:
        /* Need ISADJ() - do TT entry too */
        case WNOUN:
            s = isnoun(Word);
            break;
        case WPREP:
            s = isprep(Word);
            break;
        case WPLAYER:
            if (strcmp(Word, "me") == 0 || strcmp(Word, "myself") == 0)
                s = -3;
            break;
        case WROOM:
            s = isRoom(Word);
            break;
        case WSYN:
            alog(AL_WARN, "Synonyms not supported yet");
            s = WANY;
            break;
        case WTEXT:
            s = getTextString(Word, false);
            break;
        case WVERB:
            s = is_verb(Word);
            break;
        case WCLASS:
            s = WANY;
        case WNUMBER:
            if (Word[0] == '-')
                s = atoi(Word + 1);
            else
                s = atoi(Word);
        default:
            alog(AL_ERROR, "Internal Error: Invalid w-type");
        }

        if (n == WNUMBER && (s > 100000 || -s > 100000)) {
            char tmp[64];
            snprintf(tmp, sizeof(tmp), "Invalid number: %d", s);
            vbprob(tmp, block);
        }
        if (s == -1 && n != WNUMBER) {
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "%s=: Invalid setting: %s", syntax[n + 1], Word);
            vbprob(tmp, block);
        }
        if (s == -3 && n == WNOUN)
            s = -1;

    skipeq: /* (Skipped the equals signs) */
        /* Now fit this into the correct slot */
        cs = 1; /* Noun1 */
        switch (n) {
        case WADJ:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                vbprob("Invalid NOUN NOUN ADJ combination", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("Invalid NOUN ADJ NOUN ADJ combination", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[0] != WNONE && vbslot.wtype[3] != WNONE) {
                vbprob("More than two adjectives on a line", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[0] != WNONE)
                cs = 3;
            else
                cs = 0;
            break;
        case WNOUN:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                vbprob("Invalid noun arrangement", block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE)
                cs = 4;
            break;
        case WPREP:
            if (vbslot.wtype[2] != WNONE) {
                vbprob("Invalid PREP arrangement", block);
                n = -5;
                break;
            }
            cs = 2;
            break;
        case WPLAYER:
        case WROOM:
        case WSYN:
        case WTEXT:
        case WVERB:
        case WCLASS:
        case WNUMBER:
            if (vbslot.wtype[1] != WNONE && vbslot.wtype[4] != WNONE) {
                char tmp[128];
                snprintf(block, sizeof(block), "No free noun slot for %s entry", syntax[n + 1]);
                vbprob(tmp, block);
                n = -5;
                break;
            }
            if (vbslot.wtype[1] != WNONE)
                cs = 4;
            break;
        }
        if (n == -5)
            goto sp2;
        /* Put the bits into the slots! */
        vbslot.wtype[cs] = n;
        vbslot.slot[cs] = s;
        goto sp2;

    endsynt:
        vbslot.cna.clear();

    commands:
        lastc = 'x';
        proc = 0;

        p = getTidyBlock(ifp);
        if (!p || !*p || isEol(*p)) {
            lastc = 1;
            goto writeslot;
        }

        if (canSkipLead("syntax=", &p)) {
            lastc = 0;
            goto writeslot;
        }

        vbslot.ents++;
        r = -1;
        vt.pptr = (opparam_t *)FPos;

        /* Process the condition */
    notloop:
        p = precon(p);
        p = getword(p);

        /* always endparse */
        if (strcmp(Word, ALWAYSEP) == 0) {
            vt.condition = CALWAYS;
            vt.action = -(1 + AENDPARSE);
            goto writecna;
        }
        if (strcmp(Word, "not") == 0 || (Word[0] == '!' && Word[1] == 0)) {
            r = -1 * r;
            goto notloop;
        }

        /* If they forgot space between !<condition>, eg !toprank */
    notlp2:
        if (Word[0] == '!') {
            memmove(Word, Word + 1, sizeof(Word) - 1);
            Word[sizeof(Word) - 1] = 0;
            r = -1 * r;
            goto notlp2;
        }

        if ((vt.condition = iscond(Word)) == -1) {
            proc = 1;
            if ((vt.action = isact(Word)) == -1) {
                if ((vt.action = isRoom(Word)) != -1) {
                    vt.condition = CALWAYS;
                    goto writecna;
                }
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "Invalid condition, '%s'", Word);
                vbprob(tmp, block);
                proc = 0;
                goto commands;
            }
            vt.condition = CALWAYS;
            goto doaction;
        }
        p = skipspc(p);
        proc = 1;
        if ((p = checkConditionParameters(vbslot, p, &conditions[vt.condition], ofp4)) == nullptr) {
            goto commands;
        }
        if (*p == 0) {
            if ((vt.action = isact(conditions[vt.condition].name)) == -1) {
                vbprob("Missing action", block);
                goto commands;
            }
            vt.action = 0 - (vt.action + 1);
            vt.condition = CALWAYS;
            goto writecna;
        }
        if (r == 1)
            vt.condition = -1 - vt.condition;
        p = preact(p);
        p = getword(p);
        if ((vt.action = isact(Word)) == -1) {
            if ((vt.action = isRoom(Word)) != -1)
                goto writecna;
            char tmp[128];
            snprintf(tmp, sizeof(tmp), "Invalid action, '%s'", Word);
            vbprob(tmp, block);
            goto commands;
        }
    doaction:
        p = skipspc(p);
        if ((p = checkActionParameters(p, &actions[vt.action], ofp4)) == nullptr) {
            goto commands;
        }
        vt.action = 0 - (vt.action + 1);

    writecna: /* Write the C & A lines */
        checkedfwrite((char *)&vt.condition, sizeof(vt), 1, ofp3);
        proc = 0;
        of3p += sizeof(vt);
        goto commands;

    writeslot:
        checkedfwrite(vbslot.wtype, sizeof(vbslot), 1, ofp2);
        proc = 0;
        of2p += sizeof(vbslot);
        if (lastc > 1)
            goto commands;
        if (lastc == 0)
            goto synloop;

        lastc = '\n';
    write:
        checkedfwrite(&g_verb, sizeof(g_verb), 1, ofp1);
        proc = 0;
        *(vbtab + (g_gameConfig.numVerbs - 1)) = g_verb;
    }
}

