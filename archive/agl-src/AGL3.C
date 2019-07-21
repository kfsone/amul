//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AGL3.C		AGL System - Subset (3 of 4)
//
//	LMod: oliver 19/06/93	aquit(ASK|FORCE)
//	      oliver 11/06/93	AMUL -> AGL
//

#define	FRAME 1
#define	PORTS 1

#include "adv:frame/aglinc.h"

char	llen;

extern char *obputs[],advfn[];

ans(char *s) {
	register int xp;
	xp=me2->xpos; if(me->flags & ufANSI) txs("[%s",s); me2->xpos=xp;
}

asave() {	save_me(); txn(acp(SAVED),mySCORE);	}

waitfree() {		// PlayerData locking system
	register int i,j;

	if(!ip) iocheck();
	me2->flags=me2->flags&(~PFWORKING);	// I'm not locked yet...
	do {
		Forbid();
		for(i=0,j=0; i<MAXU; i++) {
			if(i!=Af && (lstat+i)->flags&PFWORKING) j++;
		}
		if(!j) me2->flags+=PFWORKING;	// * LOCKED *
		Permit();
		if(j && !ip) iocheck();
	} while(j);				// Wait till no locks.
}

save_me() {		// Update my record.
	if(me2->rec==-1) return;
	if(mySCORE<0) mySCORE=0; waitfree();
	fopena(plyrfn); fseek(afp,me2->rec*sizeof(*me),0);
	fwrite(myNAME,sizeof(*me),1,afp); fclose(afp); afp=NULL;
	me2->flags=me2->flags^PFWORKING;
}

aquit(int ask) {
	char rep[6];
	if(ask) {
		sys(REALLYQUIT); block[0]=0; Inp(rep,5);
		if(toupper(rep[0])=='Y') ask=FALSE;
	}
	if(!ask) {	// Not ELSE because of ^^
		nohelp(); exeunt=1; save_me(); donet=ml+1;
	}
}

char *scopy(register char *d,register char *s) {
	do *(d++)=*s; while (*(s++)); return d;
}

whohere() {
	register int i; char here[3000]; register char *p;

	if(LightHere==NO) return;
	if(((rmtab+myROOM)->flags & HIDE) && myRANK!=ranks-1) { sys(WHO_HIDE); return; }
	*(p=here)=0;
	for(i=0; i<MAXU; i++) {
		if(i!=Af && cansee(Af,i)==YES && !((lstat+i)->flags & PFMOVING)) {
			PutARankInto(str, i); sprintf(p,acp(ISHERE),pNAME(i),str);
			p+=strlen(p);
			if(pFLAG(i,PFSITTING))	p=scopy(p,", sat down");
			if(pFLAG(i,PFLYING))	p=scopy(p,", laid down");
			if(pFLAG(i,PFASLEEP))	p=scopy(p,", asleep");
			if((lstat+i)->state == CHATTING) p=scopy(p,", chatting");
			if((lstat+i)->numobj) p=invent(i,scopy(p,", "));
			else p=scopy(p,". ");
		}
	}
	if(here[0]!=0) tx(here);
}

awho(int type) {
	register int i, j;

	if(type==TYPEV) {
		for(i=0; i<MAXU; i++)
			if(*pNAME(i) && (lstat+i)->state>1 && (!((lstat+i)->flags & PFSINVIS))) {
				str[0]=0; if(isPINVIS(i)) { str[0]='('; str[1]=0; }
				strcat(str,pNAME(i)); strcat(str," the ");
				PutARankInto(str+strlen(str), i);
				strcat(str,acp(ISPLAYING));
				if(i==Af) strcat(str," (you)");
				if(isPINVIS(i)) strcat(str,").\n");
				else strcat(str,".\n");
				tx(str);
			}
	}
	else {
		j=0; str[0]=0;
		for(i=0; i<MAXU; i++)
			if(*pNAME(i) && (lstat+i)->state>1 && (!((lstat+i)->flags & PFSINVIS))) {
				if(i!=Af) {
					if(j++) strcat(str,", ");
					if(isPINVIS(i))
						sprintf(spc,"(%s)",pNAME(i));
					else	sprintf(spc,"%s",pNAME(i));
					strcat(str,spc);
				}
			}
		if(j) txs("%s and you!\n",str); else tx("Just you.\n");
	}
}

