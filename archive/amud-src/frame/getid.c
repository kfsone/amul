//
// AMUD/frame/GetID.C		Handles the login process
//

getid() {
	register int i,fails; register FILE *fp;

	iverb=iadj1=inoun1=iadj2=inoun2=actor=-1; *(me2->pre)=*(me2->post)=0;
	strcpy((me2->arr=arr),acp(ARRIVED)); strcpy((me2->dep=dep),acp(LEFT));
	last_him=last_her=-1; Random(1);

	if(*me->passwd) strcpy(him.passwd,me->passwd); else him.passwd[0]=0;

	sprintf(block,"%s%s",dir,plyrfn);
	if(!(fp=fopen(block,"ab+"))) {
		tx("Can't create player file!\n"); quit();
	}
	afp=fp; fp=NULL;

	listpers(); fopena(plyrfn);	// Show personas on their account

	me2->rec=-1; me2->flags=0;
	do {
		fails = -1; me->llen=DLLEN; me->slen=DSLEN; me->flags=DFLAGS;
		getname(); strcpy(him.name,input);

		if(!findpers()) fails=newid();
		else fails=getpasswd();
	} while(fails);

	SendIt(MLOGGED,0,me->name);	// Inform AMan we've logged in.
	for(i=ranks-1; i>=0; i--) {
		if(me->score >= (rktab+i)->score && taskcnt(Af)>=(rktab+i)->tasks) {
			myRANK=i; break;
		}
	}
	// Check if we have enough score to go up a rank
	if(myRANK!=ranks-1 && me->score >= (rktab+myRANK+1)->score) sys(NOTASK);
	me->plays++;		// I've now played atleast once.
	if(me->plays>1) sys(WELCOMEBAK);

	fopenr(rooms2fn);	// Room descriptions
	roomtab=rmtab; me2->room=-1; lroom=-1;
	for(i=0; i<rooms; i++,roomtab++) {
		if(roomtab->flags & ANTERM) me2->room=i;
	}
	if(me2->room==-1) me2->room=arandgo(0);
	roomtab=rmtab+me2->room;

	me2->wield=-1; me2->helping=-1; me2->helped=-1; me2->following=-1; me2->followed=-1; me2->fighting=-1;
	me2->numobj=0; me2->weight=0; me2->hadlight=0; me2->light=0; me2->flags=0; me2->sctg=0;
	ans("1m"); if(me->flags&ufANSI) sys(ANSION); ans("0m");

	me->strength=(rktab+myRANK)->strength;	// Same session?
	if(me->last_session!=him.last_session) {
		me2->dextadj=0;
		me->stamina =	(rktab+myRANK)->stamina;
		me->magicpts=	(rktab+myRANK)->magicpts;
		me->experience=	(rktab+myRANK)->experience;
		me->wisdom  =	(rktab+myRANK)->wisdom;
	}
	else if(me->stamina<(rktab+myRANK)->stamina) me->stamina+=((rktab+myRANK)->stamina-me->stamina)/(myRANK+1);

	calcdext(); me->last_session=him.last_session; save_me(); txs(*(errtxt-6),vername);
	if((i=isverb("!start"))!=-1) lang_proc(i,0);
	action(acp(COMMENCED),AOTHERS); look(roomtab->id, me->rdmode);
}

getname() {
	char *p; int i;

	do {
begin:		word=-3; crsys(WHATNAME);
		*input=0; Inp(input,NAMEL+1); txc('\n'); if(!*input) quit();
		p=input;
		while(*p) {
			if(*p==32 || *p==9) { *(p++)='-'; continue; }
			if(!isalnum(*(p++))) {
				tx("^GInvalid character name!\n"); goto begin;
			}
		}
		if(strlen(input)<3) {
			sys(LENWRONG); continue;
		}

		p=input;
		if((i=type(&p))>-1 && i!=WPLAYER) { sys(NAME_USED); continue; }
		if(i == WPLAYER && word!=Af) {
			utx(word,acp(LOGINASME)); strcpy(me->name,input);
			sys(ALREADYIN); continue;
		}
		word=-2;
	} while(word!=-2);
}

newid() {
	register int i;

	me->llen=DLLEN; me->slen=DSLEN; me->flags=DFLAGS;
	strcpy(me->name,him.name); sys(CREATE); *me->name=0;
	*input=0; Inp(input,3); if(toupper(*input)!='Y') return -1;

	if(him.plays >= 5) {
		txn(*(errtxt-7),5);
		return -1;
	}

	me->score=0; me->plays=0; me->strength=rktab->strength;
	me->stamina=rktab->stamina; me->dext=rktab->dext;
	me->wisdom=rktab->wisdom; me->experience=rktab->experience;
	me->magicpts=rktab->magicpts; me->tasks=0;
	me->tries=me->sex=myRANK=me->rdmode=0; me->last_session=0;

	do {
		crsys(WHATGENDER);
		*input=0; strcpy(me->name,him.name); Inp(input,2);
		*input=toupper(*input);
		if(*input!='M' && *input!='F') { crsys(GENDINVALID); }
	} while(*input!='M' && *input!='F');
	me->sex=(*input=='M') ? 0 : 1;

	if(him.passwd[0]) strcpy(me->passwd,him.passwd);
	else {
		crsys(ENTERPASSWD); *input=0; Inp(input,-20);
		if(!*input) return -1;
		strcpy(me->passwd,input);
	}

	me->name[0]=toupper(me->name[0]);

	for(i=1;i<strlen(me->name);i++) {
		if(me->name[i-1]==' ') me->name[i] = toupper(me->name[i]);
		else me->name[i] = tolower(me->name[i]);
	}

	flagbits(); me2->rec=-1;
	ShowFile("scenario",NULL); pressret(); crsys(YOUBEGIN); txc('\n');

	waitfree();		// PlayerData lock
	him.name[0]=0;		// Find first free record
	{	struct _PLAYER mx;
		mx=*me; if(findpers()) me2->rec--; *me=mx;
	}
	me->last_session=0;
	return 0;
}

getpasswd() {
	int i;
	me2->rec--;		// Move back a record
	if(him.passwd[0] && stricmp(me->passwd,him.passwd)) {
		tx(*(errtxt-8)); return -1;
	}
	if(him.passwd[0]) return 0;

	for(i=0;i<4;i++) {
		if(i == 3) {
			sys(TRIESOUT); // Update bad try count
			me->tries++; save_me(); return quit();
		}
		txn("\nTry #%d -- ",i+1); sys(ENTERPASSWD);
		*input=0; Inp(input,-20); if(!*input) return -1;
		if(!stricmp(input,me->passwd)) break;
	}
	if(me->tries>0) {
		ans("1m"); txc(0x7); txc('\n'); txn(acp(FAILEDTRIES),me->tries); txc('\n'); ans("0m");
	}

	me->tries=0; return 0;
}
