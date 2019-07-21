//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/Parser.C	Various Parser-related functions
//
//	LMod: oliver 17/06/93	Added cdcheck()
//	      oliver 11/06/93	AMUL->AGL
//

grab() {
	register char *s,*d,*p,c;	// Source & Destination

	s=input; more=2; forced=0; exeunt=0; failed=NO; died=0; donet=-1; ml=0;

	do {
		d=block; block[0]=0;
loop:		*d=0; while(isspace(*s)) s++;		// Skip spaces
		if(!*s) { *(s+1)=0; goto proc; }
quotes:		if(block[0]) *(d++)=' ';		// Force spacing
		if(*s=='\'' || *s=='\"') {
			c=*(d++)=*(s++);		// Store which
			while(*s!=c && *s)		// Find match or ASCIZ
				*(d++)=*(s++);
			if(!*s) *(s+1)=0;
			*(d++)=*(s++); *d=0;
			if(*s) s++;			// Skip " or ' at end
			goto loop;
		}

		p=d;
		while(*s && !isspace(*s) && *s!='!' && *s!=';' && *s!=',' && *s!='.' && *s!='\"' && *s!='\'') *(d++)=*(s++);
		*d=0; *(d+1)=0;
		if(!strcmp(p,"then") || !strcmp(p,"and")) { *p=0; goto proc; }
		if(*s=='\'' || *s=='\"') goto quotes;
		if(isspace(*s)) goto loop;
proc:		if(*s && *s!='\'' && *s!='\"') s++;
		if(!block[0]) continue;
		if(more!=2) {
			ans("3m"); tx((rktab+myRANK)->prompt); txs("%s\n",block); ans("0m");
		}
		if(parser()==-1) return;
	} while(*s && !more && !exeunt && !forced && failed==NO && !died);
	iocheck();
}

parser() {
	char *p; register int x,om=more;

	if(!strlen(block)) return; more=0; 

	iocheck(); if(forced || exeunt || died) return;

phrase:	wtype[0]=wtype[1]=wtype[2]=wtype[3]=WNONE;
	iadj1=inoun1=iadj2=inoun2=WNONE; actor=-1;
	p=block+strlen(block)-1;
	while(p!=block && isspace(*p)) *(p--)=0;
	if(minsgo>0 && myRANK >= (minsgo-1) && (x=isroom(block))!=-1) {
		action(acp(SGOVANISH),ACANSEE);
		StopFollow(); LoseFollower(); sys(SGO); moveto(x);
		action(acp(SGOAPPEAR),ACANSEE);
		return;
	}
	p=block;
	if(*p=='\"' || *p=='\'') {
		register char *p2;
		if(!*(p+1)) return;
		if((iverb=isverb("!speech"))==-1) { sys(CANTDO); return -1; }
		if(isverb("say")!=-1) iverb=isverb("say");
		p2=p+1;
loop:		while(*p != *p2 && *p2) p2++;
		*(p2+1)=*(p2)=0; inoun1=(long) p+1; wtype[1]=WTEXT;
		goto skip;
	}
	if((word=isaverb(&p)) == -1) {
		char *bp=p;
		if(om==10 || (x=type(&bp)) == -2) {
			sys(INVALIDVERB); more=1; return -1;
		}
		word=iverb;
	}
	x=WVERB; vbptr=vbtab+word;
	if((me2->flags & PFASLEEP) && !(vbptr->flags & VB_DREAM)) {
		tx("You can't do anything until you wake up!\n"); failed=YES; return -1;
	}
	if(vbptr->flags&VB_TRAVEL) {
		if((x=isverb("!travel"))!=-1) {
			vbptr=vbtab+word; if(!lang_proc(x,1)) return;
		}
		vbptr = vbtab+word;
	}
	if(iverb>=0) lverb=iverb;
	iverb=word; vbptr=vbtab+iverb;

l1:	if(!*p) goto ended;	// Adjectives are optional, so assume NOUN
	wtype[1]=type(&p); inoun1=word;
	if(wtype[1]==WADJ) {
		if(wtype[0]!=WNONE) { sys(NONOUN); return -1; }
		wtype[0]=WADJ; iadj1=inoun1; wtype[1]=inoun1=WNONE; goto l1;
	}
	if(wtype[1]==WNOUN) it=inoun1;
l2:	if(!*p) goto ended;
	wtype[3]=type(&p); inoun2=word;
	if(wtype[3]==WADJ) {
		if(wtype[2]!=WNONE) { sys(NONOUN); return -1; }
		wtype[2]=WADJ; iadj2=inoun2; wtype[3]=inoun2=WNONE; goto l2;
	}
	if(wtype[3]==WNOUN) it=inoun2;
ended:	overb=iverb; vbptr=vbtab+iverb;
skip:	iocheck(); if(forced || exeunt || died || failed!=NO) return;
	sethimher(wtype[1],inoun1); sethimher(wtype[3],inoun2);
	return lang_proc(iverb,0);
}