numb(register long x,register long n) {
	if(n==x) { return YES; }
	if((n&MORE)==MORE) { n=n-MORE; if(n>x) return YES; }
	if((n&LESS)==LESS) { n=n-LESS; if(n<x) return YES; }
	return NO;
}

atreatas(ULONG verbno) {
	donet=0; inc=0;
	if(tt.verb==-1) {
		vbptr=vbtab+verbno; ml=-(1+verbno);
	}
	iverb=verbno; iocheck();
}

afailparse() {	donet=ml+1; ml=-1; failed=YES; return; }

afinishparse()	{	return;	}

aabortparse() {	donet=ml+1; more=0;	}

ado(int verb) {
	long old_ml,old_donet,old_verb,old_ttv,old_rm; struct _TT_ENT *old_ttabp;
	struct _VERB_STRUCT *ovbptr; struct _SLOTTAB *ostptr;

	old_ml = ml; old_donet = donet; old_verb = iverb; iverb=verb; old_ttv=tt.verb; old_rm=myROOM; old_ttabp=ttabp;
	ovbptr = vbptr; ostptr = stptr;
	lang_proc(verb, 1);
	iverb = old_verb;
	if( failed!=NO || forced || died || exeunt) { donet=ml+1; ml=-1; failed=YES; return; }
	donet = old_donet; ml = old_ml; vbptr=vbtab+donet; tt.verb=old_ttv;
	roomtab=rmtab+old_rm; ttabp=old_ttabp; vbptr = ovbptr; stptr=ostptr;
}

/****** AGL3.C/add_obj ******************************************
*
*   NAME
*	add_obj -- Add an object to a players inventory (movement)
*
*   SYNOPSIS
*	add_obj( Player )
*
*	void add_obj( int );
*
*   FUNCTION
*	Updates the players statistics and settings to indicate addition
*	of an object to his inventory. The objects location IS set to
*	indicate the player (you don't need to move it!). The object must
*	be pointed to by global variable objtab.
*
*   INPUTS
*	Player - number of the player to give it to.
*	objtab - must point to the objects structure.
*
*   NOTES
*	If objtab does NOT point to the right object, things will go astray!
*
*   SEE ALSO
*	rem_obj
*
******************************************************************************
*
*/

add_obj(register int to) {	// Add an object to players inventory
	*objtab->rmlist=-(PGOT+to);
	(lstat+to)->numobj++; (lstat+to)->weight+=STATE->weight;
	(usr+to)->strength-=(((rktab+pRANK(to))->strength*STATE->weight)/(rktab+pRANK(to))->maxweight)*9/10; calcdext();
	if(STATE->flags & SF_LIT) pLIGHT(to)++;
}

/****** AGL3.C/rem_obj ******************************************
*
*   NAME
*	rem_obj -- Remove an object from a players inventory (no move).
*
*   SYNOPSIS
*	rem_obj( Player, Noun )
*
*	void rem_obj( int, int );
*
*   FUNCTION
*	Removes an object from the players inventory without changing the
*	location of the object. Simply all flags pertaining to the posession
*	of the object or requiring it (eg decreased strength, or if its
*	wielded) are cleared.
*
*   INPUTS
*	Player - who's inventory its in.
*	Noun   - the object number.
*
*   NOTES
*	The calling function MUST change the objects location, otherwise
*	the player will effectively still own the object.
*
*   SEE ALSO
*	add_obj()
*
******************************************************************************
*
*/

rem_obj(register int to,register int ob) { // Remove object from inventory
	(lstat+to)->numobj--; (lstat+to)->weight-=STATE->weight;
	(usr+to)->strength+=(((rktab+pRANK(to))->strength*STATE->weight)/(rktab+pRANK(to))->maxweight)*9/10;
	calcdext();
	if(STATE->flags & SF_LIT) pLIGHT(to)--;
	if(me2->wield == ob) me2->wield = -1;	// Don't let me wield it
}

