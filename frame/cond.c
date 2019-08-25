/* Conditions for AMUL */

// Is my room lit?
bool
lit(int r)
{
    if (!((rmtab + r)->flags & DARK))
        return true;
    Forbid();
    you2 = linestat;
    for (int i = 0; i < MAXU; i++, you2++)
        if (you2->room == r && you2->hadlight != 0) {
            Permit();
            return true;
        }
    struct _OBJ_STRUCT *objp = objtab;
    for (int i = 0; i < nouns; i++, objp++) {
        if ((STATE->flags & SF_LIT) != SF_LIT)
            continue;
        for (int j = 0; j < objp->nrooms; j++)
            if (*(objp->rmlist + j) == r) {
                Permit();
                return true;
            }
    }
    Permit();
    return false;
}

int
loc(int o)
{
    if (*(obtab + o)->rmlist >= -1)
        return *(obtab + o)->rmlist;
    if (*(obtab + o)->rmlist >= -5 && *(obtab + o)->rmlist <= -(5 + MAXU))
        return (int)(linestat + owner(o))->room;
    /* Else its in a container */
}

int
carrying(int obj)
{
    if (me2->numobj == 0)
        return -1;
    if (owner(obj) == Af)
        return obj;
    return -1;
}

bool
nearto(int ob)
{
    if (!canSeeObject(ob, Af))
        return false;
    if (isin(ob, me2->room))
        return true;
    if (carrying(ob) != -1)
        return true;
    return false;
}

// Can others in this room see me?
bool
isVisible()
{
    if (!lit(me2->room))
        return false;
    if (IamINVIS || IamSINVIS)
        return false;
    return true;
}

// If player could manage to carry object
bool
canGive(int obj, int plyr)
{
    objtab = obtab + obj;

    if ((linestat + plyr)->weight + STATE->weight > (rktab + (usr + plyr)->rank)->maxweight)
        return false;
    if ((linestat + plyr)->numobj + 1 > (rktab + (usr + plyr)->rank)->numobj)
        return false;
    if ((objtab->flags & OF_SCENERY) || (objtab->flags & OF_COUNTER) || objtab->nrooms != 1)
        return false;
    return true;
}

int
isverb(char *s)
{
    vbptr = vbtab;
    for (int i = 0; i < verbs; i++, vbptr++)
        if (match(vbptr->id, s) == NULL)
            return i;
    return -1;
}

int
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

// Is VERB syn
int
isvsyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) < -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    return (int)(csyn = -1);
}

