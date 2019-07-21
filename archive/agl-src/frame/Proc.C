//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/Proc.C	Miscellaneous processing routines
//
//	LMod: oliver 19/06/93	aquit(ASK|FORCE) and aextend(n)
//	      oliver 11/06/93	AMUL->AGL
//

ttproc() {
	int i,match,dun,l; register struct _TT_ENT *tp;

	exeunt=died=donet=skip=0; failed=NO;
	roomtab=rmtab+myROOM; l=-1; tt.verb=iverb;
	if(roomtab->tabptr==-1) { sys(CANTGO); return 0; }
	dun=-1; ml=roomtab->ttlines; tp=ttp+roomtab->tabptr;

	if(Af<MAXU) iocheck(); if(forced || died || exeunt) return 0;
more:	i=donet;
	for(i=donet; i<ml; i++) {
		ttabp=tp+i; match=-1; donet++; tt.pptr=ttabp->pptr;
		if(skip) { skip--; continue; }
		if(ttabp->verb==iverb && (l=cond(ttabp->condition,l))!=-1) {
			match=i; break;
		}
	}
	skip=0;
	if(ttabp->condition==CSPELL && match==-1) return 0;
	if(match==-1) return dun;
	tt.condition=ttabp->condition;
	inc=1; act(ttabp->action,ttabp->pptr); if(inc==1) dun=0;
	if(ml<-1) { ml=roomtab->ttlines; donet=0; }
	if(donet<ml && !exeunt && !died) goto more;
	return 0;
}

