/* Subsection of AMUL.C, Copyright (C) Oliver Smith 1990.
   All code within this file is copyright KingFisher Software, 1990	*/

// GetID.C -- Get user name & details. Create record for new users.

#include "amulinc.h"
#include "file_handling.h"

short int start_rm[512];

void
getid()
{
    int   i, ok, nrs;
    FILE *fp;

    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = actor = -1;
    *(me2->pre) = *(me2->post) = 0;
    strcpy(me2->arr, acp(ARRIVED));
    strcpy(me2->dep, acp(LEFT));
    last_him = last_her = -1;
    me->rchar = 0;

    me2->rec = -1;
    me2->flags = 0;
    do {
        ok = -1;

        getname();

        strcpy(him.name, block);
        sprintf(block, "%s%s", dir, plyrfn);

        // Ensure the players file has been created
        if ((fp = fopen(block, "ab+")) == NULL) {
            tx("Unable to create new player file...\n");
            quit();
        }
        fclose(fp);
        fp = afp = NULL;

        fopena(plyrfn);  // See if user is already in file
        while (!feof(afp)) {
            freadsafe(*me, afp);
            if (stricmp(me->name, him.name) == NULL)
                break;
        }
        me2->rec = ftell(afp) / sizeof(him);

        if (stricmp(me->name, him.name) != NULL)
            ok = newid();
        else
            ok = getpasswd();
    } while (ok != 0);

    // Inform AMAN that we have begun!
    SendIt(MSG_LOGGED_IN, 0, me->name);
    for (i = ranks - 1; i >= 0; i--)
        if (me->score >= (rktab + i)->score) {
            me->rank = i;
            break;
        }
    refresh();
    if (me->plays != 1)
        sys(WELCOMEBAK);

    // Work out the players start location
    fopenr(Resources::Compiled::roomData());  // Get descriptions file
    roomtab = rmtab;
    me2->room = -1;
    lroom = -1;
    nrs = 0;
loop:
    for (i = 0; i < rooms; i++, roomtab++) {
        if (roomtab->flags & RF_PLAYER_START)
            start_rm[nrs++] = i;
    }

    if (nrs == 0)
        me2->room = 0;
    if (nrs == 1)
        me2->room = start_rm[0];
    if (nrs > 1)
        me2->room = start_rm[mod(rnd(), nrs - 1)];
    roomtab = rmtab + me2->room;

    me2->wield = -1;
    me2->helping = -1;
    me2->helped = -1;
    me2->following = -1;
    me2->followed = -1;
    me2->fighting = -1;
    me2->numobj = 0;
    me2->weight = 0;
    me2->hadlight = 0;
    me2->light = 0;
    me2->flags = 0;
    me2->sctg = 0;
    ans("1m");
    if (me->flags & UF_ANSI)
        sys(ANSION);
    ans("0;37m");
    me->strength = (rktab + me->rank)->strength;
    me->stamina = (rktab + me->rank)->stamina;
    me->magicpts = (rktab + me->rank)->magicpts;
    if ((i = isverb("\"start")) != -1)
        lang_proc(i, 0);
    action(acp(COMMENCED), AOTHERS);
    look(roomtab->id, me->rdmode);
}

void
getname()
{
    char *p;
    int   i;

    do {
        word = -3;

        crsys(WHATNAME);
        block[0] = 0;
        Inp(block, NAMEL + 1);
        txc('\n');
        if (block[0] == 0)
            quit();
        if (strlen(block) < 3 || strlen(block) > NAMEL) {
            sys(LENWRONG);
            continue;
        }

        p = block;
        if ((i = type(&p)) > -1 && i != TC_PLAYER) {
            sys(NAME_USED);
            continue;
        }
        if (i == TC_PLAYER && word != Af) {
            utx(word, acp(LOGINASME));
            strcpy(me->name, block);
            sys(ALREADYIN);
            continue;
        }
        word = -2;
    } while (word != -2);
}

bool
newid()
{
    int i;

    strcpy(me->name, him.name);
    sys(CREATE);
    *me->name = 0;
    block[0] = 0;
    Inp(block, 3);
    if (toupper(block[0]) != 'Y')
        return false;

    me->score = 0;
    me->plays = 1;
    me->strength = rktab->strength;
    me->stamina = rktab->stamina;
    me->dext = rktab->dext;
    me->wisdom = rktab->wisdom;
    me->experience = rktab->experience;
    me->magicpts = rktab->magicpts;
    me->tasks = 0;
    me->tries = me->sex = me->rank = me->rdmode = 0;
    me->llen = UD_LINE_LENGTH;
    me->slen = UD_SCREEN_LINES;

    do {
        crsys(WHATGENDER);
        block[0] = 0;
        strcpy(me->name, him.name);
        Inp(block, 2);
        block[0] = toupper(block[0]);
        if (block[0] != 'M' && block[0] != 'F') {
            crsys(GENDINVALID);
        }
    } while (block[0] != 'M' && block[0] != 'F');
    me->sex = (block[0] == 'M') ? 0 : 1;

    do {
        crsys(ENTERPASSWD);
        block[0] = 0;
        Inp(block, 10);
        if (strlen(block) < 3 || strlen(block) > 8) {
            crsys(PASLENWRONG);
        }
    } while (strlen(block) < 3 || strlen(block) > 8);
    strcpy(me->passwd, block);

    me->name[0] = toupper(me->name[0]);
    for (i = 1; i < strlen(me->name); i++) {
        if (me->name[i - 1] == ' ')
            me->name[i] = toupper(me->name[i]);
        else
            me->name[i] = tolower(me->name[i]);
    }

    crsys(ASK4ANSI);
    block[0] = 0;
    Inp(block, 4);
    me->flags = (toupper(block[0]) == 'Y') ? me->flags | UF_ANSI : me->flags & -(1 + UF_ANSI);

    flagbits();

    save_me();
    txc('\n');

    // Send them the scenario
    ShowFile("scenario");

    crsys(YOUBEGIN);
    txc('\n');
    return true;
}

void
getpasswd()
{
    int i;

    me2->rec--;  // Move 'back' a record

    for (i = 0; i < 4; i++) {
        if (i == 3) {
            sys(TRIESOUT);  // Update bad try count
            me->tries++;
            save_me();
            quit();
        }
        txn("\nTry #%d -- ", i + 1);
        sys(ENTERPASSWD);
        block[0] = 0;
        Inp(block, 10);
        if (stricmp(block, me->passwd) == NULL)
            break;
    }
    me->plays++;
    if (me->tries > 0) {
        ans("1m");
        txc(0x7);
        txc('\n');
        txn(acp(FAILEDTRIES), me->tries);
        txc('\n');
        ans("0;37m");
    }
    me->tries = 0;
}
