//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/Cond.C	Various condition routines
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

cond(long n,int l) {		// Execute a C&A condition
	int mult,ret,i; register int cp1;

	mult=1; ret=1; if(n<0) { n=~n; mult=-1; }; tt.condition=n;
	if(ncop[tt.condition]>0) cp1=CP1;

	switch(n) {
		case CALTEP:
		case CSTAR:
		case CALWAYS:	break;
		case CANTEP:
		case CAND: ret=l; break;
		case CELTEP:
		case CELSE: ret=-l; break;
		case CLIGHT:
			if(LightHere==NO) ret=-1; break;
		case CISHERE:
			if(isin(cp1,myROOM)==NO) ret=-1; break;
		case CMYRANK:
			if(numb(myRANK+1,cp1) == NO) ret=-1; break;
		case CSTATE:
			if((obtab+cp1)->flags & OF_ZONKED || *(obtab+cp1)->rmlist==-1) ret=-1;
			if(numb((obtab+cp1)->state,CP2) == NO) ret=-1; break;
		case CMYSEX:
			if(me->sex != cp1) ret=-1; break;
		case CLASTVB:
			if(lverb != cp1) ret=-1; break;
		case CLASTDIR:
			if(ldir != cp1) ret=-1; break;
		case CLASTROOM:
			if(lroom != cp1) ret=-1; break;
		case CASLEEP:
			if(!(me2->flags & PFASLEEP)) ret=-1; break;
		case CSITTING:
			if(!(me2->flags & PFSITTING)) ret=-1; break;
		case CLYING:
			if(!(me2->flags & PFLYING)) ret=-1; break;
		case CRAND:
			if(numb(Random(cp1),*(tt.pptr+1)) == NO) ret=-1; break;
		case CRDMODE:
			if(me->rdmode != cp1) ret=-1; break;
		case CONLYUSER:
			for(i=0; i<MAXU; i++)
			if(*pNAME(i) && (lstat+i)->state>1) ret=-1; break;
		case CALONE:
			for(i=0; i<MAXU; i++)
			if((pROOM(i) == myROOM) && i != Af) ret=-1; break;
		case CINROOM:
			if(myROOM != cp1) ret=-1; break;
		case COPENS:
			if(!((obtab+cp1)->flags&OF_OPENS)) ret=-1; break;
		case CGOTNOWT:
			if(me2->numobj) ret=-1; break;
		case CCARRYING:
			if(gotin(cp1,-1)==NO) ret=-1; break;
		case CNEARTO:
			if(nearto(cp1)==NO) ret=-1; break;
		case CHIDDEN:
			if(visible()==YES) ret=-1; break;
		case CCANGIVE:
			if(cangive(cp1,CP2)==NO) ret=-1; break;
		case CINFL:
		case CINFLICTED:
			if(infl(cp1,CP2)==NO) ret=-1; break;
		case CSAMEROOM:
			if(pROOM(cp1) != myROOM) ret=-1; break;
		case CTOPRANK:
			if(myRANK != ranks-1) ret=-1; break;
		case CSOMEONEHAS:
			if(((*(obtab+cp1)->rmlist) > -5) || ((*(obtab+cp1)->rmlist) < (-5-MAXU))) ret=-1; break;
		case CGOTA:
			if(gotin(cp1,CP2)==NO) ret=-1; break;
		case CACTIVE:
			SendIt(MCHECKD,cp1,NULL); if(Ad==-1) ret=-1; break;
		case CTIMER:
			SendIt(MCHECKD,cp1,NULL); if(Ad==-1 || numb(Ap1,CP2)==NO) ret=-1; break;
		case CBURNS:
			if(!((obtab+cp1)->flags&OF_FLAMABLE)) ret=-1; break;
		case CCONTAINER:
			if((obtab+cp1)->contains<=0) ret=-1; break;
		case CEMPTY:
			if((obtab+cp1)->inside) ret=-1; break;
		case COBJSIN:
			if(numb((obtab+cp1)->inside,CP2) == NO) ret=-1; break;
		case CHELPING:
			if(me2->helping != cp1) ret=-1; break;
		case CGOTHELP:
			if(me2->helped == -1) ret=-1; break;
		case CANYHELP:
			if(me2->helping == -1) ret=-1; break;
		case CSTAT:
			if(stat(CP2,cp1,CP3)==NO) ret=-1; break;
		case COBJINV:
			if(!isOINVIS(cp1)) ret=-1; break;
		case CFIGHTING:
			if(!((lstat+cp1)->flags&PFFIGHT)) ret=-1; break;
		case CTASKSET:
			if(!(me->tasks & (1<<(cp1-1)))) ret=-1; break;
		case CCANSEE:
			if(cansee(Af,cp1)==NO) ret=-1; break;
		case CVISIBLETO:
			if(cansee(cp1,Af)==NO) ret=-1; break;
		case CNOUN1:
			if(wtype[1]!=WNOUN) { ret=-1; break; }
			if(cp1==inoun1) break;
			if(strcmp((obtab+cp1)->id,(obtab+inoun1)->id)) ret=-1;
			break;
		case CNOUN2:
			if(wtype[3]!=WNOUN) { ret=-1; break; }
			if(strcmp((obtab+cp1)->id,(obtab+inoun2)->id)) ret=-1;
			break;
		case CAUTOEXITS:
			if(!autoexits) ret=-1; break;
		case CDEBUG:
			if(!debug) ret=-1; break;
		case CFULL:
			if(stfull(cp1,CP2) == NO) ret=-1; break;
		case CTIME:
			if(numb(*rescnt,cp1) == NO) ret=-1; break;
		case CDEC:
			if(((obtab+cp1)->flags & OF_ZONKED) || (obtab+cp1)->state <= 0) { ret=-1; break; }
			asetstat(cp1, (obtab+cp1)->state-1); break;
		case CINC:
			if(((obtab+cp1)->flags & OF_ZONKED) || (obtab+cp1)->state< 0 || (obtab+cp1)->state >= ((obtab+cp1)->nstates-1)) { ret=-1; break; }
			asetstat(cp1, (obtab+cp1)->state+1); break;
		case CLIT:
			if(!((obtab+cp1)->flags&OF_FLAMABLE)) ret=-1;
			if(!(State(cp1)->flags & SF_LIT)) ret=-1; break;
		case CFIRE:
			if(! ((obtab+cp1)->flags & OF_SHOWFIRE) ) ret=-1; break;
		case CHEALTH:
			if(numb((((usr+cp1)->stamina*100)/(rktab+pRANK(cp1))->stamina),CP2) == NO) ret=-1; break;
		case CMAGIC:
			if(magic(cp1,CP2,CP3)==NO) ret=-1; break;
		case CSPELL:
			if(numb((usr+cp1)->wisdom,Random(CP2))==NO) ret=-1; break;
		case CIN:
			if(isin(CP2,cp1)==NO) ret=-1; break;
		case CEXISTS:
			if(State(cp1)<0 || (obtab+cp1)->flags&OF_ZONKED || *(obtab+cp1)->rmlist==-1) ret=-1; break;
		case CWILLGO:
			if(!willgointo(cp1,CP2)) ret=-1; break;
		default: ret=-1;
	}

	return mult*ret;
}

