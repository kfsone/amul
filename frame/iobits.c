iocheck()
{
    REALiocheck();
    addcr = NO;
}

REALiocheck()
{
    int   i;
    long  t, d, f;
    char *pt;
    int            p[4];

loopit:
    t = 0;
    if ((ap = (struct Aport *)GetMsg(repbk)) != NULL) {
        t = ap->type;
        ReleaseMem(&ap);
        goto loopit;
    }
    if (t == -'R')
        goto here;
    if ((ap = (struct Aport *)GetMsg((struct MsgPort *)reply)) == NULL)
        return;
    ip = 1;
    addcr = YES;
    if (ap->type == MCLOSEING || ap->type == -'R') {
    here:
        me2->helping = me2->helped = me2->following = me2->followed = -1;
        sys(RESETSTART);
        if (MyFlag == am_USER) {
            fopenr("reset.txt");
            do /*  Print the Reset Text */
            {
                i = fread(block, 1, 800, ifp);
                block[i] = 0;
                tx(block);
            } while (i == 800);
            fclose(ifp);
            ifp = NULL;
            sprintf(spc, "\n%s is resetting ... Saving at %ld.\n\nPlease call back later.\n\n",
                    adname, me->score);
            tx(spc);
            pressret();
        }
        ap->from = -'O';
        ReplyMsg((struct Message *)ap);
        link = 0;
        quit();
    }
    t = ap->type;
    d = ap->data;
    f = ap->from;
    pt = ap->ptr;
    p[0] = ap->p1;
    p[1] = ap->p2;
    p[2] = ap->p3;
    p[3] = ap->p4;
    if (t == MDAEMON) {
        long p1, p2, p3, p4, v;
        ReplyMsg((struct Message *)ap);
        if (MyFlag == am_DAEM)
            tx("Processing daemon!\n");
        p1 = inoun1;
        p2 = inoun2;
        p3 = wtype[2];
        p4 = wtype[5];
        v = iverb;
        inoun1 = p[0];
        inoun2 = p[1];
        wtype[2] = p[2];
        wtype[5] = p[3];
        ip = 0;
        lang_proc(d, 0);
        inoun1 = p1;
        inoun2 = p2;
        wtype[2] = p3;
        wtype[4] = p4;
        iverb = v;
        ip = 1;
        goto voila;
    }
    if (t == MFORCE)
        strcpy(input, ap->ptr);
    SendIt(MBUSY, NULL, NULL);
    if (f != -1 && (linestat + f)->state == PLAYING)
        ReplyMsg((struct Message *)ap);
    else
        ReleaseMem(&ap);
    lockusr(Af);
    /* Any messages we receive should wake us up. */

    if (me2->flags & PFASLEEP) {
        cure(Af, SSLEEP);
        sys(IWOKEN);
        i = 1;
    } else
        i = 0;

    if (t == MSUMMONED) {
        if (d != me2->room) {
            sys(BEENSUMND);
            if (lit(me2->room) && !(me2->flags & PFINVIS) && !(me2->flags & PFSINVIS))
                action(acp(SUMVANISH), AOTHERS);
            moveto(d);
            if (lit(me2->room) YES && !(me2->flags & PFINVIS) && !(me2->flags & PFSINVIS))
                action(acp(SUMARRIVE), AOTHERS);
        }
        i = 0; /* wake in transit. */
        goto endlok;
    }
    if (t == MDIE) {
        akillme();
        goto endlok;
    }
    if (t == MEXECUTE) {
        tt.condition = 0;
        act(d, (long *)&p[0]);
    }
    if (t == MFORCE) {
        if (d == 0) /* 0=forced, 1=follow */
            txs("--+ You are forced to \"%s\" +--\n", input);
        else {
            sprintf(block, "You follow %s %s...\n", (usr + f)->name, input);
            tx(block);
            fol = 1;
        }
        forced = d + 1;
    }
    if (t == MRWARN) {
        addcr = YES;
        tx(pt);
        goto endlok;
    }
    if (t != MMESSAGE)
        goto endlok;
wait: /* Lock my IO so I can read & clear my output buffer */
loked:
    addcr = YES;
    tx(ob);
    *ob = 0;
endlok:
    me2->IOlock = -1;
    if (i == 1 && !IamINVIS && !(me2->flags & PFSINVIS))
        action(acp(WOKEN), AOTHERS);
voila:
    ip = 0;
    SendIt(MFREE, NULL, NULL);
    goto loopit; /* Check for further messages */
}

lockusr(int u)
{
    long t, d, p;
    do {
        t = At;
        d = Ad;
        p = (long)Ap;
        SendIt(MLOCK, u, NULL);
        if (Ad != u && ip == 0) {
            iocheck();
            Ad = -1;
        }
    } while (Ad != u);
    At = t;
    Ad = d;
    Ap = (char *)p;
}

ioproc(char *s)
{
    char *p;

    p = ow;

lp:
    if (*s == 0) {
        *p = 0;
        return p - ow;
    }
    if ((*p = *(s++)) == '@' && esc(s, p) != 0) {
        p += strlen(p);
        s += 2;
    } else if (*(p++) == '\n')
        *(p++) = '\r';
    goto lp;
}

