char *precon(register char *s)
{	register char *s2;

	s2=s;

	if((s=skiplead("if ",s))!=s2) { s=skipspc(s); s2=s; }
	if((s=skiplead("the ",s))!=s2) { s=skipspc(s); s2=s; }
	if((s=skiplead("i ",s))!=s2) { s=skipspc(s); s2=s; }
	s=skiplead("am ",s);
	return s;
}

char *preact(register char *s)
{	char *s2;

	s2=s;
	if((s=skiplead("then ",s))!=s2) { s=skipspc(s); s2=s; }
	if((s=skiplead("goto ",s))!=s2) { s=skipspc(s); s2=s; }
	if((s=skiplead("go to ",s))!=s2) { s=skipspc(s); s2=s; }
	s=skiplead("set ",s);
	return s;
}

long chknum(char *p)
{	register long n;

	if(!isdigit(*p) && !isdigit(*(p+1))) return -1000001;
	if(*p=='>' || *p=='<' || *p=='-' || *p=='=') n=atoi(p+1);
	else n=atoi(p);
	if(n>=1000000)
	{
		printf("\x07\n*** Number %d exceeds limits!",n);
		return -1000001;
	}
	if(*p=='-') return (long) -n;
	if(*p=='>') return (long) (n+LESS);
	if(*p=='<') return (long) (n+MORE);
	return n;
}

char *optis(char *p)
{	register char *p2;
	p2=p;

	p=skiplead("the ",p); p=skiplead("of ",p); p=skiplead("are ",p);
	p=skiplead("is ",p); p=skiplead("has ",p); p=skiplead("next ",p);
	p=skiplead("with ",p); p=skiplead("to ",p); p=skiplead("set ",p);
	p=skiplead("from ",p); p=skiplead("for ",p); p=skiplead("by ",p);
	p=skiplead("and ",p); p=skiplead("was ",p); p=skiplead("i ",p);
	p=skiplead("am ",p); p=skiplead("as ",p); p=skipspc(p);
	return p;
}

