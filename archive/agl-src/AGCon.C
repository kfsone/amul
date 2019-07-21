/*
 *	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
 *
 *	File: AGCon.C		AGL Manager Controller
 *
 *	LMod:	oliver 09/01/94 Converted to SAS 6.3
 *			oliver 20/06/93	Created
 */

#define	AMAN  1				/* Use AMAN only includes */
#define	PORTS 1				/* Use port-definition includes */

#include <adv:h/defs.h>
#include <adv:h/incs.h>
#include <adv:h/vars.h>
#include <adv:h/data.h>

/* Amiga Stuff */
struct Aport	*am;

char	*mannam="AMan Port";
char	*foruse="Type amcon -? for usage.";
char	quiet;

struct Library	*AMULBase;		/* For agl.library */


int CXBRK(){			/* Prevent CTRL-C'ing */
	return 0;
}

                         /* >>>> MAIN ROUTINE <<<< // */

void main(int argc,char *argv[])
	{
	int		i;
	char	*arg;
	char	buffer[128];

	/* Make sure AMan is running */
	if(!(port = FindPort(mannam)))
		{
		Printf("AMan not running!\n");
		exit(0);
		}

	/* Open the AGL library */
	if(!(AMULBase=OpenLibrary(AMULNAME,0L)))
		{
		Printf("Can't open %s library!\n", AMULNAME);
		exit(0);
		}

	quiet = FALSE;

	if(argc == 1 || !strcmp(argv[1],"-?") || argv[1][0]!='-')
		{
		Printf("AGCon Usage:\n\tagcon [options]\nOptions:\n");
		Printf("\t-k\t\tShut-down current game\n");
		Printf("\t-m <n>\t\tSet user time-limit to <n> minutes\n");
		Printf("\t-q\t\tQuiet mode\n");
		Printf("\t-r [<n>]\tForce reset [in <n> minutes]\n");
		Printf("\t-s <path>\tSet source path for next game\n");
		Printf("\t-x <n>\t\tExtend game by <n> minutes\n");
		exit(0);
		}

	for(i = 1 ; i<argc ; )
		{
		if(*(arg = argv[i++]) != '-' || !isalpha(*(arg+1)))
			{
			Printf("Invalid argument \"%s\". %s\n", arg, foruse);
			exit(0);
			}
		strcpy(buffer,arg+2);	// Allow, say, -r5 or -padv:game/
		arg+2=0;
		while(i<argc && *argv[i]!='-') {
			if(buffer[0]) strcat(buffer," ");
			strcat(buffer,argv[i++]);
		};
		if(!ArgProc(arg,(buffer[0])?buffer:NULL)) {
			printf("Invalid command line. %s\n",foruse);
			quit();
		}
	}
}

ArgProc(char *arg,char *parms) {
	switch(toupper(*(arg+1)) {
		// -K KILL CURRENT GAME
		case 'K':	if(parms) return FALSE; ASend(MKILL,NULL,NULL);
				break;
		// -M <n> RESTRICT USER ON-LINE TIME TO <n> MINUTES
				if(!parms) return FALSE;
				secs=Number(parms);
				// If limit is bad, report BUT continue.\n");
				if(secs && secs!=-1 && secs<10) {
					printf("-M time limit (%ld) must be -1, 0 or >10 minutes.\n",secs);
					break;
				}
				ASend(MTLIMIT,secs,NULL); break;
		// -Q QUIET MODE
		case 'Q':	if(parms) return FALSE; quiet=TRUE; break;
		// -R [<n>] RESET CURRENT GAME [IN <n> MINUTES]
		case 'R':	secs=Number(parms); ASend(MRESET,secs*60,NULL);
				break;
		// -S SWAP ADVENTURE PATH
		case 'S':	if(!parms) return FALSE;
				SetPath(nxtdir,parms); ASend(MSWAP,NULL,nxtdir);
				break;
		// -X <n> EXTEND CURRENT GAME
		case 'X':	if(!(secs=Number(parms))) return FALSE;
				ASend(MEXTEND,secs*60,NULL);
				break;
		default:	printf("Unknown option \"%s\". %s\n",arg,foruse);
				quit();
	}
	return TRUE;
}

ASend(int type, int data, char *s) {	// Shutdown request
	char *p;
	if((p=CommsPrep(NULL,&reply,&agl))) { printf("Failed: %s",p); quit(); }
	At = type; Ad = data; Af = -1; Ap = s;
	PutMsg(port,agl); WaitPort(reply); GetMsg((struct MsgPort*) reply);
	Delay(20);
	if(!quiet) switch(Ad) {
		case 'R':	printf("\x07- Reset Invoked -\n"); break;
		case 'O':	printf("Manager removed!\n"); break;
		case 'U':	printf("AMan error at other end!\n");
		case -'X':	printf("Reset set for %ld minutes\n",Ap1/60); break;
		case -'R':	printf("Reset in progress\n"); break;
		case 'E':	printf("Game extended by %ld minutes\n",Ap1/60); break;
		case 'S':	printf("AMan will load from '%s' at next reset\n",s); break;
		case -'S':	printf("Swap request failed\n"); break;
		case 'M':	printf("User time limit set to %ld minutes.\n",
		default:	printf("** Internal error ** (Returned '%c%c')\n",
				 (Ad<0)?'-':'+',(char)(Ad&127));
			break;
	}
	DeleteAPort(reply,agl);
}

quit() {			// Exit tidily
	if(AGLBase)	CloseLibrary((struct Library *)AGLBase); AGLBase=NULL;
	exit(0);
}

SetPath(register char *to,register char *from,register char err) {
	register int l; FILE *fp;
	if(!from || (l=strlen(from))) return FALSE;
	strcpy(to,from);
	if(*(from+l)!='/' && *(from+l)!=':') strcat(to,"/");
	if(!(fp=fopen(to,"rb"))) {
		printf("Can't access directory "%s"\n",to);
		quit();
	} else fclose(ifp);
}
