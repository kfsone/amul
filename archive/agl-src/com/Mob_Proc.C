//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Mob_Proc.C	Process Mobile.Txt
//
//	LMod: oliver 11/06/93	AMUL->AGL. mobpget. skiplead(='s) not strncmp
//

char *mobmis(char *s) {
	error("%s: Missing %s field.\n",mob.id,s);
}

	/*=* Pass 1: Indexes mobile names *=*/

mob_proc1() {
	register char	*p,*s; long n;

	mobchars=0;
	fopenw(mobfn); if(nextc(0)==-1) return;

	cleanget(&moblen,&mobdat,0L); p=mobdat; tx("Proc:");

	do {
		do p=skipline(s=p); while(*s!='!' && *s); if(!*s) continue;
		mobchars++; s=getword(s+1); strcpy(mob.id,Word);
		mob.dmove=-1; mob.dead=1;
		do {
			if(!*s) break;
			if(!strncmp(s,"dead=",5)) {
				s=getword(s+5); mob.dead=atoi(Word); continue;
			}
			if(!strncmp(s,"dmove=",6)) {
				s=getword(s+6); mob.dmove=isroom(Word);
				if(mob.dmove==-1) {
					error("%s: invalid DMove '%s'.\n",mob.id,Word);
				}
				continue;
			}
		} while(*s);

		do {
			p=skipline(s=p);
			if(!*s) {
				error("%s: Unexpected end of mobile!\n",mob.id);
				continue;
			}
		while(!*(s=skipspc(s)));

		if(!(s=mobpget("speed=",&n))) goto end; mob.speed=n;
		if(!(s=mobpget("travel=",&n))) goto end; mob.travel=n;
		if(!(s=mobpget("fight=",&n))) goto end; mob.fight=n;
		if(!(s=mobpget("act=",&n))) goto end; mob.act=n;
		if(!(s=mobpget("wait=",&n))) goto end; mob.wait=n;
		if(mob.travel+mob.fight+mob.act+mob.wait!=100) {
			warne("%s: Travel+Fight+Act+Wait don't add to 100%! Please check!\n",mob.id);
		}

		if(!(s=mobpget("fear=",&n))) goto end; mob.fear=n;
		if(!(s=mobpget("attack=",&n))) goto end; mob.attack=n;
		if(!(s=mobpget("hitpower=",&n))) goto end; mob.hitpower=n;

		px=p;
		if((n=getmobmsg("arrive="))==-1) { p=px; continue; } mob.arr=n;
		if((n=getmobmsg("depart="))==-1) { p=px; continue; } mob.dep=n;
		if((n=getmobmsg("flee="))==-1) { p=px; continue; } mob.flee=n;
		if((n=getmobmsg("strike="))==-1) { p=px; continue; } mob.hit=n;
		if((n=getmobmsg("miss="))==-1) { p=px; continue; } mob.miss=n;
		if((n=getmobmsg("dies="))==-1) { p=px; continue; } mob.death=n;
		p=px;

		fwrite(mob.id,sizeof(mob),1,ofp1);
end:		if(!s) p=skipdata(p);
	} while(*p);

	errabort();		/* Abort if an error */
	if(mobchars) {
		if(!(mobp=(struct _MOB_ENT *)AllocMem(sizeof(mob)*mobchars,MEMF_PUBLIC))) {
			printf("###> OUT OF MEMORY!\n"); quit();
		}
		fopena(mobfn); fread((char *)mobp,sizeof(mob)*mobchars,1,afp); close_ofps();
	}
}

char *mobpget(char *s,char *p,long *n) {
	p=getword(skiplead(p,s)); if(!*Word) {
		mobmis(s); return FALSE;
	}
	*n=atoi(Word); return p;
}

/*=* Fetch mobile message line *=*/
getmobmsg(char *s) {
	register char *q; register int n;

loop:	px=skipline(q=px); if(!*q) { mobmis(s); return -1; }
	q=skipspc(q); if(!*q) goto loop;

	q=skiplead(q,s);
	if(*q=='\'' || *q=='\"') {
		register char *cheat;
		strcpy(block,q); cheat=(q=block)+1;
		while(*cheat && *cheat!=*q) cheat++;
		*cheat++=0;
	}
	if((n=ttumsgchk(q))==-1)
		error("%s: Bad text on '%s' line!\n",mob.id,s);
	else	return n;
}