act(long ac,register long *tpt) {
	long x1,x2;
	tt.action=ac; if(tt.condition<0) tt.condition=-1-tt.condition;
	tt.pptr=tpt; tpt+=ncop[tt.condition];

	if(ac<0) {		// Is it an action?
		ac=-1-ac;
		if(MyFlag==am_DAEM) switch(ac) {
			case ASAVE:
			case ASCORE:
			case AQUIT:
			case ALOOK:
			case ACHANGESEX:
			case ARDMODE:
			case AEXITS:
			case ASETPRE:
			case ASETPOST:
			case ASETARR:
			case ASETDEP:
			case ATOPRANK:
			case AWAIT: return;
			case AMOVE: acmove(tP1); break;
		}
		switch(ac) {
			case ASAVE:		asave();	break;
			case ASCORE:		ascore(tP1);	break;
			case ASETSTAT:		asetstat(tP1,tP2);	break;
			case ALOOK:		look((rmtab+myROOM)->id, 1);break;
			case AWHAT:		list_what(myROOM,1);break;
			case AWHERE:		awhere(tP1);	break;
			case AWHO:		awho(tP1);	break;
			case ATREATAS:		atreatas(tP1);	return;
			case ASKIP:		skip+=tP1; break;
			case ATRAVEL:		x1=donet; x2=ml; if(ttproc()) { ml=x2; donet=x1; } else donet=ml=x2; break;
			case AQUIT:		aquit(tP1);
			case AENDPARSE:		donet=ml+1; 	break;
			case AKILLME:		akillme(DED);	break;
			case AFAILPARSE:	afailparse();	break;
			case AFINISHPARSE:	afinishparse();	break;
			case AABORTPARSE:	aabortparse();	break;
			case AWAIT:		fwait(tP1);	break;
			case AWHEREAMI:		txs("Current room is %s.\n",(rmtab+myROOM)->id); break;
			case ASEND:		send(tP1,tP2); 	break;
			case AERROR:		afailparse();
			case ARESPOND:		donet=ml+1; // then PRINT
			case APRINT:		tx(aP1); break;
			case AANOUN:		announce(aP2,tP1); break;
			case ACHANGESEX:	achange(tP1);	break;
			case ASIT:		me2->flags = me2->flags | PFSITTING; me2->flags = me2->flags & (-1-PFLYING); break;
			case ASTAND:		me2->flags = me2->flags & (-1-PFSITTING-PFLYING); break;
			case ALIE:		me2->flags = me2->flags | PFLYING; me2->flags = me2->flags & (-1-PFSITTING); break;
			case ARDMODE:		me->rdmode = tP1; txs("%s mode selected.\n",(me->rdmode==RDRC)?"Roomcount":(me->rdmode==RDVB)?"Verbose":"Brief"); break;
			case ARESET:		SendIt(MRESET,0,NULL);	break;
			case AACTION:		action(aP2,tP1);	break;
			case AMOVE:		moveto(tP1); roomtab=rmtab+me2->room; get_dmove(); break;
			case AMSGIN:		announcein(tP1,aP2); break;
			case AACTIN:		actionin(tP1,aP2);   break;
			case AMSGFROM:		announcefrom(tP1,aP2); break;
			case AACTFROM:		actionfrom(tP1,aP2);   break;
			case ATELL:		if(!((lstat+tP1)->flags&PFDEAF)) { setmxy(NOISE,tP1); utx(tP1,aP2); } break;
			case AGET:		agive(tP1,Af);	break;
			case ADROP:		adrop(tP1,myROOM,YES); break;
			case AINVENT:		invent(Af,block); txs("You are %s\n",block); break;
			case AGIVE:		agive(tP1,tP2);	break;
			case AINFLICT:		inflict(tP1, tP2); break;
			case ACURE:		cure(tP1, tP2);	break;
			case ASUMMON:		summon(tP1);	break;
			case AADD:		aadd(tP1,tP2,tP3); break;
			case ASUB:		asub(tP1,tP2,tP3); break;
			case ACHECKNEAR:	achecknear(tP1); break;
			case ACHECKGET:		acheckget(tP1);	break;
			case ADESTROY:		adestroy(tP1);	break;
			case ARECOVER:		arecover(tP1);	break;
			case ASTART:		dpstart(tP1,tP2); break;
			case AGSTART:		dgstart(tP1,tP2); break;
			case ACANCEL:		SendIt(MDCANCEL,tP1,NULL); break;
			case ABEGIN:		dbegin(tP1);	break;
			case ASHOWTIMER:	dshow(tP1);	break;
			case AOBJAN:		objannounce(tP1,aP2); break;
			case AOBJACT:		objaction(tP1,aP2); break;
			case ACONTENTS:		str[0]=0; showin(tP1,YES); txs("%s\n",str); break;
			case AFORCE:		aforce(tP1,tP2); break;
			case AHELP:		me2->helping=tP1; (lstat+tP1)->helped=Af; break;
			case ASTOPHELP:		(lstat+me2->helping)->helped=-1; me2->helping=-1; break;
			case AFIX:		afix(tP1,tP2);	break;
			case AOBJINVIS:		(obtab+tP1)->flags = (obtab+tP1)->flags | OF_INVIS; break;
			case AOBJSHOW:		(obtab+tP1)->flags = (obtab+tP1)->flags & (-1-OF_INVIS); break;
			case AFIGHT:		afight(tP1);	break;
			case AFLEE:		dropall(pROOM(me2->fighting)); clearfight(); break;
			case ALOG:		log(aP1);	break;
			case ACOMBAT:		acombat();	break;
			case AWIELD:		me2->wield = tP1; break;
/* - */			case AFOLLOW:		(lstat+tP1)->followed = Af; me2->following = tP1; break;
/* - */			case ALOSE:		LoseFollower(); break;
/* - */			case ASTOPFOLLOW:	StopFollow(); break;
			case AEXITS:		exits();	break;
			case ATASK:		me->tasks=me->tasks | (1<<(tP1-1)); break;
			case ASHOWTASK:		show_tasks(Af);	break;
			case ASYNTAX:		asyntax(*(tpt),*(tpt+1)); break;
			case ASETPRE:		iocopy((lstat+tP1)->pre, aP2, 79); break;
			case ASETPOST:		iocopy((lstat+tP1)->post, aP2, 79); break;
			case ASETARR:		iocopy((lstat+tP1)->arr, aP2, 79); strcat((lstat+tP1)->arr,"\n"); break;
			case ASETDEP:		iocopy((lstat+tP1)->dep, aP2, 79); strcat((lstat+tP1)->dep,"\n"); break;
			case ASENDDAEMON:	dsend(tP1,tP2,tP3); break;
			case ADO:		ado(tP1); break;
			case AINTERACT:		ainteract(tP1); break;
			case AAUTOEXITS:	autoexits = (char)tP1; break;
			case ABURN:		osflag(tP1,State(tP1)->flags | SF_LIT); break;
			case ADOUSE:		osflag(tP1,State(tP1)->flags & -(1+SF_LIT)); break;
			case AINC:		if((obtab+tP1)->state >=0 && (obtab+tP1)->state < ((obtab+tP1)->nstates-1)) asetstat(tP1, (obtab+tP1)->state+1); break;
			case ADEC:		if((obtab+tP1)->state > 0) asetstat(tP1, (obtab+tP1)->state-1); break;
			case ATOPRANK:		toprank(tP1); break;
			case ADEDUCT:		deduct(tP1, tP2); break;
			case ADAMAGE:		damage(tP1, tP2); break;
			case AREPAIR:		repair(tP1, tP2); break;
			case APROVOKE:		aprovoke(tP1, tP2); break;
			case ABLAST:		ablast(tP1, aP2, aP3); break;
			case ARANDGO:		moveto(arandgo(tP1)); roomtab=rmtab+me2->room; get_dmove(); break;
			case AHIT:		atakehit(tP1); break;
			case APUT:		putobjin(tP1,tP2); break;
			case AEXTEND:		SendIt(MEXTEND,tP1,NULL); break;
			default	:	txn("@! illegal action %ld!\n",ac);
		}
	}
	else acmove(ac);
	if(tt.condition==CANTEP || tt.condition==CALTEP || tt.condition==CELTEP) donet=ml+1;
}