sethimher(int t,int pn) {
	if(t==WPLAYER && pn>=0 && pn!=Af) {
		if((usr+pn)->sex) last_her=pn; else last_him=pn;
	}
}

lang_proc(int v,char e) {
	int i,j,l,m,d;	register int jswt,jwt,jsl;

	forced=0; exeunt=0; failed=NO; died=0; donet=0; ml=0; d=-2; tt.verb=-1; vbptr=vbtab+v;
caloop:	for(i=0; i<(vbtab+v)->ents; i++) {
		m=0; stptr=vbptr->ptr+i; donet=0; ml=stptr->ents;
		if(!(stptr->wtype[1]==WANY && stptr->wtype[3]==WANY)) for(j=0; j<4 && !m; j++) {
			if((jswt=stptr->wtype[j])==WANY) continue;
			if(jswt!=(jwt=wtype[j])) { m=1; continue; }
			// We have a match, now see if its the same word!
			if((jsl=stptr->slot[j])==WANY) continue;
			switch(j) {
				case 0: if(iadj1!=jsl) m=1; break;
				case 1:	if(jsl==WNONE && inoun1==WNONE) break;
					if(jswt==WPLAYER && inoun1==Af && jsl==-3) break;
					if(jswt==WTEXT   && !stricmp((char *)inoun1,umsgp+*(umsgip+jsl))) break;
					if(jswt==WNOUN   && !strcmp((obtab+inoun1)->id,(obtab+jsl)->id)) break;
					if(inoun1!=jsl) m=1; break;
				case 2: if(iadj2!=jsl) m=1; break;
				case 3:	if(jsl==WNONE && inoun2==WNONE) break;
					if(jswt==WPLAYER && inoun2==Af && jsl==-3) break;
					if(jswt==WTEXT   && !stricmp((char *)inoun2,umsgp+*(umsgip+stptr->slot[j]))) break;
					if(jswt==WNOUN   && !strcmp((obtab+inoun2)->id,(obtab+jsl)->id)) break;
					if(inoun2!=jsl) m=1; break;
			}
		}
		if(m) goto after;
		l=-1; d=-1;
		for(donet=0; donet < ml; donet++) {
			cdcheck();		// 17/06/93 oliver
			stptr=vbptr->ptr+i; vtabp=stptr->ptr+donet;
			tt.action=vtabp->action; tt.pptr=vtabp->pptr;
			if(skip) { skip--; continue; }
			if((l=cond(vtabp->condition,l))==-1) continue;
			inc=1; act(tt.action,vtabp->pptr); if(inc) d=0;
			if(ml < -1) { d=lang_proc(iverb,e); return d; }
			if(ml<0 || failed!=NO || forced || died || exeunt) break;
		}
		if(failed!=NO || forced || died || exeunt) break;
after:		if(donet>ml) break;
	}
	if(d>-1) return 0;	// If we processed something.

	vbptr=vbtab+v;
	if(vbptr->flags & VB_TRAVEL)
	{	register int iv=iverb;
		iverb=v; if(ttproc()) d=-1; else return 0;
		iverb=iv;
	}
	if(d==-2 && !e) sys(ALMOST);
	if(d==-1 && !e) {
		if(vbptr->flags&VB_TRAVEL) sys(CANTGO); else sys(CANTDO);
	}
	return -1;
}
