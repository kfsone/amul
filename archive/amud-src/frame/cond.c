//
// AMUD/frame/Cond.C		Various conditionals
//

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
	if(*(obtab+o)->rmlist>=-1) return *(obtab+o)->rmlist;
	if(*(obtab+o)->rmlist<=-5 && *(obtab+o)->rmlist>=-(5+MAXU))
		return (int)pROOM(owner(o));
	// Else it's a container
}

carrying(int obj) {
	if(Af==MAXU && obj==me2->rec) return obj;
	if(!me2->numobj) return -1;
	if(owner(obj)==Af) return obj;
	return -1;
}

nearto(int ob) {
	sprintf(str,">Nearto (%s): ",(obtab+ob)->id); dtx(str);
	if(canseeobj(ob,Af)==NO) { dtx("No.\n"); return NO; }
	if(isin(ob,myROOM)==YES) { dtx("Yes.\n"); return YES; }
	if(carrying(ob)!=-1) { dtx("Yes.\n"); return YES; }
	dtx("No.\n"); return NO;
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

isverb(char *s) {
	register int i;
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

isvsyn(char *s) {	// Is VERB syn
	register int i; register char *p;
	p=synp;
	for(i=0; i<syns; i++,p+=strlen(p)+1) {
		if(*(synip+i)<-1 && !match(p,s)) { csyn=*(synip+i); return strlen(p); }
	}
	return (int) (csyn=-1);
}

isnsyn(char *s) {	// Is noun syn
	register int i; register char *p;
	p=synp;
	for(i=0; i<syns; i++,p+=strlen(p)+1) {
		if(*(synip+i)>-1 && !match(p,s)) { csyn=*(synip+i); return strlen(p); }
	}
	csyn=-1; return -1;
}

issyn(char *s) {
	register int i; register char *p;
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

isnoun(char *s,int adj,char *pat) {
	register int x,done_e,lsuc; int start,lobj,pass; char passx;

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

isanoun(char *s) {
	register int i; struct _OBJ_STRUCT *obpt;

	obpt=obtab;
	for(i=0; i<nouns; i++,obpt++)
		if(!(obpt->flags & OF_COUNTER) && !strcmp(s,obpt->id)) return i;
	return -1;
}

scan(int start,char Type,int tst,char *s,int adj) {
	int i,last;	register struct _OBJ_STRUCT *obpt;

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

CHAEtype(int obj) {
	register int i;
	if(carrying(obj)!=-1) return 'C';
	if(isin(obj,myROOM)==YES) return 'H';
	if((i=owner(obj))!=-1 && pROOM(i)==myROOM) return 'A';
	return 'E';
}

isadj(char *s) {
	register int i; register char *p;

	p=adtab;
	for(i=0;i<adjs;i++,p+=IDL+1) if(match(p,s)!=-1) return i;
	return -1;
}

isin(int o,int r) {
	register int i; register struct _OBJ_STRUCT *obpt;
	obpt=obtab+o;
	if(*(obpt->rmlist)<-5 && *(obpt->rmlist)>=-(5+MAXU) && pROOM(-(5+*(obpt->rmlist)))==r) return YES;
	for(i=0;i<obpt->nrooms;i++) if(*(obpt->rmlist+i)==r) return YES;
	return NO;
}

isroom(char *s) {
	register int r;

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

objvisibleto(int obj,int who) {
	register int x; register struct _OBJ_STRUCT *op;

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
