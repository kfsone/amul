/* Phrase/sentance processing */

ttproc()
{
    register int             i, match, dun, l;
    register struct _TT_ENT *tp;

    exeunt = died = donet = skip = 0;
    failed = NO;
    roomtab = rmtab + me2->room;
    l = -1;
    tt.verb = iverb;
    if (roomtab->tabptr == -1) {
        sys(CANTGO);
        return 0;
    }
    dun = -1;
    ml = roomtab->ttlines;
    tp = ttp + roomtab->tabptr;

    iocheck();
    if (forced != 0 || died != 0 || exeunt != 0)
        return 0;
more:
    i = donet;
    for (i = donet; i < ml; i++) {
        ttabp = tp + i;
        match = -1;
        donet++;
        tt.pptr = ttabp->pptr;
        if (skip != 0) {
            skip--;
            continue;
        }
        if (ttabp->verb == iverb && (l = cond(ttabp->condition, l)) != -1) {
            match = i;
            break;
        }
    }
    skip = 0;
    if (ttabp->condition == CSPELL && match == -1)
        return 0;
    if (match == -1)
        return dun;
    tt.condition = ttabp->condition;
    inc = 1;
    act(ttabp->action, ttabp->pptr);
    if (inc == 1)
        dun = 0;
    if (ml < -1) {
        ml = roomtab->ttlines;
        donet = 0;
    }
    if (donet < ml && exeunt == 0 && died == 0)
        goto more;
    return 0;
}