// Is noun syn
int
isnsyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (*(synip + i) > -1 && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

int
issyn(char *s)
{
    char *p = synp;
    for (int i = 0; i < syns; i++, p += strlen(p) + 1) {
        if (toupper(*p) == toupper(*s) && match(p, s) == NULL) {
            csyn = *(synip + i);
            return strlen(p);
        }
    }
    csyn = -1;
    return -1;
}

// Due to the complexity of a multi-ocurance/state environment, I gave up
// trying to do a 'sort', storing the last, nearest object, and went for
// an eight pass seek-and-return parse. This may be damned slow with a
// few thousand objects, but if you only have a thousand, it'll be OK!

int
isnoun(char *s, int adj, char *pat)
{
    int pass, x;

    if (iverb != -1) {
        verb.sort[0] = *pat;
        strncpy(verb.sort + 1, pat + 1, 4);
    } else {
        strcpy(verb.sort + 1, "CHAE");
        verb.sort[0] = -1;
    }
    if ((obtab + start)->adj == adj && CHAEtype(start) == verb.sort[1] &&
        (obtab + start)->state == verb.sort[0])
        return start;

    bool done_e = 0;
    int lsuc = -1, lobj = -1;
    int start = isanoun(s);
    for (int pass = 1; pass < 5; pass++) {
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
            done_e = true;
    }
    if (!done_e)
        return scan(0, 'E', -1, s, adj);
    else
        return lobj;
}

int
isanoun(char *s)
{
    struct _OBJ_STRUCT *obpt = obtab;
    for (int i = 0; i < nouns; i++, obpt++)
        if (!(obpt->flags & OF_COUNTER) && stricmp(s, obpt->id) == NULL)
            return i;
    return -1;
}

int
scan(int start, char Type, int tst, char *s, int adj)
{
    int last = -1;
    struct _OBJ_STRUCT *obpt = obtab + start;
    for (int i = start; i < nouns; i++, obpt++) {
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

char
CHAEtype(int obj)
{
    if (carrying(obj) != -1)
        return 'C';
    if (isin(obj, me2->room))
        return 'H';
    if (int i = owner(obj); i != -1 && (linestat + i)->room == me2->room)
        return 'A';
    return 'E';
}

int
isadj(char *s)
{
    char *p = adtab;
    for (int i = 0; i < adjs; i++, p += IDL + 1)
        if (match(p, s) != -1)
            return i;
    return -1;
}

int
isprep(char *s)
{
    for (int i = 0; i < NPREP; i++)
        if (stricmp(s, prep[i]) == NULL)
            return i;
    return -1;
}

bool
isin(int o, int r)
{
    for (int i = 0; i < (obtab + o)->nrooms; i++)
        if (*((obtab + o)->rmlist + i) == r)
            return true;
    return false;
}

int
isroom(char *s)
{
    for (int r = 0; r < rooms; r++)
        if (stricmp((rmtab + r)->id, s) == 0)
            return r;
    return -1;
}

bool
isInflicted(int plyr, int spell)
{
    you2 = linestat + plyr;
    switch (spell) {
    case SGLOW:
        if (you2->flags & PFGLOW)
            return true;
        break;
    case SINVIS:
        if (you2->flags & PFINVIS)
            return true;
        break;
    case SDEAF:
        if (you2->flags & PFDEAF)
            return true;
        break;
    case SBLIND:
        if (you2->flags & PFBLIND)
            return true;
        break;
    case SCRIPPLE:
        if (you2->flags & PFCRIP)
            return true;
        break;
    case SDUMB:
        if (you2->flags & PFDUMB)
            return true;
        break;
    case SSLEEP:
        if (you2->flags & PFASLEEP)
            return true;
        break;
    case SSINVIS:
        if (you2->flags & PFSINVIS)
            return true;
        break;
    }
    return false;
}

bool
testStat(int plyr, int st, int x)
{
    switch (st) {
    case STSTR: return isValidNumber((linestat + plyr)->strength, x);
    case STSTAM: return isValidNumber((linestat + plyr)->stamina, x);
    case STEXP: return isValidNumber((usr + plyr)->experience, x);
    case STWIS: return isValidNumber((linestat + plyr)->wisdom, x);
    case STDEX: return isValidNumber((linestat + plyr)->dext, x);
    case STMAGIC: return isValidNumber((linestat + plyr)->magicpts, x);
    case STSCTG: return isValidNumber((linestat + plyr)->sctg, x);
    default: return false;
    }
}

// If <player1> can see <player2>
bool
canSee(int p1, int p2)
{
    /* You can't see YOURSELF, and check for various other things... */
    if (*(usr + p2)->name == 0 || p1 == p2)
        return false;
    if ((linestat + p2)->state != PLAYING)
        return false;
    /* If different rooms, or current room is dark */
    if (pROOM(p1) != pROOM(p2))
        return false;
    /* If p2 is Super Invis, he can't be seen! */
    if ((linestat + p2)->flags & PFSINVIS)
        return false;
    /* If player is blind, obviously can't see p2! */
    if ((linestat + p1)->flags & PFBLIND)
        return false;
    if (!lit(pROOM(p1)))
        return false;
    /* If you are in a 'hide' room and he isn't a wizard */
    if (pRANK(p1) == ranks - 1)
        return true;
    if (((rmtab + pROOM(p1))->flags & HIDE))
        return false;
    /* If he isn't invisible */
    if (!isPINVIS(p2))
        return true;
    /* Otherwise */
    if (isPINVIS(p1) && pRANK(p1) >= invis - 1)
        return true;
    if (pRANK(p1) >= invis2 - 1)
        return true;
    return false;
}

bool
canSeeObject(int obj, int who)
{
    if (((obtab + obj)->flags & OF_SMELL) && !((linestat + who)->flags & PFBLIND))
        return false;
    if ((linestat + who)->flags & PFBLIND &&
        (!((obtab + obj)->flags & OF_SMELL) || *(obtab + obj)->rmlist != -(5 + who)))
        return false;
    if (!isOINVIS(obj))
        return true;
    if (isPINVIS(who) && pRANK(who) >= invis - 1)
        return true;
    if (pRANK(who) < invis2 - 1)
        return false;
    return true;
}

bool
castWillSucceed(int rnk, int points, int chance)
{
    if (me->rank < rnk - 1) {
        sys(LOWLEVEL);
        return false;
    }

    if (me2->magicpts < points) {
        sys(NOMAGIC);
        return false;
    }

    if (me->rank < rnk)
        chance = ((me->rank) + 1 - rnk) * ((100 - chance) / (ranks - rnk)) + chance;
    if (mod(rnd(), 100) < chance) {
        sys(SPELLFAIL);
        return false;
    }
    me->magicpts -= points;
    return true;
}
