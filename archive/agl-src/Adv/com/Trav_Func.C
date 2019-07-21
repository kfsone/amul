//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Trav_Func.C	Misc. functions for Travel.Txt and Lang.Txt
//
//	LMod: oliver 19/06/93	Implemented QUIT ASK|FORCE
//	      oliver 11/06/93	AMUL->AGL. Implemented CompInf structs
//

// Tables for comparisons

typedef struct CompInf { char *text; short val; }

struct CompInf	gender[] =	{ "m", 0, "f", NULL, 0 },
	antype[] =	{
		"glo", AGLOBAL, "every", AEVERY1, "out", AOUTSIDE,
		"here", AHERE, "oth", AOTHERS, "all", AALL,
		"not", ANOTSEE, "cant", ACANTSEE, "can", ACANSEE,
		NULL, NULL
	},
	rdmode[] =	{ "r", RDRC, "v", RDVB, "b", RDBF, NULL, NULL },
	spell[] =	{
		"gl", SGLOW, "in", SINVIS, "de", SDEAF,
		"du", SDUMB, "bl", SBLIND, "cr", SCRIPPLE,
		"sl", SSLEEP, "si", SSINVIS, NULL, NULL
	},
	stat[] =	{
		"sctg", STSCTG, "sc", STSCORE, "poi", STSCORE, "str", STSTR,
		"sta", STSTAM, "de", STDEX, "wi", STWIS, "ex", STEXP,
		"mag", STMAGIC, NULL,NULL
	},
	bvmode[] =	{
		"v", TYPEV, "b", TYPEB, NULL,NULL
	},
	onoff[] =	{
		"on", 1, "ye", 1, "true", 1,
		"no", 0, "false", 0, NULL, NULL
	},
	randgo[] =	{
		"s", 0, "a", 1, NULL, NULL
	},
	quitaf[] =	{
		"for", 0, "ask", 1, NULL, NULL
	};

char *precon(char *s) {
	s=skiplead("the ",skiplead("if ",s)); s=skiplead("i ",s);
	return skiplead("am ",s);
}

char *preact(char *s) {
	return skiplead("to ",skiplead("go ",skiplead("goto ",skiplead("then ",s))));
}

long chknum(char *p) {
	register long n;

	if(!isdigit(*p) && !isdigit(*(p+1))) return -1000001;
	if(*p=='>' || *p=='<' || *p=='-' || *p=='=') n=atoi(p+1);
	else n=atoi(p);
	if(n>=1000000) {
		error("%s: Number \"%s\" exceeds limit!\n",(proc==1)?verb.id:(rmtab+rmn)->id,p); return -1000001;
	}
	if(*p=='-') return (long) -n;
	if(*p=='>') return (long) (n+LESS);
	if(*p=='<') return (long) (n+MORE);
	return n;
}

char *optis(char *p) {
	p=precon(p);		p=skiplead("of ",p);	p=skiplead("are ",p);
	p=skiplead("has ",p);	p=skiplead("with ",p);	p=skiplead("to ",p);
	p=skiplead("set ",p);	p=skiplead("from ",p);	p=skiplead("by ",p);
	p=skiplead("and ",p);	p=skiplead("was ",p);	return skipspc(p);
}