esc(char *p, char *s) /* Find @ escape sequences */
{
    char c;

    c = tolower(*(p + 1));
    switch (tolower(*p)) {
    case 'm':
        switch (c) {
        case 'e': strcpy(s, me->name); return 1;
        case '!': sprintf(s, "%-21s", me->name); return 1;
        case 'r': PutRankInto(s); return 1;
        case 'f':
            if (me2->following == -1)
                strcpy(s, "no-one");
            else
                strcpy(s, (usr + me2->following)->name);
            return 1;
        case 'g': sprintf(s, "%ld", me2->magicpts); return 1;
        default: return 0;
        }
    case 'g':
        switch (c) {
        case 'n': strcpy(s, (me->sex == 0) ? "male" : "female"); return 1;
        case 'e': strcpy(s, (me->sex == 0) ? "he" : "she"); return 1;
        case 'o': strcpy(s, (me->sex == 0) ? "his" : "her"); return 1;
        case 'h': strcpy(s, (me->sex == 0) ? "him" : "her"); return 1;
        case 'p': sprintf(s, "%ld", me->plays); return 1;
        }
    case 's':
        if (c == 'c') {
            sprintf(s, "%ld", me->score);
            return 1;
        }
        if (c == 'g') {
            sprintf(s, "%ld", me2->sctg);
            return 1;
        }
        if (c == 'r') {
            sprintf(s, "%ld", me2->strength);
            return 1;
        }
        if (c == 't') {
            sprintf(s, "%ld", me2->stamina);
            return 1;
        }
        return 0;
    case 'v':
        if (c == 'b') {
            strcpy(s, (vbtab + overb)->id);
            return 1;
        }
        if (c == 'e') {
            strcpy(s, (vbtab + iverb)->id);
            return 1;
        }
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            sprintf(s, "%ld", scaled(State(inoun1)->value, State(inoun1)->flags));
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            sprintf(s, "%ld", scaled(State(inoun2)->value, State(inoun2)->flags));
            return 1;
        }
    case 'w':
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            sprintf(s, "%ldg", ((obtab + inoun1)->states + (long)(obtab + inoun1)->state)->weight);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            sprintf(s, "%ldg", ((obtab + inoun2)->states + (long)(obtab + inoun2)->state)->weight);
            return 1;
        }
        if (c == 'i') {
            sprintf(s, "%ld", me2->wisdom);
            return 1;
        }
    case 'n':
        if (c == '1' && inoun1 >= 0 && wtype[2] == WNOUN) {
            strcpy(s, (obtab + inoun1)->id);
            return 1;
        }
        if (c == '1' && wtype[2] == WTEXT) {
            strcpy(s, (char *)inoun1);
            return 1;
        }
        if (c == '1' && inoun1 >= 0 && wtype[2] == WPLAYER) {
            strcpy(s, (usr + inoun1)->name);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WNOUN) {
            strcpy(s, (obtab + inoun2)->id);
            return 1;
        }
        if (c == '2' && wtype[5] == WTEXT) {
            strcpy(s, (char *)inoun2);
            return 1;
        }
        if (c == '2' && inoun2 >= 0 && wtype[5] == WPLAYER) {
            strcpy(s, (usr + inoun2)->name);
            return 1;
        }
        strcpy(s, "something");
        return 1;
    case 'e':
        if (c == 'x') {
            sprintf(s, "%ld", me->experience);
            return 1;
        }
    case 'l':
        if (c == 'r') {
            strcpy(s, lastres);
            return 1;
        }
        if (c == 'c') {
            strcpy(s, lastcrt);
            return 1;
        }
    case 'p':
        if (c == 'l') {
            strcpy(s, (usr + me2->fighting)->name);
            return 1;
        }
        if (c == 'w') {
            strcpy(s, me->passwd);
            return 1;
        }
        if (isdigit(c)) {
            fwait(c - '0');
            return 1;
        }
    case 'r':
        if (c == 'e') {
            timeto(s, *rescnt);
            return 1;
        }
    case 'h': /* The person helping you */
        if (c == 'e' && me2->helped != -1) {
            strcpy(s, (usr + me2->helped)->name);
            return 1;
        }
    case 'f': /* <friend> - person you are helping */
        if (c == 'r' && me2->helping != -1) {
            strcpy(s, (usr + me2->helping)->name);
            return 1;
        }
        if (c == 'm' && me2->followed != -1) {
            strcpy(s, (usr + me2->followed)->name);
            return 1;
        }
        strcpy(s, "no-one");
        return 1;
    case 'o':
        if (c == '1' && me2->wield != -1) {
            strcpy(s, (obtab + (me2->wield))->id);
            return 1;
        }
        if (c == '2' && (linestat + (me2->fighting))->wield != -1) {
            strcpy(s, (obtab + ((linestat + (me2->fighting))->wield))->id);
            return 1;
        }
        strcpy(s, "bare hands");
        return 1;
    case 'x':
        if (c == 'x')
            strcpy(s, mxx);
        if (c == 'y')
            strcpy(s, mxy);
        return 1;
    default: return 0;
    }
}

interact(int msg, int n, int d)
{
    if ((linestat + n)->state < PLAYING)
        return;
    lockusr(n);
    if (msg == MMESSAGE)
        strcat((linestat + n)->buf, ow);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = msg;
    IAd = d;
    (linestat + n)->IOlock = -1;
    PutMsg((linestat + n)->rep, (struct Message *)intam);
}

sendex(int n, int d, int p1, int p2, int p3, int p4)
{
    if ((linestat + n)->state < PLAYING)
        return;
    lockusr(n);
    if ((intam = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    IAm.mn_Length = (UWORD)sizeof(*amul);
    IAf = Af;
    IAm.mn_Node.ln_Type = NT_MESSAGE;
    IAm.mn_ReplyPort = repbk;
    IAt = MEXECUTE;
    IAd = -(1 + d);
    intam->p1 = p1;
    intam->p2 = p2;
    intam->p3 = p3;
    intam->p4 = p4;
    (linestat + n)->IOlock = -1;
    PutMsg((linestat + n)->rep, (struct Message *)intam);
}
