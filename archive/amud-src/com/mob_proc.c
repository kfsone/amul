//
// AMUD/com/Mob_Proc.C		Mobiles.Txt processor routines
//

char *mobmis(char *s,char *s2) {
	error("%s: Missing %s field.\n",mob.id,s); return skipdata(s2);
}

mob_proc() {
	register char	*p,*s; register long n;

	mobchars=0;
	ofp1=fopenw(mobfn); if(nextc(0)==-1) return;

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

mloop:		p=skipline(s=p); if(!*s) { error("%s: Unexpected end of mobile!\n",mob.id); continue; }
		s=skipspc(s); if(!*s) goto mloop;

		if(strncmp(s,"speed=",6)) { s=mobmis("speed=",s); continue; }
		s=getword(s+6); mob.speed = atoi(Word);
		if(strncmp(s,"travel=",7)) { s=mobmis("travel=",s); continue; }
		s=getword(s+7); mob.travel = atoi(Word);
		if(strncmp(s,"fight=",6)) { s=mobmis("fight=",s); continue; }
		s=getword(s+6); mob.fight = atoi(Word);
		if(strncmp(s,"act=",4)) { s=mobmis("act=",s); continue; }
		s=getword(s+4); mob.act = atoi(Word);
		if(strncmp(s,"wait=",5)) { s=mobmis("wait=",s); continue; }
		s=getword(s+5); mob.wait = atoi(Word);
		if(mob.travel+mob.fight+mob.act+mob.wait!=100) {
			warne("%s: Travel+Fight+Act+Wait don't add to 100%! Please check!\n",mob.id);
		}
		if(strncmp(s,"fear=",5)) { s=mobmis("fear=",s); continue; }
		s=getword(s+5); mob.fear = atoi(Word);
		if(strncmp(s,"attack=",7)) { s=mobmis("attack=",s); continue; }
		s=getword(s+7); mob.attack = atoi(Word);
		if(strncmp(s,"hitpower=",9)) { s=mobmis("hitpower=",s); continue; }
		s=getword(s+9); mob.hitpower=atoi(Word);

		px=p;
		if((n=getmobmsg("arrive="))==-1) { p=px; continue; } mob.arr=n;
		if((n=getmobmsg("depart="))==-1) { p=px; continue; } mob.dep=n;
		if((n=getmobmsg("flee="))==-1) { p=px; continue; } mob.flee=n;
		if((n=getmobmsg("strike="))==-1) { p=px; continue; } mob.hit=n;
		if((n=getmobmsg("miss="))==-1) { p=px; continue; } mob.miss=n;
		if((n=getmobmsg("dies="))==-1) { p=px; continue; } mob.death=n;
		p=px;

		fwrite(mob.id,sizeof(mob),1,ofp1);
	} while(*p);

	errabort();		/* Abort if an error */
	if(mobchars) {
		if(!(mobp=(struct _MOB_ENT *)AllocMem(sizeof(mob)*mobchars,MEMF_PUBLIC))) {
			printf("###> OUT OF MEMORY!\n"); quit();
		}
		fopena(mobfn); fread((char *)mobp,sizeof(mob)*mobchars,1,afp); close_ofps();
	}
}

getmobmsg(char *s) {	// Get mobile "message"
	register char *q; register int n;

loop:	px=skipline(q=px); if(!*q) { error("%s: Unexpected end of mobile!\n",mob.id); return -1; }
	q=skipspc(q); if(!*Word) goto loop;

	if(strncmp(q,s,(n=strlen(s)))) { mobmis(s,px); return -1; }
	q+=n;
	if(*q=='\'' || *q=='\"') {
		register char *cheat;
		strcpy(block,q); cheat=(q=block)+1;
		while(*cheat && *cheat!=*q) cheat++;
		*cheat++=0;
	}
	n=ttumsgchk(q);
	if(n==-1) error("%s: Bad text on '%s' line!\n",mob.id,s);
	return n;
}
