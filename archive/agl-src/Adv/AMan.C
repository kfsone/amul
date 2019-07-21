//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AMan.C		AGL Manager - Main File
//
//	LMod: oliver 20/06/93	Removed various arguments -> ACon
//				Added user time limiting (-M option)
//	      oliver 19/06/93	Mobile verbs -> -2
//	      oliver 11/06/93	AMUL Manager -> AGL Manager
//

#define MAXD	250
#define	XR	16667

#define	AMAN  1				// Use AMAN only includes
#define	PORTS 1				// Use port-definition includes
#define TRQ timerequest			// abbreviation
#define	DELFAC	4			// Slow-time at session start
#define	PRIV	-1			// Local/private log entry

#define	RMINFO	rooms * (4+sizeof(*rmtab)) // 4="seen" bit-flags, sizeof(long)
#define	UINFO	((sizeof(*usr)+sizeof(*lstat))*MAXNODE)+(RMINFO)

#include <adv:h/defs.h>
#include <adv:h/incs.h>
#include <adv:h/vars.h>
#include <adv:h/data.h>

#include <devices/timer.h>
#include <time.h>
#include <proto/timer.h>
#include <intuition/intuition.h>

// Bit Flags			TRUE When:
short	reset		: 1;		// game should reset
short	shutdn		: 1;		// game should shut down
short	forcereset	: 1;		// reset was forced
short	flushmem	: 1;		// memory should be flushed
short	quiet		: 1;		// "quiet" mode active
short	mobact		: 1;		// mobiles are enabled
short	toggle		: 1;		// toggle between DIR and NXTDIR
short	timeropen	: 1;		// timer open?
short	timefactor	: 3;		// RealTime:GameTime ratio
short	online		: 5;		// 1+ users online (online=count)

char	lastres[24],lastcrt[24],bid[MAXNODE],busy[MAXNODE],*now();
char	nxtdir[64];			// Path for next game if swapping
char	nxtgame[42];
long	limit,maxtime,secs,calls,tick;
long	obtlen,mobmem;			// Memory allocations
short	mobln,daemons;			// Which line are mobiles on?
int	amsz;
long	clock,cclock,rststamp;

// Daemon Information
struct _DAEMON	daem[MAXD],*free,*daemon;

// Amiga Stuff
struct	Task *mytask,*FindTask();
struct Aport	*am;
struct TRQ	ResReq;			// Timer request
struct MsgPort	*trport;		// Timer port
long	scrx,scry,scrw,scrh;		// Screen settings
char	scrm;

struct Library	*AGLBase;		// For agl.library
char	*ttxt,**errtxt;			// agl.library pointers

char	*Mbase;	long	Mlen;		// For givebackmemory/memrelease

char	*mannam="AMan Port";
char	Lnos[]="#0123456789ABCD*M";	// Local, 0-14, Daemon & Mobile lines

// #defines

#ifndef	TRUE
#define	TRUE	1
#define	FALSE	0
#endif

// Text #defines

#define	AGLPATH		"GAME:"		// Default path
#define	INVPARMNO	"Invalid number of parameters!\n"


                             //--- Functions ---\\


int CXBRK(){			// Prevent CTRL-C'ing
	return 0;
}

char *now(){			// Get current time/date
	if(!clock) time(&clock);
	ttxt=(char *)ctime(&clock)+4; clock=0;
	*(ttxt+strlen(ttxt)-1)=0;
	return ttxt;
}

char *readf(char *s, char *p){
	fopenr(s); fread(p,32767,32767,ifp); fclose(ifp); ifp=NULL; return p;
}

fsize(char *s){
	register int n;
	fopenr(s); fseek(ifp,0,2L); n=ftell(ifp);
	fclose(ifp); ifp=NULL; return ((n+2)&-4);
}

void getlibs(){
	register ULONG *base;
	if(!(AGLBase=OpenLibrary(AGLNAME,0L))) quitS("\nmissing library!\n");
	base=(ULONG *)LibTable(); errtxt=TextPtr();
	IntuitionBase=(struct Library *)*(base+INTBoff);
	GfxBase=(struct Library *)*(base+GFXBoff);
}

long nextno() { long x; fread((char *)&x,4,1,ifp); return x; }

Error(long x) { if(x>512) printf((char *)x); else printf(*(errtxt+x)); }

