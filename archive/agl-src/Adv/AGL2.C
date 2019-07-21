//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AGL2.C		AGL System - Subset (2 of 4)
//
//	Lmod: oliver 17/06/93	Changed ascore() to include pts-to-nxt-lvl
//	      oliver 11/06/93	AMUL -> AGL
//

#include "adv:frame/AGLInc.H"

extern	char	ncop[],*scopy(char *,char *);

#include	<ADV:Frame/Daemons.C>
#include	<ADV:Frame/FileBits.C>
#include	<ADV:Frame/Parser.C>
#include	<ADV:Frame/GetID.C>

owner(int obj) {
	register int r;
	r=-5-*(obtab+obj)->rmlist; if(r>=MAXU || r<0) return -1; return r;
}

moveto(int r) {
	register int i;

	StopFollow(); me2->flags = me2->flags | PFMOVING;
	if(Af<MAXU && me2->followed>-1 && me2->followed!=Af && cansee(me2->followed,Af)==YES)
		LoseFollower();
	lroom=myROOM; i=myLIGHT; myLIGHT=0; lighting(Af,AOTHERS);
	if(Af>=MAXU) *(obtab+me2->rec)->rmlist=r; // Move mobiles correctly
	if(((rmtab+r)->flags & ANTERM) && r==myROOM) r=arandgo(0);
	myROOM=r; myLIGHT=i; myHLIGHT=0; lighting(Af,AOTHERS);
	if(Af<MAXU && !(me2->flags & PFASLEEP))
		look((rmtab+myROOM)->id, me->rdmode);
	me2->flags = me2->flags & ~PFMOVING;
}

gotin(int obj,int st)	// Do I own a "obj" in state "stat"?
{	register int i;	register char *oi=(obtab+obj)->id;

	if(Af>=MAXU) {
		dtx(">gotin - Af>=MAXU!\n");
		if(me2->rec==obj && ((obtab+obj)->state==st || st==-1)) return YES;
		return NO;
	}
	for(i=0; i<nouns; i++) {
		if(!strcmp((obtab+i)->id,oi) && ((obtab+i)->state==st || st==-1) && owner(i)==Af)
			return YES;
	}
	if(debug) { sprintf(str,">gotin(%ld:%s,%ld) FALSE.\n",obj,oi,st); tx(str); }
	return NO;
}

achecknear(int obj) {	// Check near object, else msg + endparse
	if(nearto(obj)==NO) { txs("I can't see the %s!\n",(obtab+obj)->id); donet=ml+1; return -1; }
	inc=0; return 0;
}

acheckget(int obj) {	// Check the player can 'get' the object
	if(carrying(obj)!=-1) {
		tx("You've already got it!\n"); donet=ml+1; return -1;
	}
	if(achecknear(obj)==-1) return -1; else inc=1;
	objtab=obtab+obj;
	if((objtab->flags&OF_SCENERY) || (STATE->flags&SF_ALIVE) || objtab->nrooms!=1) {
		tx("You can't move that!\n"); donet=ml+1; return -1;
	}
	if((rktab+myRANK)->numobj<=0) {
		tx("You can't manage it!\n"); donet=ml+1; return -1;
	}
	if(STATE->weight > (rktab+myRANK)->maxweight) {
		tx("You're not strong enough to lift that!\n"); donet=ml+1; return -1;
	}
	if(me2->numobj + 1 > (rktab+myRANK)->numobj) {
		tx("You can't carry any more!");
		if((rktab+myRANK)->numobj>0) tx(" You'll have to drop something else first.");
		txc('\n'); donet=ml+1; return -1;
	}
	if(STATE->weight + me2->weight > (rktab+myRANK)->maxweight) {
		tx("It's too heavy.");
		if(me2->numobj<(rktab+myRANK)->numobj)
			tx(" You'll have to drop something else first.");
		txc('\n'); donet=ml+1; return -1;
	}
	inc=0; return 0;
}

look_here(int f,int rm) {
	register char il;

	if(me2->flags & PFBLIND) { list_what(rm,0); return; } // Can I see?
	if((il=LightHere)==NO) {	// Can we see in here?
		sys(TOODARK); *(rctab+rm)=*(rctab+rm)&rclr;
	}
	else desc_here(f);
	if((roomtab->flags & DEATH) && myRANK!=ranks-1) {
		if(!*dmove) strcpy(dmove,(rmtab+lroom)->id);
		akillme(DED); return; 
	}
	if(il!=NO) { whohere(); list_what(rm,0); }
	NewLine();
}