act(long ac, long *pt)
{
    register long x1, x2;
    register int  i;
    tt.action = ac;
    tt.pptr = pt;
    if (tt.condition < 0)
        tt.condition = -1 - tt.condition;

    if (ac < 0) {
        ac = -1 - ac;
        switch (ac) {
        case ASAVE: asave(); break;
        case ASCORE: ascore(TP1); break;
        case ASETSTAT: asetstat(TP1, TP2); break;
        case ALOOK: look((rmtab + me2->room)->id, 1); break;
        case AWHAT: list_what(me2->room, 1); break;
        case AWHERE: awhere(TP1); break;
        case AWHO: awho(TP1); break;
        case ATREATAS: atreatas(TP1); return;
        case ASKIP: skip += TP1; break;
        case ATRAVEL:
            x1 = donet;
            x2 = ml;
            if (ttproc() == 0)
                donet = ml = x2;
            else {
                ml = x2;
                donet = x1;
            };
            break;
        case AQUIT: aquit();
        case AENDPARSE: donet = ml + 1; break;
        case AKILLME: akillme(); break;
        case AFAILPARSE: afailparse(); break;
        case AFINISHPARSE: afinishparse(); break;
        case AABORTPARSE: aabortparse(); break;
        case AWAIT: fwait(TP1); break;
        case ABLEEP: ableep(TP1); break;
        case AWHEREAMI: txs("Current room is known as \"%s\".\n", (rmtab + me2->room)->id); break;
        case ASEND: send(TP1, TP2); break;
        case AERROR: afailparse();     /* Set those flags! */
        case ARESPOND: donet = ml + 1; /* Then do reply etc */
        case AREPLY:
        case AMESSAGE: tx(AP1); break;
        case AANOUN: announce(AP2, TP1); break;
        case ACHANGESEX: achange(TP1); break;
        case ASLEEP: me2->flags = me2->flags | PFASLEEP; break;
        case AWAKE: me2->flags = me2->flags & (-1 - PFASLEEP); break;
        case ASIT:
            me2->flags = me2->flags | PFSITTING;
            me2->flags = me2->flags & (-1 - PFLYING);
            break;
        case ASTAND: me2->flags = me2->flags & (-1 - PFSITTING - PFLYING); break;
        case ALIE:
            me2->flags = me2->flags | PFLYING;
            me2->flags = me2->flags & (-1 - PFSITTING);
            break;
        case ARDMODE:
            me->rdmode = TP1;
            txs("%s mode selected.\n",
                (me->rdmode == RDRC) ? "Roomcount" : (me->rdmode == RDVB) ? "Verbose" : "Brief");
            break;
        case ARESET: SendIt(MRESET, 0, NULL); break; /* Tell AMAN that we want a reset! */
        case AACTION: action(AP2, TP1); break;
        case AMOVE: moveto(TP1); break;
        case AMSGIN: announcein(TP1, AP2); break;
        case AACTIN: actionin(TP1, AP2); break;
        case AMSGFROM: announcefrom(TP1, AP2); break;
        case AACTFROM: actionfrom(TP1, AP2); break;
        case ATELL:
            if (!((linestat + TP1)->flags & PFDEAF)) {
                setmxy(NOISE, TP1);
                utx(TP1, AP2);
            }
            break;
        case AADDVAL: aadd(scaled(State(TP1)->value, State(TP1)->flags), STSCORE, Af); break;
        case AGET: agive(TP1, Af); break;
        case ADROP: adrop(TP1, me2->room, YES); break;
        case AINVENT:
            strcpy(block, "You are ");
            invent(Af);
            break;
        case AGIVE: agive(TP1, TP2); break;
        case AINFLICT: inflict(TP1, TP2); break;
        case ACURE: cure(TP1, TP2); break;
        case ASUMMON: summon(TP1); break;
        case AADD: aadd(TP1, TP2, TP3); break;
        case ASUB: asub(TP1, TP2, TP3); break;
        case ACHECKNEAR: achecknear(TP1); break;
        case ACHECKGET: acheckget(TP1); break;
        case ADESTROY: adestroy(TP1); break;
        case ARECOVER: arecover(TP1); break;
        case ASTART: dpstart(TP1, TP2); break;
        case AGSTART: dgstart(TP1, TP2); break;
        case ACANCEL: SendIt(MDCANCEL, TP1, NULL); break;
        case ABEGIN: dbegin(TP1); break;
        case ASHOWTIMER: dshow(TP1); break;
        case AOBJAN: objannounce(TP1, AP2); break;
        case AOBJACT: objaction(TP1, AP2); break;
        case ACONTENTS:
            str[0] = 0;
            showin(TP1, YES);
            break;
        case AFORCE: aforce(TP1, TP2); break;
        case AHELP:
            me2->helping = TP1;
            (linestat + TP1)->helped = Af;
            break;
        case ASTOPHELP:
            (linestat + me2->helping)->helped = -1;
            me2->helping = -1;
            break;
        case AFIX: afix(TP1, TP2); break;
        case AOBJINVIS: (obtab + TP1)->flags = (obtab + TP1)->flags | OF_INVIS; break;
        case AOBJSHOW: (obtab + TP1)->flags = (obtab + TP1)->flags & (-1 - OF_INVIS); break;
        case AFIGHT: afight(TP1); break;
        case AFLEE:
            dropall((linestat + me2->fighting)->room);
            clearfight();
            break;
        case ALOG: log(AP1); break;
        case ACOMBAT: acombat(); break;
        case AWIELD:
            me2->wield = TP1;
            break;
        /* - */ case AFOLLOW:
            (linestat + TP1)->followed = me2->unum;
            me2->following = TP1;
            break;
        /* - */ case ALOSE:
            LoseFollower();
            break;
        /* - */ case ASTOPFOLLOW:
            StopFollow();
            break;
        case AEXITS: exits(); break;
        case ATASK: me->tasks = me->tasks | (1 << (TP1 - 1)); break;
        case ASHOWTASK: show_tasks(Af); break;
        case ASYNTAX:
            asyntax(*(tt.pptr + ncop[tt.condition]), *(tt.pptr + ncop[tt.condition] + 1));
            break;
        case ASETPRE: iocopy((linestat + TP1)->pre, AP2, 79); break;
        case ASETPOST: iocopy((linestat + TP1)->post, AP2, 79); break;
        case ASETARR:
            qcopy((linestat + TP1)->arr, AP2, 79);
            strcat((linestat + TP1)->arr, "\n");
            break;
        case ASETDEP:
            qcopy((linestat + TP1)->dep, AP2, 79);
            strcat((linestat + TP1)->dep, "\n");
            break;
        case ASENDDAEMON: dsend(TP1, TP2, TP3); break;
        case ADO: ado(TP1); break;
        case AINTERACT: ainteract(TP1); break;
        case AAUTOEXITS: autoexits = (char)TP1; break;
        case ABURN: osflag(CP1, State(CP1)->flags | SF_LIT); break;
        case ADOUSE: osflag(CP1, State(CP1)->flags & -(1 + SF_LIT)); break;
        case AINC:
            if ((obtab + CP1)->state != ((obtab + CP1)->nstates - 1))
                asetstat(CP1, (obtab + CP1)->state + 1);
            break;
        case ADEC:
            if ((obtab + CP1)->state != 0)
                asetstat(CP1, (obtab + CP1)->state - 1);
            break;
        case ATOPRANK: toprank(); break;
        case ADEDUCT: deduct(TP1, TP2); break;
        case ADAMAGE: damage(TP1, TP2); break;
        case AREPAIR: repair(TP1, TP2); break;
        default: txn("** Internal error - illegal action %ld!\n", ac);
        }
    } else {
        register int flag;

        needcr = NO;
        iocheck();
        if (exeunt != 0 || died != 0)
            return;
        if (fol == 0)
            StopFollow();
        Forbid();
        if ((rmtab + ac)->flags & SMALL) /* Allow for losing follower! */
        {
            for (i = 0; i < MAXU; i++)
                if ((linestat + i)->room == ac) {
                    Permit();
                    sys(NOROOM);
                    actionin(ac, acp(NOROOMIN));
                    LoseFollower();
                    return;
                };
        }
        me2->flags = me2->flags | PFMOVING; /* As of now I'm out of here. */
        if (visible() == YES) {
            Permit();
            action(me2->dep, AOTHERS);
            Forbid();
        }
        ldir = iverb;
        flag = NO;
        lroom = me2->room;
        i = me2->light;
        me2->light = 0;
        Permit();
        lighting(Af, AOTHERS);
        StopFollow();
        me2->room = ac;
        me2->light = i;
        me2->hadlight = 0;
        lighting(Af, AOTHERS);
        if (visible() == YES)
            action(me2->arr, AOTHERS);
        me2->flags = me2->flags & -(1 + PFMOVING);
        if (me2->followed > -1 && me2->followed != Af && (!IamINVIS) && (!IamSINVIS)) {
            /* If we didn't just execute a travel verb, we've lost them.
               If the other player hasn't caught up with us, lose them! */
            if (((vbtab + overb)->flags & VB_TRAVEL) || (linestat + me2->followed)->room != lroom ||
                ((linestat + me2->followed)->flags & PFMOVING))
                LoseFollower();
            else {
                DoThis(me2->followed, (vbtab + overb)->id, 1);
                flag = YES;
            }
        } else
            me2->followed = -1;
        look((rmtab + me2->room)->id, me->rdmode);
        if (exeunt != 0 || died != 0)
            return;
        if (autoexits != 0)
            exits();
    }
    if (tt.condition == CANTEP || tt.condition == CALTEP || tt.condition == CELTEP)
        donet = ml + 1;
}