ERROR(long x) { quitS((x>512)?(char *)x:*(errtxt+x)); }

                         // >>>> MAIN ROUTINE <<<< //

main(int argc,char *argv[]){
	reset=shutdn=forcereset=flushmem=quiet=mobact=toggle=timeropen=FALSE;

	amsz=sizeof(*agl);		// Size of AGL structure
	getlibs();			// Get agl.library etc.

	// Set task name
	setvername(vername); (mytask=FindTask(0L))->tc_Node.ln_Name = vername;

	dir[0]=nxtdir[0]=0; limit=-1;

	Arguments(argc, argv); secs=0; limit=0;

	if((port=FindPort(mannam))) quitS("AMan already running!\n");
	if(!(port=CreatePort(mannam,0L))) ERROR(NoPORT);
	if(!(trport=(struct MsgPort *)CreatePort(0L,0L))) ERROR(NoPORT);
	if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)&ResReq,0L))
		quitS("Can't open timer.device!\n");
	ResReq.tr_node.io_Message.mn_ReplyPort=trport; timeopen=TRUE;
	SwapGames(); setup();

	if(!quiet) printf("\n[ %s %s ]\n",vername,"LOADED");
	while(Kernel());
	
	if(!quiet) printf("\n[ %s %s ]\n",vername,"KILLED");
	quit();
}

Arguments(register int argc,char *argv[]) {
	register int i; register char *arg;
	char	buffer[128];

	for( i=1 ; i<argc ; ) {
		if(*(arg=argv[i++])!='-' || !isalpha(*(arg+1))) {
			printf("Invalid argument \"%s\".\n",arg); Usage();
		}
		strcpy(buffer,arg+2);	// Allow, say, -r5 or -padv:game/
		arg+2=0;
		while(i<argc && *argv[i]!='-') {
			if(buffer[0]) strcat(buffer," ");
			strcat(buffer,argv[i++]);
		};
		if(!ArgProc(arg,(buffer[0])?buffer:NULL)) {
			printf("Invalid parameters after argument %s\n",arg);
			quit();
		}
	}
	if(!dir[0]) strcpy(dir,DEFAGLPATH);
	if(!nxtdir[0]) strcpy(nxtdir,dir);
}

