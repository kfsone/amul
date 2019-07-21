//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/IOBits.C	Various low-level io functions
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

iocheck() {
	txed=FALSE; REALiocheck(); if(!died && !exeunt && txed) ARefresh();
}

REALiocheck() {
	int i; long t,d,f; register char *pt; int p[4];

loopit:	t=0;
bkloop:	if(!(ap=(struct Aport *)GetMsg((struct MsgPort *)reply))) return;
	ip=1; addcr=YES;
	if(ap->type==MCLOSEING || ap->type==-'R') toldtoreset();
	t=ap->type; d=ap->data; f=ap->from; pt=ap->ptr;
	p[0]=ap->p1; p[1]=ap->p2; p[2]=ap->p3; p[3]=ap->p4;
	if(t==MDAEMON) {
		long	p1,p2,p3,p4,v;
		ReplyMsg((struct Message *)ap);
		p1=inoun1; p2=inoun2; p3=wtype[1]; p4=wtype[3]; v=iverb;
		inoun1=p[0]; inoun2=p[1]; wtype[1]=p[2]; wtype[3]=p[3]; ip=0;
		lang_proc(d,0);
		inoun1=p1; inoun2=p2; wtype[1]=p3; wtype[3]=p4; iverb=v; ip=1;
		goto voila;
	}
	if(t==MMOBILE) {	// ONLY happens to Mobile/Daemon processor
		ReplyMsg((struct Message *)ap);
		Mobile_Proc(d,p[0]); goto voila;
	}
	if(t==MFORCE) strcpy(input,ap->ptr);
	SendIt(MBUSY,NULL,NULL);
	if(f!=-1 && (lstat+f)->state>=PLAYING) ReplyMsg((struct Message *)ap);
	else DeleteAPort(0L,ap); 
	LockUser(Af);
	// Any messages we receive should wake us up.

	if(me2->flags & PFASLEEP){
		cure(Af, SSLEEP); sys(IWOKEN); i=1;
	} else i=0;

	if(t==MSUMMONED){
		if(d!=me2->room){
			sys(BEENSUMND); action(acp(SUMVANISH),ACANSEE);
			moveto(d); action(acp(SUMARRIVE),ACANSEE);
		}
		i=0;	// wake in transit.
		goto endlok;
	}
	if(t==MDIE) { akillme(DED); goto endlok; }
	if(t==MTLIMIT) { tx("\n\n"); tx(pt); akillme(DED); goto endlok; }
	if(t==MEXECUTE){
	long	ostam,odied;
		odied=died; died=0; ostam=me->stamina;
		tt.condition=0; act(d,(long *)&p[0]);
		// Did they just kill us?
		if((odied!=2 && died==2) || (ostam!=me->stamina && me->stamina<=0)) {
			ioproc(acp(KILLEDPL));
			InterSend(f,(rktab+me->rank)->minpksl,MKILLED,NULL,NULL,NULL,NULL);
			died=2; exeunt=TRUE; actor=f;
		}
		goto endlok;
	}
	if(t==MKILLED){
		addcr=YES; tx(ob); *ob=0; aadd(d,STSCORE,Af);
		if(me->dext+me2->dextadj < (rktab+myRANK)->dext) me2->dextadj++;
		goto endlok;
	}
	if(t==MFORCE){
		if(d){	// 0=forced, 1=follow
			sprintf(block,"You follow %s %s.\n",(usr+f)->name,input);
			tx(block); fol=1;
		}
		else	txs("* You are forced to \"%s\" *\n",input);
		forced=d+1;
	}
	if(t==MRWARN){
		addcr=YES; tx(pt); goto endlok;
	}
	if(t!=MMESSAGE) goto endlok;
wait:	// Lock my IO so I can read & clear my output buffer
loked:	addcr=YES; tx(ob); *ob=0;
endlok:	FreeUser(Af); if(i==1) action(acp(WOKEN),ACANSEE);
voila:	ip=0; SendIt(MFREE,NULL,NULL);
	goto loopit;		// Check for further messages
}

