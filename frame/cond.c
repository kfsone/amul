/* Conditions for AMUL */

/* Is my room lit? */
bool
lit(int r)
{
    int        i, j;
    struct _OBJ_STRUCT *tobjtab;

    tobjtab = objtab;

    if (!((rmtab + r)->flags & DARK))
        return true;
    Forbid();
    you2 = linestat;
    for (i = 0; i < MAXU; i++, you2++)
        if (you2->room == r && you2->hadlight != 0) {
            Permit();
            return true;
        }
    objtab = obtab;
    for (i = 0; i < nouns; i++, objtab++) {
        if ((STATE->flags & SF_LIT) != SF_LIT)
            continue;
        for (j = 0; j < objtab->nrooms; j++)
            if (*(objtab->rmlist + j) == r) {
                objtab = tobjtab;
                Permit();
                return true;
            }
    };
    objtab = tobjtab;
    Permit();
    return false;
}

loc(int o)
{
    if (*(obtab + o)->rmlist >= -1)
        return *(obtab + o)->rmlist;
    if (*(obtab + o)->rmlist >= -5 && *(obtab + o)->rmlist <= -(5 + MAXU))
        return (int)(linestat + owner(o))->room;
    /* Else its in a container */
}

carrying(int obj)
{
    if (me2->numobj == 0)
        return -1;
    if (owner(obj) == Af)
        return obj;
    return -1;
}

nearto(int ob)
{
    if (canseeobj(ob, Af) == NO)
        return NO;
    if (isin(ob, me2->room) == YES)
        return YES;
    if (carrying(ob) != -1)
        return YES;
    return NO;
}

visible() /* Can others in this room see me? */
{
    if (LightHere == NO)
        return NO;
    if (IamINVIS || IamSINVIS)
        return NO;
    return YES;
}

cangive(int obj, int plyr) /* If player could manage to carry object */
{
    objtab = obtab + obj;

    if ((linestat + plyr)->weight + STATE->weight > (rktab + (usr + plyr)->rank)->maxweight)
        return NO;
    if ((linestat + plyr)->numobj + 1 > (rktab + (usr + plyr)->rank)->numobj)
        return NO;
    if ((objtab->flags & OF_SCENERY) || (objtab->flags & OF_COUNTER) || objtab->nrooms != 1)
        return NO;
    return YES;
}

isverb(char *s)
{
    int i;
    vbptr = vbtab;
    for (i = 0; i < verbs; i++, vbptr++)
        if (match(vbptr->id, s) == NULL)
            return i;
    return -1;
}

isaverb(char **s)
{
    int ret;

    if ((ret = isverb(*s)) != -1) {
        (*s) += strlen((vbtab + ret)->id);
        return ret;
    }
    if ((ret = isvsyn(*s)) != -1) {
        (*s) += ret;
        return -2 - csyn;
    }
    return -1;
}

isvsyn(char *s) /* Is VERB syn */
{
    int   i;
    char *p;
    p = synp;
    for (i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) < -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    return (int)(csyn = -1);
}