cond(long n, int l) /* Execute a condition on me */
{
    register int mult, ret, i;

    mult = 1;
    ret = 1;
    tt.condition = (n < 0) ? -1 - n : n;
    if (n < 0) {
        n = -1 - n;
        mult = -1;
    }

    /* Do the conditions */
    switch (n) {
    case CALTEP:
    case CSTAR:
    case CALWAYS: break;
    case CANTEP:
    case CAND: ret = l; break;
    case CELTEP:
    case CELSE: ret = -l; break;
    case CLIGHT:
        if (lit(me2->room) == NO)
            ret = -1;
        break;
    case CISHERE:
        if (isin(CP1, me2->room) == NO)
            ret = -1;
        break;
    case CMYRANK:
        if (numb(me->rank + 1, CP1) == NO)
            ret = -1;
        break;
    case CSTATE:
        if ((obtab + CP1)->flags & OF_ZONKED)
            ret = -1;
        if (numb((obtab + CP1)->state, CP2) == NO)
            ret = -1;
        break;
    case CMYSEX:
        if (me->sex != CP1)
            ret = -1;
        break;
    case CLASTVB:
        if (lverb != CP1)
            ret = -1;
        break;
    case CLASTDIR:
        if (ldir != CP1)
            ret = -1;
        break;
    case CLASTROOM:
        if (lroom != CP1)
            ret = -1;
        break;
    case CASLEEP:
        if (!(me2->flags & PFASLEEP))
            ret = -1;
        break;
    case CSITTING:
        if (!(me2->flags & PFSITTING))
            ret = -1;
        break;
    case CLYING:
        if (!(me2->flags & PFLYING))
            ret = -1;
        break;
    case CRAND:
        if (numb(mod(rnd(), CP1), *(tt.pptr + 1)) == NO)
            ret = -1;
        break;
    case CRDMODE:
        if (me->rdmode != CP1)
            ret = -1;
        break;
    case CONLYUSER:
        for (i = 0; i < MAXU; i++)
            if ((usr + i)->name[0] != 0 && (linestat + i)->state > 1)
                ret = -1;
        break;
    case CALONE:
        for (i = 0; i < MAXU; i++)
            if (((linestat + i)->room == me2->room) && i != Af)
                ret = -1;
        break;
    case CINROOM:
        if (me2->room != CP1)
            ret = -1;
        break;
    case COPENS:
        if (!((obtab + CP1)->flags & OF_OPENS))
            ret = -1;
        break;
    case CGOTNOWT:
        if (me2->numobj != 0)
            ret = -1;
        break;
    case CCARRYING:
        if (gotin(CP1, -1) == NO)
            ret = -1;
        break;
    case CNEARTO:
        if (nearto(CP1) == NO)
            ret = -1;
        break;
    case CHIDDEN:
        if (visible() == YES)
            ret = -1;
        break;
    case CCANGIVE:
        if (cangive(CP1, CP2) == NO)
            ret = -1;
        break;
    case CINFL:
    case CINFLICTED:
        if (infl(CP1, CP2) == NO)
            ret = -1;
        break;
    case CSAMEROOM:
        if ((linestat + CP1)->room != me2->room)
            ret = -1;
        break;
    case CTOPRANK:
        if (me->rank != ranks - 1)
            ret = -1;
        break;
    case CSOMEONEHAS:
        if (((*(obtab + CP1)->rmlist) > -5) || ((*(obtab + CP1)->rmlist) < (-5 - MAXU)))
            ret = -1;
        break;
    case CGOTA:
        if (gotin(CP1, CP2) == NO)
            ret = -1;
        break;
    case CACTIVE:
        SendIt(MCHECKD, CP1, NULL);
        if (Ad == -1)
            ret = -1;
        break;
    case CTIMER:
        SendIt(MCHECKD, CP1, NULL);
        if (Ad == -1 || numb(Ap1, CP2) == NO)
            ret = -1;
        break;
    case CBURNS:
        if (!((obtab + CP1)->flags & OF_FLAMABLE))
            ret = -1;
        break;
    case CCONTAINER:
        if ((obtab + CP1)->contains <= 0)
            ret = -1;
        break;
    case CEMPTY:
        if ((obtab + CP1)->inside != 0)
            ret = -1;
        break;
    case COBJSIN:
        if (numb((obtab + CP1)->inside, CP2) == NO)
            ret = -1;
        break;
    case CHELPING:
        if (me2->helping != CP1)
            ret = -1;
        break;
    case CGOTHELP:
        if (me2->helped == -1)
            ret = -1;
        break;
    case CANYHELP:
        if (me2->helping == -1)
            ret = -1;
        break;
    case CSTAT:
        if (stat(CP2, CP1, CP3) == NO)
            ret = -1;
        break;
    case COBJINV:
        if (!isOINVIS(CP1))
            ret = -1;
        break;
    case CFIGHTING:
        if (!((linestat + CP1)->flags & PFFIGHT))
            ret = -1;
        break;
    case CTASKSET:
        if (!(me->tasks & (1 << (CP1 - 1))))
            ret = -1;
        break;
    case CCANSEE:
        if (cansee(Af, CP1) == NO)
            ret = -1;
        break;
    case CVISIBLETO:
        if (cansee(CP1, Af) == NO)
            ret = -1;
        break;
    case CNOUN1:
        if (wtype[2] != WNOUN) {
            ret = -1;
            break;
        }
        if (CP1 == inoun1)
            break;
        if (CP1 == WNONE) {
            ret = -1;
            break;
        }
        if (stricmp((obtab + CP1)->id, (obtab + inoun1)->id) != NULL)
            ret = -1;
        break;
    case CNOUN2:
        if (wtype[5] != WNOUN) {
            ret = -1;
            break;
        }
        if (CP1 == WNONE) {
            ret = -1;
            break;
        }
        if (stricmp((obtab + CP1)->id, (obtab + inoun2)->id) != NULL)
            ret = -1;
        break;
    case CAUTOEXITS:
        if (autoexits == 0)
            return -1;
        break;
    case CDEBUG:
        if (debug == 0)
            return -1;
        break;
    case CFULL:
        if (stfull(CP1, CP2) == NO)
            return -1;
        break;
    case CTIME:
        if (numb(*rescnt, CP1) == NO)
            return -1;
        break;
    case CDEC:
        if ((obtab + CP1)->state == 0)
            return -1;
        asetstat(CP1, (obtab + CP1)->state - 1);
        break;
    case CINC:
        if ((obtab + CP1)->state >= ((obtab + CP1)->nstates - 1))
            return -1;
        asetstat(CP1, (obtab + CP1)->state + 1);
        break;
    case CLIT:
        if (!(State(CP1)->flags & SF_LIT))
            return -1;
        break;
    case CFIRE:
        if (!((obtab + CP1)->flags & OF_SHOWFIRE))
            return -1;
        break;
    case CHEALTH:
        if (numb((((linestat + CP1)->stamina * 100) / (rktab + (usr + CP1)->rank)->stamina), CP2) ==
            NO)
            ret = -1;
        break;
    case CMAGIC:
        if (magic(CP1, CP2, CP3) == NO)
            return -1;
        break;
    case CSPELL:
        if (numb((linestat + CP1)->wisdom, mod(rnd(), CP2)) == NO)
            ret = -1;
        break;
    case CIN:
        if (isin(CP2, CP1) == NO)
            ret = -1;
        break;
    default: ret = -1;
    }

    return mult * ret;
}

