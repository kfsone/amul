/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL.C.......Adventure System      ****
              ****               Main Program!              ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.

*/

#define AMUL 1
#define AMUL1 1
#define FRAME 1
#define PORTS 1

#include <frame/amulinc.h> /* Main Include file */
#include <h/amul.cons.h>   /* Predefined Constants etc */
#include <h/amul.h>        /* Version info etc. */

main(int argc, char *argv[]) /* Main Program */
{
    register int i;

    lverb = -1;
    iverb = -1;
    ip = 1;
    needcr = NO;
    addcr = NO;
    MyFlag = am_USER;

    sprintf(vername, "AMUL v%d.%d (%s)", VERSION, REVISION, DATE);

    if (argc > 1 && argv[1][0] != '-') {
        printf("\n\x07!! Invalid argument, %s!\n", argv[1]);
        quit();
    }
    if (argc > 1) {
        switch (toupper(*(argv[1] + 1))) {
        case 3: /* Daemon processor */
            MyFlag = am_DAEM;
            iosup = LOGFILE;
            break;
        case 4:
            MyFlag = am_MOBS;
            iosup = LOGFILE;
            break;
        case 'C': /* Custom screen */
            switchC(argc, *(argv[1] + 2));
            break;
        case 'S': /* Serial Device */
            switchS((argc > 2) ? argv[2] : NULL, (argc > 3) ? argv[3] : NULL,
                    (argc > 4) ? argv[4] : "0", (argc > 5) ? argv[5] : "Y");
            break;
        case 'N': /* NTSC Screen */
            switchC(argc, 'N');
            break;
        default
            : /* None specified */
        {
            txs("\nInvalid parameter '%s'!\n", argv[1]);
            quit();
        }
        }
    } else
        switchC(argc, 0);
    if ((ob = (char *)AllocateMem(5000)) == NULL)
        memfail("IO buffers");
    if ((ow = (char *)AllocateMem(3000)) == NULL)
        memfail("IO buffers");
    if ((input = (char *)AllocateMem(400)) == NULL)
        memfail("IO buffers");
    if ((port = FindPort(mannam)) == NULL) {
        tx("Manager not running!\n");
        quit();
    }
    if ((reply = CreatePort(0L, 0L)) == NULL)
        memfail("user port");
    if ((repbk = CreatePort(0L, 0L)) == NULL)
        memfail("comms reply");
    if ((amanrep = CreatePort(0L, 0L)) == NULL)
        memfail("aman port");
    if ((amul = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    if ((amanp = (struct Aport *)AllocateMem(sizeof(*amul))) == NULL)
        memfail("comms port");
    Am.mn_Length = (UWORD)sizeof(*amul);
    Am.mn_Node.ln_Type = NT_MESSAGE;
    Am.mn_ReplyPort = amanrep;
    switch (MyFlag) /* What type of line? */
    {
    case am_DAEM:
        Af = MAXU;
        break;
    case am_MOBS:
        Af = MAXU + 1;
        break;
    }
    *amanp = *amul;
    link = 1;
    SendIt(MCNCT, -10, NULL); /* Go for a connection! */
    linestat = (struct LS *)Ad;
    me2 = linestat + Af;
    me2->IOlock = Af;
    ip = 0;
    usr = (struct _PLAYER *)Ap;
    me = usr + Af;
    me2->rep = reply;
    if (Ad == -'R') {
        tx("\n...Reset In Progress...\n");
        Delay(50);
        quit();
    }
    reset(); /* Read in data files */
    if (Af < 0) {
        sys(NOSLOTS);
        pressret();
        quit();
    }
    if (iosup == CUSSCREEN) {
        sprintf(wtil, "%s   Line: %2d  ", vername, Af);
        strcat(wtil, "Logging in!");
        SetWindowTitles(wG, wtil, wtil);
    }
    me2->unum = Af;
    me2->sup = iosup;
    me2->buf = ob;
    *ob = 0;
    me2->IOlock = -1;
    *ob = 0;
    SendIt(MFREE, NULL, NULL);
    iocheck();

    /* Special processors go HERE: */

    if (Af >= MAXU)
        Special_Proc();

    /* Clear room flags, and send scenario */
    rset = (1 << Af);
    rclr = -1 - rset;
    for (i = 0; i < rooms; i++)
        *(rctab + i) = (*(rctab + i) & rclr);

    do /* Print the title */
    {
        i = fread(block, 1, 900, ifp);
        block[i] = 0;
        tx(block);
    } while (i == 900);
    fclose(ifp);
    ifp = NULL;

    getid(); /*  GET USERS INFO */

    if (iosup == CUSSCREEN) {
        sprintf(wtil, "%s   Line: %2d  ", vername, Af);
        strcat(wtil, "Player: ");
        strcat(wtil, me->name);
        SetWindowTitles(wG, wtil, wtil);
    }

    last_him = last_her = it = -1;

    do {
        died = 0;
        actor = -1;
        fol = 0;
        needcr = NO;
        addcr = NO;
        if (last_him != -1 && (linestat + last_him)->state != PLAYING)
            last_him = -1;
        if (last_her != -1 && (linestat + last_her)->state != PLAYING)
            last_her = -1;
        iocheck();
        tx((rktab + me->rank)->prompt);
        needcr = YES;
        block[0] = 0;
        Inp(input, 390);
        if (exeunt != 0)
            break;
        if (stricmp(input, "help") == NULL) {
            sys(HELP);
            continue;
        }
        if (input[0] == '/') {
            internal(input + 1);
            continue;
        }
        if (stricmp(input, "***debug") == NULL) {
            debug = debug ^ 1;
            continue;
        }
        if (input[0] == 0)
            continue;
    gloop:
        failed = NO;
        forced = 0;
        died = 0;
        exeunt = 0;
        if (grab() == -1)
            break;
        iocheck();
        if (forced != 0 && died == 0 && exeunt == 0)
            goto gloop;
    } while (exeunt == 0 && died == 0);

quitgame: /* Quite the game, tidily. */
    if (died == 0)
        action(acp(EXITED), AGLOBAL);
    else
        action(acp(HEDIED), AGLOBAL);
    forced = 0;
    exeunt = 0;
    died = 0;
    if (me->plays == 1)
        sys(BYEPASSWD);
    if (dmove[0] != 0)
        dropall(isroom(dmove));
    else
        dropall(me2->room);
    LoseFollower(); /* Lose who ever is following us. */
    if (iosup == CUSSCREEN)
        pressret();
    quit();
}

look(char *s, int f)
{
    register int roomno, mod;

    /* Some complex stuff here!
      if f==0 (rdmode=RoomCount) and we have been here before,
        look(brief)
      if f==0 (rdmode=RoomCount) and this is our first visit,
        look(verbose)
      if f==0 visit the room
                                   */

    if ((roomno = isroom(s)) == -1)
        return;
    roomtab = rmtab + roomno;
    if (f == RDRC && ((*(rctab + roomno) & rset) != rset))
        mod = RDVB;
    else
        mod = f;
    if (f != 2)
        *(rctab + roomno) = *(rctab + roomno) | rset;
    look_here(mod, roomno);
}

agive(register int obj, register int to) /* Add object to players inventory */
{
    register int own, orm;

    objtab = obtab + obj;

    if ((objtab->flags & OF_SCENERY) || (STATE->flags & SF_ALIVE) || objtab->nrooms != 1)
        return;
    if ((own = owner(obj)) != -1)
        rem_obj(own, obj);
    orm = *objtab->rmlist;
    add_obj(to);

    /*== The lighting conditions for transfering an object between a
         variable source and destination are complex! See below!	*/
    if (STATE->flags & SF_LIT) {
        if (own == -1) /*== Did I just pick and was it from here? */
        {
            if (orm == (linestat + own)->room)
                return;
        } else { /*== If I got it from someone, and he is near me... */
            if ((linestat + own)->room == (linestat + to)->room)
                return;
            lighting(own, AHERE); /*== Else check his lights! */
        }
        lighting(to, AHERE);
    }
}

adrop(int ob, int r, int f) /* Drop the object (to a room) */
{
    objtab = obtab + ob;
    *objtab->rmlist = r;
    rem_obj(Af, ob);
    lighting(Af, AHERE);

    /* If the room IS a 'swamp', give em points */
    if ((rmtab + me2->room)->flags & SANCTRY) {
        /*== Only give points if player hasn't quit. */
        if (exeunt == 0)
            aadd(scaled(STATE->value, STATE->flags), STSCORE, Af);
        *objtab->rmlist = -1;
    }
}