lit(int r) {		// Is my room lit?
	register int i,j; struct _OBJ_STRUCT *tobjtab;

	tobjtab=objtab;

	if(!((rmtab+r)->flags & DARK)) return YES;
	Forbid();
	you2=lstat;
	for(i=0; i<MAXU; i++,you2++)
		if(hisROOM==r && hisHLIGHT) { Permit(); return YES; }
	objtab=obtab;
	for(i=0; i<nouns; i++,objtab++) {
		if((STATE->flags & SF_LIT)!=SF_LIT) continue;
		for(j=0; j<objtab->nrooms; j++)
			if(*(objtab->rmlist+j)==r) { objtab=tobjtab; Permit(); return YES; }
	}
	objtab=tobjtab; Permit(); return NO;
}

loc(int o) {
	register struct _OBJ_STRUCT *op;
	op=obtab+o;
	if((op->flags&OF_ZONKED) || *op->rmlist==-1 || op->state<0) return -1;
	if(*op->rmlist>=-1) return *op->rmlist;
	if(*op->rmlist<=-5 && *op->rmlist>=-(5+MAXU))
		return (int)pROOM(owner(o));
	// Else it's in a container
	return loc(-(INS+*op->rmlist));
}

carrying(int obj) {
	if(Af==MAXU && obj==me2->rec) return obj;
	if(!me2->numobj) return -1;
	if(owner(obj)==Af) return obj;
	return -1;
}