char *chkp(char *p,register char t,int c,int z,FILE *fp) {
	char qc,*p2; long x;

	p=optis(p); p2=p=skipspc(p);	// Strip crap out
	if(!*p) {
		error("%s: Missing paramters (%s '%s')\n",
			(proc==1)?verb.id:(rmtab+rmn)->id,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
		return NULL;
	}
	if(*p!='\"' && *p!='\'') while(*p && *p!=32) p++;
	else {
		qc=*(p++);		// Search for same CLOSE quote
		while(*p && *p!=qc) p++;
	}
	if(*p) *(p++)=0;
	if((t>=0 && t<=10) || t==-70) {		// Processing lang tab?
		if(*p2=='>' || *p2=='<') x=actualval(p2+1,t);
		else x=actualval(p2,t);
		if(x==-1) {	// If it was an actual, but wrong type
			error("%s: Invalid variable, '%s', after %s '%s'\n",
				verb.id,p2,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
			return NULL;
		}
		if(x!=-2) {
			if(*p2=='>') x=x|MORE; if(*p2=='<') x=x|LESS; goto write;
		}
	}
	switch(t) {
		case -8:	x=checktab(&quitaf,p2); break;
		case -7:	x=checktab(&randgo,p2); break;
		case -6:	x=checktab(&onoff,p2); break;
		case -5:	x=checktab(&bvmode,p2); break;
		case -4:	x=checktab(&stat,p2); break;
		case -3:	x=checktab(&spell,p2); break;
		case -2:	x=checktab(&rdmode,p2); break;
		case -1:	x=checktab(&antype,p2); break;
		case PROOM:	x=isroom(p2); break;
		case PVERB:	x=is_verb(p2); break;
		case PADJ:	break;
		case -70:
		case PNOUN:	x=isnounh(p2); break;
		case PUMSG:	x=ttumsgchk(p2); break;
		case PNUM:	x=chknum(p2); break;
		case PRFLAG:	x=isrflag(p2); break;
		case POFLAG: 	x=isoflag1(p2); break;
		case PSFLAG: 	x=isoflag2(p2); break;
		case PSEX:	x=checktab(&gender,p2); break;
		case PDAEMON:	if((x=is_verb(p2))==-1 || *p2!='.') x=-1; break;
		case PMOBILE:	if((x=isnoun(p2))==-1) break;
				if((obtab2+x)->mobile==-1) x=-1;
				break;
		default:
			if(!(proc==1 && t>=0 && t<=10)) {
				warne("%s = %s.\n", (z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
				error("> Internal error, invalid PTYPE (val: %d, item: %s)!\n",
					(proc==1)?verb.id:(rmtab+rmn)->id,t,p2);
				return NULL;
			}
	}
	if(x==-2 && (t==-70 || t==PUMSG)) { if(t==-70) x=-1; }
	else if(((x==-1 || x==-2) && t!=PNUM) || x==-1000001) {
		error("%s: Invalid parameter '%s' after %s '%s'.\n",
			(proc==1)?(verb.id):(rmtab+rmn)->id,p2,
			(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
		return NULL;
	}
write:	if(!z && c==ATREATAS && x==IWORD+IVERB) {
		error("%s: Action 'treatas verb' is illegal.\n",(proc==1)?(verb.id):(rmtab+rmn)->id);
		return NULL;
	}
	fwrite((char *)&x,4,1,fp); FPos+=4;	// Writes a LONG
	return p;
}

isnounh(char *s) {	// Test noun state, checking rooms
	register int i,l,j; FILE *fp; long orm;

	if(!strcmp(s,"none")) return -2;
	fp=(FILE *)rfopen(objrmsfn); l=-1; objtab2=obtab2;
	
	for(i=0; i<nouns; i++,objtab2++) {
		if(!strcmp(s,objtab2->id)) continue;
		fseek(fp,(long)objtab2->rmlist,0L);
		for(j=0;j<objtab2->nrooms;j++) {
			fread((char *)&orm,4,1,fp);
			if(orm==rmn) {
				fclose(fp); return i;
			}
		}
		l=i;
	}
	fclose(fp); return l;
}

char *chkaparms(char *p,int c,FILE *fp) {
	register int i;
	if(!nacp[c]) return p;
	for(i=0; i<nacp[c]; i++)
		if(!(p=chkp(p,tacp[c][i],c,0,fp))) return NULL;
	return p;
}

char *chkcparms(char *p,int c,FILE *fp) {
	register int i;
	if(!ncop[c]) return p;
	for(i=0; i<ncop[c]; i++)
		if(!(p=chkp(p,tcop[c][i],c,1,fp))) return NULL;
	return p;
}

checktab(struct CompInf *ci,char *s) {
	register char *p,*s2;

	if(!*s) return -1;

	while(*(p=ci->text)) {
		s2=s;
		while(*(s2++)==*p) p++;
		if(!*p) return ci->val;
		ci++;
	};
	return -1;
}