get_dmove() {	// MUST set roomtab first
	fseek(ifp,roomtab->desptr,0L);
	if(roomtab->flags & DMOVE) fgets(dmove,IDL,ifp); else *dmove=0;
}

desc_here(int f) {
	get_dmove(); *(block+1021)=0;
	if(!(roomtab->flags & DEATH)) ans("1m");
	fgets(block,(OWLIM-4),ifp); tx(block); ans("0m");
	if(f == RDVB) {		// Display LONG description
more:		block[0]=0; block[800]=0; block[801]=0;
		fread(block,1,800,ifp);		// aprox. two disk blocks
		if(block[0]!=0) {
			tx(block); if(strlen(block)>=800) goto more;
		}
	}
	block[0]=0;
}

char *descobj(int Ob,char *s) {
	*s=0; obj.states=(obtab+Ob)->states+(long)(obtab+Ob)->state;
	if(!obj.states->descrip) return s;
	sprintf(s,desctab+obj.states->descrip,adtab+((obtab+Ob)->adj * (IDL + 1)));
	s+=strlen(s);
	if(*(s-1)=='\n') *(--s)=0;	if(*(s-1)=='{') *(--s)=0;
	if(((obtab+Ob)->flags&OF_SHOWFIRE) && (obj.states->flags&SF_LIT)) {
		sprintf(s," The %s is on fire.",(obtab+Ob)->id); s+=strlen(s);
	}
	if((obtab+Ob)->contains>0) {
		showin(Ob,NO); if(*str) sprintf(s," %s",str);
		s+=strlen(s);
	}
	*(s++)=' '; *s=0; return s;
}

list_what(int r,int i) {
	register int o,or; register char *p;

	if(LightHere==NO) return sys(TOOMAKE);
	if(me2->flags & PFBLIND) sys(YOURBLIND);
	if(i && ((rmtab+r)->flags & HIDEWY) && myRANK != ranks-1) {
		sys(NOWTSPECIAL); // TopRanks can in hideaways!
	}
	*(p=block)=0;
	for(o=0;o<nouns;o++) {		// All objects
		if(canseeobj(o,Af)==NO) continue;
		if(((rmtab+r)->flags & HIDEWY) && (!i || (i==1 && myRANK!=ranks-1)) && !((obtab+o)->flags & OF_SCENERY)) continue;
		if(LightHere==NO && !((obtab+o)->flags & OF_SMELL)) continue;
		obj=*(obtab+o);
		for(or=0; or<obj.nrooms; or++) {
			if(*(obj.rmlist+or)==r && State(o)->descrip>=0) {
				p=descobj(o,p);
			}
		}
	}
	tx(block); NewLine(); if(i && p==block) sys(NOWTSPECIAL);
}

inflict(int x, int s) {
	you2=lstat+x; if(you2->state < PLAYING) return;
	switch(s) {
		case SGLOW:	if(!(you2->flags&PFGLOW)) { you2->flags=(you2->flags|PFGLOW); hisLIGHT++; } break;
		case SDEAF:	you2->flags=you2->flags|PFDEAF; break;
		case SBLIND:	you2->flags=you2->flags|PFBLIND; break;
		case SCRIPPLE:	you2->flags=you2->flags|PFCRIP; break;
		case SDUMB:	you2->flags=you2->flags|PFDUMB; break;
		case SSLEEP:	you2->flags=you2->flags|PFASLEEP; break;
		case SSINVIS:	you2->flags=you2->flags|PFSINVIS;
		case SINVIS:	you2->flags=you2->flags|PFINVIS;
				if(me2->followed!=-1 && cansee(me2->followed,Af)==NO)
					LoseFollower();
				break;
	}
	calcdext(); lighting(x,AHERE);
}

cure(int x,int s) {
	you2=lstat+x; if(you2->state < PLAYING) return;
	switch(s) {
		case SGLOW:	if(you2->flags&PFGLOW) { you2->flags=(you2->flags&(-1-PFGLOW)); hisLIGHT--; } break;
		case SDEAF:	you2->flags=you2->flags & -(1+PFDEAF); break;
		case SBLIND:	you2->flags=you2->flags & -(1+PFBLIND); break;
		case SCRIPPLE:	you2->flags=you2->flags & -(1+PFCRIP); break;
		case SDUMB:	you2->flags=you2->flags & -(1+PFDUMB); break;
		case SSLEEP:	you2->flags=you2->flags & -(1+PFASLEEP); break;
		case SSINVIS:	you2->flags=you2->flags & -(1+PFSINVIS); break;
		case SINVIS:	you2->flags=you2->flags & -(1+PFINVIS); break;
	}
	calcdext(); lighting(x,AHERE);
}