ArgProc(char *arg,char *parms) {
	switch(toupper(*(arg+1)) {
		// -? HELP
		case '?':	Usage();	// Includes "quit" command
		// -L IMPOSE SESSION TIME LIMIT
		case 'L':	if((limit=Number(parms))<15) return FALSE;
				break;
		// -M IMPOSE USER TIME LIMIT
		case 'M':	maxtime=Number(parms);
				if(maxtime && maxtime!=-1 && maxtime<10) {
					if(!quiet) printf("Invalid user time limit ignored.\n");
					maxtime=0;
				}
				break;
		// -P PATH SPECIFY
		case 'P':	if(!parms) return FALSE;
				SetPath(dir,parms); break;
		// -Q QUIET MODE
		case 'Q':	quiet=TRUE; break;
		// -T TOGGLE PATH (Toggles between -P and -T paths)
		case 'T':	if(!parms) return FALSE;
				SetPath(block,parms);
				if(!(DoSwap(block))) return FALSE;
				toggle=TRUE; break;
		default:	printf("Unknown option "%s"\n",arg); Usage();
	}
	return TRUE;
}

Kernel() {
	register int i;	register FILE *fp;

	online=reset=forcereset=FALSE; timefactor=DELFAC; DInit();

	for(i=0; i<MAXNODE; i++) {
		register struct LS *l;
		(l=lstat+i)->m_left=l->m_used=0;
		l->IOlock=-1; l->room=bid[i]=-1; busy[i]=FALSE;
		l->helping=-1; l->following=-1;
	}
	LogEntry("##",PRIV,">>> STARTED: %s <<<",adname);

	// Activate Daemon/Mobile processor
	sprintf(block,"run >NIL: <NIL: agl -%c",3);
	Execute(block,0L,0L);			// activate it!
	mobln=MAXU;				// mobile line no.

	AskTimer(); tick=59;

	while(!reset) {
		Wait(-1);		// Wait for any event
readport:	while(GetMsg((struct MsgPort *)trport))
			dotimer();
		if(!(agl=(struct Aport *)GetMsg((struct MsgPort *)port)))
			continue;
		switch(At) {
			case MTIMER:	dotimer(); break;
			case MKILL:	kill(); break;
			case MCNCT:	cnct(); break;
			case MDISCNCT:	discnct(); break;
			case MDATAREQ:	data();	break;
			case MLOGGED:	login(); break;
			case MRESET:	rest(); break;
			case MLOCK:	lock(); break;
			case MBUSY:	busy[Af]=1; break;
			case MFREE:	busy[Af]=0; break;
			case MDSTART:	DStart(Af); break;
			case MDCANCEL:	DKill(Ad); break;
			case MCHECKD:	DCheck(Ad); break;
			case MMADEWIZ:	logwiz(Af); break;
			case MLOG:	Log("@@",Af,Ap); break;
			case MEXTEND:	extend(Ad); forcereset=0; break;
			case MGDSTART:	DStart(MAXU); break;
			case MSWAP:	SwapRec(); break;
			case MPROVOKE:	provoke(); break;
			case MTLIMIT:	if(maxtime==Ad) break;
					LogEntry("**",Af,"User time limit changed to %ld",maxtime=Ad);
					break;
			default:	At=-1;
					LogEntry("**",PRIV,"*Invalid message type %ld",At);
		}
		ReplyMsg((struct Message *)agl); if(!reset) goto readport;
	}
	AbortIO(&ResReq.tr_node);
	while((agl=(struct Aport*)GetMsg((struct MsgPort *) port))) { Ad=At=-'R'; ReplyMsg((struct Message *)agl); }

	if(reset) {	// Asked for a reset?
		res(); givebackmemory(); setup();
		if(!quiet) printf("\n[ %s %s ]\n",vername,"RESET");
		if((fp=fopen("reset.bat","rb"))) {
			fclose(fp); Execute("execute reset.bat",0L,0L);
		}
		online=reset=FALSE;
	}
	return shutdn;
}

dotimer() {	// Process counter table
	register int i;
	register _DAEMON *dp;

	if(--secs<=0) {
		secs=-10; reset=TRUE;
		if(!forcereset && !quiet) printf("[ Automatic Reset ]\n");
		return;
	}

	if((dp=daemon)) do {
		if((--dp->count)<=0) {
			// Mobile/daemons when it's not loaded
			if(dp->own>=MAXU && (lstat+dp->own)->state<PLAYING) {
				dp->count+=60; continue;
			}
			setam();
			am->data=dp->num; am->p1=dp->val[0]; am->p2=dp->val[1];
			am->p3=dp->typ[0]; am->p4=dp->typ[1];
			am->type=MDAEMON; PutMsg((lstat+dp->own)->rep,am);
			DRemove(dp);
		}
	} while((dp=dp->nxt));

	if(secs>15 && mobact) {
		register int j;
		mobtab=mtab; j=0;
		for(i=0; i<mobs; i++,mobtab++) {
			if(mobtab->count<0) continue;
			if(!(mobtab->count--)) mobsig(j,0);
			j+=3; if(j>mobs) j=1;
		}
	}
	if(secs<301) switch(secs) {
		case 300: warn("* 5 minutes to next reset *\n"); break;
		case 120: warn("* 120 seconds until next reset *\n"); break;
		case 60:  warn("* FINAL WARNING - 60 SECONDS TO RESET *\n"); break;
		case 30:
			if(stricmp(nxtdir,dir)) {
				warn(block,"* A different game will be loaded after reset! *\n",nxtgame);
			}
	}
	if(!(--tick)) {			// Do every minute
		register struct LS *l;
		tick=59;		// Reset counter
		for(i=0,l=lstat; i<MAXU; i++,l++)
			if(l->state>=PLAYING) tickmin(i,l);
	}
	AskTimer();			// Setup next timer request
}

tickmin(int i,register struct LS *l) {	// Knock a minute of players time
	regiter char	*p;

	p=NULL; Forbid();
	l->m_used++;
	// Ensure their time is limited
	if((l=lstat+i)->m_left) {
		if((--(l->m_left))==300) {
			p="* You only have 5 minutes left.\n";
		}
		if(l->m_left==60) {
			p="* You have exactly 1 minute left.\n";
		}
		if(!l->m_left) {
			p="* Time limit exceeded. Disconnecting.\n";
		}
	}
	Permit();
	if(block[0] && (l->m_left*60) > (secs+59)) {
		if(!l->m_left) {	// Disconnect user
			setam(); am->type=MTLIMIT;
			am->ptr="\n\n* Time limit exeeded - disconnecting.\n";
			PutMsg((lstat+i)->rep,am);
		}
		else warna(block,i);
	}
}

kill() {		// Shutdown receiver
	reset_users("SHUTDOWN");	// To ensure users etc are flushed
	Ad=At='O'; reset=TRUE;
	Log(">>",PRIV,"|||  GAME  OVER!  |||");
}

cnct() {		// User connection!
	register int i;

	Ad = (long)lstat; Ap = (char *)usr;
	if(Af >= MAXU) {
		if((lstat+Af)->state) Af=-1; else (lstat+Af)->state=PLAYING;
		if(Af>=MAXU && mobs>0) mobact=TRUE;
		return;
	}
	Af=-1;
	for(i=0;i<MAXU;i++) {	// Allow for daemons & mobiles
		if((lstat+i)->state) continue;
		Af=i; (lstat+i)->state=LOGGING; online++; calls++;
		timefactor=1; return;
	}
	Log("##",PRIV,"*Connect request denied - game full!");
}

discnct() {		// User disconnection
	if(Af < MAXU && (lstat+Af)->state>=PLAYING) {
		LogEntry("<-",Af,"\"%s\" disconnected.",(usr+Af)->name);
	}
	if(Af < MAXU) online--;
	(usr+Af)->name[0]=0; (lstat+Af)->room=-1; (lstat+Af)->helping=-1;
	(lstat+Af)->m_left=(lstat+Af)->m_used=0;
	(lstat+Af)->following=-1; DKill(-1); (lstat+Af)->state=0; Af=-1; Ad=-1;
}

data() {		// Sends pointers to database
	At=MDATAREQ;
	switch(Ad) {
		case -1: Ad=online; Ap=(char *)usr; Ap1=calls; Ap2=(long) vername; Ap3=(long) adname; Ap4=(long) lstat; break;
		case  0: strcpy(Ap,dir); Ap1=(long)&secs; Ap2=(long)lastres; Ap3=(long)lastcrt; Ap4=rststamp; break;
		case  1: Ad=rooms; Ap=(char *)rmtab; Ap1=(long)errtxt; break;
		case  2: Ad=ranks; Ap=(char *)rktab; break;
		case  3: Ad=nouns; Ap=(char *)obtab; Ap1=(long) ormtab; Ap2=(long)statab; Ap3=(long)adtab; Ap4=(long)desctab; break;
		case  4: Ad=verbs; Ap=(char *)vbtab; Ap1=(long)vtp; Ap2=(long)vtpp; break;
		case  5: Ap=(char *)ttp; Ap1=(long)ttpp; break;
		case  6: Ap=(char *)umsgip; Ap1=(long)umsgp; break;
		case  7: Ap=(char *)rctab; break;
		case  8: Ap=(char *)slottab; break;
		case  9: Ap=(char *)synp; Ap1=(long)synip; break;
		case 10: Ap=(char *)mobp; Ap1=(long)mtab; break;
		default: Ap=(char *)-1;
	}
}

login() {		// Receive & log a player login
	LogEntry("->",Af,"\"%s\" logged in.",(usr+Af)->name);
	(lstat+Af)->state=PLAYING;
	(lstat+Af)->m_left=maxtime; (lstat+Af)->m_used=0;
}

DoSwap(char *s) {			// Set next game
	char new[128];	FILE *fp;
	sprintf(new,"%s%s",s,advfn); if(!(ifp=fopen(new,"rb"))) return FALSE;
	fgets(nxtgame,41,fp); new[strlen(nxtgame)-1]=0; fclose(fp);
	strcpy(nxtdir,s);
	return TRUE;
}

SwapRec() {
	char new[64];
	if(!(DoSwap(Ap))) { Ad=At=-S; return FALSE; }
	Ad=At='S';
	sprintf(block,"** NOTICE: Final session of \"%s\".\n** Next session will be \"%s\".\n",adname,nxtgame);
	warn(block); LogEntry(">>",PRIV,"Next session from \"%s\"",nxtdir);
}

rest() {			// RESeT in progress
	forcereset = 1;
	if(Ad > 0) {
		Ap1=Ad; secs=Ad+1;
		if(secs<301 && secs!=120 && secs!=60) sprintf(block, "* System reset invoked - %ld seconds remaining *\n",Ad);
		if(secs>300) sprintf(block, "* System reset now due in %ld minutes *\n",Ad/60);
		warn(block); Ad=At=-'X'; return;
	}
	Ad=At='R'; secs=1;
}

extend(int tics) {		// Extend by this many ticks
	register short int newtime;

	Ad=At='U'; if(!tics) return;

	newtime = secs+tics+1;
	if(secs>120) sprintf(block,"* Game time extended - reset now due in %ld %s and %ld %s *\n",newtime/60,"minutes",newtime-((newtime/60)*60),"seconds");
	else sprintf(block,"* Reset postponed - now due in %ld %s *\n",newtime,"seconds");
	warn(block); Ap1=tics; secs=newtime; Ad='E';
}

res() {				// Reset <receiver>
	reset_users("RESET");
	Log("<<",Priv,"Reset completed.");
	Delay(60);	// Always wait atleast a few seconds
}

reset_users(char *func) {	// Force users to log-out & kill extra lines
	register int i; struct Message *xx;

	sprintf(block,(online)
			?"%s! Disconnecting %ld %s.\n"
			:"%s! No users online.\n",
			func,online,(online==1)?"user":"users");
	Log(">>",Af,block);	online=0;			// Allow for daemons & mobiles
	for(i=0; i<MAXNODE; i++) {
		if((lstat+i)->state<=0) continue; online++;
		setam(); am->type=MCLOSEING; am->msg.mn_ReplyPort = port;
		PutMsg((lstat+i)->rep,am);
	}
	while(online) {
		WaitPort(port); am=(struct Aport *)GetMsg(port);
loop:		if(am->from!=-'O') {
			printf("\x07 !! Invalid message!\n");
			am->type=am->data=-'R'; ReplyMsg((struct Message *)am);
			goto skip;
		}
		FreeMem((char *)am,amsz); online--;
skip:		if((am=(struct Aport *)GetMsg(port))) goto loop;
	}
	while((xx=GetMsg(port))) ReplyMsg(xx);
	while(GetMsg((struct MsgPort *)trport));
	online=0;
}

SwapGames() {
	char tempdir[128];
	strcpy(tempdir,(toggle)?dir:nxtdir);
	strcpy(dir,nxtdir); strcpy(nxtdir,tempdir);
	if(toggle) strcpy(nxtgame,adname);
}

setup() {			// Read in & evaluate data files
	register long i,j,k,l; long act; long *pt; register char *p;

	SwapGames();
	fopenr(advfn);
	fgets(adname,41,ifp); adname[strlen(adname)-1]=0;
	rooms=nextno(); ranks=nextno(); verbs=nextno(); syns=nextno();
	nouns=nextno(); adjs=nextno(); ttents=nextno(); umsgs=nextno();
	cclock=nextno(); secs=nextno()*60; invis=nextno(); invis2=nextno();
	minsgo=nextno(); mobs=nextno(); rscale=nextno(); tscale=nextno();
	mobchars=nextno();
	if(limit!=-1) secs=limit*60;
	fgets(block,1024,ifp); 
	sscanf(block,"%ld %ld %ld %ld %c\n",&scrx,&scry,&scrw,&scrh,&scrm);
	fgets(logname,41,ifp); logname[strlen(logname)-1]=0;
	if(!(p=AllocMem(UINFO,STDMEM))) memfail("data tables");
	usr=(struct _PLAYER *)p;	p+=sizeof(*usr)*MAXNODE;
	lstat=(struct LS *)p;		p+=sizeof(*lstat)*MAXNODE;
	rctab=(short *)p;		p+=(rooms*2);
	rmtab=readf(rooms1fn, p);	p+=(rooms*sizeof(*rmtab));

	fopenr(ranksfn);		// 1: Player ranks
	if(!(rktab=(struct rank *)AllocMem(ranks*sizeof(*rktab),MEMF_PUBLIC)))
		memfail("player ranks"); // Allocate memory
	if((i=fread((char *)rktab,sizeof(*rktab),ranks,ifp))!=ranks)
		readfail("player ranks",i,ranks);

	fopenr(lang1fn);		// 2: Verbs
	if(!(vbtab=(struct verb *)AllocMem(verbs*sizeof(*vbtab),MEMF_PUBLIC)))
		memfail("verb table");
	if((i=fread(vbtab->id,sizeof(*vbtab),verbs,ifp))!=verbs)
		readfail("verb table",i,verbs);

					// 3: Read objects
	obtlen=fsize(objsfn); desctlen=fsize(obdsfn); ormtablen=fsize(objrmsfn); statablen=fsize(statfn);
	if(!(p=AllocMem(obtlen+desctlen+ormtablen+statablen,MEMF_PUBLIC))) memfail("object data");
	obtab   = (struct obj   *)readf(objsfn, p);
	desctab = (      char   *)readf(obdsfn, (p=p+obtlen));
	ormtab  = (      long    )readf(objrmsfn, (p=p+desctlen));
	statab  = (struct state *)readf(statfn, p+ormtablen);

	// Update object room-list & state pointers
	statep=statab;
	for(i=0,j=0;i<nouns;i++) {
		objtab=obtab+i;
		objtab->rmlist=(long *)(ormtab+j); j+=objtab->nrooms*sizeof(long);
		objtab->states=statep; statep=statep+(long)objtab->nstates;
	}

					// 4: Text
	umsgil=fsize(umsgifn); umsgl=fsize(umsgfn);
	if(!(p=AllocMem(umsgil+umsgl,MEMF_PUBLIC))) memfail("user messages");
	umsgip=(long *)readf( umsgifn, p );
	umsgp =(char *)readf( umsgfn,  p+umsgil );

					// 5: Travel
	ttlen=fsize(ttfn); ttplen=fsize(ttpfn);
	if(!(p=AllocMem(ttlen+ttplen,MEMF_PUBLIC))) memfail("travel table");
	ttp =(struct tt *)readf( ttfn,  p );
	ttpp=(     long *)readf( ttpfn, p+ttlen );
	ttents=ttlen/sizeof(tt); ttabp=ttp; pt=ttpp;
	for(i=0;i<ttents;i++) {
		ttabp=ttp+i; k=(long)ttabp->pptr; ttabp->pptr=pt;
		if(k==-2) continue;
		act=ttabp->condition; if(act<0) act=-1-act; pt+=ncop[act];
		act=ttabp->action;  if(act<0) { act=-1-act; pt+=nacp[act]; }
	}

					// 6: Slot tables
	stlen=fsize(lang2fn); vtlen=fsize(lang3fn); vtplen=fsize(lang4fn);
	if(!(p=AllocMem(stlen+vtlen+vtplen,MEMF_PUBLIC))) memfail("language data");
	slottab=(struct _SLOTTAB *)readf( lang2fn, p );
	vtp    =(struct _VBTAB   *)readf( lang3fn, p+stlen );
	vtpp   =(        long    *)readf( lang4fn, p+stlen+vtlen );

					// 7: Syns & Adjs
	synlen=fsize(synsfn); synilen=fsize(synsifn); adtablen=fsize(adjfn);
	if(!(p=AllocMem(synlen+synilen+adtablen,MEMF_PUBLIC))) memfail("synonym data");
	synp =(     char *)readf( synsfn, p );
	synip=(short int *)readf( synsifn, (p=p+synlen) );
	adtab=(     char *)readf( adjfn, p+synilen );

					// 8: Times!
	strcpy(lastres,now()); rststamp=clock; clock=cclock; strcpy(lastcrt,now());

	// Adjust verb-related pointers
	vbptr=vbtab; stptr=slottab; vtabp=vtp; l=0;
	for(i=0; i<verbs; i++,vbptr++) {
		vbptr->ptr=stptr;
		for(j=0; j<vbptr->ents; j++,stptr++) {
			stptr->ptr=vtabp;
			for(k=0; k<stptr->ents; k++,vtabp++) {
				vtabp->pptr=vtpp+l;
				act=vtabp->condition; if(act<0) act=-1-act;
				l+=ncop[act];		// ^^ allow for NOT
				act=vtabp->action;  if(act<0) { act=-1-act; l+=nacp[act]; }
			}
		}
	}
					// 9: Mobiles
	l=mobmem=sizeof(*mtab)*(mobs+1); mobmem+=fsize(mobfn);
	if(!(p=AllocMem(mobmem,MEMF_PUBLIC))) memfail("mobile data");
	mtab=(struct _MOB_TAB *)p;
	mobp=(struct _MOB_ENT *)readf( mobfn, (char *) p+l );

	objtab=obtab; mobtab=mtab; l=1;	// Fix object "inside" flags
	for(i=0; i<nouns; i++,objtab++) {
		if(*(objtab->rmlist)<=-INS) {
			register struct _OBJ *op;
			(op=obtab+(-(INS+*(objtab->rmlist))))->inside++;
			op->winside+=STATE->weight;
		}
		if(objtab->mobile!=-1) {
			mobtab->obj = i; mobile=mobp+objtab->mobile;
			mobtab->count=(mobtab->speed=mobile->speed)+(l++);
			mobtab->pflags = NULL; mobtab++;
		}
	}
	for(i=0, mobile=mobp; i<mobchars; i++, mobile++) {
		mobile->verb=-2;
	}
	if(ifp) fclose(ifp); ifp=NULL; mobact=FALSE;
}

memfail(char *s) {
	LogEntry("**",PRIV,"*Can't allocate memory for %s!",s); quit();
}

readfail(char *s,int got,int wanted) {
	printf("** Error: Expected %d %s entries, only got %d!\n",wanted,s,got);
	quit();
}

// Attempt to release all of memory in one chunk
MemRelease(char *b,long l) {
	if((Mbase+Mlen)==b) Mlen+=l;
	else if((Mbase-l))==b) {
		Mbase=b; Mlen+=l;
	} else {
		FreeMem(Mbase,Mlen); Mbase=b; Mlen=l;
	}
}

