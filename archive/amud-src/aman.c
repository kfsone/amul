//
// Current Job:
//	Using new daemon structure instead of old variables
//

//
//                AMan.C - AMUD Multi-User Manager/Controller
//
//  Copyright (C) Oliver Smith, 1991-2. Copyright (C) Kingfisher s/w 1991-2
//         Program Designed, Developed and Written By: Oliver Smith
//

#define MAXD	240			// Maximum daemons allowed
#define	XR	16667			// To do with timer.device

#define	AMAN  1				// Use AMAN only includes
#define	PORTS 1				// Use port-definition includes
#define TRQ timerequest			// abbreviation
#define	DELFAC	4			// Slow-time at session start
#define	PRIV	-1			// Local/private log entry

#define	UINFO	((sizeof(*usr)+sizeof(*lstat))*MAXNODE)+(rooms*(2+sizeof(*rmtab)))

#include <adv:h/amud.defs.h>
#include <adv:h/amud.incs.h>
#include <adv:h/amud.vars.h>
#include <adv:h/amud.cons.h>
#include <devices/timer.h>
#include <time.h>
#include <proto/timer.h>
#include <intuition/intuition.h>

char	*ttxt,**errtxt;
char	lastres[24],lastcrt[24],bid[MAXNODE],busy[MAXNODE],*xread(),*now();
char	timefactor;			// Time factor
char	resety,forcereset,quiet;
char	nxtdir[64];			// Path for next game
char	nxtgame[42],mobact;		// MobAct: TRUE if mobiles thawed
long	reslp,TDBase,daemons,mins;	// Daemons to process!
struct	Task *mytask,*FindTask();
struct Aport	*am;
struct TRQ	ResReq;			// Timer request
struct MsgPort	*trport;		// Timer port
long	calls;				// no. of calls
long	obtlen,mobmem;			// Memory allocations
SHORT	mobln;				// Which line are mobiles on?
long	amsz;

// Daemon information:
_DAEMON	daem[MAXD],*free,*daemon;	// Perhaps shouldn't make this inbuilt
short	Count;				// Reset count

long	online,clock,cclock,rststamp;

long	scrx,scry,scrw,scrh;		// Screen settings
char	scrm;

struct Library	*AMUDBase;		// For AMUD.library
char	*mannam="AMan Port";
extern char *MyUsage;			// see AManX.I

char	Lnos[]="#0123456789ABCD*M";	// Local, 0-14, Daemon, Mobile lines


int CXBRK(){			// Prevent CTRL-C'ing
	return 0;
}

char *now(){			// Get current time/date
	if(!clock) time(&clock);
	ttxt=(char *)ctime(&clock)+4; clock=0; *(ttxt+strlen(ttxt)-1)=0;
	return ttxt;
}

char *readf(char *s, char *p){
	fopenr(s); fread(p,32767,32767,ifp); fclose(ifp); ifp=NULL; return p;
}

fsize(char *s){
	register int n;
	fopenr(s); fseek(ifp,0,2L); n=ftell(ifp);
	fclose(ifp); ifp=NULL; return ((n+2)&-2);
}

void getlibs(){
	register ULONG *base;
	if(!(AMUDBase=OpenLibrary("amud.library",0L))) {
		printf("\nno amud.library\n"); quit();
	}
	base=(ULONG *)LibTable(); errtxt=TextPtr();
	IntuitionBase=(struct Library *)*(base+INTBoff);
	GfxBase=(struct Library *)*(base+GFXBoff);
}

long nextno() { long x; fread(ifp,4,1,x); return x; }

Error(long x) { if(x>512) printf((char *)x); else printf(*(errtxt+x)); }

ERROR(long x) { Error(x); quit(); }

                         // >>>> MAIN ROUTINE <<<< //