summon(int plyr) {
	if(pROOM(plyr)==myROOM) { txs(acp(CANTSUMN),pNAME(plyr)); return; }
	interact(MSUMMONED,plyr,myROOM);
}

count_set(int obj,int x) {
	register int i;
	for(i=0; i<mobs; i++)
		if((mtab+i)->obj==obj) (mtab+i)->count=x;
}

adestroy(int obj) {
	register int i; register struct _OBJ_STRUCT *op;

	Forbid(); op=obtab+obj;
	if(op->mobile!=-1) {
		mobile=mobp+op->mobile; count_set(obj,-1);
		op->state=mobile->dead;		// Death state
		i=(int)*op->rmlist; *op->rmlist=mobile->dmove;
	}
	else {
		loseobj(obj); for(i=0; i<(obtab+obj)->nrooms; i++) *((obtab+obj)->rmlist+i)=-1;
	}
	op->flags=op->flags | OF_ZONKED;
	Permit();
	if(op->mobile!=-1) action(i,acp(mobile->death));
}

arecover(int obj) {
	register int i;

	if(State(obj)->flags & SF_LIT) myLIGHT++;
	for(i=0; i<(obtab+obj)->nrooms; i++) *((obtab+obj)->rmlist+i)=myROOM;
	(obtab+obj)->flags=(obtab+obj)->flags & -(1+OF_ZONKED);
	if((obtab+obj)->mobile!=-1) count_set(obj,(mobp+(obtab+obj)->mobile)->speed+8);
	lighting(Af,AHERE);
}

zapme() {
	register char *p; register int i,j;

	nohelp(); LoseFollower(); Forbid();
	p=(char *)myNAME; exeunt=1; j=sizeof(him);
	for(i=0; i<j; i++) *(p++)=0;
	Permit(); save_me();
}

send(int o,int to) {
	register short int x,i;

	x=lit(to); loseobj(o); for(i=0; i<objtab->nrooms; i++) *(objtab->rmlist+i)=to;
	if(lit(to)!=x) actionin(to,acp(NOWLIGHT));
}

achange(int u) {
	if(u==Af) { me->sex=1-me->sex; sys(CHANGESEX); }
	else sendex(u,ACHANGESEX,u,NONE);
}

taskcnt(int p) {
	register int i,t;
	if((usr+p)->tasks==-1) return 32;
	for(i=0,t=0;i<32;i++) if((usr+p)->tasks&(i<<1)) t++;
	return t;
}

newrank(int plyr,int r) {
	register int or;

	or=myRANK;
	if(taskcnt(Af) < (rktab+r)->tasks) { sys(NOTASK); return; }
	myRANK=r; sys(MADERANK);
	me->strength += (rktab+r)->strength - (rktab+or)->strength;
	if(me->strength > (rktab+r)->strength) me->strength=(rktab+r)->strength;
	me->stamina   =(rktab+r)->stamina;
	me->wisdom    =(rktab+r)->wisdom;
	me->dext      =(rktab+r)->dext;
	me->experience+=(rktab+r)->experience-(rktab+or)->experience;
	me->magicpts  =(rktab+r)->magicpts;

	if(r == ranks-1) { sys(TOPRANK); SendIt(MMADEWIZ,0,myNAME); }
}

aadd(long howmuch, int stat, int plyr) {
	register int r,mr;
	if(howmuch < 0) return asub(-howmuch, stat, plyr);
	if(howmuch == 0) return;
	if(plyr!=Af) {
		sendex(plyr,AADD,howmuch,stat,plyr); return;
	}
	switch(stat) {
		case STSCORE:
			mySCORE+=howmuch; me2->sctg+=howmuch;
			ans("1m"); utxn(plyr,"(%ld)\n",mySCORE); ans("0m");
			for(mr=0,r=ranks-1; r>=0; r--) {
				if(mySCORE>=(rktab+r)->score) {
					// If we don't have enough tasks
					if(taskcnt(Af)<(rktab+myRANK)->tasks) {
						if(!mr) sys(NOTASK); mr++; continue;
					}
					if(myRANK>=r && !mr) break;
					newrank(plyr,r); break;
				}
			}
			break;
		case STSTR:	me->strength+=howmuch; break;
		case STSTAM:	me->stamina+=howmuch; if(me->stamina>me->stamina) me->stamina=me->stamina;
				if(!(me2->flags & PFASLEEP)) dis_stam();
				break;
		case STDEX:	me2->dextadj+=howmuch; break;
		case STEXP:	me->experience+=howmuch; break;
		case STWIS:	me->wisdom+=howmuch; break;
		case STMAGIC:	me->magicpts+=howmuch; break;
	}
}