#define	MemRel(x,y) MemRelease((char *)(x),(long)(y)); x=NULL;

givebackmemory() {
	Mbase=usr; Mlen=UINFO;
	MemRel(rktab,ranks*sizeof(*rktab));
	MemRel(vbtab,verbs*sizeof(*vbtab));
	MemRel(obtab,obtlen);
	MemRel(desctab,destlen);
	MemRel(ormtab,ormtablen);
	MemRel(statab,statablen);
	MemRel(umsgip,umsgil);
	MemRel(umsgp,umsgl);
	MemRel(ttp,ttlen);
	MemRel(ttpp,ttplen);
	MemRel(slottab,stlen);
	MemRel(vtp,vtlen);
	MemRel(vtpp,vtplen);
	MemRel(synp,synlen);
	MemRel(synip,synilen);
	MemRel(adtab,adtablen);
	MemRel(mtab,mobmem);
	MemoryRelease(NULL,NULL);
}

fopenr(char *s) {
	if(ifp) fclose(ifp);
	sprintf(block,"%s%s",dir,s);
	if(!(ifp=fopen(block,"rb"))) oer(block,"read");
}

oer(char *s,char *t) {		// Open ERror
	printf("\x07** Error: Can't open file %s for %sing!\n\n",s,t);
	quit();
}

