//	AWHO		List AGL Users Online
//	~~~~		~~~~~~~~~~~~~~~~~~~~~
//
// v2.0 oliver	Adapted to suite "AGL"
// v1.2 oliver	Trimmed to reduce size (why? it was 9000 bytes!)
// v1.1 oliver	Now uses basic amul.library functions!
// v1.0 oliver	First RELEASE re-done by Oliver Smith
// v0.9 rpike	First versions, based on "who" by richard pike
//


#define	AMAN 1			/* Get AMan includes */
#define	FRAME 1			/* Avoids various includes */

#include <stdio.h>
#include <exec/types.h>
#include <exec/exec.h>
#include "adv:h/defs.h"
#include "adv:h/stct.h"
#include "amlib:libraries/agl.h"
#include "amlib:proto/agl.h"

struct	MsgPort	*port,*reply;
struct	Aport *agl;
struct	Library *AGLBase;

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
		printf("Usage : %s [BRIEF]\nPurpose: Lists current status of AGL adventure.\n",argv[0]);
		exit(0);
	}

	if(argc==2 && !stricmp(argv[1],"full")) argc--;
	if(argc==2 && stricmp(argv[1],"quiet")) {
		printf("\x08Invalid usage! Type \"%s ?\" for help.\n",argv[0]);
		exit(0);
	}

	if(!(AGLBase=OpenLibrary("agl.library",0L))) {
		printf("Can't open %s!\n","agl.library"); exit(0);
	}

	if((p=CommsPrep(&port,&reply,&agl))) { printf("Error: %s",p); quit(); }

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

	printf("Statstics for \"%s\"\n\n",adname);
	printf("Calls since loaded: %6ld	 Online now: %4ld\n",calls,online);
	printf("Rooms: %6ld    Objects: %6ld    Ranks: %6ld\n\n",rooms,nouns,ranks);
	if(argc<2) {
	    register int i;

	    if(online) {
		printf("-----------%s----------+-----------%s----------+---%s---+--%s--\n","name","rank","location","score");
		for(i=0; i<MAXU; i++) {
		    if((lstat+i)->state <= 0) continue;
		    if((lstat+i)->state == 2) printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n",(usr+i)->name,((usr+i)->sex)?(rktab+(usr+i)->rank)->female:(rktab+(usr+i)->rank)->male,(rmtab+(lstat+i)->room)->id,(usr+i)->score);
		    else printf("%-25.25s|%-25.25s|%-14.14s|%9.9ld\n","   << Logging In! >>","","...Nowhere!...",0);
		}
		printf("----------%s-----------+----------%s-----------+---%s---+--%s--\n","----","----","--------","-----");
	    } else printf(" ** No users online presently.\n\n");
	}
	rescnt=(short *)Ap1;
	printf("Next reset in %ld mins, %ld secs.\n",*rescnt/60,*rescnt-((*rescnt/60)*60));
	quit();
}

sendmsg(register int i) {
	Ad=i;

	PutMsg(port,agl); WaitPort(reply); GetMsg((struct MsgPort*)reply);
	switch(Ad & At) {
		case 'O':
		case 'X':
		case 'U': printf("AMAN error at other end... (returned '%c')\n",Ad); quit();
		case 'R':
		case -'R': printf("...Reset in progress...\n"); quit();
	}
}

quit() {
	if(reply)	DeleteAPort(reply,agl);
	if(AGLBase)	CloseLibrary(AGLBase);
	exit(0);
}