dis_stam() {
	NewLine();
	ans("3m"); txs("<STAM: @st/%ld>\n",(rktab+me->rank)->stamina); ans("0m");
}

asub(long howmuch, int stat, int plyr) {
	register int r,flag;
	if(howmuch < 0) return aadd(-howmuch, stat, plyr);
	if(howmuch == 0) return;
	if(stat&STINFIGHT) { flag=TRUE; stat=stat&-(1+STINFIGHT); }
	if(plyr!=Af) {
		sendex(plyr,ASUB,howmuch,stat,plyr); return;
	}
	switch(stat) {
		case STSCORE:
			mySCORE-=howmuch; me2->sctg-=howmuch;
			if(mySCORE<0) mySCORE=0;
			ans("1m"); utxn(plyr,"(%ld)\n",mySCORE); ans("0m");
			for(r=0; r<ranks-1; r++) {
				if(mySCORE<(rktab+(r+1))->score) {
					if(myRANK==r) break;
					newrank(plyr,r); break;
				}
			}
			break;
		case STSTR:	me->strength-=howmuch; if (me->strength < 0) me->strength=0; break;
		case STSTAM:	me->stamina-=howmuch; if (me->stamina < 0) me->stamina=0;
				if(!(me2->flags & PFASLEEP)) dis_stam();
				if(me->stamina <= 0) akillme(1);
				break;
		case STDEX:	me2->dextadj-=howmuch; break;
		case STWIS:	me->wisdom-=howmuch; if (me->wisdom < 0) me->wisdom=0; break;
		case STEXP:	me->experience-=howmuch; if (me->experience < 0) me->experience=0; break;
		case STMAGIC:	me->magicpts-=howmuch; if (me->magicpts < 0) me->magicpts=0; break;
	}
}

afix(int stat, int plyr) {
	if(plyr!=Af) {
		sendex(plyr,AFIX,stat,plyr,0); return;
	}
	switch(stat) {
		case STSTR:	me->strength=((rktab+myRANK)->strength
					*(rktab+myRANK)->maxweight-me2->weight)
					/(rktab+myRANK)->maxweight; break;
		case STSTAM:	me->stamina=(rktab+myRANK)->stamina; break;
		case STDEX:	me2->dextadj=0; calcdext(); break;
		case STWIS:	me->wisdom=(rktab+myRANK)->wisdom; break;
		case STEXP:	me->experience=(rktab+myRANK)->experience; break;
		case STMAGIC:	me->magicpts=(rktab+myRANK)->magicpts; break;
	}
}

announce(char *s,int towho) {	// Loud noises/events
	register int i,x;

	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & PFDEAF)) continue;
		if(i != Af && pROOM(i) != myROOM &&
			((rmtab+pROOM(i))->flags & SILENT)) continue;
		x=0;
		switch(towho) {
			case AALL:
			case AEVERY1:	x=1; break;
			case AGLOBAL:	if(i != Af) x=1; break;
			case AOTHERS:	if(i == Af) break;
			case AHERE:	if(pROOM(i)==myROOM) x=1; break;
			case AOUTSIDE:	if(pROOM(i)!=myROOM) x=1; break;
			case ACANSEE:	if(cansee(i,Af)!=NO) x=1; break;
			case ANOTSEE:	if(i!=Af && pROOM(i)==myROOM && cansee(i,Af)==NO) x=1; break;
		}
		if(x==1){setmxy(NOISE,i); utx(i,s);}
	}
}

announcein(int toroom,char *s) {	// Loud noises/events
	register int i;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & PFDEAF) || pROOM(i)!=toroom) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

announcefrom(int obj,char *s) {	// Loud noises/events
	register int i,o;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & PFDEAF) || pROOM(i) == myROOM) continue;
		if((o=owner(obj))!=-1 && pROOM(o)!=pROOM(i)) continue;
		if(o==-1 && isin(obj,pROOM(i))==NO) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