quitS(char *s) {
	printf(s); quit();
}

quit() {			// Exit tidily
	if(ifp)		fclose(ifp); ifp=NULL;
	if(port)	DeleteAPort(port,0L);
	if(timeropen) {
		AbortIO((struct IORequest *)&ResReq);
		CloseDevice((struct IORequest *)&ResReq);
	}
	if(trport)	DeleteAPort(trport,0L);
	if(AGLBase)	CloseLibrary((struct Library *)AGLBase); AGLBase=NULL;
	givebackmemory(); exit(0);
}

lock() {				// Lock user IO
	bid[Af]=Ad;
	if((lstat+Ad)->IOlock!=-1 || (busy[Ad] && Ad!=Af && bid[Ad]!=Af)) { Ad=-1; return;}
	(lstat+Ad)->IOlock=Af; bid[Af]=-1;
}

Log(char *f,char l,char *s) {		// Write string to log file
	WriteLog(f,l,s,NULL);
}

LogEntry(char *f,char l,char *s,char *m) {	// Write stringS to log file
	WriteLog(f,l,s,m);
}

WriteLog(char *f,char l,char *s,char *m) {
	register FILE *fp;

	l=Lnos[l+1];
	if((fp=fopen(logname,"ab+"))) {
		fseek((FILE *)fp,0,2L);
		fprintf((FILE *)fp,"%s (%c) %s: ",f,l,now());
		fprintf((FILE *)fp,s,m); fputc('\n'(FILE *)fp);
		fclose((FILE *)fp); return;
	}
	else if(!quiet) printf("\n\x07*CAN'T WRITE TO %s!*\n",logname);
}

