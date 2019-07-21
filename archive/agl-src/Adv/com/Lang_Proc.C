//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Lang_Proc.C	Lang.Txt Processor
//
//	LMod: oliver 11/06/93	AMUL -> AGL
//

#define	INVALID	"Invalid syntax= line!"

lang_proc() {
	register char	*p,*s; char *ls; char pct[41];
	// n=general, cs=Current Slot, x=slot, of2p=ftell(ofp2)
	register int	n,cs,x;			int r;
	unsigned long	of2p,of3p;
	char		lastc;

	verbs=0; nextc(1); proc=1;
	fopenw(lang1fn); close_ofps(); fopena(lang1fn); ofp1=afp; afp=NULL;
	fopenw(lang2fn); fopenw(lang3fn); fopenw(lang4fn);

	cleanget(&vbmem,(char **)&vbtab,64*(sizeof(verb))); vbptr=vbtab+64;
	p=skipspc((char *)vbptr); vbptr=vbtab;
	of2p=ftell(ofp2); of3p=ftell(ofp3); FPos=ftell(ofp4);
	tx("Proc:");

	do {
loop:		p=skipline(ls=s=p); if(!*s) continue;
		s=skipspc(s); if(!*s) goto loop;
		verb.id[0]=0;
		if(!strncmp(s,"travel=",7)) {
		    s+=7; verb.ents=0; verb.ptr=(struct _SLOTTAB *)of2p; verb.flags=VB_TRAVEL;
		    do {
			s=getword(s); if(!*Word) break; verbs++;
			if(is_verb(Word)!=-1) { error("%s: Verb already defined\n",Word); }
			else {
				strcpy(verb.id,Word);
				fwrite(verb.id,sizeof(verb),1,ofp1);
				*(vbtab+(verbs-1))=verb;
			}
		    } while(*s);
		    continue;
		}
		s=skiplead("verb=",s); s=getword(s);
		if(!*Word) {
			warne("found a verb= without an ID!\n"); p=skipdata(p); goto loop;
		}
		if(strlen(Word)>IDL) {
			error("%s: ID too long.\n",Word); p=skipdata(p); goto loop;
		}
		verbs++;
		if(is_verb(Word)!=-1) { error("%s: Verb already defined\n",Word); p=skipdata(p); goto loop; }
		strcpy(verb.id,Word);

		strcpy(verb.sort,"\xff\x43\x48\x45\xff\x43\x48\x45"); // -1CHE

		verb.flags=NULL; if(!*s) goto noflags; s=getword(s);
		if(!strcmp("travel",Word)) { verb.flags=VB_TRAVEL; s=getword(s); }
		if(!strcmp("dream",Word)) { verb.flags+=VB_DREAM; s=getword(s); }
		if(!*Word) goto noflags;

		if(chae_proc(Word,verb.sort)==-1) goto noflags;
		s=getword(s); if(*Word) chae_proc(Word,verb.sort2);

noflags:	verb.ents=0; verb.ptr=(struct _SLOTTAB *)of2p;

stuffloop:	p=skipline(ls=s=p);
		if(!*s) {
			if(!verb.ents && !verb.flags&VB_TRAVEL) warne("%s: No entries!\n",verb.id);
			goto write;
		}
		s=skipspc(s); if(!*s) goto stuffloop; setslots(WANY);
		if(strncmp(s,"syntax=",7)) { verb.ents++; *(p-1)=10; p=s; goto endsynt; }

		// Syntax line loop
synloop:	s=skipspc(skiplead("syntax=",s)); setslots(WNONE);
		verb.ents++;
		// ? line is '[syntax=][verb] any' or '[syntax=][verb] none'
		if(!strncmp("any",s,3)) { setslots(WANY); goto endsynt; }
		if(!strncmp("none",s,4)){ setslots(WNONE); goto endsynt; }

sp2:		// Syntax line processing
		s=getword(s); if(!*Word) goto endsynt;
		if((n=iswtype(Word))==-3) {
			sprintf(block,"Invalid phrase, '%s', on syntax line!",Word);
			vbprob(block,ls); goto commands;
		}
		if(!*Word) { x=WANY; goto skipeq; }

		// Eliminate illegal combinations
		if(n==WNONE || n==WANY) {
			sprintf(block,"Tried to use %s= on syntax line",syntax[n]); vbprob(block,ls);
			goto endsynt;
		}
		if(n==WPLAYER && strcmp(Word,"me")) {
			vbprob("Tried to specify player other than self",ls);
			goto endsynt;
		}

		// Check "tag" is the correct type
		x=-1;
		switch(n) {
		    case WADJ: // Need ISADJ() - do TT entry too
		    case WNOUN: x=isnoun(Word); break;
		    case WPLAYER:
			if(!strcmp(Word,"me")) x=-3; break;
		    case WROOM:	x=isroom(Word); break;
		    case WSYN:	warne("%s/Internal: Syns not supported!\n",verb.id); x=WANY;
		    case WTEXT:	x=chkumsg(Word); break;
		    case WVERB:	x=is_verb(Word); break;
		    case WCLASS: x=WANY;
		    case WNUMBER:
				if(Word[0]=='-') x=-atoi(Word+1);
				else x=atoi(Word);
		    default:	 printf("** Internal: Invalid W-type!\n");
		}

		if(n==WNUMBER && x>100000 || -x>100000) {
			sprintf(temp,"Invalid number, %ld",x); vbprob(temp,ls);
		}
		if(x==-1 && n!=WNUMBER) {
			sprintf(temp,"Invalid setting, '%s' after %s=",Word,syntax[n+1]);
			vbprob(temp,ls);
		}
		if(x==-3 && n==WNOUN) x=-1;

skipeq:		// (Skipped the equals signs)
		// Now fit into correct slot
		cs=1;		// Noun1
		switch(n) {
		    case WADJ:
			if(vbslot.wtype[0]!=WNONE && (vbslot.wtype[1]!=WNONE || vbslot.wtype[3]!=WNONE)) {
				vbprob(INVALID,ls); n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE) cs=2; else cs=0;
			break;
		    case WNOUN:
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[3]!=WNONE) {
				vbprob(INVALID,ls); n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE) cs=3;
			break;
		    case WPLAYER:
		    case WROOM:
		    case WSYN:
		    case WTEXT:
		    case WVERB:
		    case WCLASS:
		    case WNUMBER:
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[3]!=WNONE) {
				vbprob(INVALID,ls); n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE) cs=3;
			break;
		}
		if(n==-5) goto sp2;
		vbslot.wtype[cs]=n; vbslot.slot[cs]=x;
		goto sp2;

