/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL4.C......Adventure System      ****
              ****            Special Processors!           ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.


                      Mobile & Global Daemon Processors
                                        */

#include "frame/amulinc.h"

long minop; /* Mobiles in operation */

Special_Proc() /* Special Processor core */
{
    register int i;

    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    wtype[0] = wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = WNONE;
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = last_him = last_her = it = -1;
    switch (MyFlag) /* What type of processor ? */
    {
    case am_DAEM: /* Execute the boot-up daemon */
        if ((i = isverb("\"boot")) != -1)
            lang_proc(i, 0);
        Daem_Proc(); /* Daemon Processor */
    case am_MOBS: printf("-- Mobile processor loaded\n");
    default: printf("-- Unsupported special processor requested\n");
    }
    quit(); /* Don't go anywhere */
}

Daem_Proc() /* Daemon processing host */
{
    long         lastt;
    register int i, ded, next;

    /* Setup Mobile bits & pieces */
    /* Whats the highest travel verb number? */
    for (i = 0; i < verbs; i++)
        if (!(vbptr->flags & VB_TRAVEL))
            lastt = i;

    do {
        Wait(-1);
        iocheck();
    } while (1 == 1);
}
