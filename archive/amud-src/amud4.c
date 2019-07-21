//
//            ****       AMUD4.C......Adventure System      ****
//            ****            Special Processors!           ****
//
//  Copyright (C) Oliver Smith, 1991-2. Copyright (C) Kingfisher s/w 1991-2
//         Program Designed, Developed and Written By: Oliver Smith.
//

#define	DEF

#include "adv:frame/AMUDInc.H"

long	hi_t,low_t;	// Highest & lowest travel verb

Special_Proc()		// Special Processor core
{	register int i;

	low_t=-1; vbptr=vbtab;
	for(i=0; i<verbs; i++,vbptr++)
		if(!(vbptr->flags & VB_TRAVEL)) {
			hi_t=i; if(low_t==-1) low_t=i;
		}

	if(ifp)fclose(ifp); ifp=NULL;
	iosup = LOGFILE;	// So it gets written to a log
	wtype[0]=wtype[1]=wtype[2]=wtype[3]=WNONE;
	iverb=iadj1=inoun1=iadj2=inoun2=-1; actor=last_him=last_her=it=-1;
	me2->followed=me2->following=me2->helping=me2->helped=me2->fighting=-1;
	switch(MyFlag)		// What type of processor?
	{
		case am_DAEM:	// Execute the boot-up daemon
				if((i=isverb("!boot"))!=-1) lang_proc(i,0);
				Daem_Proc();	// Daemon Processor
		case am_MOBS:	printf("-- Mobile processor requested - unable to comply.\n");
		default:	printf("-- Unsupported special processor requested\n");
	}
	quit();		// Don't go anywhere
}

Daem_Proc()		// Daemon processing host
{
	do { Wait(-1); iocheck(); } while(1==1);
}

Mobile_Proc(long mob_no,int flags) {
	mobile_proc(mob_no,flags);
}

mobile_proc(long mob_no,int flags)	// Process mobile
{	long randm;

	// Mobiles currently cannot fight

	me2->rec = mob_no;	// So we know which mobile
	me2->room = *((obtab+mob_no)->rmlist);	// Set location
	me2->flags = flags;	// Set my flag
	mobile=mobp+(obtab+mob_no)->mobile;	// Get character header
	failed=NO; forced=died=exeunt=0;
	sprintf(me->name,"the %s",(obtab+mob_no)->id);
	me2->arr=(char *)acp(mobile->arr);
	me2->dep=(char *)acp(mobile->dep);

	randm=Random(100);		// What are they doing?
					// travel, fight, act or wait
	if(randm<mobile->travel)  return Mob_Travel(); randm-=mobile->travel;
	if(randm<mobile->fight)   return Mob_Fight(); randm-=mobile->fight;
	if(randm<mobile->act)     return Mob_Act();
	return;		// It's waiting
}

Mob_Travel()
{	register int i,rm;
	rm=me2->room;
	for(i=0; i<3 && me2->room==rm; i++) {
		iverb=low_t+Random(hi_t-low_t); tt.verb=iverb;
		if(!((vbtab+iverb)->flags & VB_TRAVEL)) ttproc();
	}
	return;
}

Mob_Fight() { return; }

Mob_Act()
{	register int v,i,x,l;

	sprintf(block,"!%s",mobile->id); if((v=isverb(block))<NULL) return;
	tt.verb=-1; inoun1=me2->rec; wtype[2]=WNOUN;
loop:	i=Random((vbptr->ptr)->ents-1); donet=i; l=-1; ml=(vbptr->ptr)->ents; overb=iverb=v;
loop2:	vbptr=vbtab+v; stptr=vbptr->ptr; vtabp=(stptr->ptr)+donet;
	x=vtabp->condition; if(x<0) x=-(1+x);
	if(x==CANTEP || x==CAND || x==CELSE || x==CELTEP) {
		if(donet==i) goto loop;
	} else if(donet>i) return;

	tt.action=vtabp->action; tt.pptr=vtabp->pptr;
	if((l=cond(vtabp->condition,l))==-1) return;
	act(tt.action,vtabp->pptr); if(ml<-1) return lang_proc(iverb,0);
	if(ml<0 || donet>=ml || failed!=NO || forced || died || exeunt) return;
	donet++; goto loop2;
}