type(register char **s)
{
    register char *p, *p2;

strip:
    p = *s;
    while (isspace(*p))
        p++;
    *s = p;
    if (*p == 0) {
        word = -1;
        return -1;
    } /* none */

    /* Check for a players name BEFORE checking for white words! */

    for (word = 0; word < MAXU; word++)
        if ((linestat + word)->state == PLAYING && match((usr + word)->name, p) == NULL) {
            *s += strlen((usr + word)->name);
            return WPLAYER;
        }

    /* cut any white words out */
    if (tolower(*p) == 'a') {
        if (match("an", p) == NULL) {
            *s += 2;
            goto strip;
        }
        if (match("at", p) == NULL) {
            *s += 2;
            goto strip;
        }
        if (match("a", p) == NULL) {
            *s++;
            goto strip;
        }
    }
    if (tolower(*p) == 't') {
        if (match("the", p) == NULL) {
            *s += 3;
            goto strip;
        }
        if (match("to", p) == NULL) {
            *s += 2;
            goto strip;
        }
    }
    if (tolower(*p) == 'f') {
        if (match("for", p) == NULL) {
            *s += 3;
            goto strip;
        }
        if (match("from", p) == NULL) {
            *s += 4;
            goto strip;
        }
    }
    if (match("is", p) == NULL) {
        *s += 2;
        goto strip;
    }
    if (match("using", p) == NULL) {
        *s += 5;
        goto strip;
    }
    if (match("with", p) == NULL) {
        *s += 4;
        goto strip;
    }

    if (*p == '\"' || *p == '\'') {
        register char c;
        c = *p;
        p2 = ++p;
        word = (int)p;
        while (*p2 != 0 && *p2 != c)
            p2++;
        if (*p2 != 0)
            *(p2++) = 0;
        else
            *(p2 + 1) = 0;
        *s = p2;
        return WTEXT;
    }

    if (match("it", p) == NULL) {
        word = it;
        *s += 2;
        return (it != -1) ? WNOUN : -2;
        ;
    }

    if (tolower(*p) == 'm') {
        if (match("me", p) == NULL) {
            word = Af;
            *s += 2;
            return WPLAYER;
        }
        if (match("myself", p) == NULL) {
            word = Af;
            *s += 2;
            return WPLAYER;
        }
    }

    /* inoun/sort crap is related to object chae patterns */

    if ((word = issyn(*s)) != -1) {
        *s += word;
        if (csyn < -1) {
            word = -2 - csyn;
            return WVERB;
        }
        if (inoun1 == inoun2 == -1) {
            word = csyn;
            return WNOUN;
        }
        if ((word =
                     isnoun((obtab + csyn)->id, (inoun1 == -1) ? iadj1 : iadj2,
                            (inoun1 == -1) ? (vbtab + iverb)->sort : (vbtab + iverb)->sort2)) != -1)
            return WNOUN;
        return -1;
    }
    if (inoun1 == inoun2 == -1) {
        if ((word = isanoun(*s)) != -1) {
            *s += strlen((obtab + word)->id);
            return WNOUN;
        }
    } else if (
            (word = isnoun(
                     *s, (inoun1 == -1) ? iadj1 : iadj2,
                     (inoun1 == -1) ? (vbtab + iverb)->sort : (vbtab + iverb)->sort2)) != -1) {
        *s += strlen((obtab + word)->id);
        return WNOUN;
    }
    if ((word = isadj(*s)) != -1) {
        *s += strlen(adtab + (word * (IDL + 1)));
        return WADJ;
    }
    if ((word = isroom(*s)) != -1) {
        *s += strlen((rmtab + word)->id);
        return WROOM;
    }
    if ((word = isprep(*s)) != -1) {
        *s += strlen(prep[word]);
        goto strip;
    }
    if ((word = isverb(*s)) != -1) {
        *s += strlen(vbptr->id);
        return WVERB;
    }
    while (!isspace(*p) && *p != 0)
        p++;
    word = -1;
    return -2; /* Unknown */
}