main(int argc,char *argv[]){
	amsz=sizeof(*amud);		// Size of AMUD structure
	getlibs();			// Get amud.library etc.

	setvername(vername); (mytask=FindTask(0L))->tc_Node.ln_Name = vername;

	if(argc==1){
		printf("\x07** No source path specified!\n"); quit();
	}
	if(argc>4){
		printf("Invalid arguments!\n"); quit();
	}

	if(argv[1][0]=='-' && toupper(argv[1][1])!='Q') {
		mins=0; if(argc==3) sscanf(argv[2],"%ld",&mins);
		Count=mins;
		switch(toupper(*(argv[1]+1))) {
			case '?':	printf(MyUsage,vername); quit();
			case 'K':	shutreq(0); 
			case 'R':	shutreq(1);
			case 'X':	if(argc != 3) printf("** Missing parameter after -%c option.\n",'x');
					sendext(Count);
			case 'S':	swapreq(argv[1]+2);
			default:	printf("\x07** Invalid command line! Type aman -? for usage.\n");
					quit();
		}
	}
	quiet=0;

	if(argc!=1){
		if(!stricmp(argv[1],"-q")) {
			quiet=1; dir[0]=0; if(argc>2) strcpy(dir,argv[2]);
		}
		else strcpy(dir,argv[1]);
		if(dir[strlen(dir)-1]!='/' && dir[strlen(dir)-1]!=':')
			strcat(dir,"/");
	}
	else dir[0]=0; strcpy(nxtdir,dir);	// Copy the directory

	if((port=FindPort(mannam))) {
		printf("AMan already running!\n"); quit();
	}
	if(!(port=CreatePort(mannam,0L))) ERROR(NoPORT);
	if(!(trport=(struct MsgPort *)CreatePort(0L,0L))) ERROR(NoPORT);
	if(OpenDevice(TIMERNAME,UNIT_VBLANK,(struct IORequest *)&ResReq,0L))
		{ printf("Can't open timer.device!\n"); quit(); }
	ResReq.tr_node.io_Message.mn_ReplyPort=trport; TDBase=1;
	setup();

	if(!quiet) printf("\n[ %s %s ]\n",vername,"LOADED");
	do {
		kernel();
	} while(resety!=-1);
	
	if(!quiet) printf("\n[ %s %s ]\n",vername,"KILLED");
	quit();
}

kernel() {
	register int i;	register FILE *fp;

	online=resety=forcereset=0; timefactor=DELFAC; DInit();
	for(i=0; i<MAXNODE; i++) {
		(lstat+i)->IOlock=-1; (lstat+i)->room=bid[i]=-1; busy[i]=0;
		(lstat+i)->helping=(lstat+i)->following=-1;
	}
	LogEntry("##",PRIV,">>> Start Run \"%s\"",adname);
	AskTimer(); forcereset;

	// Activate daemon/mobile processing line
	sprintf(block,"run >NIL: <NIL: amud -%c",3);
	Execute(block,0L,0L);			// activate it!
	mobln=MAXU;				// mobile line no.

	while(!resety) {
		Wait(-1);
readport:	while(GetMsg((struct MsgPort *)trport))
			dotimer();
		if(!(amud=(struct Aport *)GetMsg((struct MsgPort *)port)))
			continue;
		switch(At) {
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
			case MSWAP:	swaprec(); break;
			case MPROVOKE:	provoke(); break;
			default: At=-1;
				LogEntry("**",PRIV,"*INVALID MESSAGE TYPE %ld",At);
				break;
		}
		ReplyMsg((struct Message *)amud); if(!resety) goto readport;
	}
	while((amud=(struct Aport*)GetMsg((struct MsgPort *) port))) {
		Ad=At=-'R'; ReplyMsg((struct Message *)amud);
	}
	AbortIO(&ResReq.tr_node);	// Cancel timer.device requests

	if(resety == 1) {	// Asked for a reset?
		res(); givebackmemory(); setup();
		if(!quiet) printf("\n[ %s %s ]\n",vername,"RESET");
		if((fp=fopen("reset.bat","rb"))) {
			fclose(fp); Execute("execute reset.bat",0L,0L);
		}
		online=resety=0;
	}
	else resety = -1;
}

