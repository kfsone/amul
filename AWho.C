/*

	AWHO			List AMUL Users Online
	~~~~			~~~~~~~~~~~~~~~~~~~~~~


	v1.0	First release, written by Oliver Smith.

*/

#define	AMAN 1			/* Get AMan includes */

#include <stdio.h>
#include <exec/types.h>
#include <exec/exec.h>
#include "h/amul.defs.h"
#include "h/amul.stct.h"
#include "h/amul.cons.h"

struct	MsgPort	*port,*reply,*FindPort(),*CreatePort();
struct	Task *mytask,*FindTask();
struct	Aport *amul;

struct	_PLAYER		*usr;
struct	LS		*lstat;
struct	_ROOM_STRUCT	*rmtab;
struct	_RANK_STRUCT	*rktab;
struct	_OBJ_STRUCT	*obtab;

long	calls,online,rooms,ranks,nouns;
char	*adname;
short	*rescnt;


main(int argc,char *argv[])			/* Accept no arguments */
{
	/* Look for AMUL Manager Port */
	printf("\n[31;1mAWho - [0;33mWho's on [32mAMUL[33m???[0m ");

	if(argc==2 && strcmp(argv[1],"?")==NULL || strcmp(argv[1],"-?")==NULL)
	{
		printf("\n\nUsage: [33m%s [32;1m[FULL][0m\n\n",argv[0]);
		printf("Purpose: Lists current status of AMan/AMUL adventure.\n%9sFULL option lists all users currently logged in.\n\n"," ");
		exit(0);
	}

	if(argc==2 && stricmp(argv[1],"full")!=NULL)
	{
		printf("\n\n\x08Invalid usage! Type \"[32m%s ?[0m\" for help.\n\n",argv[0]);
		exit(0);
	}

	port=FindPort(mannam);		/* Check for existing port */

	if(port==NULL)
	{
		printf("Manager not running...\n\n"); exit(0);
	}

	reply=CreatePort("feedback",0L);
	if(reply==NULL)
	{
		printf("Unable to create %s.\n","reply port"); exit(0);
	}

	amul=(struct Aport *)AllocMem((long)sizeof(*amul),MEMF_CLEAR+MEMF_PUBLIC);
	if(amul==NULL)
	{
		printf("Unable to create %s.\n","message structure"); quit();
	}
	Am.mn_Node.ln_Type=NT_MESSAGE; Am.mn_ReplyPort = reply;
	Am.mn_Length = (UWORD)sizeof(*amul);
	At = MSG_DATA_REQUEST;
	Ad = Af = -1;

	/* Get user structure and # online. */
	sendmsg(-1); usr=(struct _PLAYER *)Ap; lstat=(struct LS *)Ap4; online=Ad; calls=Ap1; adname=(char *)Ap3;
	printf("[31;1mCurrent Manager: [0;32m%s[0m\n",(char *) Ap2);

	/* Get rooms */
	sendmsg(1); rooms=Ad; rmtab=(struct _ROOM_STRUCT *) Ap;

	/* Get ranks */
	sendmsg(2); ranks=Ad; rktab=(struct _RANK_STRUCT *) Ap;

	/* Get nouns */
	sendmsg(3); nouns=Ad; obtab=(struct _OBJ_STRUCT  *) Ap;

	/* Get time till next reset */
	sendmsg(0);

	printf("\n[32mStatstics for \"[33m%s[32m\"[0m\n\n",adname);
	printf("Calls since loaded: [1;32m%6ld[0m	 Online now: [1;32m%4ld[0m\n",calls,online);
	printf("Rooms: [1;33m%6ld[0m    Objects: [1;33m%6ld[0m    Ranks: [1;33m%6ld[0m\n\n",rooms,nouns,ranks);
	if(argc==2)
	{   register int i;

	    if(online==0) printf("[1;33m ** No users online presently.[0m\n\n");
	    else
	    {
		printf("-----------%s----------+-----------%s----------+---%s---+--%s--\n","name","rank","location","score");
		for(i=0; i<MAXU; i++)
		{
		    if((lstat+i)->state <= 0) continue;
		    if((lstat+i)->state == 2) printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n",(usr+i)->name,((usr+i)->sex==0)?(rktab+(usr+i)->rank)->male:(rktab+(usr+i)->rank)->female,(rmtab+(lstat+i)->room)->id,(usr+i)->score);
		    else printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n","   << Logging In! >>","","...Nowhere!...",0);
		}
		printf("----------%s-----------+----------%s-----------+---%s---+--%s--\n\n","----","----","--------","-----");
	    }
	}
	rescnt=(short *)Ap1;
	printf("Next reset in [33m%ld[0m mins, [33m%ld[0m secs.\n\n",*rescnt/60,*rescnt-((*rescnt/60)*60));
	quit();
}

sendmsg(register int i)
{
	Ad=i;

	PutMsg(port,amul); WaitPort(reply); GetMsg((struct MsgPort*) reply);
	switch(Ad & At)
	{
		case 'R': printf("\x07*-- Reset In Progress --*\n"); quit(); break;
		case 'O':
		case 'X':
		case 'U': printf("AMAN error at other end... (returned '%c')\n",Ad); quit(); break;
		case -'R':printf("..... Reset in progress .....\n"); quit(); break;
	}
}

quit()
{
	if(reply != NULL) DeletePort((struct MsgPort *)reply);
	if(amul != NULL)  FreeMem((char *)amul,(long)sizeof(*amul));
	exit(0);
}
