/*

	AWHO		List AMUD Users Online
	~~~~		~~~~~~~~~~~~~~~~~~~~~~


	v1.2	Trimmed to reduce size (why? it was 9000 bytes!)
	v1.1	Now uses basic amud.library functions!
	v1.0	First release, written by Oliver Smith.

							*/

#define	AMAN 1			/* Get AMan includes */
#define	FRAME 1			/* Avoids various includes */

#include <stdio.h>
#include <exec/types.h>
#include <exec/exec.h>
#include "adv:h/amud.defs.h"
#include "adv:h/amud.stct.h"
#include "amlib:libraries/amud.h"
#include "amlib:proto/amud.h"

struct	MsgPort	*port,*reply;
struct	Task *mytask,*FindTask();
struct	Aport *amud;
struct	Library *AMUDBase;

struct	_PLAYER		*usr;
struct	LS		*lstat;
struct	_ROOM_STRUCT	*rmtab;
struct	_RANK_STRUCT	*rktab;
struct	_OBJ_STRUCT	*obtab;

long	calls,online,rooms,ranks,nouns;
char	*adname;
short	*rescnt;


main(int argc,char *argv[]) {			/* Accept no arguments */
	register char *p;

	if(argc==2 && !strcmp(argv[1],"?") || !strcmp(argv[1],"-?")) {
		printf("Usage: %s [BRIEF]\n\nPurpose: Lists current status of AMUD adventure.\n\n",argv[0]);
		exit(0);
	}

	if(argc==2 && !stricmp(argv[1],"full")) argc--;
	if(argc==2 && stricmp(argv[1],"quiet")) {
		printf("\x08Invalid usage! Type \"%s ?\" for help.\n\n",argv[0]);
		exit(0);
	}

	if(!(AMUDBase=OpenLibrary("amud.library",0L))) {
		printf("Can't open %s!\n","amud.library"); exit(0);
	}

	if((p=CommsPrep(&port,&reply,&amud))) { printf("Error: %s",p); quit(); }

	At = MDATAREQ; Ad=-1;

	/* Get user structure and # online. */
	sendmsg(-1); usr=(struct _PLAYER *)Ap; lstat=(struct LS *)Ap4; online=Ad; calls=Ap1; adname=(char *)Ap3;

	/* Get rooms */
	sendmsg(1); rooms=Ad; rmtab=(struct _ROOM_STRUCT *) Ap;

	/* Get ranks */
	sendmsg(2); ranks=Ad; rktab=(struct _RANK_STRUCT *) Ap;

	/* Get nouns */
	sendmsg(3); nouns=Ad; obtab=(struct _OBJ_STRUCT  *) Ap;

	/* Get time till next reset */
	sendmsg(0);

	printf("Statstics for \"[1m%s[0m\"\n\n",adname);
	printf("Calls since loaded: [1m%6ld[0m	 Online now: [1m%4ld[0m\n",calls,online);
	printf("Rooms: [1m%6ld[0m    Objects: [1m%6ld[0m    Ranks: [1m%6ld[0m\n\n",rooms,nouns,ranks);
	if(argc<2)
	{   register int i;

	    if(online) {
		printf("-----------%s----------+-----------%s----------+---%s---+--%s--\n","name","rank","location","score");
		for(i=0; i<MAXU; i++)
		{
		    if((lstat+i)->state <= 0) continue;
		    if((lstat+i)->state == 2) printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n",(usr+i)->name,((usr+i)->sex)?(rktab+(usr+i)->rank)->female:(rktab+(usr+i)->rank)->male,(rmtab+(lstat+i)->room)->id,(usr+i)->score);
		    else printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n","   << Logging In! >>","","...Nowhere!...",0);
		}
		printf("----------%s-----------+----------%s-----------+---%s---+--%s--\n","----","----","--------","-----");
	    }
	    else printf("[1m ** No users online presently.[0m\n\n");
	}
	rescnt=(short *)Ap1;
	printf("Next reset in [1m%ld[0m mins, [1m%ld[0m secs.\n\n",*rescnt/60,*rescnt-((*rescnt/60)*60));
	quit();
}

sendmsg(register int i)
{
	Ad=i;

	PutMsg(port,amud); WaitPort(reply); GetMsg((struct MsgPort*)reply);
	switch(Ad & At) {
		case 'O':
		case 'X':
		case 'U': printf("AMAN error at other end... (returned '%c')\n",Ad); quit();
		case 'R':
		case -'R': printf("..... Reset in progress .....\n"); quit();
	}
}

quit()
{
	if(reply)	DeleteAPort(reply,amud);
	if(AMUDBase)	CloseLibrary(AMUDBase);
	exit(0);
}