DInit() {			// Initialise daemons
	register _DAEMON *dp;

	daemon=NULL; dp=&daem[MAXD-1]; dp->prv=dp-1; (dp--)->nxt=NULL;
	for( ; dp>&daem[0] ; dp-- ) { dp->prv=dp-1; dp->nxt=dp+1; }
	dp->prv=NULL; dp->nxt=dp+1; daemons=0; free=dp;
}

DRemove(register _DAEMON *dp) {	// Remove daemon from the list
	if(dp==daemon) daemon=dp->nxt;	// Removing top of the list
	// unlink daemon from chain
	if(dp->nxt) dp->nxt->prv=dp->prv;
	if(dp->prv) dp->prv->nxt=dp->nxt;
	// Place it in the "free" list
	dp->nxt=free; dp->prv=NULL; free->prv=dp; free=dp;
	daemons--;
}

DKill(short int d) {		// Cancel daemon
	register _DAEMON *dp;

	if(!(dp=daemon)) return;	// No daemons
	if(d==-1) {			// Kill all daemons on line Af
		do {
			if(dp->own==Af) DRemove(dp);
		} while((dp=dp->nxt));	// Until end of chain
	} else {
		do {
			if(dp->num==d && (dp->own>=MAXU || dp->own==Af))
				DRemove(dp);
		} while((dp=dp->nxt));
	}
}

