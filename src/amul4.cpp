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

/* Daemon processing host */
void
Daem_Proc()
{
    printf("-- Daemon processor loaded\n");
    /* Setup Mobile bits & pieces */
    /* Whats the highest travel verb number? */
	long lastt = -1;
    for (i = 0; i < verbs; i++)
        if (!(vbptr->flags & VB_TRAVEL))
            lastt = i;

    for ( ; ; ) {
        Wait(-1);
        iocheck();
    }
}

void
Mobile_Proc()
{
    printf("-- Mobile processor loaded\n");

	// Not implemented.
	for ( ; ; ) {
		Wait(-1);
		iocheck();
	}
}

// Special Processor core
void
Special_Proc()
{
    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    wtype[0] = wtype[1] = wtype[2] = wtype[3] = wtype[4] = wtype[5] = WNONE;
    iverb = iadj1 = inoun1 = iprep = iadj2 = inoun2 = -1;
    actor = last_him = last_her = it = -1;
    switch (MyFlag) /* What type of processor ? */
    {
    case am_DAEM: /* Execute the boot-up daemon */
        if (int i = isverb("\"boot"); i != -1)
            lang_proc(i, 0);
        Daem_Proc(); /* Daemon Processor */
		break;
    case am_MOBS:
		Mobile_Proc();
    default:
        printf("-- Unsupported special processor requested\n");
    }
    quit(); /* Don't go anywhere */
}