isnsyn(char *s) /* Is noun syn */
{
    int   i;
    char *p;
    p = synp;
    for (i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) > -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

issyn(char *s)
{
    int   i;
    char *p;
    p = synp;
    for (i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (toupper(*p) == toupper(*s) && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

/*
    Notice:
    ------

    Due to the complexity of a multi-ocurance/state environment, I gave up
    trying to do a 'sort', storing the last, nearest object, and went for
    an eight pass seek-and-return parse. This may be damned slow with a
    few thousand objects, but if you only have a thousand, it'll be OK!

                                        */

isnoun(char *s, int adj, char *pat)
{
    int pass, x, done_e, lsuc, lobj;
    int          start;

    if (iverb != -1) {
        verb.sort[0] = *pat;
        strncpy(verb.sort + 1, pat + 1, 4);
    } else {
        strcpy(verb.sort + 1, "CHAE");
        verb.sort[0] = -1;
    }
    done_e = 0;
    lsuc = lobj = -1;
    start = isanoun(s);
    if ((obtab + start)->adj == adj && CHAEtype(start) == verb.sort[1] &&
        (obtab + start)->state == verb.sort[0])
        return start;
    for (pass = 1; pass < 5; pass++) {
        /* At this point, we out to try BOTH phases, however, in the
           second phase, try for the word. Next 'pass', if there is
           no suitable item, drop back to that from the previous... */
        if (verb.sort[pass] == 'X')
            return lobj;
        if (verb.sort[0] == -1) {
            if ((x = scan(start, verb.sort[pass], -1, s, adj)) != -1) {
                if (adj != -1 || (obtab + x)->adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lsuc = 0;
                lobj = x;
            } else if (lsuc == 0)
                return lobj;
        } else {
            /* Did we get a match? */
            if ((x = scan(start, verb.sort[pass], 0, s, adj)) != -1) {
                /* If adjectives match */
                if (adj != -1 || (obtab + x)->adj == adj)
                    return x;
                if (lsuc == 0)
                    return lobj;
                lobj = x;
                lsuc = 0;
                continue;
            }
            if ((x = scan(start, verb.sort[pass], -1, s, adj)) != -1) {
                if (adj != -1 || (obtab + x)->adj == adj) {
                    lobj = x;
                    lsuc = 0;
                    continue;
                } /* Must have matched */
                if (lsuc == 0)
                    return lobj;
            }
            if (lsuc == 0)
                return lobj;
        };
        if (verb.sort[pass] == 'E')
            done_e = 1;
    }
    if (done_e == 0)
        return scan(0, 'E', -1, s, adj);
    else
        return lobj;
}

isanoun(char *s)
{
    int        i;
    struct _OBJ_STRUCT *obpt;

    obpt = obtab;
    for (i = 0; i < nouns; i++, obpt++)
        if (!(obpt->flags & OF_COUNTER) && stricmp(s, obpt->id) == NULL)
            return i;
    return -1;
}

scan(int start, char Type, int tst, char *s, int adj)
{
    int                          i, last;
    struct _OBJ_STRUCT *obpt;

    last = -1;
    obpt = obtab + start;
    for (i = start; i < nouns; i++, obpt++) {
        if ((obpt->flags & OF_COUNTER) || (adj != -1 && obpt->adj != adj))
            continue;
        if (match(obpt->id, s) != NULL || CHAEtype(i) != Type)
            continue;
        /* If state doesn't matter or states match, we'll try it */
        if (verb.sort[0] == -1 || tst == -1 || obpt->state == verb.sort[0]) {
            if (adj == obpt->adj)
                return i;
            else
                last = i;
        } else if (last == -1)
            last = i;
    }
    return last;
}

CHAEtype(int obj)
{
    int i;

    if (carrying(obj) != -1)
        return 'C';
    if (isin(obj, me2->room) == YES)
        return 'H';
    if ((i = owner(obj)) != -1 && (linestat + i)->room == me2->room)
        return 'A';
    return 'E';
}

isadj(char *s)
{
    int   i;
    char *p;

    p = adtab;
    for (i = 0; i < adjs; i++, p += IDL + 1)
        if (match(p, s) != -1)
            return i;
    return -1;
}

isprep(char *s)
{
    int i;
    for (i = 0; i < NPREP; i++)
        if (stricmp(s, prep[i]) == NULL)
            return i;
    return -1;
}

isin(int o, int r)
{
    int i;
    for (i = 0; i < (obtab + o)->nrooms; i++)
        if (*((obtab + o)->rmlist + i) == r)
            return YES;
    return NO;
}

isroom(char *s)
{
    int r;

    for (r = 0; r < rooms; r++)
        if (stricmp((rmtab + r)->id, s) == 0)
            return r;
    return -1;
}

infl(int plyr, int spell)
{
    you2 = linestat + plyr;
    switch (spell) {
    case SGLOW:
        if (you2->flags & PFGLOW)
            return YES;
        break;
    case SINVIS:
        if (you2->flags & PFINVIS)
            return YES;
        break;
    case SDEAF:
        if (you2->flags & PFDEAF)
            return YES;
        break;
    case SBLIND:
        if (you2->flags & PFBLIND)
            return YES;
        break;
    case SCRIPPLE:
        if (you2->flags & PFCRIP)
            return YES;
        break;
    case SDUMB:
        if (you2->flags & PFDUMB)
            return YES;
        break;
    case SSLEEP:
        if (you2->flags & PFASLEEP)
            return YES;
        break;
    case SSINVIS:
        if (you2->flags & PFSINVIS)
            return YES;
        break;
    }
    return NO;
}

stat(int plyr, int st, int x)
{
    switch (st) {
    case STSTR: return numb((linestat + plyr)->strength, x);
    case STSTAM: return numb((linestat + plyr)->stamina, x);
    case STEXP: return numb((usr + plyr)->experience, x);
    case STWIS: return numb((linestat + plyr)->wisdom, x);
    case STDEX: return numb((linestat + plyr)->dext, x);
    case STMAGIC: return numb((linestat + plyr)->magicpts, x);
    case STSCTG: return numb((linestat + plyr)->sctg, x);
    }
}

/* If <player1> can see <player2> */
cansee(int p1, int p2)
{
    /* You can't see YOURSELF, and check for various other things... */
    if (*(usr + p2)->name == 0 || p1 == p2)
        return NO;
    if ((linestat + p2)->state != PLAYING)
        return NO;
    /* If different rooms, or current room is dark */
    if (pROOM(p1) != pROOM(p2))
        return NO;
    /* If p2 is Super Invis, he can't be seen! */
    if ((linestat + p2)->flags & PFSINVIS)
        return NO;
    /* If player is blind, obviously can't see p2! */
    if ((linestat + p1)->flags & PFBLIND)
        return NO;
    if (!lit(pROOM(p1)))
        return NO;
    /* If you are in a 'hide' room and he isn't a wizard */
    if (pRANK(p1) == ranks - 1)
        return YES;
    if (((rmtab + pROOM(p1))->flags & HIDE))
        return NO;
    /* If he isn't invisible */
    if (!isPINVIS(p2))
        return YES;
    /* Otherwise */
    if (isPINVIS(p1) && pRANK(p1) >= invis - 1)
        return YES;
    if (pRANK(p1) >= invis2 - 1)
        return YES;
    else
        return NO;
}

canseeobj(int obj, int who)
{
    if (((obtab + obj)->flags & OF_SMELL) && !((linestat + who)->flags & PFBLIND))
        return NO;
    if ((linestat + who)->flags & PFBLIND &&
        (!((obtab + obj)->flags & OF_SMELL) || *(obtab + obj)->rmlist != -(5 + who)))
        return NO;
    if (!isOINVIS(obj))
        return YES;
    if (isPINVIS(who) && pRANK(who) >= invis - 1)
        return YES;
    if (pRANK(who) < invis2 - 1)
        return NO;
    else
        return YES;
}

magic(int rnk, int points, int chance)
{
    if (me->rank < rnk - 1) {
        sys(LOWLEVEL);
        return NO;
    }

    if (me2->magicpts < points) {
        sys(NOMAGIC);
        return NO;
    }

    if (me->rank < rnk)
        chance = ((me->rank) + 1 - rnk) * ((100 - chance) / (ranks - rnk)) + chance;
    if (mod(rnd(), 100) < chance) {
        sys(SPELLFAIL);
        return NO;
    }
    me->magicpts -= points;
    return YES;
}