char *chkp(char *p,char t,int c,int z,FILE *fp)
{	char qc,*p2; long x;

	p=optis(p); p2=(p=skipspc(p));	/*=* Strip crap out *=*/
	if(*p==0)
	{
		printf("\x07\%s \"%s\" has incomplete C&A line! (%s='%s')\n\n",
			(proc==1)?"Verb":"Room",(proc==1)?verb.id:roomtab->id,
			(z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
		quit();
	}
	if(*p!='\"' && *p!='\'') while(*p!=32 && *p!=0) p++;
	else
	{
		qc=*(p++);		/* Search for same CLOSE quote */
		while(*p!=0 && *p!=qc) p++;
	}
	if(*p!=0) *p=0; else *(p+1)=0;
	if((t>=0 && t<=10) || t==-70)		/* Processing lang tab? */
	{
		x=actualval(p2,t);
		if(x==-1)	/* If it was an actual, but wrong type */
		{
			printf("\x07\nInvalid slot label, '%s', after %s '%s' in verb '%s'.\n",
				p2,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c],
				verb.id);
			return NULL;
		}
		if(x!=-2) goto write;
	}
	switch(t)
	{
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
		default:
		{
			if(!(proc==1 && t>=0 && t<=10))
			{
				printf("\n\n\x07!! Internal error, invalid PTYPE (val: %d) in %s %s!\n\n",
					t,(proc==1)?"verb":"room",(proc==1)?verb.id:(rmtab+rmn)->id);
				printf("%s = %s.\n", (z==1)?"condition":"action",(z==1)?conds[c]:acts[c]);
				quit();
			}
		}
	}
	if(t==-70 && x==-2) x=-1;
	else if(((x==-1 || x==-2) && t!=PNUM) || x==-1000001)
	{
		printf("\x07\nInvalid parameter, '%s', after %s '%s' in %s '%s'.\n",
			p2,(z==1)?"condition":"action",(z==1)?conds[c]:acts[c],
			(proc==1)?"verb":"room",(proc==1)?(verb.id):(rmtab+rmn)->id);
		return NULL;
	}
write:	fwrite((char *)&x,4,1,fp); FPos+=4;	/* Writes a LONG */
	*p=32; return skipspc(p);
}

isgen(char c)
{
	if(c=='M') return 0;
	if(c=='F') return 1;
	return -1;
}

antype(char *s)
{
	if(strcmp(s,"global")==NULL) return AGLOBAL;
	if(strcmp(s,"everyone")==NULL) return AEVERY1;
	if(strcmp(s,"outside")==NULL) return AOUTSIDE;
	if(strcmp(s,"here")==NULL) return AHERE;
	if(strcmp(s,"others")==NULL) return AOTHERS;
	if(strcmp(s,"all")==NULL) return AALL;
	printf("\x07\nInvalid anouncement-group, '%s'...\n",s);
	return -1;
}

isnounh(char *s)	/* Test noun state, checking rooms */
{	register int i,l,j; FILE *fp; long orm;

	if(stricmp(s,"none")==NULL) return -2;
	fp=(FILE *)rfopen(objrmsfn); l=-1; objtab2=obtab2;
	
	for(i=0; i<nouns; i++,objtab2++)
	{
		if(stricmp(s,objtab2->id)!=NULL) continue;
		fseek(fp,(long)objtab2->rmlist,0L);
		for(j=0;j<objtab2->nrooms;j++)
		{
			fread((char *)&orm,4,1,fp);
			if(orm == rmn)
			{
				l=i; i=nouns+1; j=objtab2->nrooms;
				break;
			}
		}
		if(i < nouns) l=i;
	}
	fclose(fp); return l;
}

rdmode(char c)
{
	if(c == 'R') return RDRC;
	if(c == 'V') return RDVB;
	if(c == 'B') return RDBF;
	return -1;
}

spell(register char *s)
{
	if(strcmp(s,"glow")==NULL) return SGLOW;
	if(strcmp(s,"invis")==NULL)return SINVIS;
	if(strcmp(s,"deaf")==NULL) return SDEAF;
	if(strcmp(s,"dumb")==NULL) return SDUMB;
	if(strcmp(s,"blind")==NULL)return SBLIND;
	if(strcmp(s,"cripple")==NULL)return SCRIPPLE;
	if(strcmp(s,"sleep")==NULL)return SSLEEP;
	if(strcmp(s,"sinvis")==NULL)return SSINVIS;
	return -1;
}

stat(register char *s)
{
	if(strcmp(s,"sctg")==NULL) return STSCTG;
	if(strncmp(s,"sc",2)==NULL) return STSCORE;
	if(strncmp(s,"poi",3)==NULL) return STSCORE;
	if(strncmp(s,"str",3)==NULL) return STSTR;
	if(strncmp(s,"stam",4)==NULL) return STSTAM;
	if(strncmp(s,"dext",4)==NULL) return STDEX;
	if(strncmp(s,"wis",3)==NULL) return STWIS;
	if(strncmp(s,"exp",3)==NULL) return STEXP;
	if(strcmp(s,"magic")==NULL) return STMAGIC;
	return -1;
}

bvmode(char c)
{
	if(c=='V') return TYPEV;
	if(c=='B') return TYPEB;
	return -1;
}

char *chkaparms(char *p,int c,FILE *fp)
{	int i;

	if(nacp[c]==0) return p;
	for(i=0; i<nacp[c]; i++)
		if((p=chkp(p,tacp[c][i],c,0,fp))==NULL) return NULL;
	return p;
}

char *chkcparms(char *p,int c,FILE *fp)
{	int i;

	if(ncop[c]==0) return p;
	for(i=0; i<ncop[c]; i++)
		if((p=chkp(p,tcop[c][i],c,1,fp))==NULL) return NULL;
	return p;
}

onoff(char *p)
{
	if(stricmp(p,"on")==NULL || stricmp(p,"yes")==NULL) return 1;
	return 0;
}