ainteract(register int who) {
	actor = ((lstat+who)->state < PLAYING) ? -1 : who;
}

/****** AGL3.C/asyntax ******************************************
*
*   NAME
*	asyntax -- set new noun1 & noun2, using slot labels too!
*
*   SYNOPSIS
*	asyntax( Noun1, Noun2 )
*
*	void asyntax( ulong, ulong );
*
*   FUNCTION
*	Alters the content of inoun1 and inoun2 along with the word-type
*	slots, thus altering the current input syntax. The actual value
*	of Noun1 and Noun2 is calculated by asyntax, so effectives and
*	slot labels should be passed RAW. This allows a call something
*	like: asyntax( IWORD + INOUN2, IWORD + INOUN1); which is the
*	equivalent of "- syntax noun2 noun1".
*
*   INPUTS
*	Noun1 - unprocessed item for noun1.
*	Noun2 - unprocessed item for noun2.
*
*   EXAMPLE
*	asyntax(*(tt.pptr+ncop[tt.condition]), *(tt.pptr+ncop[tt.condition]+1));
*
*   NOTES
*	If noun1 or noun2 are not REAL values, they will be processed here.
*	Passing a REAL value will assume the passed item was a noun. If you
*	use asyntax(TP1, TP2) you could EASILY be passing a player number!
*
*   SEE ALSO
*	ado, atreatas
******************************************************************************
*
*/

asyntax(int n1,int n2) {
	register unsigned long t1, t2;

	inc=0;
	if(n1==WNONE) { t1=n1; goto noun2; }
	if((n1 & IWORD)) {	// Is it an IWORD?
		switch(n1^IWORD) {
			case IVERB:	n1=iverb;	t1=WVERB;	break;
			case IADJ1:	n1=iadj1;	t1=wtype[0];	break;
			case INOUN1:	n1=inoun1;	t1=wtype[1];	break;
			case IADJ2:	n1=iadj2;	t1=wtype[2];	break;
			case INOUN2:	n1=inoun2;	t1=wtype[3];	break;
			default: n1=inoun1; t1=wtype[1]; break;
		}
	}
	else {
		n1=isnoun((obtab+n1)->id,(obtab+n1)->adj,(vbtab+iverb)->sort); t1=WNOUN;
	}

noun2:	if(n2==WNONE) { t2=n2; goto enoun2; }
	if((n2 & IWORD)) {	// Is it an IWORD?
		switch(n2^IWORD) {
			case IVERB:	n2=iverb;	t2=WVERB;	break;
			case IADJ1:	n2=iadj1;	t2=wtype[0];	break;
			case INOUN1:	n2=inoun1;	t2=wtype[1];	break;
			case IADJ2:	n2=iadj2;	t2=wtype[2];	break;
			case INOUN2:	n2=inoun2;	t2=wtype[3];	break;
			default: n2=inoun1; t2=wtype[1]; break;
		}
	}
	else {
		n2=isnoun((obtab+n2)->id,(obtab+n2)->adj,(vbtab+iverb)->sort2); t2=WNOUN;
	}

enoun2:	inoun1=n1; wtype[1]=t1; inoun2=n2; wtype[3]=t2;
	ml = -(iverb+1);
}

/****** AGL3.C/iocopy ******************************************
*
*   NAME
*	iocopy -- process and copy a string (restricted length).
*
*   SYNOPSIS
*	iocopy( Dest, Source, MaxLen )
*
*	void iocopy( BYTE, BYTE, ULONG );
*
*   FUNCTION
*	Puts the source string through ioproc, causing any escape characters
*	to be processed, and then copies the output to another string.
*
*   INPUTS
*	Dest   - The target string
*	Source - The input string (unprocessed)
*	MaxLen - Maximum number of characters to be copied.
*
*   RESULT
*	Dest   - Remains unchanged.
*	Source - Contains the processed string, upto MaxLen bytes long.
*
*   NOTES
*	MaxLen does NOT include the NULL byte, always allow for this.
*
*   SEE ALSO
*	frame/IOBits.C:ioproc()
*
******************************************************************************
*
*/

