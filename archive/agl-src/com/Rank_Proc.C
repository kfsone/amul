//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Rank_Proc.C	Ranks.Txt Processor
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

rank_proc() {
	register char	*p,*s; int	n;

	nextc(1); ranks=0;
	fopenw(ranksfn); cleanget(&datal,&data,0L); p=skipspc(data);
	tx("Proc:");

	do {
loop:		p=skipline(s=p); if(!*s) continue;
		s=skipspc(s); if(!*s) goto loop;

		ranks++; s=getword(s); if(chkline(s)) continue;
		*rank.male=*rank.female=0;
		if(strlen(Word)<3 || strlen(Word)>RANKL) {
			error("%s rank name \"%s\" is invalid.\n","Male",Word); Word[RANKL]=0;
		}
		n=0;
		do {
			if(Word[n]=='_') Word[n]=' ';
			rank.male[n]=rank.female[n]=Word[n];
			n++;
		} while(Word[n-1]);

		s=getword(s); if(chkline(s)) continue;
		if(strcmp(Word,"=")!=NULL && strlen(Word)<3 || strlen(Word)>RANKL) {
			error("%s rank name \"%s\" is invalid.\n","Female",Word); Word[RANKL]=0;
		}
		if(Word[0]!='=') {
			n=0;
			do {
				rank.female[n]=(Word[n]=='_')?' ':Word[n];
			} while(Word[n++]);
		}

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("score");
		else rank.score=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("strength");
		else rank.strength=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("stamina");
		else rank.stamina=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("dexterity");
		else rank.dext=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("wisdom");
		else rank.wisdom=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("experience");
		else rank.experience=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("magic points");
		else rank.magicpts=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("max. weight carried");
		else rank.maxweight=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("max. objects carried");
		else rank.numobj=atoi(Word);

		s=getword(s); if(chkline(s)) continue;
		if(!isdigit(*Word)) badrank("player kill-value");
		else rank.minpksl=atoi(Word);

		s=getword(s);
		if(!isdigit(*Word)) badrank("task number");
		else rank.tasks=atoi(Word);

		s=skipspc(s); if(*s=='\"') s++; strcpy(block,s); s=block;
		while(*s && *s!='\"') s++;
		*(s++)=0;
		if(s-block>11) { // Greater than prompt length?
			error("%3ld/%s: Prompt too long (\"%s\")\n",ranks,rank.male,block);
			*block=0;
		}
		if(!*block) strcpy(rank.prompt,"> ");
		else strcpy(rank.prompt,block);

		wizstr=rank.strength;
		fwrite(rank.male,sizeof(rank),1,ofp1);
	} while(*p);
	errabort();
}

chkline(char *p) {
	if(*p) return 0;
	error("Rank line %ld incomplete!\n",ranks); return 1;
}

badrank(char *s) {
	error("%3ld/%s: Invalid number for %s - \"%s\".\n",ranks,rank.male,s,Word);
}