dotimer(){	// Process daemon table
	register int i;
	register _DAEMON *dp;

	if(--Count<=0) {
		Count=-10; resety=1;
		if(!forcereset && !quiet) printf("[ Automatic Reset ]\n");
		return;
	}
	
	if((dp=daemon)) do {
		if((--dp->count)<=0) {
			// Mobile/daemons when it's not loaded.
			if(dp->own>=MAXU && (lstat+own[i])->state<PLAYING) {
				dp->count+=60; continue;
			}
			// Standard user daemon
			setam();
			am->data=dp->num; am->p1=dp->val[0]; am->p2=dp->val[1];
			am->p3=dp->typ[0]; am->p4=dp->typ[1];
			am->type=MDAEMON; PutMsg((lstat+dp->own)->rep,am);
			DRemove(dp);
		}
	} while((dp=dp->nxt));

	if(Count>20 && mobact==TRUE) {
		register int j;
		mobtab=mtab; j=0;
		for(i=0; i<mobs; i++,mobtab++) {
			if(mobtab->count<0) continue;
			if(!(mobtab->count--)) mobsig(j,0);
			j+=3; if(j>mobs) j=1;
		}
	}
	if(Count<301) switch(Count) {
		case 300:
			if(!stricmp(nxtdir,dir))
				warn("* 5 minutes to next reset *\n");
			else {
				sprintf(block,"* NOTICE: Swapping to \"%s\" in 5 minutes *\n",nxtgame);
				warn(block);
			};
			break;
		case 120: warn("* 120 seconds until next reset *\n"); break;
		case 60:
			if(!stricmp(nxtdir,dir))
				warn("* FINAL WARNING - 60 SECONDS TO RESET *\n");
			else warn("\x07* FINAL WARNING! RESET AND SWAP IN 60 SECONDS! *\n");
			break;
		case 10:
			if(stricmp(nxtdir,dir)) {
				sprintf(block,
					"* \"%s\" will be loaded after reset *\n",nxtgame);
				warn(block);
			}
			break;
	}
reschk:
	AskTimer();			// Setup next timer request
}

kill() {		// Shutdown receiver
	if(online) {
		LogEntry("**",PRIV,"Shutdown requested: %ld users online!",online);
		Ad=At='X';
	}
	else	{
		reset_users();		// To ensure Daemon flush
		Ad=At='O'; resety=-1; Log(">>",PRIV,"Shutting Down...");
	}
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
		timefactor=1; break;
	}
}

discnct() {		// User disconnection
	if(Af < MAXU && (lstat+Af)->state>=PLAYING) {
		LogEntry("<-",Af,"User %s disconnected.",(usr+Af)->name);
	}
	if(Af < MAXU) online--;
	(usr+Af)->name[0]=0; (lstat+Af)->room=-1; (lstat+Af)->helping=-1;
	(lstat+Af)->following=-1; DKill(-1); (lstat+Af)->state=0; Af=-1; Ad=-1;
}