objannounce(int obj,char *s) {	// Loud noises/events
	register int i,o;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & PFDEAF)) continue;
		if((o=owner(obj))!=-1 && pROOM(o)!=pROOM(i)) continue;
		if(o==-1 && isin(obj,pROOM(i))==NO) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

action(char *s,int towho) {	// Quiet actions/notices
	register int i,x;
	for(i=0; i<MAXU; i++) {
		if(actor==i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & (PFBLIND+PFASLEEP))) continue;
		x=0;
		switch(towho) {
			case AALL:
			case AEVERY1:	x=1; break;
			case AGLOBAL:	if(i != Af) x=1; break;
			case AOTHERS:	if(i == Af) break;
			case AHERE:	if(pROOM(i) == myROOM && cansee(i,Af)==YES) x=1; break;
			case AOUTSIDE:	if(pROOM(i) != myROOM) x=1; break;
			case ACANSEE:	if(cansee(i,Af)!=NO) x=1; break;
			case ANOTSEE:	if(i!=Af && pROOM(i)==myROOM && cansee(i,Af)==NO) x=1; break;
		}
		if(x == 1) {
			setmxy(ACTION,i); utx(i,s);
		}
	}
}

actionin(int toroom,char *s) {	// Quiet actions/notices
	register int i;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & (PFBLIND+PFASLEEP)) || pROOM(i)!=toroom) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

actionfrom(int obj,char *s) {	// Quiet actions/notices
	register int i,o;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & (PFBLIND+PFASLEEP)) || pROOM(i) == myROOM) continue;
		if((o=owner(obj))!=-1 && pROOM(o)!=pROOM(i)) continue;
		if(o==-1 && isin(obj,pROOM(i))==NO) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

objaction(int obj,char *s) {	// Quiet actions/notices
	register int i,o;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & (PFBLIND+PFASLEEP))) continue;
		if((o=owner(obj))!=-1) if(pROOM(o)!=pROOM(i)) continue;
		if(o==-1 && isin(obj,pROOM(i))==NO) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

fwait(long n) {
	register int i;
	if(n<1) n=1;
	for(i=0; i<7; i++) {	Delay(n*7); iocheck();	}
}

lighting(int x, int twho) {	// x=player to check, twho=action() type.
	int oact;

	if(!((rmtab+pROOM(x))->flags & DARK) || pLIGHT(x)==pHADLIGHT(x)) return;
	oact=actor; if(twho==AOTHERS) actor=Af;
	if(pLIGHT(x)<=0) {
		if(pHADLIGHT(x) <= 0) { pLIGHT(x)=pHADLIGHT(x)=0; actor=oact; return; }
		pHADLIGHT(x)=pLIGHT(x)=0;
		if(lit(pROOM(x))==NO) actionin(pROOM(x),acp(NOWTOODARK));
	}
	else {
		if(pHADLIGHT(x) <= 0 && lit(pROOM(x))==NO) actionin(pROOM(x),acp(NOWLIGHT));
		pHADLIGHT(x)=pLIGHT(x);
	}
	actor=oact;
}

loseobj(int obj) {	// Remove object from its owners inventory
	register int o,i;

	objtab=obtab+obj;
	if((o=owner(obj))!=-1) {
		for(i=0; i<objtab->nrooms; i++) *(objtab->rmlist+i)=-1;
		rem_obj(o); lighting(o,AHERE);
	}
}

nohelp() {
	register int i;

	if(me2->helping!=-1) utx(me2->helping,"@me is no-longer able to help you...\n");
	(lstat+me2->helping)->helped--; me2->helping=-1; you2=lstat;
	for(i=0; i<MAXU; i++,you2++)
		if(you2->helping==Af) {
			utx(i,"You are no longer able to help @me.\n");
			you2->helping=-1;
		}
	me2->helping=me2->helped=-1;
}

aforce(int x,char *cmd) {	InterSend(x,0,MFORCE,cmd,NULL,NULL,NULL,NULL);	}

afight(int plyr){
	if(plyr==Af) { donet=ml+1; return; }
	if((rmtab+myROOM)->flags&PEACEFUL) { sys(NOFIGHT); donet=ml+1; return; }
	if(me2->fighting != -1) { txs("You are already fighting %s!\n",pNAME(me2->fighting)); donet=ml+1; return; }
	if((lstat+plyr)->fighting != -1) { txs("%s is already in a fight!\n",pNAME(plyr)); donet=ml+1; return; }
	Forbid();
	you2=lstat+plyr;
	you2->flags=you2->flags | PFFIGHT; you2->fighting = Af; me2->fighting = plyr;
	me2->flags=me2->flags|(PFFIGHT+PFATTACKER);	// I get first hit
	Permit();
}

