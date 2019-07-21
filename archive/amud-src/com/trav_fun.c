//
// AMUD/com/Trav_Func.C		Travel/Language functions for compiler
//

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
	p=precon(p); p=skiplead("with ",skiplead("has ",skiplead("are ",p)));
	p=skiplead("set ",p);	p=skiplead("from ",p);
	p=skiplead("was ",p);	return skipspc(p);
}

char *chkp(char *p,register char t,int c,int z,FILE *fp) {
	char qc,*p2; long x;

	p=optis(p); p2=p=skipspc(p);	/*=* Strip crap out *=*/
	if(!*p) {
		error("%s: Missing paramters (%s '%s')\n",
			(proc==1)?verb.id:(rmtab+rmn)->id,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
		return NULL;
	}
	if(*p!='\"' && *p!='\'') while(*p && *p!=32) p++;
	else {
		qc=*(p++);		/* Search for same CLOSE quote */
		while(*p && *p!=qc) p++;
	}
	if(*p) *(p++)=0;
	if((t>=0 && t<=10) || t==-70) {		/* Processing lang tab? */
		if(*p2=='>' || *p2=='<') x=actualval(p2+1,t);
		else x=actualval(p2,t);
		if(x==-1) {	/* If it was an actual, but wrong type */
			error("%s: Invalid variable, '%s', after %s '%s'\n",
				verb.id,p2,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
			return NULL;
		}
		if(x!=-2) {
			if(*p2=='>') x=x|MORE; if(*p2=='<') x=x|LESS; goto write;
		}
	}
	switch(t) {
		case -7:	x=randgo(p2); break;
		case -6:	x=onoff(p2); break;
		case -5:	x=bvmode(toupper(*p2)); break;
		case -4:	x=stat(p2); break;
		case -3:	x=spell(p2); break;
		case -2:	x=rdmode(toupper(*p2)); break;
		case -1:	x=antype(p2); break;
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
		case PSEX:	x=isgen(toupper(*p2)); break;
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
	fwrite((char *)&x,4,1,fp); FPos+=4;	/* Writes a LONG */
	return p;
}

isgen(char c) {
	if(c=='M') return 0; if(c=='F') return 1;
	return -1;
}

antype(char *s) {
	if(!strcmp(s,"global")) return AGLOBAL;
	if(!strcmp(s,"everyone")) return AEVERY1;
	if(!strcmp(s,"outside")) return AOUTSIDE;
	if(!strcmp(s,"here")) return AHERE;
	if(!strcmp(s,"others")) return AOTHERS;
	if(!strcmp(s,"all")) return AALL;
	if(!strcmp(s,"notsee")) return ANOTSEE;
	if(!strcmp(s,"cansee")) return ACANSEE;
	if(!strcmp(s,"cantsee")) return ANOTSEE;
	return -1;
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
			if(orm==rmn) { fclose(fp); return i; }
		}
		l=i;
	}
	fclose(fp); return l;
}

rdmode(char c) {
	if(c=='R') return RDRC; if(c=='V') return RDVB; if(c=='B') return RDBF;
	return -1;
}

spell(register char *s) {
	if(!strncmp(s,"gl",2)) return SGLOW;
	if(!strncmp(s,"in",2))return SINVIS;
	if(!strncmp(s,"de",2)) return SDEAF;
	if(!strncmp(s,"du",2)) return SDUMB;
	if(!strncmp(s,"bl",2))return SBLIND;
	if(!strncmp(s,"cr",2))return SCRIPPLE;
	if(!strncmp(s,"sl",2))return SSLEEP;
	if(!strncmp(s,"si",2))return SSINVIS;
	return -1;
}

stat(register char *s) {
	if(strcmp(s,"sctg")==NULL) return STSCTG;
	if(strncmp(s,"sc",2)==NULL) return STSCORE;
	if(strncmp(s,"poi",3)==NULL) return STSCORE;
	if(strncmp(s,"str",3)==NULL) return STSTR;
	if(strncmp(s,"sta",3)==NULL) return STSTAM;
	if(strncmp(s,"de",2)==NULL) return STDEX;
	if(strncmp(s,"wi",2)==NULL) return STWIS;
	if(strncmp(s,"ex",2)==NULL) return STEXP;
	if(strcmp(s,"magic")==NULL) return STMAGIC;
	return -1;
}

bvmode(char c) {
	if(c=='V') return TYPEV; if(c=='B') return TYPEB;
	return -1;
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

onoff(char *p) {
	if(!strcmp(p,"on") || !strcmp(p,"yes")) return 1;
	return 0;
}

randgo(char *p) {
	if(tolower(*p)=='s') return 0; if(tolower(*p)=='a') return 1;
	return -1;
}