acmove(int ac) {
	if(Af<MAXU) iocheck(); if (exeunt || died) return;
	if(Af<MAXU) LockUser(Af);		// Prevent others accessing
	}
	acmove_sub(ac);				// Do the movement
	if(Af<MAXU) FreeUser(Af);		// Remove the lock
}

acmove_sub(int ac) {
	register int flag,i;
	if(fol) StopFollow();		// No-longer following
	if((rmtab+ac)->flags & SMALL) {	// Allow for losing follower!
		for(i=0; i<MAXU; i++)
			if(pROOM(i)==ac) {
				sys(NOROOM);
				actionin(ac,acp(NOROOMIN));
				LoseFollower(); return;
			}
	}
	me2->flags = me2->flags|PFMOVING; // I'm out of here.
	action(me2->dep,ACANSEE);
	ldir=iverb; flag=NO;
	lroom=myROOM; i=myLIGHT; myLIGHT=0; lighting(Af,AOTHERS);
	if(((rmtab+ac)->flags & ANTERM) && ac==myROOM) ac=arandgo(0);
	myROOM=ac; if(Af>=MAXU) *(obtab+me2->rec)->rmlist=ac;
	if(Af<MAXU && me2->followed>-1 && me2->followed != Af && (! IamINVIS) && (! IamSINVIS)) {
		if(!((vbtab+overb)->flags&VB_TRAVEL) || pROOM(me2->followed) != lroom || ((lstat+me2->followed)->flags & PFMOVING)) LoseFollower();
		else { DoThis( me2->followed, (vbtab+overb)->id, 1 ); flag=YES; }
	} else  if(me2->followed!=-1) LoseFollower();
	myLIGHT=i; myHLIGHT=0; lighting(Af,AHERE); action(me2->arr,ACANSEE);
	me2->flags=me2->flags^PFMOVING;
	if(Af<MAXU && !(me2->flags & PFASLEEP)) look((rmtab+myROOM)->id, me->rdmode);
	if(exeunt || died) return;
	if(Af<MAXU && autoexits) exits();
}

isplayer(char *s) {
	register int i;
	for(i=0; i<MAXU; i++)
		if((lstat+i)->state>=PLAYING && !match(pNAME(i),s))
			return i;
	return -1;
}