clearfight(){
	Forbid();
	if(me2->fighting!=-1 && (lstat+me2->fighting)->state >= PLAYING) {
		finishfight(me2->fighting);
	}
	finishfight(Af); Permit();
}

finishfight(int plyr){
	you2=lstat+plyr; you2->fighting=-1; you2->flags=you2->flags&~(PFFIGHT+PFATTACKER);
}

#define	PCTG(x,y)	(x*100/y)

ahit(int f,int stab,int strb,int dexb) {	// Take a single blow
	struct _RANK_STRUCT *trank;

	int	str,mdex,msta,ydex,ysta,old_fight;

	if(f<0||f>=MAXU) return FALSE;	// Any reason to stop fight?
	you=usr+f; you2=lstat+f;	// Who is he and what he's worth?
	if(you2->state<PLAYING || hisROOM!=myROOM) return FALSE;

	me2->dextadj+=dexb; calcdext(); me2->dextadj-=dexb; str=0;

	trank=rktab+(ranks-1);
	msta=PCTG(me->stamina,trank->stamina)+stab;	ysta=PCTG(you->stamina,trank->stamina);
	mdex=PCTG(me->dext,trank->dext);	ydex=PCTG(you->dext,trank->dext);

	if(Random(msta+mdex+10) < 10) {
		sys(MISSED); utx(f,acp(HEMISSED));
	/*	if((i=isverb("!miss"))!=-1) lang_proc(i,0);		*/
	}
	else {				// *HIT!
		register int att,def;
		att=(mdex+msta)/2; att+=(Random(att)-att/2);
		def=(ydex+ysta)/2; def+=(Random(def)-def/2);
		if(att<=def) {		// *Blocked!
			if (you2->wield != -1) {
				sys(WBLOCK); utx(f,acp(WDEFEND));
			}
			else { sys(BLOCK); utx(f,acp(DEFEND)); }
		/*	if((i=isverb("!block"))!=-1) lang_proc(i,0);	*/
		}
		else {			// STRIKE!
			str=(me->strength/10)+Random((me->strength/6)+me->rank)+1;
			if(me2->wield != -1) {
				sys(WATTACK);
				str+=Random(State(me2->wield)->strength)+1;
				utx(f,acp(WHIT));
				damage(me2->wield,1);
			}
			else { sys(ATTACK); utx(f,acp(AMHIT)); }
			asub(str+Random(2*strb),STINFIGHT | STSTAM,f);
		/*	if((i=isverb("!hit"))!=-1) lang_proc(i,0);	*/
		}
	}
	calcdext(); return TRUE;
}

swapattack() {
	me2->flags=me2->flags&~PFATTACKER; you2->flags=you2->flags|PFATTACKER;
}

atakehit(int sucker) {	// Take a hit-turn without starting combat
	int	fighting;

	fighting=me2->fighting;		// Who am I ACTUALLY fighting?
	setmxy(ACTION,(me2->fighting=sucker));
	if( sucker == -1 ) goto fini;
	you2=lstat+me2->fighting; you=usr+me2->fighting;
	if(you2->state < PLAYING || hisROOM != myROOM) goto fini;
	ahit(sucker,Random(me->rank)+1,Random(me->rank+6)-2,Random(me->rank+4)-1);
fini:	me2->fighting=fighting;
}

acombat() {	// Execute a combat sequence
	register int tsta,tdex,texp;

	tsta=(rktab+(ranks-1))->stamina; tdex=(rktab+(ranks-1))->dext; texp=(rktab+(ranks-1))->experience;

	if( me2->fighting == -1 ) goto fini;
	you2=lstat+me2->fighting; you=usr+me2->fighting;
	if(you2->state < PLAYING || hisROOM != myROOM) goto fini;
	calcdext(); setmxy(ACTION,me2->fighting);
	// Determine who is going to take this turn
	if( !(me2->flags & PFATTACKER) || me2->dextadj<=-3 || 
	    (
		Random(PCTG(me->stamina,tsta)+PCTG(me->dext,tdex)+PCTG(me->experience,texp)) <
		Random(PCTG(you->stamina,tsta)+PCTG(you->dext,tdex)+PCTG(you->experience,texp))/5
	  ) ) {
		swapattack();
		sendex(me2->fighting,ACOMBAT,0,0,0,0);
		me2->dextadj+=2; return;
	}
	if(!ahit(me2->fighting,0,0,0)) {
fini:		clearfight(); donet=ml+1; return;
	}
	me2->dextadj--;		// If I keep hitting, slow down a little
	swapattack();		// Default HIM to next hit
}