data() {		// Sends pointers to database
	At=MDATAREQ;
	switch(Ad) {
		case -1: Ad=online; Ap=(char *)usr; Ap1=calls; Ap2=(long) vername; Ap3=(long) adname; Ap4=(long) lstat; break;
		case  0: strcpy(Ap,dir); Ap1=(long)&Count; Ap2=(long)lastres; Ap3=(long)lastcrt; Ap4=rststamp; break;
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
}

shutreq(int x) {
	asend((x)?MRESET:MKILL,Count); quit();
}

swapreq(char *s){		// REQUEST for swap
	if(!*s) exit(0*printf("** Missing parameter after -%c option.\n",'s'));
	strcpy(dir,s);
	if(dir[strlen(dir)-1]!='/' && dir[strlen(dir)-1]!=':') strcat(dir,"/");
	sprintf(nxtdir,"%s%s",dir,advfn);
	if(!(ifp=fopen(nxtdir,"rb")))
		exit(0*printf("** Can't open '%s' to swap.\n",nxtdir));
	fclose(ifp);
	asend( MSWAP, NULL, dir ); quit();
}

swaprec() {			// RECEIVE swap
	char new[64];
	Ad=At=-'S';			// Default to 'fail'
	if(ifp) fclose(ifp);
	sprintf(new,"%s%s",Ap,advfn); if(!(ifp=fopen(new,"rb"))) return;
	fgets(nxtgame,41,ifp); nxtgame[strlen(nxtgame)-1]=0; fclose(ifp); ifp=NULL;
	Ad=At='S';
	sprintf(block,"** NOTICE: Final session of \"%s\".\n** Next session will be \"%s\".\n",adname,nxtgame);
	strcpy(nxtdir,Ap); warn(block);
	LogEntry(">>",PRIV,"Next session \"%s\"",nxtgame);
}

sendext(int t) {
	asend( MEXTEND, t ); quit();
}

asend(int type, int data, char *s)	// Shutdown request
{	char *p;
	if((p=CommsPrep(&port,&reply,&amud))) {
		printf("Failed: %s",p); quit();
	}
	At = type; Ad = data; Af = -1; Ap = s;
	PutMsg(port,amud); WaitPort(reply); GetMsg((struct MsgPort*) reply);
	Delay(30);
	if(quiet) goto noreport;
	switch(Ad) {
		case 'R': printf("\x07Game resetting\n"); break;
		case 'O': printf("Manager removed!\n"); break;
		case 'X': printf("Cannot remove with users connected.\n"); break;
		case 'U': printf("AMan error at other end!\n");
		case -'X':printf("Reset set for %ld seconds\n",Ap1); break;
		case -'R':printf("Reset in progress\n"); break;
		case 'E': printf("Game extended by %ld seconds\n",Ap1); break;
		case 'S': printf("AMan will load from '%s' at next reset\n",s); break;
		case -'S': printf("Swap request failed\n"); break;
		default: printf("** Internal error ** (Returned '%c')\n",Ad); break;
	}
noreport:
	DeleteAPort(reply,amud); printf("\n");
}

rest() {			// RESeT in progress
	forcereset = 1;
	if(Ad > 0) {
		Ap1=Ad; Count=Ad+1;
		if(Count<301 && Count!=120 && Count!=60)
			sprintf(block,"* System reset invoked - %ld seconds remaining *\n",Ad);
		if(Count>300)
			sprintf(block,"* System reset now due in %ld minutes *\n",Ad/60);
		warn(block); Ad=At=-'X'; return;
	}
	Ad=At='R'; Count=1;
}

extend(int tics) {		// Extend by this many ticks
	register short int newtime;

	Ad=At='U'; if(!tics) return;
	newtime=Count+tics+1;
	if(Count>120)
		sprintf(block,"* Game time extended - reset now due in %ld %s and %ld %s *\n",
			newtime/60,"minutes",newtime-((newtime/60)*60),"seconds");
	else sprintf(block,"* Reset postponed - now due in %ld %s *\n",newtime,"seconds");
	warn(block); Ap1=tics; Count=newtime; Ad='E';
}

res() {				// Reset <receiver>
	sprintf(block,(online)
			?"RESET! Disconnecting %ld %s...\n"
			:"RESET! No users online.\n",
			online,(online==1)?"user":"users");
	Log(">>",Af,block); reset_users();
	Log("<<",PRIV,"Reset completed.");
	Delay(60);		// Always wait atleast a few seconds
}

reset_users() {		// Force users to log-out & kill extra lines
	register int i; struct Message *xx;

	online=0;			// Allow for daemons & mobiles
	for(i=0; i<MAXNODE; i++) {
		if((lstat+i)->state<=0) continue; online++;
		setam(); am->type=MCLOSEING; am->msg.mn_ReplyPort = port;
		PutMsg((lstat+i)->rep,am);
	}
	while(online>0) {
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

setup()	{			// Read in & evaluate data files
	register long i,j,k,l; long act; long *pt; register char *p;

	strcpy(dir,nxtdir);	// Switch to next path!
	fopenr(advfn);
	fgets(adname,41,ifp); adname[strlen(adname)-1]=0;
	rooms=nextno(); ranks=nextno(); verbs=nextno(); syns=nextno();
	nouns=nextno(); adjs=nextno(); ttents=nextno(); umsgs=nextno();
	cclock=nextno(); mins=nextno(); invis=nextno(); invis2=nextno();
	minsgo=nextno(); mobs=nextno(); rscale=nextno(); tscale=nextno();
	mobchars=nextno();
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
		objtab=obtab+i; objtab->rmlist=(long *)(ormtab+j);
		j+=objtab->nrooms*sizeof(long);
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
	if(!(p=AllocMem(stlen+vtlen+vtplen,MEMF_PUBLIC)))
		memfail("language data");
	slottab=(struct _SLOTTAB *)readf( lang2fn, p );
	vtp    =(struct _VBTAB   *)readf( lang3fn, p+stlen );
	vtpp   =(        long    *)readf( lang4fn, p+stlen+vtlen );

					// 7: Syns & Adjs
	synlen=fsize(synsfn); synilen=fsize(synsifn); adtablen=fsize(adjfn);
	if(!(p=AllocMem(synlen+synilen+adtablen,MEMF_PUBLIC)))
		memfail("synonym data");
	synp =(     char *)readf( synsfn, p );
	synip=(short int *)readf( synsifn, (p=p+synlen) );
	adtab=(     char *)readf( adjfn, p+synilen );

					// 8: Times!
	strcpy(lastres,now());
	rststamp=clock; clock=cclock; strcpy(lastcrt,now());

	// Adjust verb-related pointers
	vbptr=vbtab; stptr=slottab; vtabp=vtp; l=0;
	for(i=0; i<verbs; i++,vbptr++) {
		vbptr->ptr=stptr;
		for(j=0; j<vbptr->ents; j++,stptr++) {
			stptr->ptr=vtabp;
			for(k=0; k<stptr->ents; k++,vtabp++) {
				vtabp->pptr=vtpp+l;
				act=vtabp->condition; if(act<0) act=-1-act; l+=ncop[act];
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
		if(*(objtab->rmlist)<=-INS)
			(obtab+(-(INS+*(objtab->rmlist))))->inside++;
		if(objtab->mobile!=-1) {
			mobtab->obj = i; mobile=mobp+objtab->mobile;
			mobtab->count=(mobtab->speed=mobile->speed)+(l++);
			mobtab->pflags = NULL; mobtab++;
		}
	}
	if(ifp) fclose(ifp); ifp=NULL; mobact=FALSE;
}

memfail(char *s) {
	LogEntry("**",PRIV,"** Can't allocate memory for %s **",s); quit();
}

readfail(char *s,int got,int wanted) {
	printf("** Error: Expected %d ",wanted); printf(s);
	printf(" entries, only got %d!\n",got); quit();
}

givebackmemory() {
	if(usr)		FreeMem((char *)usr,	UINFO);
	if(rktab)	FreeMem((char *)rktab,	sizeof(*rktab)*ranks);
	if(vbtab)	FreeMem((char *)vbtab,	verbs*sizeof(*vbtab));
	if(obtab)	FreeMem((char *)obtab,	obtlen+desctlen+ormtablen+statablen);
	if(umsgip)	FreeMem((char *)umsgip,	umsgil+umsgl);
	if(ttp)		FreeMem((char *)ttp,	ttlen+ttplen);
	if(slottab)	FreeMem((char *)slottab,stlen+vtlen+vtplen);
	if(synp)	FreeMem((char *)synp,	synlen+synilen+adtablen);
	if(mobmem)	FreeMem((char *)mtab,	mobmem);

	lstat=NULL; umsgip=NULL; umsgp=NULL; ttpp=NULL; ttp=NULL;
	vbtab=NULL; slottab=NULL; vtp=NULL; vtpp=NULL; adtab=NULL;
	statab=NULL; ormtab=NULL; desctab=NULL; obtab=NULL; rktab=NULL;
	rmtab=NULL; usr=NULL; rctab=NULL; synp=NULL; synip=NULL; mobmem=NULL;
	mtab=NULL; mobp=NULL;
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

quit() {			// Exit tidily
	if(ifp)		fclose(ifp); ifp=NULL;
	if(port)	DeleteAPort(port,0L);
	if(TDBase) {
		AbortIO((struct IORequest *)&ResReq);
		CloseDevice((struct IORequest *)&ResReq);
	}
	if(trport)	DeleteAPort(trport,0L);
	if(AMUDBase)	CloseLibrary((struct Library *)AMUDBase); AMUDBase=NULL;
	givebackmemory(); exit(0);
}

char *xread(char *s,long *n,char *t)	// Size/Read data files
{	register char *p; register int i;

	fopenr(s);
	fseek(ifp,0,2L); *n=ftell(ifp); fseek(ifp,0,0L);
	if(*n) {
		if(!(p=(char *)AllocMem(*n,MEMF_PUBLIC))) memfail(t);
		if((i=fread(p,1,*n,ifp))!=*n) readfail(t,i,*n);
	}
	return p;
}

lock(){				// Lock user IO
	bid[Af]=Ad;
	if((lstat+Ad)->IOlock!=-1 || (busy[Ad] && Ad!=Af && bid[Ad]!=Af)) {
		Ad=-1; return;
	}
	(lstat+Ad)->IOlock=Af; bid[Af]=-1;
}

Log(char *f,char l,char *s) {			// Write string to log file
	WriteLog(f,l,s,NULL);
}

LogEntry(char *f,char l,char *s,char *m) {	// Write strings to log file
	WriteLog(f,l,s,m);
}

WriteLog(char *f,char l,char *s,long m)
	register FILE *fp;

	l=Lnos[l+1];			// Use line No. character
	if((fp=fopen(logname,"ab+"))) {
		fseek((FILE *)fp,0,2L);
		fprintf((FILE *)fp,"%s (%c) %s: ",now());
		fprintf((FILE *)fp,s,m);
		fclose((FILE *)fp); return;
	}
	if(!quiet) printf("\x07*** CAN'T WRITE TO %s!\n\n",logname);
}

DInit() {			// Initialise daemons
	register _DAEMON *dp;	register int i;
	daemon=NULL; dp=&daem[MAXD-1]; dp->prv=dp-1; (dp--)->nxt=NULL;
	for( ; dp>&daem[0] ; dp-- ) { dp->prv=dp-1; dp->nxt=dp+1; }
	dp->prv=NULL; dp->nxt=dp+1; daemons=0; Count=mins*60; free=dp;
}

DRemove(register _DAEMON *dp) {	// Remove daemon from the list
	if(dp==daemon) daemon=dp->nxt;	// Top of "in use" list?
	// Unlink daemon from chain
	if(dp->nxt) dp->nxt.prv=dp->prv;
	dp->prv.nxt=dp->nxt;
	// Place it in the "free" list
	dp->nxt=free; dp->prv=NULL; free->prv=free=dp;
	daemons--;		// One less daemon
}

DKill(SHORT d) {		// Cancel daemon
	register _DAEMON *dp;

	if(!(dp=daemon)) return;	// No daemons
	if(d==-1) {
		do {
			if(dp->own==Af) DRemove(dp);
		} while((dp=dp->nxt));
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

	dp=free; free=dp->nxt; free->prv=NULL;	// Off free list
	if((dp->nxt=daemon)) daemon->prv=dp;	// Onto in-use list
	daemon=dp; daemons++;			// At head of in-use list
	dp->val[0]=Ap1; dp->val[1]=Ap2; dp->typ[0]=Ap3; dp->typ[1]=Ap4;
	dp->own=owner; dp->count=(SHORT)Ap; dp->num=Ad;
}

DCheck(int d) {			// Check if daemon active
	register _DAEMON *dp;
	Ad=-1; Ap1=-1; if(!(dp=daemon)) return;
	do {
		if(dp->num==d && (dp->own==Af || dp->own>=MAXU) {
			Ad=dp; Ap1=dp->count; break;
		}
	} while((dp=dp->nxt));
}

setam(){			// Get message with replies to TrashPot
	CommsPrep(0L,0L,&am); am->from = -1;
}

logwiz(int who){
	sprintf(block,"\"%s\" reached top rank (%ld)!\n",Af+'0',now(),(usr+Af)->name,(usr+Af)->rank+1);
	Log("[]",Af,block);
}

warn(char *s){
	int i;
	for(i=0; i<MAXU; i++)
		if((lstat+i)->state != OFFLINE) warna(s,i);
}

warna(char *s,int i){
	setam(); am->ptr=s; am->type=MRWARN;
	PutMsg((lstat+i)->rep,am);
}

provoke(){
	mobtab=mtab+Ad;
	if(mobtab->count < 0) return;
	mobsig(mobtab->speed/5,(long)Ap);
}

mobsig(int j,long k){
	setam(); am->type=MMOBILE; am->data=mobtab->obj;
	am->p1=mobtab->pflags; am->p2=k;
	PutMsg((lstat+mobln)->rep,am); mobtab->count=mobtab->speed+j;
}

AskTimer() {			// Schedule a timer request
	ResReq.tr_time.tv_secs=timefactor; ResReq.tr_time.tv_micro=XR;
	ResReq.tr_node.io_Command=TR_ADDREQUEST; SendIO(&ResReq.tr_node);
}