actual(register unsigned long n)
{
    if (n & RAND0)
        return (int)mod(rnd(), n ^ RAND0);
    if (n & RAND1)
        return (int)(mod(rnd(), n ^ RAND1) + (n ^ RAND1) / 2);
    if (n & PRANK)
        return (int)pRANK(actual(n & -(1 + PRANK)));

    if ((n & (OBVAL + OBDAM + OBWHT)) != 0) {
        register int x;
        x = actual(n & (-1 - (OBVAL + OBDAM + OBWHT)));
        switch (n & (OBVAL + OBDAM + OBWHT)) {
        case OBVAL: return (int)scaled(State(x)->value, State(x)->flags);
        case OBWHT: return (int)State(x)->weight;
        case OBDAM: return (int)State(x)->strength;
        case OBLOC: return (int)loc(x);
        }
        return -1;
    }
    if ((n & IWORD) == IWORD) {
        /* Replace with no. of a users word */
        switch (n & (-1 - IWORD)) {
        case IVERB: return iverb;
        case IADJ1: return iadj1;
        case INOUN1: return inoun1;
        case IPREP: return iprep;
        case IADJ2: return iadj2;
        case INOUN2: return inoun2;
        }
        return -1;
    }
    if ((n & MEPRM) == MEPRM) {
        /* Replace with details of self */
        switch (n & (-1 - MEPRM)) {
        case LOCATE: return -1; /* Not implemented */
        case SELF: return (int)Af;
        case HERE: return (int)me2->room;
        case RANK: return (int)me->rank;
        case FRIEND: return (int)me2->helping;
        case HELPER: return (int)me2->helped;
        case ENEMY: return (int)me2->fighting;
        case WEAPON: return (int)me2->wield;
        case SCORE: return (int)me->score;
        case SCTG: return (int)me2->sctg;
        case STR: return (int)me2->strength;
        case LASTROOM: return (int)lroom;
        case LASTDIR: return (int)ldir;
        case LASTVB: return (int)lverb;
        }
        return -1;
    }
    return (int)n;
}

actptr(unsigned long n)
{
    if (n == -1 || n == -2)
        return (int)&"\0";
    if ((n & IWORD) == IWORD) {
        /* Replace with no. of a users word */
        switch (n & (-1 - IWORD)) {
        case INOUN2: return (int)&"@n2\n";
        default: return (int)&"@n1\n";
        }
    }
    return (int)umsgp + *(umsgip + n);
}

deduct(int plyr, int howmuch)
{
    int amount;

    if (howmuch < 0)
        return;
    if (plyr == Af) {
        amount = me->score * howmuch / 100;
        asub(amount, STSCORE, Af);
    } else
        sendex(plyr, ADEDUCT, plyr, howmuch, 0, 0); /* Tell them to clear up! */
}