iocopy( char *Dest, char *Source, unsigned long Max ) {
	if(!Source || !*Source) { *Dest=0; return; }
	ioproc(Source); qcopy(Dest, ow, Max );
}

// Quick copy - used by iocopy and others

qcopy(char *p2, char *p,int max)
{	register int i;
	Forbid();
	for(i=0; i<max && *p && *p!='\n'; i++) *(p2++)=*(p++);
	*p2=0;
	Permit();
}

/****** AGL3.C/DoThis ******************************************
*
*	DoThis( Player, Command, Type )
*	void DoThis( SHORT, BYTE, SHORT );
*   FUNCTION
*	Forces another player to perfom an action. Type tells the other end
*	how to cope with this. Used for FORCE and FOLLOW.
*   INPUTS
*	Player  - Number of the player to tell.
*	Command - Pointer to the text to be processed.
*	Type    - 0 to FORCE player, 1 to make player FOLLOW.
*
******************************************************************************
*
*/

DoThis(int x, char *cmd, short int type) {
	InterSend(x,type,MFORCE,cmd,NULL,NULL,NULL,NULL);
}

StopFollow() {
	Forbid();
	if(fol || me2->following == -1 || !((vbtab+overb)->flags & VB_TRAVEL)) { Permit(); return; }
	if((lstat+me2->following)->state != PLAYING || (lstat+me2->following)->followed != Af) { me2->following=-1; Permit(); return; }
	(lstat+me2->following)->followed = -1; Permit();
	tx("You are no-longer following @mf.\n"); me2->following = -1;
}

#include "adv:frame/Internal.C"		// Internal Command processor

LoseFollower() {
	if(me2->followed == -1) return;
	(lstat+(me2->followed))->following=-1;	// Unhook them
	utx(me2->followed, "You are no-longer able to follow @me.\n");
	me2->followed = -1; // Unhook me
}

FILE *SFtry(char *f,char *x1,char *x2,FILE *fp) {
	if(fp) return fp; sprintf(block,f,x1,x2); return fopen(block,"rb");
}

ShowFile(char *s,char *def) {
	FILE *fp; register long fsize; register char *p;

	fp=NULL;
	fp=SFtry("%s%s.text",dir,s,fp); fp=SFtry("%s%s",dir,s,fp);
	fp=SFtry("%s.text",s,NULL,fp);  fp=SFtry("%s",s,NULL,fp);
	if(!fp) {
		if(def) tx(def);
		else { sprintf(block,"*Missing file %s\n",s); log(block); }
		return;
	}
show:
	fseek(fp,0,2L); fsize=((ftell(fp)/OWLIM)+1)*OWLIM; fseek(fp,0,0L);
	if(!(p=(char *)AllocMem(fsize,STDMEM))) {
		fclose(fp);
		strcpy(block,"*System out of memory!\n"); log(block);
		txc(7); tx(block);
		forced=1; exeunt=1; kquit("out of memory!\n");
	}
	fread(p, fsize, 1, fp); fclose(fp); tx(p); FreeMem(p, fsize);
}

scaled(long value,short int flags) {	// Scale an object value
	register scalefactor;

	if(!(flags & SF_SCALED)) return value;
	scalefactor = ((rscale * myRANK * 100 / ranks) + (tscale * *rescnt * 100 / (mins * 60))) / 100;
	return value - ( value * scalefactor / 100 );
}

/****** AGL3.C/showin ******************************************
*
*   NAME
*	showin -- Display the contents of an object.
*
*   SYNOPSIS
*	showin( Object, Mode )
*
*	void showin( int, int );
*
*   FUNCTION
*	Displays the contents of an object, modified depending on the
*	objects 'putto' flag. Mode determines whether output is given when
*	the contents of the object cannot be seen or there it is empty.
*
*   INPUTS
*	Object -- the object's id number.
*	Mode   -- YES to force display of contents, or to inform the player
*		  if the object is empty.
*		  NO not to list the contents of the object if it is opaque,
*		  and not to display anything when it is empty.
*
******************************************************************************
*
*/