type(char **s) {
	register char *p,*p2;

strip:	p=*s; while(isspace(*p)) p++; *s=p;
	if(!*p) { word=-1; return -1;	}

	if((word=isplayer(p))!=-1) { *s+=strlen(pNAME(word)); return WPLAYER; }

	if(tolower(*p)=='a') {
		if(!match("an",p))	{ *s+=2; goto strip; }
		if(!match("at",p))	{ *s+=2; goto strip; }
		if(!match("as",p))	{ *s+=2; goto strip; }
		if(!match("a",p))	{ *s++;  goto strip; }
	}
	if(tolower(*p)=='t') {
		if(!match("that",p))	{ *s+=4; goto strip; }
		if(!match("the",p))	{ *s+=3; goto strip; }
		if(!match("to",p))	{ *s+=2; goto strip; }
	}
	if(tolower(*p)=='f') {
		if(!match("for",p))	{ *s+=3; goto strip; }
		if(!match("from",p))	{ *s+=4; goto strip; }
	}
	if(tolower(*p)=='i') {
		if(!match("is",p))	{ *s+=2; goto strip; }
		if(!match("in",p))	{ *s+=2; goto strip; }
		if(!match("into",p))	{ *s+=4; goto strip; }
		if(!match("it",p))	{ word=it; *s+=2; return (it!=-1) ? WNOUN : -2; }
	}
	if(!match("using",p)){ *s+=5; goto strip; }
	if(!match("with",p)) { *s+=4; goto strip; }
	if(!match("on",p))   { *s+=2; goto strip; }
	if(!match("some",p))   { *s+=2; goto strip; }

	if(*p=='\"' || *p=='\'') {	// Text?
		register char c;
		c=*p; p2=++p; word=(int)p;
		while(*p2 && *p2!=c) p2++;
		if(*p2) *(p2++)=0; else *(p2+1)=0;
		*s=p2; return WTEXT;
	}

	if(tolower(*p)=='h') {
		if(!match("him",p) || !match("his",p))
			{ word=last_him; *s+=3; return (word!=-1) ? WPLAYER : -2; }
		if(!match("her",p)) { word=last_her; *s+=3; return (word!=-1) ? WPLAYER : -2; }
	}

	if(tolower(*p)=='m') {
		if(!match("me",p)) { word=Af; *s+=2; return WPLAYER; }
		if(!match("myself",p)) { word=Af; *s+=2; return WPLAYER; }
	}

	if((word=issyn(*s))!=-1) {
		*s+=word;
		if(csyn<-1) { word=-2-csyn; return WVERB; }
		if(inoun1==inoun2==-1) { word=csyn; return WNOUN; }
		if((word=isnoun((obtab+csyn)->id,(inoun1==-1)?iadj1:iadj2,(inoun1==-1)?(vbtab+iverb)->sort:(vbtab+iverb)->sort2))!=-1) return WNOUN;
		return -1;
	}
	if(inoun1==inoun2==-1)	{
		if((word=isanoun(*s))!=-1) { *s+=strlen((obtab+word)->id); return WNOUN; }
	}
	else	if((word=isnoun(*s,(inoun1==-1)?iadj1:iadj2,(inoun1==-1)?(vbtab+iverb)->sort:(vbtab+iverb)->sort2))!=-1) { *s+=strlen((obtab+word)->id); return WNOUN; }
	if((word=isadj(*s))!=-1)  { *s+=strlen(adtab+(word*(IDL+1))); return WADJ; }
	if((word=isroom(*s))!=-1) { *s+=strlen((rmtab+word)->id); return WROOM; }
	if((word=isverb(*s))!=-1) { *s+=strlen(vbptr->id); return WVERB; }
	while(!isspace(*p) && *p) p++; word=-1;
	return -2;		// Unknown
}

actual(long n) {
	register int x;
	if(n&RAND0) return (int) Random(n^RAND0);
	if(n&RAND1) return (int) Random(n^RAND1)+(n^RAND1)/2;
	if(n&PRANK) return (int) pRANK(actual(n^PRANK))+1;
	if(n&OBVAL) return (int) scaled(State((x=actual(n^OBVAL))),State(x)->flags);
	if(n&OBWHT) return (int) State(actual(n^OBWHT))->weight;
	if(n&OBDAM) return (int) State(actual(n^OBDAM))->strength;
	if(n&OBLOC) {
		if((x=loc(actual(n^OBLOC)))==-1) x=0; return x;
	}
	if(n&IWORD) {		// Replace with no. of a users word
		switch(n^IWORD) {
			case IVERB:	return iverb;
			case IADJ1:	return iadj1;
			case INOUN1: 	return inoun1;
			case IADJ2:	return iadj2;
			case INOUN2:	return inoun2;
		}
		return -1;
	}
	if((n&MEPRM)==MEPRM) {	// Replace with details of self
		switch(n^MEPRM) {
			case LOCATE:	return -1;	// Not implemented
			case SELF:	return (int)Af;
			case HERE:	return (int)myROOM;
			case RANK:	return (int)myRANK+1;
			case FRIEND:	return (int)me2->helping;
			case HELPER:	return (int)me2->helped;
			case ENEMY:	return (int)me2->fighting;
			case WEAPON:	return (int)me2->wield;
			case SCORE:	return (int)mySCORE;
			case SCTG:	return (int)me2->sctg;
			case STR:	return (int)me->strength;
			case LASTROOM:	return (int)lroom;
			case LASTDIR:	return (int)ldir;
			case LASTVB:	return (int)lverb;
		}
		return -1;
	}
	return (int)n;
}

actptr(long n) {
	if(n==-1 || n==-2) return (int)&"\0";
	if((n&IWORD)==IWORD) {	// Replace with no. of a users word
		switch(n&(-1-IWORD)) {
			case INOUN2:	return (int)&"@n2\n";
			default: 	return (int)&"@n1\n";
		}
	}
	return (int)umsgp+*(umsgip+n);
}