exits() {
	int v,i,x,ac,brk; struct _TT_ENT *tp,*otp;
	int maxl,l,c,a; long *pptr;

	roomtab=rmtab+myROOM;
	if(roomtab->tabptr==-1) { tx("There are no visible exits.\n"); return; }

	vbptr=vbtab; x=0; c=tt.condition; a=tt.action; otp=ttabp; pptr=tt.pptr;

	for(v=0; v<verbs; v++,vbptr++) {
		if(!(vbptr->flags&VB_TRAVEL)) continue; // Not a travel verb

		roomtab=rmtab+myROOM;
		l=-1; maxl=roomtab->ttlines; tp=ttp+roomtab->tabptr; brk=0;
		for(i=0; i<maxl && !brk; i++) {
			ttabp=tp+i; tt.condition=ttabp->condition; tt.pptr=ttabp->pptr;
			if(ttabp->verb==v && (l=cond(ttabp->condition,l))!=-1) {
				if(ttabp->action>=0) {
					txs("%-10s ",vbptr->id); brk=1;
					roomtab=rmtab+(ttabp->action);
					if(roomtab->flags & DEATH) sys(CERTDEATH); else desc_here(RDBF);
					break;
				}
				ac=-1-ttabp->action;
				switch(ac) {
					case AKILLME:
					case ARANDGO:
						txs("%s: You can't tell.\n",vbptr->id);
					case AERROR: sprintf(str,"%s: ",vbptr->id,AP1); txs(str);
					case AENDPARSE:		case AFAILPARSE:
					case AABORTPARSE:	case ARESPOND:
								maxl=-1; break;
					case ASKIP:		i+=TP1;
				}
				if(tt.condition==CANTEP || tt.condition==CALTEP || tt.condition==CELTEP) maxl=-1;
			}
		}
	}
	tt.condition=c; tt.action=a; ttabp->pptr=pptr; ttabp=otp;
}

isaroom(char *s) {
	register int r;

	roomtab=rmtab;
	for(r=0;r<rooms;r++,roomtab++) if(!strcmp(roomtab->id,s)) return r;
	return -1;
}

PutARankInto(char *s,int x) {
	char *p;

	you=(usr+x); you2=(lstat+x);

	if(*you2->pre) {
		p=you2->pre;
		while(*p) *(s++)=*(p++); *(s++)=' ';
	}
	p=(you->sex)?(rktab+hisRANK)->female:(rktab+hisRANK)->male;
	while(*p) *(s++)=*(p++);
	if(*you2->post) {
		p=you2->pre;
		p=you2->post; *(s++)=' '; while(*p) *(s++)=*(p++);
	}
	*s=0;
}

akillme(int ded) {
	if(MyFlag==am_DAEM || MyFlag==am_MOBS) return adestroy(me2->rec);
	if(me2->fighting != -1) clearfight();
	iocheck(); me->plays=-1; exeunt=forced=1; died=ded; donet=ml+1; ml=-1; me2->state=0;
	nohelp(); ans("1m"); sys(DIED); ans("0m"); died=ded;
}

show_tasks(int p) {
	sprintf(block,"Tasks completed by %s: ",(p==Af)?"you":pNAME(p));
	if(me->tasks) {
		register int i; char tsk[6];
		tsk[0]=0;
		for(i=0; i<32; i++) {
			if((usr+p)->tasks & (1<<i)) {
				sprintf(tsk,(*tsk)?", %d":"%d",i+1);
				strcat(block,tsk);
			}
		}
		strcat(block,".\n");
	}
	else strcat(block,"None.\n");
	tx(block);
}

dropall(int torm) {		// Drop everything to a room
	register int i;

	for(i=0; i<nouns && me2->numobj>0; i++)
		if(*(obtab+i)->rmlist == -(5+Af)) adrop(i,torm,NO);
	me2->numobj=0;
}