showin(int o,int mode) {	// Show contents of object
	register int i,j,k,l; register char *p,*s;

	*(p=str)=0; if(State(o)->flags & SF_OPAQUE && mode==NO) return;
	if(!((obtab+o)->inside<=0 && mode==NO)) {
		if((obtab+o)->putto==0) sprintf(p,"The %s contains ",(obtab+o)->id);
		else sprintf(p,"%s the %s you find: ",obputs[(obtab+o)->putto],(obtab+o)->id);
	}
	if((obtab+o)->inside<=0) {
		if(mode==YES) strcat(p,"nothing."); return;
	}

	p+=strlen(p);

	for(i=0,j=0,k=-(INS+o),l=(obtab+o)->inside; i<nouns && l>0; i++) {
		if(*(obtab+i)->rmlist!=k) continue;
		if(j) { *(p++)=','; *(p++)=' '; }
		s=(obtab+i)->id; while(*s) *(p++)=*(s++); *p=0;
		j=1; l--;
	}
	strcat(p,".");
}

stfull(int st, int p) {	// full <st> <player>
	you = (usr+p); you2 = (lstat+p);
	switch(st) {
		case STSCORE:	return NO;
		case STSCTG:	return NO;
		case STSTR:	if(you->strength < (rktab+you->rank)->strength) return NO; break;
		case STDEX:	if(you->dext < (rktab+you->rank)->dext) return NO; break;
		case STSTAM:	if(you->stamina < (rktab+you->rank)->stamina) return NO; break;
		case STWIS:	if(you->wisdom < (rktab+you->rank)->wisdom) return NO; break;
		case STMAGIC:	if(you->magicpts < (rktab+you->rank)->magicpts) return NO; break;
		case STEXP:	if(you->experience < (rktab+hisRANK)->experience) return NO; break;
	}
	return YES;
}

asetstat(int obj,int stat) {
	register long i,j,x,f; long w;

	objtab=obtab+obj;
	w=ObjRemove(obj); x=owner(obj); i=lit(loc(obj));
	// Remove from owners inventory
	if(x!=-1) { w=(lstat+x)->wield; rem_obj(x,obj); }
	f=STATE->flags & SF_LIT; objtab->state=stat;
	if(x!=-1) { add_obj(x); (lstat+x)->wield=w; }
	else ObjInto(obj,w);
	if(objtab->flags & OF_SHOWFIRE) {
		if(f) STATE->flags = STATE->flags | SF_LIT;
		else STATE->flags = STATE->flags & -(1+SF_LIT);
		return;	// Don't need to check for change of lights!
	}

	if(x!=-1) lighting(x,AHERE);

	if((j=lit(loc(obj))) == i) return;
	if(j==NO) { actionfrom(obj,acp(NOWTOODARK)); sys(NOWTOODARK); }
	else	  { actionfrom(obj,acp(NOWLIGHT)); sys(NOWLIGHT); }
}

awhere(register int obj) {
	register int i,j,k; register long *rp; short int found;

	found=-1;
	for(i=0; i<nouns; i++) {	// Check all
		if(!strcmp((obtab+obj)->id,(obtab+i)->id)) {
			if(objvisibleto(i,Af)==NO) continue;
			if((j=owner(i))!=-1) {
				if(lit(pROOM(j))==YES) {
					if(j!=Af) {
						tx("You see "); ans("1m"); tx(pNAME(j)); ans("0m"); tx(".\n");
					}
					else tx("You are carrying one.\n");
					found++;
				}
				continue;
			}
			rp=(obtab+i)->rmlist;
			for(j=0; j<(obtab+i)->nrooms; j++) {
				if(*(rp+j)==-1) continue;
				if(*(rp+j)>=0) {
					if(*(rp+j)==-1 || lit(*(rp+j))==NO) continue;
					roomtab=rmtab+*(rp+j); desc_here(2);
				}
				else {
					k=-(INS+*(rp+j));
					sprintf(block,"In the %s!\n",(obtab+k)->id);
					tx(block);
				}
				found++;
			}
		}
	}
	if(found==-1) sys(SPELLFAIL);
}