// Ad=#, p1=inoun1, p2=inoun2, p3=wtype[2], p4=wtype[5], Ap=count //
DStart(char owner) {		// Initiate daemon
	register _DAEMON *dp;

	// Take the first free daemon OFF the free chain
	dp=free; free=dp->nxt; free->prv=NULL;
	// Put it onto the "in-use" list
	if((dp->nxt=daemon)) daemon->prv=dp;	// Make sure daemon <> 0
	daemon=dp; daemons++;
	dp->val[0]=Ap1; dp->val[1]=Ap2; dp->typ[0]=Ap3; dp->typ[1]=Ap4;
	dp->own=owner; dp->count=(short int)Ap; dp->num=Ad;
}

DCheck(int d) {			// Check if daemon active
	register _DAEMON *dp;
	if(!(dp=daemon)) return;
	Ad=-1; Ap1=-1;
	do {
		if(dp->num==d && (dp->own==Af || dp->own>=MAXU)) {
			Ad=dp; Ap1=dp->count; break;
		}
	} while((dp=dp->nxt));
}

setam(){			// Get message with replies to TrashPot
	CommsPrep(0L,0L,&am); am->from = -1;
}

logwiz(int who) {
	sprintf(block,"User \"%s\" reached top rank (%ld)!\n",
		(usr+Af)->name,(usr+Af)->rank+1);
	Log("[]",Af,block);
}

warn(char *s) {
	int i;
	for(i=0; i<MAXU; i++)
		if((lstat+i)->state != OFFLINE) warna(s,i);
}

warna(char *s,int i) {
	setam(); am->ptr=s; am->type=MRWARN;
	PutMsg((lstat+i)->rep,am);
}

provoke() {
	mobtab=mtab+Ad;
	if(mobtab->count < 0) return;
	mobsig(mobtab->speed/5,(long)Ap);
}

mobsig(int j,long k) {
	setam(); am->type=MMOBILE; am->data=mobtab->obj;
	am->p1=mobtab->pflags; am->p2=k;
	PutMsg((lstat+mobln)->rep,am); mobtab->count=mobtab->speed+j;
}

AskTimer() {			// Schedule a timer request
	ResReq.tr_time.tv_secs=timefactor; ResReq.tr_time.tv_micro=XR;
	ResReq.tr_node.io_Command=TR_ADDREQUEST; SendIO(&ResReq.tr_node);
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

Number(char *s) {
	if(!s || !*s) return FALSE;
	return atoi(s);
}
