//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AGL.C		AGL System - Main File (1 of 4)
//
//	LMod: oliver 11/06/93	AMUL -> AGL
//

#define	AGL	1
#define	AGL1	1
#define	FRAME	1
#define	PORTS	1

#include <ADV:H/Cons.H>
#include <ADV:Frame/AGLInc.H>			// Main Include

main(int argc,char *argv[])			// == MAIN PROGRAM ==
{	register int i;	register char *p;

	lverb=-1; iverb=-1; ip=1; addcr=NO;	MyFlag = am_USER; iosup=0;

	setvername(vername);

	if(argc>1) {
		if(argv[1][0]=='?' || argv[1][1]=='?') {
			printf("%s -- AGL frame.\n\nSee docs for usage.\n",vername);
			quit();
		}
		if(*(argv[1]+1)==3) MyFlag = am_DAEM;
		if(*(argv[1]+1)==4) MyFlag = am_MOBS;
	}

	if((AGLBase=OpenLibrary("agl.library",0L))==NULL) { tx("\nNo agl.library!\n"); quit(); }
	if((p=CommsPrep(&port,&amanrep,&agl))) { tx(p); quit(); }
	if(!(reply=CreateAPort(0L))) memfail("user port");
	/* amanp is allocated incase of emergencies! */
	if(!(amanp=(struct Aport *)AllocMem(sizeof(*agl),STDMEM))) memfail("comms port");
	if(!(ob=(char *)AllocMem(OWLIM,STDMEM))) memfail("IO buffers");
	if(!(ow=(char *)AllocMem(OWLIM,STDMEM))) memfail("IO buffers");
	if(!(input=(char *)AllocMem(400,STDMEM))) memfail("IO buffers");

	switch(MyFlag) {	// Special processor line?
		case am_DAEM:	Af = MAXU; break;
		case am_MOBS:	Af = MAXU+1; break;
		default:	Af = -2;
	}
	*amanp = *agl; link=1; SendIt(MCNCT,-10,NULL);	// connection!
	if(Af<0)	{ sys(NOSLOTS); quit(); }
	lstat=(struct LS *)Ad; me2=lstat+Af; me2->IOlock=Af; ip=0;
	usr=(struct _PLAYER *)Ap; me=usr+Af; me2->rep=reply; me2->buf=ob;
	me->passwd[0]=0; aman_connect();	// Connect to AMan
	iverb=SERIO;

// Two types of "serial" connection are supported
//	-D gives "direct" serial, without carrier checking
//	-S gives (assumed) modem with carrier checking

	if(argc>1) {
		for(i=1; i<argc; ) {
			if(argv[i][0]!='-') {
				txs("Invalid argument %s!\n",argv[i]); quit();
			}
			if(argv[i][1]==3 || argv[i][1]==4) { iosup=LOGFILE; i=argc; break; }
			block[0]=toupper(*(argv[i]+1)); i++;
			switch(block[0]) {
				case 'A':		// Account Name
					if(i>=argc || argv[i][0]=='-') break;
					strcpy(me->passwd,argv[i++]);
					while(i<argc && argv[i][0]!='-') {
						strcat(me->passwd," ");
						strcat(me->passwd,argv[i++]);
					}
					break;
				case 'D': iverb=DSERIO;	// IO Type
				case 'S': if(i>=argc) { tx("No baud rate!\n"); quit(); }
					  switchS(argv[i++],(i<argc)?argv[i++]:NULL,(i<argc)?argv[i++]:"0",(i<argc)?argv[i++]:"Y"); break;
				default	:
					txs("Invalid argument %s!\n",argv[i-1]); quit();
			}
		}
		if(!iosup) switchC();
	} else switchC();

	me2->xpos=0; iverb=-1; FreeUser(Af); *ob=0;
	SendIt(MFREE,NULL,NULL); iocheck(); me->llen=DLLEN; me->slen=DSLEN;

	if(Af >= MAXU) Special_Proc();	// Special processors go here!

	rset=(1 << Af); rclr=-1-rset;	// Clear room flags!
	for(i=0; i < rooms; i++) *(rctab+i)=(*(rctab+i) & rclr);

	ShowFile("TITLE",*(errtxt-2)); getid(); settitle();
	last_him=last_her=it=-1;

	do {
		died=0; actor=-1; fol=0; addcr=NO;
		if(last_him != -1 && (lstat+last_him)->state < PLAYING) last_him=-1;
		if(last_her != -1 && (lstat+last_her)->state < PLAYING) last_her=-1;
		iocheck(); ShowPrompt();
		block[0]=0; Inp(input,390);
		if(exeunt) break; if(!input[0]) continue;
		if(!strcmp(input,"help") || strcmp(input,"?") { sys(HELP); continue; }
		if(!strcmp(input,"***debug")) { debug=debug ^ 1; continue; }
		if(!strcmp(input,"/chat")) {
			if(me2->state==CHATTING) continue;
			if(isverb("!speech")==-1 || isverb("say")==-1) { sys(CANTDO); continue; }
			tx(*(errtxt-5));
			me2->state=CHATTING; continue;
		}
		if(!strcmp(input,"/endchat")) {
			if(me2->state!=CHATTING) { tx("You aren't in chat mode!\n"); continue; }
			me2->state=PLAYING; tx("** CHAT MODE ENDED **\n"); continue;
		}
		if(me2->state==CHATTING) {
			if(input[0]!='/') {
				wtype[0]=wtype[1]=wtype[2]=wtype[3]=WNONE;
				iadj1=iadj2=inoun2=-1;
				inoun1=(long) input; wtype[1]=WTEXT;
				i=isverb("!speech");
				if((iverb=overb=isverb("say"))==-1) iverb=overb=i;
				lang_proc(i,0); txc('\n'); continue;
			}
			strcpy(input,input+1);	// Skip the "/"
		}
		if(input[0]=='/') { internal(input+1); continue; }
gloop:		failed=NO; forced=0; died=0; exeunt=0;
		if(grab()==-1) break;
		iocheck();
		if(forced && !died && !exeunt) goto gloop;
	} while(!exeunt && !died);

quitgame:		// Quit the game, tidily.
	if(died) action(acp(HEDIED),AGLOBAL);
	    else action(acp(EXITED),AGLOBAL);
	if(died==2 && me->rank!=ranks-1) zapme();
	forced=0; exeunt=0; died=0;
	if(me->plays==1) sys(BYEPASSWD);
	if(dmove[0]) dropall(isroom(dmove)); else dropall(myROOM);
	LoseFollower();
	if(iosup==CUSSCREEN) pressret(); quit();
}