osflag(int o, short int flag) {
	register int own,i,oflag; register long *rm; register struct _OBJ_STRUCT *objp;

	own=owner(o); objp=objtab=obtab+o; oflag=STATE->flags;
	if(own!=-1) rem_obj(own,o);

	// Which rooms have we lit?
	if(!(oflag & SF_LIT) && (flag & SF_LIT)) {
		rm=objp->rmlist;
		for(i=0; i<objp->nrooms; i++,rm++) {
			if(lit(*rm)==NO) actionin(*rm,acp(NOWLIGHT));
		}
	}

	objp=objtab=obtab+o; STATE->flags = flag; if(own != -1) add_obj(o);

	// Which rooms have we darkened?
	if((oflag & SF_LIT) && !(flag & SF_LIT)) {
		rm=objp->rmlist;
		for(i=0; i<objp->nrooms; i++,rm++) {
			if(lit(*rm)==NO) actionin(*rm,acp(NOWTOODARK));
		}
	}
}

// Set @xx and @xy corresponding to a specific player

setmxy(register int Flags, register int Them) {
	if(Af>MAXU) return;
	if( Them == Af || cansee(Them, Af) == YES ) {	// If he can see me
		ioproc("@me");		strcpy(mxx,ow);
		ioproc("@me the @mr");	strcpy(mxy,ow);
		return;
	}
	if( pROOM(Them) == myROOM ) {
		switch(Flags) {
			case ACTION:
			case EVENT:
			case TEXTS:
				strcpy(mxx,"Someone nearby"); strcpy(mxy,"Someone nearby");
				return;
			case NOISE:
				strcpy(mxx,"Someone nearby");
				ioproc("A @gn voice nearby"); strcpy(mxy,ow);
				return;
		}
	}
	// They aren't in the same room
	switch(Flags) {
		case ACTION:
		case EVENT:
			strcpy(mxx,"Someone");
			if(myRANK == ranks-1) strcpy(mxy,"Someone very powerful");
			else strcpy(mxy,"Someone");
			return;
		case TEXTS:
			ioproc("@me");	strcpy(mxx,ow);
			if(myRANK == ranks-1) ioproc("@me the @mr");
			strcpy(mxy,ow);
			return;
		case NOISE:
			strcpy(mxx,"Someone");
			if(myRANK == ranks-1) ioproc("A powerful @gn voice somewhere in the distance");
			else ioproc("A @gn voice in the distance");
			strcpy(mxy,ow);
			return;
		default:
			strcpy(mxx,"Someone"); strcpy(mxy,"Someone");
	}
}

settitle() {
	if(iosup != CUSSCREEN) return;
	sprintf(wtil,"%s   Line: %2d  Player: %s",vername,Af,myNAME);
	SetWindowTitles(wG,wtil,wtil);
}

look(char *s,int f) {
	register int	roomno,mod;

	// Some complex stuff here!
	// if f==0 (rdmode=RoomCount) and we've been here before,  look(brief)
	// if f==0 (rdmode=RoomCount) and this is our first visit, look(verbose)
	// if f==0 visit the room

	if((roomno=isroom(s))==-1) return;
	roomtab=rmtab+roomno;
	if (f==RDRC && ((*(rctab+roomno) & rset) != rset)) mod=RDVB;
	else mod=f;
	if(f!=2) *(rctab+roomno) = *(rctab+roomno) | rset;
	look_here(mod,roomno);
}

ObjRemove(long obj) {
	register long own;
	if((own=-(INS+*(obtab+obj)->rmlist))>=0) {
		(obtab+own)->inside--; (obtab+own)->winside-=State(obj)->weight;
		return own;
	}
	return -1;
}