char *invent(int plyr,register char *p) {
	register int i,pr,j;

	p=scopy(p,"carrying ");

	if(!(lstat+plyr)->numobj) return scopy(p,"nothing. ");
	for(i=0,j=0,objtab=obtab,pr=-(5+plyr); i<nouns; i++,objtab++)
		if(*objtab->rmlist==pr && ( (myRANK>=invis2-1 || myRANK>=invis-1) || !isOINVIS(i))) {
			if(j++) p=scopy(p,", ");
			if(objtab->flags&OF_INVIS) *(p++)='*';
			p=scopy(p,objtab->id);
		}
	return scopy(p,(j)?". ":"nothing. ");
}

ascore(int type) {
	calcdext();
	if(type == TYPEV) {
		txn("Player: @me the @mr (@gn).  Games played: %ld.\nPoints: @sc points. This game: @sg.",me->plays);
		if(myRANK!=ranks-1) {
			if(mySCORE<(rktab+myRANK+1)->score) {
				sprintf(block," Next rank: %ld (%ld to go).\n",
					(rktab+myRANK+1)->score, (rktab+myRANK+1)->score - mySCORE));
			} else {
				sprintf(block," %ld task(s) to next rank.\n",(rktab+myRANK+1)->tasks-me->tasks);
			}
			tx(block);
		} else tx("\n");
		sprintf(block,"Strength: @sr/%ld  Stam: @st/%ld  Dext: %ld/%ld  Magic: @mg  Wisdom: @wi\n",
			(rktab+myRANK)->strength, (rktab+myRANK)->stamina, me->dext, (rktab+myRANK)->dext);
		tx(block);

		sprintf(block,"\nCurrent Info:\nObjects Carried: %ld/%ld,	Weight Carried: %ld/%ldg\n",
			me2->numobj, (rktab+myRANK)->numobj, me2->weight, (rktab+myRANK)->maxweight);
		tx(block);
		tx("Following: @mf.	");
		if(me2->helping!=-1) tx("Helping: @fr.  ");
		if(me2->helped!=-1)  tx("Helped by: @he.");
		if(me2->helping!=-1 || me2->helped!=-1) txc('\n');
		if(me2->wield != -1) txs("Currently wielding: %s.\n",(obtab+me2->wield)->id);
		show_tasks(Af);
	}
	else {
		sprintf(block,"Score: @sc. Str: @sr/%ld. Stam: @st/%ld. Dext: %ld/%ld. Magic: @mg/%ld\n",
			(rktab+myRANK)->strength, (rktab+myRANK)->stamina, me->dext, (rktab+myRANK)->dext, (rktab+myRANK)->magicpts);
		tx(block);
	}
}

calcdext() {
	me->dext=(rktab+me->rank)->dext;

	if(me2->flags & PFSITTING) me->dext/=2;
	else if(me2->flags & PFLYING)  me->dext/=3;
	if((LightHere==NO) || (me2->flags & PFBLIND)) me->dext/=5;

	if (IamINVIS)   me->dext+=(me->dext/10);	// Extra dexterity
	if (IamSINVIS)  me->dext+=(me->dext/15);

	me->dext-=((me->dext/3)-(((me->dext/3)*((rktab+myRANK)->maxweight-me2->weight))/(rktab+myRANK)->maxweight));

	me->dext+=me2->dextadj;
	if(me->flags&PFCRIP || me->dext<0) me->dext=0;
}

toprank(int n) {	// N = percentage chance of Rank(0) failing.
	register long i;

	n=n/(ranks-1)*(ranks-(me->rank+1));
	if(Random(100) < n) { akillme(DEDDED); return; }

	i=me->tasks; me->tasks=-1;
	aadd((rktab+ranks-1)->score-mySCORE+2, STSCORE, Af);
	me->tasks=i;
}

damage(int obj,int howmuch) {
	objtab=obtab+obj;
	if(objtab->flags&OF_SCENERY || STATE->damage<=0) return;
	if((STATE->damage-=howmuch)>0) return;
	sprintf(str,"The %s has %s to nothing. ",objtab->id,(objtab->flags&OF_SHOWFIRE)?"burnt":"broken and crumbles away");
	if(me2->wield==obj) { strcat(str,"You are no-longer wielding the @o1."); }
	strcat(str,"\n"); tx(str); adestroy(obj); me2->wield=-1; 
}

repair(int obj,int howmuch) {
	objtab=obtab+obj;
	if(objtab->flags&OF_SCENERY || STATE->strength<=0) return;
	STATE->strength+=howmuch;
}

NewLine() {			// Print NEWLINE if neccesary
	if(me2->xpos) txc('\n');
}
