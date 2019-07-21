//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Trav_Proc.C	Process Travel.Txt
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

#include "adv:com/Trav_Func.C"	// Miscellaneous functions

trav_proc() {
	int strip,lines;
	register int nvbs,i,ntt,t,r;
	register char *p,*s; long *l;

	ntt=t=0; nextc(1); fopenw(ttfn); fopenw(ttpfn); fopena(rooms1fn);
	cleanget(&datal,&data,0L); p=skipspc(data); tx("Proc:");

	do {
loop1:		p=skipline(s=p); if(!*s) continue;
		s=skipspc(s); if(!*s) goto loop1; getword(skiplead("room=",s));
		if((rmn=isroom(Word))==-1) {
			error("Undefined room: %s\n",Word); p=skipdata(p); goto loop1;
		}
		if(roomtab->tabptr!=-1) {
			error("%s: Travel already defined.\n",Word); p=skipdata(p); goto loop1;
		}
vbloop:		p=skipline(s=p);
		if(!*s) {
			// Only complain if room is not a death room
			if((roomtab->flags & DEATH)!=DEATH)
				warne("%s: No T.T entries!\n",roomtab->id);
			roomtab->tabptr=-2; ntt++; continue;
		}
		s=skipspc(s); if(!*s) goto vbloop;
		if(strncmp("verbs=",s,6)) {
			error("%s: Missing verbs= line.\n",roomtab->id); goto vbloop;
		}
		s+=6; lines=0; verb.id[0]=0;
		roomtab->tabptr=t;roomtab->ttlines=0;
vbproc:		// Process verb list
		nvbs=0; tt.pptr=(long *)-1;
		l=(long *)temp;
		// Break verb list down to verb no.s
		do {
			s=getword(s); if(!*Word) break;
			if((*l=is_verb(Word))==-1)
				error("%s: Invalid %s \"%s\"\n",(rmtab+rmn)->id,"verb",Word);
			l++; nvbs++;
		} while(*Word);
		if(!nvbs) {
			error("%s: No verbs specified after verbs=!\n",(rmtab+rmn)->id);
		}
		// Now process each instruction line
		do {
xloop:			strip=0; r=-1;
xloop2:			p=skipline(s=p); if(!*s) { strip=-1; continue; }
			s=skipspc(s); if(!*s) goto xloop;
			if(!strncmp("verbs=",s,6)) { strip=1; s+=6; break; }
			s=precon(s);		// Strip pre-condition opts
notloop:		if(*s=='!') { r=-1*r; s++; } s=getword(s);
			if(!strcmp(Word,ALWAYSEP)) {
				tt.condition=CALWAYS; tt.action=-(1+AENDPARSE);
				goto write;
			}
			if(!strcmp(Word,"not")) {
				r=-1*r; goto notloop;
			}
notlp2:			if((tt.condition=iscond(Word)) == -1) {
				tt.condition=CALWAYS;
				if((tt.action=isroom(Word))!=-1) goto write;
				if((tt.action=isact(Word))==-1) {
					error("%s: Invalid %s \"%s\"\n",
						(rmtab+rmn)->id,"condition",Word);
					goto xloop;
				}
				goto gotohere;
			}
			s=skipspc(s);
			if(!(s=chkcparms(s,tt.condition,ofp2))) goto next;
			if(r==1) tt.condition=~tt.condition;
			if(!*s) {
				error("%s: C&A line has missing action.\n",(rmtab+rmn)->id);
				goto xloop;
			}
			s=preact(s); s=getword(s);
			if((tt.action=isroom(Word))!=-1) goto write;
			if((tt.action=isact(Word))==-1) {
				error("%s: Invalid %s \"%s\"\n",
					(rmtab+rmn)->id,"action",Word);
				goto xloop;
			}
gotohere:		if(tt.action==ATRAVEL) {
				error("%s: Action 'TRAVEL' not allowd!\n");
				goto xloop;
			}
			s=skipspc(s);
			if(!(s=chkaparms(s,tt.action,ofp2))) goto next;
			tt.action=~tt.action;
write:			roomtab=rmtab+rmn;
			l=(long *)temp;
			for(i=0; i<nvbs; i++) {
				if(i<nvbs-1) tt.pptr=(long *)-2; else tt.pptr=(long *)-1;
				tt.verb=*(l++);
				fwrite((char *)&tt.verb,sizeof(tt),1,ofp1);
				roomtab->ttlines++; t++; ttents++;
			}
			lines++;
next:			strip=0;
		} while(!strip && *p);
		if(strip==1 && *p) goto vbproc;
		ntt++;
	} while(*p);
	if(!err && ntt!=rooms && warn==1) {
		roomtab=rmtab;
		for(i=0; i<rooms; i++,roomtab++)
			if(roomtab->tabptr==-1 && (roomtab->flags & DEATH)!=DEATH)
				warne("No exits for %s\n",roomtab->id);
	}
	ttroomupdate(); errabort();
}