endsynt:	vbslot.ents=0; vbslot.ptr=(struct _VBTAB *)of3p;

commands:	lastc='x';

cmdloop:	p=skipline(ls=s=p); if(!*s) { lastc=1; goto writeslot; }
		s=skipspc(s); if(!*s) goto cmdloop;
		if(!strncmp("syntax=",s,7)) { lastc=0; goto writeslot; }

		vbslot.ents++; r=-1; vt.pptr=(long *)FPos;

		// Process the condition
notloop:	s=precon(s); if(*s=='!') { r=-1*r; s++; }
		s=getword(s);

		// always endparse
		if(!strcmp(Word,ALWAYSEP)) {
			vt.condition=CALWAYS; vt.action=-(1+AENDPARSE); goto writecna;
		}
		if(!strcmp(Word,"not")) { r=-1*r; goto notloop; }

		if((vt.condition=iscond(Word)) == -1) {
			if((vt.action=isact(Word)) == -1) {
				if((vt.action=isroom(Word))!=-1) { vt.condition=CALWAYS; goto writecna; }
				sprintf(block,"Invalid condition, '%s'",Word); vbprob(block,ls);
				goto commands;
			}
			vt.condition=CALWAYS;
			goto doaction;
		}
		if(!(s=chkcparms(s,vt.condition,ofp4))) goto commands;
		if(!*s) {
			if((vt.action=isact(conds[vt.condition]))==-1) {
				sprintf(block,"Missing action after condition '%s'",conds[vt.condition]);
				vbprob(block,ls); goto commands;
			}
			vt.action=0-(vt.action+1); vt.condition=CALWAYS; goto writecna;
		}
		if(r==1) vt.condition=-1-vt.condition;
		s=preact(s); s=getword(s);
		if((vt.action=isact(Word))==-1) {
			if((vt.action=isroom(Word))!=-1) goto writecna;
			sprintf(block,"Invalid action, '%s'",Word); vbprob(block,ls);
			goto commands;
		}
doaction:	if(!(s=chkaparms(s,vt.action,ofp4))) goto commands;
		vt.action=0-(vt.action+1);

writecna:	fwrite((char *)&vt.condition,sizeof(vt),1,ofp3); of3p+=sizeof(vt);
		goto commands;

writeslot:	fwrite(vbslot.wtype,sizeof(vbslot),1,ofp2); of2p+=sizeof(vbslot);
		if(lastc>1) goto commands;
		if(*s && !lastc) goto synloop;

		lastc='\n';
write:		fwrite(verb.id,sizeof(verb),1,ofp1);
		*(vbtab+(verbs-1))=verb;
		if((long)(vbtab+(verbs-1)) > (long)p) printf("@! table overtaking p\n");
	} while(*p);
	errabort();
}