ObjInto(SHORT obj,SHORT into) {
	if(into==-1) return;
	*(obtab+obj)->rmlist=into; (obtab+into)->inside++;
	(obtab+into)->winside+=State(obj)->weight;
}

putobjin(register int obj,register int con) {	// Put obj into con
	register int own;

	// 1: Will object go?
	if(!willobjgo(obj,con)) {
		afailparse(); return FALSE;
	}

	// 2: Set fire to container if flamable and object is lit

	// 3: Remove it from it's container, if applicable
	ObjRemove(obj);

	// 4: Remove from object/players inventory (if applicable)
	if((own=owner(obj))!=-1) rem_obj(own,obj);

	// 5: Place inside destination (con)
	ObjInto(obj,con);

	// 6: Check owners lighting (if applic)
	if(own!=-1) lighting(own,AHERE);
}

agive(register int obj,register int to) {	// Add object to inventory
	register int own,orm;

	objtab=obtab+obj;

	if((objtab->flags&OF_SCENERY) || (STATE->flags&SF_ALIVE) || objtab->nrooms!=1) return;
	orm=loc(obj);
	if((own=owner(obj))!=-1) rem_obj(own,obj);
	else ObjRemove(obj);
	// Add to player/mobile's inventory
	if(to<MAXU) add_obj(to); else ObjInto(obj,me2->rec);

	// The lighting conditions for transfering an object between a
	// variable source and destination are complex! See below!
	if(STATE->flags & SF_LIT) {
		if(own==-1) {	// Did I just pick and was it from here?
			if(orm == pROOM(to)) return;
		}
		else {	// If I got it from someone, and he is near me ...
			if(pROOM(own)==pROOM(to)) return;
			lighting(own,AHERE);	// Else check his lights!
		}
		lighting(to,AHERE);
	}
}

adrop(int ob,int r,int f) {	// Drop object (to a room)
	objtab=obtab+ob; *objtab->rmlist=r; rem_obj(Af,ob); lighting(Af,AHERE);

	// If the room IS a 'swamp', give em points
	if((rmtab+myROOM)->flags&SANCTRY) {
		if(!exeunt && !died) aadd(scaled(STATE->value, STATE->flags), STSCORE, Af);
		*objtab->rmlist=-1; 
	}
}

aprovoke(int mobj,int finc) {
	register int i;
	// mob = object to provoke, finc = chance of fight increase!

	if(!(State(mobj)->flags & SF_ALIVE)) return;

	for(i=0;i<mobs;i++) {
		if((mtab+i)->obj==mobj) return SendIt(MPROVOKE,i,(char *)finc);
	}
}

ablast(int n,char *t1,char *t2) {
	register int i,o,die;

	die=NO;
	for(i=0; i<MAXU; i++) {
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & PFDEAF)) continue;
		setmxy(NOISE,i);
		if((o=owner(n))!=-1 && pROOM(o)!=pROOM(i)) { utx(i,t2); continue; }
		if(o==-1 && isin(n,pROOM(i))==NO) { utx(i,t2); continue; }
		utx(i,t1);
		if(i==Af) die=YES; else interact(MDIE,i,DED);
	}
	if(die==YES) akillme(DED);
}

arandgo(int n) {
	short int start_rm[150]; register int i,nrs;

	if(n) {		// randomgo ANY
		do 
			i=Random(rooms-1);
		while((rmtab+i)->flags & (NOGORM + NOEXITS + DEATH));
		return i;
	}
	else {		// randomgo START
		for(i=0,nrs=0;i<rooms && nrs<150;i++)
			if((rmtab+i)->flags & STARTL) start_rm[nrs++]=i;

		if(!nrs) return 0;
		return (int)start_rm[(nrs==1)?0:Random(nrs-1)];
	}
}

findpers() {
	register int j,k;
	j=FALSE; k=sizeof(him); rewind(afp);
	while(fread(me->name,k,1,afp)) {
		if(!stricmp(me->name,him.name)) {
			j=TRUE; break;
		}
	}
	me2->rec=ftell(afp)/k; return j;
}