nearto(int ob) {
	if(canseeobj(ob,Af)==NO) return NO;
	if(isin(ob,myROOM)==YES) return YES;
	if(carrying(ob)!=-1) return YES;
	return NO;
}

visible() {		// Can others in this room see me?
	if(LightHere==NO) return NO;
	if(IamINVIS || IamSINVIS) return NO;
	return YES;
}

cangive(int obj,int plyr) {	// If player could manage to carry object
	objtab=obtab+obj;
	if((lstat+plyr)->weight+STATE->weight > (rktab+pRANK(plyr))->maxweight) return NO;
	if((lstat+plyr)->numobj+1 > (rktab+pRANK(plyr))->numobj) return NO;
	if((objtab->flags&OF_SCENERY) || (objtab->flags&OF_COUNTER) || objtab->nrooms!=1) return NO;
	return YES;
}

isverb(char *s)
{	register int i;
	vbptr=vbtab;
	for(i=0;i<verbs;i++,vbptr++) if(!match(vbptr->id,s)) return i;
	return -1;
}

isaverb(char **s) {
	register int ret;

	if((ret=isverb(*s))!=-1) { (*s)+=strlen((vbtab+ret)->id); return ret; }
	if((ret=isvsyn(*s))!=-1) { (*s)+=ret; return -2-csyn; }
	return -1;
}

isvsyn(char *s)	// Is VERB syn
{	register int i; register char *p;
	p=synp;
	for(i=0; i<syns; i++,p+=strlen(p)+1) {
		if(*(synip+i)<-1 && !match(p,s)) { csyn=*(synip+i); return strlen(p); }
	}
	return (int) (csyn=-1);
}

isnsyn(char *s)	// Is noun syn
{	register int i; register char *p;
	p=synp;
	for(i=0; i<syns; i++,p+=strlen(p)+1) {
		if(*(synip+i)>-1 && !match(p,s)) { csyn=*(synip+i); return strlen(p); }
	}
	csyn=-1; return -1;
}

issyn(char *s)
{	register int i; register char *p;
	p=synp;
	for(i=0; i<syns; i++,p+=strlen(p)+1) {
		if(*p==*s && !match(p,s)) { csyn=*(synip+i); return strlen(p); }
	}
	csyn=-1; return -1;
}

//	Notice:
//	------
//	Due to the complexity of a multi-ocurance/state environment, I gave up
//	trying to do a 'sort', storing the last, nearest object, and went for
//	an eight pass seek-and-return parse. This may be damned slow with a
//	few thousand objects, but if you only have a thousand, it'll be OK!