LockUser(int u) {		// Bid/Free lock on a user
	BSendIt(MLOCK,u,NULL);
}

FreeUser(int u) {		// Release the lock
	(lstat+i)->IOlock=-1;
}

esc(char *p,char *s) {		// Find @ escape sequences
	register char c;

	c=tolower(*(p+1));
	switch(tolower(*p)){
		case 'a':
			if(c=='d') { strcpy(s,adname); return 1; }
			if(c=='n') { strcpy(s,"Oliver Smith"); return 1; }
			if(c=='v') { strcpy(s,vername); return 1; }
		case 'c':
			if(c=='r') { *(s++)='\n'; *s=0; return 1; }
		case 'e':
			if(c=='x') { sprintf(s,"%ld",me->experience); return 1; }
		case 'f':	// <friend> - person you are helping
			if(c=='r' && me2->helping!=-1) { strcpy(s,(usr+me2->helping)->name); return 1; }
			if(c=='m' && me2->followed!=-1) { strcpy(s,(usr+me2->followed)->name); return 1; }
			strcpy(s,"no-one"); return 1;
		case 'g':
			switch(c){
				case 'n': strcpy(s,(me->sex)?"female":"male"); return 1;
				case 'e': strcpy(s,(me->sex)?"she":"he"); return 1;
				case 'o': strcpy(s,(me->sex)?"her":"his"); return 1;
				case 'h': strcpy(s,(me->sex)?"her":"him"); return 1;
				case 'p': sprintf(s,"%ld",me->plays); return 1;
			}
		case 'h':	// The person helping you
			if(c=='e' && me2->helped!=-1) { strcpy(s,(usr+me2->helped)->name); return 1; }
		case 'l':
			if(c=='r') { strcpy(s,lastres); return 1; }
			if(c=='c') { strcpy(s,lastcrt); return 1; }
		case 'm':
			switch(c){
				case 'e': strcpy(s,me->name); return 1;
				case '!': sprintf(s,"%-21s",me->name); return 1;
				case 'r': PutARankInto(s,Af); return 1;
				case 'f': if(me2->following==-1) strcpy(s,"no-one"); else strcpy(s,(usr+me2->following)->name); return 1;
				case 'g': sprintf(s,"%ld",me->magicpts); return 1;
			}
		case 'n':
			if(c=='1' && inoun1>=0 && wtype[1]==WNOUN){
				strcpy(s,(obtab+inoun1)->id); return 1;
			}
			if(c=='1' && wtype[1]==WTEXT){
				strcpy(s,(char *)inoun1); return 1;
			}
			if(c=='1' && inoun1>=0 && wtype[1]==WPLAYER){
				strcpy(s,(usr+inoun1)->name); return 1;
			}
			if(c=='2' && inoun2>=0 && wtype[3]==WNOUN){
				strcpy(s,(obtab+inoun2)->id); return 1;
			}
			if(c=='2' && wtype[3]==WTEXT){
				strcpy(s,(char *)inoun2); return 1;
			}
			if(c=='2' && inoun2>=0 && wtype[3]==WPLAYER){
				strcpy(s,(usr+inoun2)->name); return 1;
			}
			strcpy(s,"something"); return 1;
		case 'o':
			if(c=='1' && me2->wield!=-1) { strcpy(s,(obtab+(me2->wield))->id); return 1; }
			if(c=='2' && (lstat+(me2->fighting))->wield!=-1) { strcpy(s,(obtab+((lstat+(me2->fighting))->wield))->id); return 1; }
			strcpy(s,"bare hands"); return 1;
		case 'p':
			if(c=='l') { strcpy(s,(usr+me2->fighting)->name); return 1; }
			if(c=='w') { strcpy(s,me->passwd); return 1; }
			if(c=='o')	// Players online
			{	register int i=0;
				for(c=0; c<MAXU; c++) if((lstat+c)->state>=PLAYING) i++;
				sprintf(s,"%ld other %s",i,(i==1)?"player":"players");
				return 1;
			}
		case 'r':
			switch(c){
				case 'e':	timeto(s,*rescnt); return 1;
				case 'd':	desc_here(RDBF); return 1;
				case 'm':	strcpy(s,(rmtab+me2->room)->id); return 1;
			}
		case 's':
			if(c=='c') { sprintf(s,"%ld",me->score); return 1; }
			if(c=='g') { sprintf(s,"%ld",me2->sctg); return 1; }
			if(c=='r') { sprintf(s,"%ld",me->strength); return 1;}
			if(c=='t') { sprintf(s,"%ld",me->stamina); return 1;}
			return 0;
		case 'v':
			if(c=='b') { strcpy(s,(vbtab+overb)->id); return 1; }
			if(c=='e') { strcpy(s,(vbtab+iverb)->id); return 1; }
			if(c=='1' && inoun1>=0 && wtype[1]==WNOUN) { sprintf(s,"%ld",scaled(State(inoun1)->value,State(inoun1)->flags)); return 1; }
			if(c=='2' && inoun2>=0 && wtype[3]==WNOUN) { sprintf(s,"%ld",scaled(State(inoun2)->value,State(inoun2)->flags)); return 1; }
		case 'w':
			if(c=='1' && inoun1>=0 && wtype[1]==WNOUN) { sprintf(s,"%ldg",((obtab+inoun1)->states+(long)(obtab+inoun1)->state)->weight); return 1; }
			if(c=='2' && inoun2>=0 && wtype[3]==WNOUN) { sprintf(s,"%ldg",((obtab+inoun2)->states+(long)(obtab+inoun2)->state)->weight); return 1; }
			if(c=='i') { sprintf(s,"%ld",me->wisdom); return 1; }
		case 'x':
			if(c=='x') strcpy(s,mxx);
			if(c=='y') strcpy(s,mxy);
			return 1;
	}
	return 0;
}