chae_proc(char *f,char *t)	// From and To
{	register int n;

	if(*f<'0' || *f>'9' && *f!='?') { chae_err(); return -1; }

	if(*f=='?') { *(t++)=-1; f++; }
	else {
		n=atoi(f); while(isdigit(*f) && *f) f++;
		if(!*f) { chae_err(); return -1; }
		*(t++)=(char) n;
	}

	for(n=1; n<4; n++) {
		if(*f=='c' || *f=='h' || *f=='a' || *f=='e') { *(t++)=toupper(*f); f++; }
		else { chae_err(); return -1; }
	}

	return 0;
}

chae_err() {
	error("%s: Invalid sort-order flags \"%s\"\n",verb.id,Word);
}

setslots(unsigned char i) {	// Set the VT slots
	vbslot.wtype[0]=WANY; vbslot.wtype[1]=i; vbslot.wtype[2]=WANY; vbslot.wtype[3]=i;
	vbslot.slot[0]=vbslot.slot[1]=vbslot.slot[2]=vbslot.slot[3]=WANY;
}

iswtype(char *s)		// Is 'text' a ptype
{	int i;
	for(i=0; i<nsynts; i++) {
		if(!strcmp(s,syntax[i])) { *s=0; return i-1; }
		if(strncmp(s,syntax[i],syntl[i])) continue;
		if(*(s+syntl[i])!='=') continue;
		strcpy(s,s+syntl[i]+1); return i-1;
	}
	return -3;
}

vbprob(char *s,char *p) {	// Declare a PROBLEM, and which verb its in!
	error("%s: line '%s'\n   > %s!\n",verb.id,p,s);
}

// Before agreeing a match, remember to check that the relevant slot isn't set
// to  NONE.   Variable  N  is  a  wtype...  If the phrases 'noun', 'noun1' or
// 'noun2'  are  used, instead of matching the phrases WTYPE with n, match the
// relevant SLOT with n...
//
// So,  if the syntax line is 'verb text player' the command 'tell noun2 text'
// will  call  isactual with *s=noun2, n=WPLAYER....  is you read the 'actual'
// structure  definition,  'noun2' is type 'WNOUN'.  WNOUN != WPLAYER, HOWEVER
// the  slot  for  noun2 (vbslot.wtype[3]) is WPLAYER, and this is REALLY what
// the user is refering to.

actualval(char *s,int n)
{	register int i;

	if(n!=-70 && (*s=='?' || *s=='%' || *s=='^' || *s=='~' || *s=='\`' || *s=='*' || *s=='#')) {
		if(n!=WNUMBER && !(n==WROOM && *s=='*')) return -1;
		if(*s=='~') return RAND0+atoi(s+1);
		if(*s=='\`') return RAND1+atoi(s+1);
		i=actualval(s+1,-70); if(i==-1) return -1;
		if(*s=='#' && (i&IWORD || (i&MEPRM && i&(SELF|FRIEND|HELPER|ENEMY)))) return PRANK+i;
		if(i&IWORD) switch(*s) {
			case '?': return OBVAL+i;
			case '%': return OBDAM+i;
			case '^': return OBWHT+i;
			case '*': return OBLOC+i;
		}
		return -1;
	}
	if(!isalpha(*s)) return -2;
	for(i=0; i<NACTUALS; i++) {
		if(strcmp(s,actual[i].name)) continue;
		// If not a slot label, and wtypes match, is okay!
		if(!(actual[i].value&IWORD))
			return (actual[i].wtype==n || n==-70) ? actual[i].value:-1;

		// we know its a slot label - check which:
		switch(actual[i].value-IWORD) {
			case IVERB:		// Verb
				if(n==PVERB || n==PREAL) return actual[i].value;
				return -1;
			case IADJ1:		// Adj #1
				if(vbslot.wtype[0]==n) return actual[i].value;
				if(*(s+strlen(s)-1) != '1' && vbslot.wtype[2]==n) return IWORD+IADJ2;
				if(n==PREAL) return actual[i].value;
				return -1;
			case INOUN1:		// noun 1
				if(vbslot.wtype[1]==n) return actual[i].value;
				if(*(s+strlen(s)-1) != '1')
				{
					if(actual[i].wtype==vbslot.wtype[1]) return actual[i].value;
					if(actual[i].wtype==vbslot.wtype[3]) return IWORD+INOUN2;
					if(vbslot.wtype[1]==n || n==PREAL) return actual[i].value;
					return -1;
				}
				if(n==PREAL) return actual[i].value;
				return -1;
			case IADJ2:
				return (vbslot.wtype[2]==n || n==-70) ? actual[i].value:-1;
			case INOUN2:
				return (vbslot.wtype[3]==n || n==-70) ? actual[i].value:-1;
			default: return -1;
		}
	}
	return -2;		// It was no actual!
}

#undef	INVALID