isnoun(char *s,int adj,char *pat)
{	register int x,done_e,lsuc; int start,lobj,pass; char passx;

	if(iverb!=-1) {
		verb.sort[0]=*pat; strncpy(verb.sort+1,pat+1,3);
	}
	else { strcpy(verb.sort+1,"CHE"); verb.sort[0]=-1; }
	done_e=0; lsuc=lobj=-1; start=isanoun(s);
	if((obtab+start)->adj==adj && CHAEtype(start)==verb.sort[1] && (obtab+start)->state==verb.sort[0]) return start;
	for(pass=1; pass<4; pass++) {
		// At this point, we out to try BOTH phases, however, in the
		// second phase, try for the word. Next 'pass', if there is
		// no suitable item, drop back to that from the previous...
		if((passx=verb.sort[pass])=='X') return lobj;
		if(verb.sort[0]==-1) {
			if((x=scan(start,passx,-1,s,adj))!=-1) {
				if(adj!=-1 || (obtab+x)->adj==adj) return x;
				if(!lsuc) return lobj;
				lsuc=0; lobj=x;
			}
			else if(!lsuc) return lobj;
		}
		else {
			// Did we get a match?
			if((x=scan(start,passx,0,s,adj))!=-1) {
				if(adj != -1 || (obtab+x)->adj==adj) return x;
				if(!lsuc) return lobj; lobj=x; lsuc=0; continue;
			}
			if((x=scan(start,passx,-1,s,adj))!=-1) {
				if(adj != -1 || (obtab+x)->adj==adj)  { lobj=x; lsuc=0; continue; }
				if(!lsuc) return lobj;
			}
			if(!lsuc) return lobj;
		}
		if(passx=='E') done_e=1;
	}
	if(done_e) return lobj;
	return scan(0,'E',-1,s,adj);
}

isanoun(char *s)
{	register int i; struct _OBJ_STRUCT *obpt;

	obpt=obtab;
	for(i=0; i<nouns; i++,obpt++)
		if(!(obpt->flags & OF_COUNTER) && !strcmp(s,obpt->id)) return i;
	return -1;
}

scan(int start,char Type,int tst,char *s,int adj)
{	int i,last;	register struct _OBJ_STRUCT *obpt;

	last=-1; obpt=obtab+start;
	for(i=start; i<nouns; i++,obpt++) {
		if((obpt->flags & OF_COUNTER) || (adj!=-1 && obpt->adj!=adj))  continue;
		if(match(obpt->id,s) || CHAEtype(i)!=Type) continue;
		// If state doesn't matter or states match, we'll try it
		if(verb.sort[0]==-1 || tst==-1 || obpt->state==verb.sort[0]) {
			if(adj==obpt->adj) return i;
			else last=i;
		}
		else if(last==-1) last=i;
	}
	return last;
}

CHAEtype(int obj)
{	register int i;
	if(carrying(obj)!=-1) return 'C';
	if(isin(obj,myROOM)==YES) return 'H';
	if((i=owner(obj))!=-1 && pROOM(i)==myROOM) return 'A';
	return 'E';
}

isadj(char *s)
{	register int i; register char *p;

	p=adtab;
	for(i=0;i<adjs;i++,p+=IDL+1) if(match(p,s)!=-1) return i;
	return -1;
}

isin(int o,int r)
{	register int i; register struct _OBJ_STRUCT *obpt;
	obpt=obtab+o;
	if(*(obpt->rmlist)<-5 && *(obpt->rmlist)>=-(5+MAXU) && pROOM(-(5+*(obpt->rmlist)))==r) return YES;
	for(i=0;i<obpt->nrooms;i++) if(*(obpt->rmlist+i)==r) return YES;
	return NO;
}

isroom(char *s)
{	register int r;

	for(r=0;r<rooms;r++) if(!stricmp((rmtab+r)->id,s)) return r;
	return -1;
}