interact(int msg,int n,int d) {
	InterSend(n,d,msg,NULL,NULL,NULL,NULL,NULL);
}

sendex(int n,int d,long p1,long p2,long p3,long p4) {
	InterSend(n,-(1+d),MEXECUTE,NULL,p1,p2,p3,p4);
}

InterSend(int n,int d,int t,char *p,int p1,int p2,int p3,int p4) {
	if((lstat+n)->state < PLAYING) return;
	LockUser(n);
	if(t==MMESSAGE || t==MKILLED) strcat((lstat+n)->buf,ow);
	CommsPrep(NULL,NULL,&intam);
	IAf=Af; IAt=t; IAd=d; IAp=p; intam->p1=p1; intam->p2=p2; intam->p3=p3; intam->p4=p4;
	FreeUser(n);
	PutMsg((lstat+n)->rep,(struct Message *)intam);
}

toldtoreset() {
	if(Af<0) goto drop;
	me2->helping=me2->helped=me2->following=me2->followed=-1;
	sys(RESETSTART);
	if(MyFlag==am_USER) { ShowFile("reset","\n"); pressret(); }
drop:	tx("\n...Resetting...\n\n");
	ap->from=-'O';
	ReplyMsg((struct Message *)ap); link=0; quit();
}

// Send an AMan command and wait on something on the AMANREP reply port
SendIt(int t,int d,char *p) {
	if(!link) return;
	AMt=t; AMd=d; AMp=p;
	PutMsg(port, (struct Message *) amanp);
	for( ; ; ) {		// Loop until the BREAK is encountered
		Wait(-1); if(GetMsg(amanrep)) break;
		iocheck();
	}
	if(AMt==-'R') toldtoreset();
	Af=AMf; Ad=AMd; Ap=AMp; At=AMt; Ap1=Apx1; Ap2=Apx2; Ap3=Apx3; Ap4=Apx4;
}

BSendIt(int t,int d,char *p) {	// Buffered send-it
	long ot,od,op;
	ot=t; od=d; op=(long)p; SendIt(t,d,p); t=ot; d=od; p=(char *)op;
}

// Lock a room - this will wait until the room is locked
LockRoom(long room) {
//	BSendIt(MRMLOCK,room,NULL);
}

// Free a room
FreeRoom(long room) {
//	BSendIt(MRMFREE,room,NULL);
}