listpers() {	// List personas on this account
	struct _PLAYER fred;	register int k,on;

	if(!him.passwd[0]) return;

	fopena(plyrfn); block[0]=0; him.plays=0; k=sizeof(fred); on=FALSE;
	while(fread(fred.name,k,1,afp)) {
		if(!stricmp(him.passwd,fred.passwd)) {
			if(block[0]) strcat(block,", ");
			if(isplayer(fred.name)!=-1) { strcat(block,"*"); on=TRUE; }
			strcat(block,fred.name); him.plays++;
		}
	}
	fclose(afp); afp=NULL;
	txs(">> Current characters: %s.\nYou can have upto 5 characters on this account.\n",(him.plays)?block:"None");
	if(on==TRUE) tx("[* indicates character currently in-play]\n");
}

log(char *s) {
	if(Af<0 || !link) return; ioproc(s); s=ow;
	while(*s) { if(*s=='\n' || *s=='\r') strcpy(s,s+1); else s++; }
	SendIt(MLOG,NULL,ow);
}

aman_connect() {
	SendIt(MDATAREQ,0,dir);		// Get basic data
	rescnt=(short *)Ap1; lastres=(char *)Ap2; lastcrt=(char *)Ap3; him.last_session=Ap4;

	fopenr(advfn);
	fgets(adname,41,ifp); adname[strlen(adname)-1]=0;
	rooms=nextno(); ranks=nextno(); verbs=nextno(); syns=nextno();
	nouns=nextno(); adjs=nextno(); ttents=nextno(); umsgs=nextno();
	word=nextno(); mins=nextno(); invis=nextno(); invis2=nextno();
	minsgo=nextno(); mobs=nextno(); rscale=nextno(); tscale=nextno();
	mobchars=nextno();
	
	fclose(ifp); ifp=NULL;

	SendIt(MDATAREQ, 1,NULL); rmtab=(struct room *)Ap; errtxt=(char **)Ap1;
	SendIt(MDATAREQ, 2,NULL); rktab=(struct rank*)Ap;
	SendIt(MDATAREQ, 3,NULL); obtab=(struct obj *)Ap; ormtab=Ap1; statab=(struct state *)Ap2; adtab=(char *)Ap3; desctab=(char *)Ap4;
	SendIt(MDATAREQ, 4,NULL); vbtab=(struct verb *)Ap; vtp=(struct vt *)Ap1; vtpp=(long *)Ap2;
	SendIt(MDATAREQ, 5,NULL); ttp=(struct tt *)Ap; ttpp=(long *)Ap1;
	SendIt(MDATAREQ, 6,NULL); umsgip=(long *)Ap; umsgp=(char *)Ap1;
	SendIt(MDATAREQ, 7,NULL); rctab=(short *)Ap;
	SendIt(MDATAREQ, 8,NULL); slottab=(struct vbslot *)Ap;
	SendIt(MDATAREQ, 9,NULL); synp=Ap; synip=(short int *)Ap1;
	SendIt(MDATAREQ,10,NULL); mobp=(struct _MOB_ENT *)Ap; mtab=(struct _MOB_TAB *)Ap1;
}

nextno() { long x; fread((char *)&x,4,1,ifp); return x; }

deduct(int plyr, int howmuch) {
	if(howmuch<0) return;
	if(plyr==Af) asub(mySCORE*howmuch/100,STSCORE,Af);
	else sendex(plyr,ADEDUCT,plyr,howmuch,0,0);
}

magic(int rnk,int points,int chance) {
	if(myRANK < rnk-1) {sys(LOWLEVEL); return NO;}

	if(me->magicpts < points) {sys(NOMAGIC); return NO;}

	if(myRANK) chance=(myRANK+1-rnk)*((100-chance)/(ranks-rnk))+chance;
	if(Random(100) < chance) { sys(SPELLFAIL); return NO; }
	me->magicpts-=points;
	return YES;
}

ShowPrompt() {			// Display players current prompt
	NewLine(); tx((me2->state==CHATTING)?"@me> ":(rktab+myRANK)->prompt);
	addcr=NO;
}