infl(int plyr,int spell) {
	you2=lstat+plyr;
	switch(spell) {
		case SGLOW:	if(you2->flags & PFGLOW) return YES; break;
		case SINVIS:	if(you2->flags & PFINVIS) return YES; break;
		case SDEAF:	if(you2->flags & PFDEAF) return YES; break;
		case SBLIND:	if(you2->flags & PFBLIND) return YES; break;
		case SCRIPPLE:	if(you2->flags & PFCRIP) return YES; break;
		case SDUMB:	if(you2->flags & PFDUMB) return YES; break;
		case SSLEEP:	if(you2->flags & PFASLEEP) return YES; break;
		case SSINVIS:	if(you2->flags & PFSINVIS) return YES; break;
	}
	return NO;
}

stat(int plyr,int st,int x) {
	switch(st) {
		case STSTR:	return numb((usr+plyr)->strength,x);
		case STSTAM:	return numb((usr+plyr)->stamina,x);
		case STEXP:	return numb((usr+plyr)->experience,x);
		case STWIS:	return numb((usr+plyr)->wisdom,x);
		case STDEX:	return numb((usr+plyr)->dext,x);
		case STMAGIC:	return numb((usr+plyr)->magicpts,x);
		case STSCTG:	return numb((lstat+plyr)->sctg,x);
	}
}

cansee(int p1,int p2){
	if(p2 >= MAXU ) return canseeobj((lstat+p2)->rec,p1);
	if(!*pNAME(p2) || p1==p2) return NO;
	if((lstat+p2)->state<PLAYING) return NO;
	if(pROOM(p1) != pROOM(p2)) return NO;
	if(lit(pROOM(p1)) == NO) return NO;
	if(isPSINVIS(p2)) return NO;
	if((lstat+p1)->flags & PFBLIND) return NO;
	if(pRANK(p1) == ranks-1) return YES;
	if(((rmtab+pROOM(p1))->flags & HIDE)) return NO;
	if(!isPINVIS(p2)&&!isPSINVIS(p2)) return YES;
	// Otherwise
	if((isPSINVIS(p1) || isPINVIS(p1)) && pRANK(p1) >= invis-1) return YES;
	if(pRANK(p1) >= invis2-1) return YES;
	else return NO;
}

canseeobj(int obj,int who){
	if(isin(obj,(lstat+who)->room)==NO && owner(obj)!=who) return NO;
	if(objvisibleto(obj,who)==NO) return NO; return YES;
}

objvisibleto(int obj,int who)
{	register int x; register struct _OBJ_STRUCT *op;

	op=obtab+obj; if((op->flags&(OF_COUNTER+OF_ZONKED))) return NO;
	if((op->flags & OF_SCENERY)) x=YES;
	else{
		if(isin(obj,(lstat+who)->room)==YES) x=lit((lstat+who)->room);
		else x=lit(*op->rmlist);
	}
	if((op->flags & OF_SMELL)){
		if(x==NO || ((lstat+who)->flags & PFBLIND)) return YES;
		dtx(">Can't smell.\n"); return NO;
	}
	if(x==NO && owner(obj)!=who || ((lstat+who)->flags & PFBLIND)) return NO;
	if(!isOINVIS(obj)) return YES;
	if((isPSINVIS(who) || isPINVIS(who)) && pRANK(who) >= invis-1) return YES;
	if(pRANK(who) < invis2-1) return NO;
	return YES;
}

// Test if object will go into (hopefully) container
willobjgo(register int obj,register int into) {
	register struct _OBJ *op,*cp;	// Object Ptr and Container Ptr
	register int i,w;

	// Is it a container?
	if((cp=obtab+into)->contains <= 0) return FALSE;
	// Is OBJ a moveable object?
	if((op=obtab+into)->flags & (OF_SCENERY+OF_COUNTER)) return FALSE;
	// Is OBJ multi-located or already inside INTO
	if(op->nrooms<>1 || *(op->rmlist)==into)) return FALSE;
	// If it needs to be, is it open?
	if((cp->flags&OF_OPENS) && !(ItsState(cp))) return FALSE;
	// Is there enough room?
	if((ItsState(op=obtab+obj))->weight + cp->winside > cp->contains)
		return FALSE;
	return TRUE;
}

