/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL3.C......Adventure System      ****
              ****            The Saga Continues!           ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.


               Variables and structures used in AMUL & AMUL2
                           and assorted routines
									    */


#define	FRAME 1
#define	PORTS 1

#include "h/AMUL.Defs.H"
#include "h/AMUL.Incs.H"
#include "h/AMUL.Vars.H"		/* all INTERNAL variables	*/

#define	dtx(x)	if(debug!=0) tx(x)
short int debug;				/* Is debug mode on?	 */
char	llen;

extern char *obputs[];			/* Object put-to's! */

struct Screen *sC;
struct Window *wG;
struct IOExtSer *serio,*wserio;
struct Task   *MyTask;
struct ViewPort vP;
struct RastPort *rpG;
struct MsgPort *ReadRep,*WriteRep,*repbk;
struct IOStdReq ReadIo, WriteIo;

/* Frame specific variables */
char	serop,MyFlag;			/* SerPort open? What am I? */
char	*input;				/* 400 bytes, 5 lines */
char	str[800],spc[200],mxx[40],mxy[60];	/* Output string */
char	wtil[80];			/* Window title */
char	iosup;				/* What kind of IO support */
char	inc,forced,failed,died,addcr,fol; /* For parsers use */
char	actor,last_him,last_her;	/* People we talked about */
char	autoexits,needcr;		/* General flags */
long	iverb,overb,iadj1,inoun1,iprep,iadj2,inoun2,lverb,ldir,lroom;
long	wtype[6],word,mins;		/* Type of word... */
unsigned short int *rescnt;		/* Reset counter from AMan */
short int donev,skip;			/* No. of vb's/TT's done */
char	exeunt,more,link;		/* If we've linked yet */
long ml,donet,it;			/* Maximum lines */
struct Aport *ap,*amanp,*intam;		/* The message pointers */
struct MsgPort *amanrep;		/* AMAN reply port */
char	*ob,*gp,*ow,*lastres,*lastcrt;	/* Pointer to output buffers etc */
short int rset, rclr, ip, csyn;		/* Masks for Room Counter */

/*===============Stuff that didn't fit into AMUL any more==================*/

ans(register char *s)
{
	if(me->flags & ufANSI) txs("[%s",s);
}

asave()
{
	save_me(); txn(acp(SAVED),me->score);
}

save_me()		/* Update my record. */
{
	if(me->score<0) me->score=0;
	fopena("Players Data"); fseek(afp,me2->rec*sizeof(*me),0);
	fwrite(me->name,sizeof(*me),1,afp); fclose(afp); afp=NULL;
}

aquit()
{
	sys(REALLYQUIT); block[0]=0; Inp(block,4);
	if(toupper(block[0])=='Y')
	{
		nohelp(); exeunt=1; save_me(); donet=ml+1;
	}
}

/*== This must abide FULLY by INVIS & INVIS2... Should we make it so that
     visible players can't see an invisible players entry, they can just
     see that they're here?						  */
whohere()
{	register int i;

	if(lit(me2->room)==NO) return;
	if(((rmtab+me2->room)->flags & HIDE)!=NULL && me->rank!=ranks-1) { sys(WHO_HIDE); return; }
	for(i=0; i<MAXU; i++)
	{
		if(i!=Af && cansee(Af,i)==YES && !((lstat+i)->flags & PFMOVING))
		{
			PutARankInto(str, i);
			sprintf(block,acp(ISHERE),(usr+i)->name,str);
			if (((lstat+i)->flags & PFSITTING) != 0) strcat(block,", sitting down");
			if (((lstat+i)->flags & PFLYING) != 0) strcat(block,", lying down");
			if (((lstat+i)->flags & PFASLEEP) != 0) strcat(block,", asleep");
			if((lstat+i)->numobj==0) { strcat(block,".\n"); tx(block); }
			else { strcat(block," "); invent(i); }
		}
	}
}

awho(register int type)
{	register int i, j;

	if(type==TYPEV)
	{	for(i=0; i<MAXU; i++)
			if((usr+i)->name[0]!=0 && (lstat+i)->state>1 && (!((lstat+i)->flags & PFSINVIS)))
			{
				str[0]=0; if(isPINVIS(i)) { str[0]='('; str[1]=0; }
				strcat(str,(usr+i)->name); strcat(str," the ");
				PutARankInto(str+strlen(str), i);
				strcat(str,acp(ISPLAYING));
				if(i==Af) strcat(str," (you)");
				if(isPINVIS(i)) strcat(str,").\n");
				else strcat(str,".\n");
				tx(str);
			}
	}
	else
	{
		j=0; str[0]=0;
		for(i=0; i<MAXU; i++)
			if((usr+i)->name[0]!=0 && (lstat+i)->state>1 && (!((lstat+i)->flags & PFSINVIS)))
			{
				if(i!=Af)
				{
					if(j++!=0) strcat(str,", ");
					if(isPINVIS(i))
						sprintf(spc,"(%s)",(usr+i)->name);
					else
						sprintf(spc,"%s",(usr+i)->name);
					strcat(str,spc);
				}
			}
		if(j==0) tx("Just you.\n"); else txs("%s and you!\n",str);
	}
}

flagbits()		/* Get the users flag bits */
{
	me->llen = DLLEN; me->slen = DSLEN; me->rchar = DRCHAR;
	sprintf(spc,"Default settings are:\n\nScreen width = %ld, Lines = %ld, Redo = '%c', Flags = ANSI: %s, Add LF: %s\n\nChange settings (Y/n): ",(me->llen),(me->slen),me->rchar,(me->flags & ufANSI)?"On":"Off",(me->flags & ufCRLF)?"On":"Off");
	tx(spc); Inp(str,2);
	if(toupper(str[0])=='N') return;

	getllen(); getslen(); getrchar(); getflags(); save_me();
}

getllen()
{
	sprintf(input,"%ld %s",me->llen,"characters"); sprintf(str,"\nEnter %s%s[%s]: ","screen width"," ",input); tx(str);
	Inp(str,4); if(str[0]!=0) me->llen=atoi(str);
	sprintf(input,"%ld %s",me->llen,"characters"); sprintf(str,"%s set to %s.\n","screen width",input); tx(str);
}

getslen()
{
	sprintf(input,"%ld %s",me->slen,"lines"); sprintf(str,"\nEnter %s%s[%s]: ","screen length"," (0 to disable MORE? prompting) ",input); tx(str);
	Inp(str,3); if(str[0]!=0) me->slen=atoi(str);
	sprintf(input,"%ld %s",me->slen,"lines"); sprintf(str,"%s set to %s.\n","screen length",input); tx(str);
}

getrchar()
{	char rchar;
	rchar=me->rchar; me->rchar=0;
	sprintf(input,"currently \"%c\"",rchar); sprintf(str,"\nEnter %s%s[%s]: ","redo-character"," ",input); tx(str);
	Inp(str,2); if(str[0]!=0) me->rchar=str[0]; else me->rchar=rchar;
	if(me->rchar=='/' || isspace(str[0])) { tx("Invalid redo-character (how do you expect to do / commands?)\n"); me->rchar = rchar; return; }
	sprintf(input,"\"%c\"",me->rchar); sprintf(str,"%s set to %s.\n","redo-character",input); tx(str);
}

getflags()
{
	tx("Follow CR with a Line Feed? "); if(me->flags & ufCRLF) tx("[Y/n]: "); else tx("[y/N]: ");
	Inp(str,2);
	if(toupper(str[0])=='Y' || toupper(str[0])=='N') me->flags=me->flags & (toupper(str[0])=='Y') ? ufCRLF : -(1+ufCRLF);
	tx("Use ANSI control codes?     "); if(me->flags & ufANSI) tx("[Y/n]: "); else tx("[y/N]: ");
	Inp(str,2);
	if(toupper(str[0])=='Y' || toupper(str[0])=='N') me->flags=me->flags & (toupper(str[0])=='Y') ? ufANSI : -(1+ufANSI);
}

/*============= Please adhere to AutoDoc style docs herewith ================*/

/*===========================================================================*
 *
 * The following functions are low-level, and considered fairly constant.
 * However, some of them (or all of them) may be moved to the AMUL.Library
 * at a later date...
 *
 *===========================================================================*/


/****** amul.library/numb ******************************************
*
*   NAME
*	numb -- mathematically compare two numbers, including <> and =.
*
*   SYNOPSIS
*	ret = numb( Number, Value )
*	d0            d0      d1
*
*	BOOLEAN numb( ULONG, ULONG );
*
*   FUNCTION
*	Quantifies and/or equates the value of two numbers.
*
*   INPUTS
*	Number - Real integer numeric value.
*	Value  - Integer value with optional quantifier (MORE/LESS) ie
*	         '<' or '>'.
*
*   RESULT
*	ret    - TRUE if Number equates with Value (ie 5 IS <10)
*
*   EXAMPLE
*	numb(10, ( LESS & 20 ));	Returns TRUE.
*    Lang.Txt:
*	numb ?brick >%wall
*
*   NOTES
*	Remember to process the values using ACTUAL before passing them,
*	thus: Numb(actual(n1), actual(n2));
*
******************************************************************************/

numb(register long x,register long n)
{
	if(n==x) { return YES; }
	if((n&MORE)==MORE) { n=n-MORE; if(n>x) return YES; }
	if((n&LESS)==LESS) { n=n-LESS; if(n<x) return YES; }
	return NO;
}

/*===========================================================================*
 *
 * The following functions are considered low-level enough to stay unchanged.
 * The will NOT be made into library calls, as they operate ONLY on local
 * variables, and are rigid enough to prevent this. Please make sure you do
 * NOT place functions likely to change regularily in here!
 *
 *===========================================================================*/


/****** AMUL3.C/atreatas ******************************************
*
*   NAME
*	atreatas -- switch parseing to another verb.
*
*   SYNOPSIS
*	atreatas( NewVerb )
*
*	void atreatas( int );
*
*   FUNCTION
*	Switches current processing to a new verb, keeping the old
*	syntax (ie iwords).
*
*   INPUTS
*	NewVerb - the number of the new verb to be processed.
*
*   RESULT
*	vbptr   - points to new verbs table entry.
*	iverb   - set to NewVerb
*	ml      - set to -(1+verb) to reset the current loop
*
*   SEE ALSO
*	amul2.c/asyntax ado
*
******************************************************************************
*
*/

atreatas(register ULONG verbno)
{
	donet=0; inc=0;
	if(tt.verb==-1)
	{
		vbptr=vbtab+verbno; ml=-(1+verbno);
	}
	iverb=verbno; iocheck();
}

/****** amul3.c/afailparse ******************************************
*
*   NAME
*	afailparse -- flags the current parse as having failed
*
*   SYNOPSIS
*	void failparse();
*
*   FUNCTION
*	Prevents continued parseing of the current phrase. Any
*	other phrases will be parsed, but this won't. Used for
*	looped-parses caused by such as 'All' and noun-classes.
*
*   SEE ALSO
*	aabortparse, afinishparse, aendparse
*
******************************************************************************
*
*/

afailparse()
{
	donet=ml+1; ml=-1; failed=YES; return;
}

/****** amul3.c/afinishparse ******************************************
*
*   NAME
*	afinishparse -- unused
*
*   SYNOPSIS
*
*   FUNCTION
*
*   SEE ALSO
*	aabortparse, afailparse, aendparse
*
******************************************************************************
*
*/

afinishparse()	{	return;	}

/****** amul3.c/aabortparse ******************************************
*
*   NAME
*	aabortparse -- unused
*
*   SYNOPSIS
*
*   FUNCTION
*
*   SEE ALSO
*	aabortparse, afailparse, aendparse
*
******************************************************************************
*
*/

aabortparse()
{
	donet=ml+1; more=0;
}

/****** AMUL3.C/ado ******************************************
*
*   NAME
*	ado -- AMUL's equivalent of GOSUB
*
*   SYNOPSIS
*	ado( Verb )
*
*	void ado( int );
*
*   FUNCTION
*	Causes the parser to process <verb> before continuing the current
*	parse. This works similiarily to GOSUB in Basic, and allows the user
*	to write sub-routine verbs.
*
*   INPUTS
*	Verb - the verb to be processed.
*
*   EXAMPLE
*	ado("\"travel");	/ * Visits the "travel verb. * /
*
*   SEE ALSO
*	atreatas
*
******************************************************************************
*
*/

ado(register int verb)
{	long old_ml,old_donet,old_verb,old_ttv,old_rm; struct _TT_ENT *old_ttabp;
	struct _VERB_STRUCT *ovbptr; struct _SLOTTAB *ostptr;

	old_ml = ml; old_donet = donet; old_verb = iverb; iverb=verb; old_ttv=tt.verb; old_rm=me2->room; old_ttabp=ttabp;
	ovbptr = vbptr; ostptr = stptr;

	lang_proc(verb, 1);

	iverb = old_verb;

	if( failed!=NO || forced!=0 || died!=0 || exeunt!=0) { donet=ml+1; ml=-1; failed=YES; return; }

	donet = old_donet; ml = old_ml; vbptr=vbtab+donet; tt.verb=old_ttv;
	roomtab=rmtab+old_rm; ttabp=old_ttabp; vbptr = ovbptr; stptr=ostptr;
}

/****** AMUL3.C/add_obj ******************************************
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

add_obj(register int to)	/*== Add an object into a players inventory */
{
	*objtab->rmlist=-(5+to);	/* It now belongs to him */
	(lstat+to)->numobj++; (lstat+to)->weight+=STATE->weight;
	(lstat+to)->strength-=((rktab+(usr+to)->rank)->strength*STATE->weight)/(rktab+(usr+to)->rank)->maxweight;
	if(STATE->flags & SF_LIT) (lstat+to)->light++;
}

/****** AMUL3.C/rem_obj ******************************************
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

rem_obj(register int to,register int ob) /*== Remove object from inventory */
{
	(lstat+to)->numobj--; (lstat+to)->weight-=STATE->weight;
	(lstat+to)->strength+=((rktab+(usr+to)->rank)->strength*STATE->weight)/(rktab+(usr+to)->rank)->maxweight;
	if(STATE->flags & SF_LIT) (lstat+to)->light--;
	if(me2->wield == ob) me2->wield = -1;	/*== Don't let me wield it */
}

/****** AMUL3.C/ainteract ******************************************
*
*   NAME
*	ainteract -- mask player out from 'action'/'announce' messages
*
*   SYNOPSIS
*	ainteract( Player )
*
*	void ainteract( short int );
*
*   FUNCTION
*	Masks a player as currently being 'interacted' with. This currently
*	only affects messageing, ie ACTION and ANNOUNCE (and their
*	derivatives) don't reach the other player. For example, if you
*	give something to someone nearby, and want to tell the OTHER
*	players in the room. In the future this may be used as a mini-locking
*	system, for example to prevent a player leaving a room halfway through
*	someone giving him something, or as someone attacks him.
*
*   INPUTS
*	Player - number of a player online
*
*   NOTES
*	PLEASE use this whenever you ARE interacting with a player, and
*	you have a few lines of Lang.Txt to go... In future this will
*	prevent ALL sorts of things, eg the player logging out three-quarters
*	of the way through your action.
*
******************************************************************************
*
*/

ainteract(register int who)
{
	actor = -1;
	if((lstat+who)->state != PLAYING) return;
	actor = who;
}

/****** AMUL3.C/asyntax ******************************************
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

asyntax(register int n1,register int n2)
{
	register unsigned long t1, t2;

	inc=0;
	/* === N1 Handling === */
	if(n1==WNONE) t1=n1;
	else if((n1 & IWORD))	/* Is it an IWORD? */
	{
		switch(n1 & -(1+IWORD))
		{
			case IVERB:	n1=iverb;	t1=wtype[0];	break;
			case IADJ1:	n1=iadj1;	t1=wtype[1];	break;
			case INOUN1:	n1=inoun1;	t1=wtype[2];	break;
			case IPREP:	n1=iprep;	t1=wtype[3];	break;
			case IADJ2:	n1=iadj2;	t1=wtype[4];	break;
			case INOUN2:	n1=inoun2;	t1=wtype[5];	break;
			default: n1=inoun1; t1=wtype[2]; break;
		}
	}
	else
	{
		n1=isnoun((obtab+n1)->id,(obtab+n1)->adj,(vbtab+iverb)->sort); t1=WNOUN;
	}

	/* === N2 Handling === */
	if(n2==WNONE) t2=n2;
	else if((n2 & IWORD))	/* Is it an IWORD? */
	{
		switch(n2 & -(1+IWORD))
		{
			case IVERB:	n2=iverb;	t2=wtype[0];	break;
			case IADJ1:	n2=iadj1;	t2=wtype[1];	break;
			case INOUN1:	n2=inoun1;	t2=wtype[2];	break;
			case IPREP:	n2=iprep;	t2=wtype[3];	break;
			case IADJ2:	n2=iadj2;	t2=wtype[4];	break;
			case INOUN2:	n2=inoun2;	t2=wtype[5];	break;
			default: n2=inoun1; t2=wtype[2]; break;
		}
	}
	else
	{
		n2=isnoun((obtab+n2)->id,(obtab+n2)->adj,(vbtab+iverb)->sort2); t2=WNOUN;
	}

	inoun1=n1; wtype[2]=t1; inoun2=n2; wtype[5]=t2;
	ml = -(iverb+1);
}

/****** AMUL3.C/iocopy ******************************************
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

iocopy( char *Dest, char *Source, unsigned long Max )
{
	ioproc(Source); qcopy(Dest, Source, Max );
}

/* -- Quick copy - used by iocopy and others -- */

qcopy(char *p2, char *p,int max)
{	register int i;
	Forbid();
	for(i=0; i<max && *p!=0 && *p!='\n'; i++) *(p2++)=*(p++);
	*p2=0;
	Permit();
}

/****** AMUL3.C/DoThis ******************************************
*
*   NAME
*	DoThis -- Tell another player to follow me or perform an action.
*
*   SYNOPSIS
*	DoThis( Player, Command, Type )
*
*	void DoThis( SHORT, BYTE, SHORT );
*
*   FUNCTION
*	Forces another player to perfom an action. Type tells the other end
*	how to cope with this. Used for FORCE and FOLLOW.
*
*   INPUTS
*	Player  - Number of the player to tell.
*	Command - Pointer to the text to be processed.
*	Type    - 0 to FORCE player, 1 to make player FOLLOW.
*
*   EXAMPLE
*	DoThis( TP1, "quit", 0 );	/-* Force player to quit *-/
*	DoThis( TP1, "east", 1 );	/-* Force them to follow you east *-/
*
******************************************************************************
*
*/

DoThis( register int x, register char *cmd, register short int type)
{
	lockusr(x);
	if((intam=(struct Aport *)AllocMem(sizeof(*amul),MEMF_PUBLIC+MEMF_CLEAR))==NULL)
		memfail("comms port");
	IAm.mn_Length = (UWORD) sizeof(*amul); IAf=Af; IAm.mn_Node.ln_Type = NT_MESSAGE; IAm.mn_ReplyPort = repbk; IAt=MFORCE; IAd=type; IAp=cmd;
	PutMsg((lstat+x)->rep,(struct Message *)intam); (lstat+x)->IOlock=-1;
}

/****** AMUL3.C/StopFollow ******************************************
*
*   NAME
*	StopFollow -- Stop being a follower.
*
*   SYNOPSIS
*	void StopFollow( void );
*
*   FUNCTION
*	If the current verb (overb) is a Travel verb and we were following
*	the we can no-longer follow them.
*
*   SEE ALSO
*	LoseFollower(), Follow(), DoThis()
*
******************************************************************************
*
*/

StopFollow()
{
	Forbid();
	if(fol!=0 || me2->following == -1 || (vbtab+overb)->flags & VB_TRAVEL) { Permit(); return; }
	if((lstat+me2->following)->state != PLAYING || (lstat+me2->following)->followed != Af) { me2->following=-1; Permit(); return; }
	(lstat+me2->following)->followed = -1; Permit();
	tx("You are no-longer following @mf.\n"); me2->following = -1;
}

#include "frame/Internal.C"		/* Internal Command processor */

/****** AMUL3.C/LoseFollower ******************************************
*
*   NAME
*	LoseFollower -- Get rid of the person following us.
*
*   SYNOPSIS
*	void LoseFollower( void );
*
*   FUNCTION
*	Unhooks the player who WAS following us (if there was one) and
*	tells them they can no-longer follow us.
*
*   SEE ALSO
*	StopFollow(), Follow(), DoThis()
*
******************************************************************************
*
*/

LoseFollower()
{
	if(me2->followed == -1) return;
	(lstat+(me2->followed))->following=-1; 	/* Unhook them */
	utx(me2->followed, "You are no-longer able to follow @me.\n");
	me2->followed = -1; /* Unhook me */
}

/****** AMUL3.C/ShowFile ******************************************
*
*   NAME
*	ShowFile -- Send file to user (add extension)
*
*   SYNOPSIS
*	ShowFile( FileName )
*
*	void ShowFile( BYTE );
*
*   FUNCTION
*	Locates the file (experiments with extensions and paths) and
*	displays it to the user. If there is insufficient memory or
*	it is unable to find the file, it informs the user and takes
*	apropriate action.
*
*   INPUTS
*	FileName - ASCIZ string containing the file name. First try
*		   to open file assumes the file is in the adventure
*		   directory with the extension .TXT
*
*   EXAMPLE
*	ShowFile("Scenario");
*	ShowFile("Ram:Scenario.Txt");
*
*   NOTES
*	Tries: <AdvPath>/<File Name>.TXT
*	       <AdvPath>/<File Name>
*	       <File Name>.TXT
*	       <File Name>
*
******************************************************************************
*
*/

ShowFile(char *s)
{	register long fsize; register char *p;

	if(ifp!=NULL) fclose(ifp);
	sprintf(block,"%s%s.txt",dir,s);
	if((ifp=fopen(block,"rb"))!=NULL) goto show;
	sprintf(block,"%s%s",dir,s);
	if((ifp=fopen(block,"rb"))!=NULL) goto show;
	sprintf(block,"%s.txt",s);
	if((ifp=fopen(block,"rb"))!=NULL) goto show;
	sprintf(block,"%s",s);
	if((ifp=fopen(block,"rb"))!=NULL) goto show;
	txs("\n--+ Please inform the dungeon master that file %s is missing.\n\n",s); return;
show:
	fseek(ifp,0,2L); fsize=ftell(ifp); fseek(ifp,0,0L);
	if((p=(char *)AllocMem( fsize + 2, MEMF_PUBLIC + MEMF_CLEAR )) == NULL)
	{
		txs("\n--+ \x07System memory too low, exiting! +--\n"); forced=1; exeunt=1; kquit("out of memory!\n");
	}
	fread(p, fsize, 1, ifp); tx(p); FreeMem( p, fsize + 2);
	pressret();
}

/****** AMUL3.C/scaled ******************************************
*
*   NAME
*	scaled -- rehash object value using time-scaling!
*
*   SYNOPSIS
*	Actual = scaled( Value )
*
*	int scaled( int );
*
*   FUNCTION
*	Recalculates an object value based on AMUL Time and Rank scaling.
*	Rather than a straight forward scaling basd on RScale and TScale,
*	you have to calculate what percentage of each of these to use! A
*	player of rank 1 in the last minute of game play will find objects
*	worth their full value. The formula is quite simple, but doing %ages
*	in 'C' always looks messy.
*
*   INPUTS
*	Value  - the value to be scaled
*
*   RESULT
*	Actual - the actual CURRENT value
*
*   NOTES
*	Note: rscale is based on %age of total ranks achieved and tscale is
*	      based on %age of game-time remaining.
*
******************************************************************************
*
*/

scaled(long value,short int flags)	/* Scale an object value */
{	register scalefactor;

	if(!(flags & SF_SCALED)) return value;
	scalefactor = ((rscale * me->rank * 100 / ranks) + (tscale * *rescnt * 100 / (mins * 60))) / 100;
	return value - ( value * scalefactor / 100 );
}

/****** AMUL3.C/showin ******************************************
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

showin(int o,int mode)		/* Show contents of object */
{	register int i,j,k,l; register char *p,*s;

	if(State(o)->flags & SF_OPAQUE && mode==NO) { tx(str); txc('\n'); return; }
	p=str+strlen(str);
	if((obtab+o)->inside<=0)
	{
		if(mode==YES)
		{
			if((obtab+o)->putto==0) sprintf(p,"The %s contains ",(obtab+o)->id);
			else sprintf(p,"%s the %s you find: ",obputs[(obtab+o)->putto]);
			strcat(p,"nothing.\n");
		}
		else sprintf(p,"\n");
		tx(str); return;
	}

	if((obtab+o)->putto==0) sprintf(p,"The %s contains ",(obtab+o)->id);
	else sprintf(p,"%s the %s you find: ",obputs[(obtab+o)->putto],(obtab+o)->id);
	p+=strlen(p);

	j=0; k=-(INS+o); l=(obtab+o)->inside;
	for(i=0; i<nouns && l>0; i++)
	{
		if(*(obtab+i)->rmlist!=k) continue;
		if(j!=0) { *(p++)=','; *(p++)=' '; }
		s=(obtab+i)->id; while(*s!=0) *(p++)=*(s++); *p=0;
		j=1; l--;
	}
	strcat(p,".\n"); tx(str);
}

/****** AMUL3.C/stfull ******************************************
*
*   NAME
*	stfull -- Check if players property is at full
*
*   SYNOPSIS
*	stfull( Stat, Player )
*
*	BOOLEAN stfull( USHORT, USHORT );
*
*   FUNCTION
*	Tests to see if a players 'stat' is at full power and returns a
*	TRUE or FALSE result (YES or NO).
*
*   INPUTS
*	stat   -- a players stat number (see h/AMUL.Defs.H)
*	player -- number of the player to check
*
*   RESULT
*	YES if it is
*	NO  if it isn't
*
******************************************************************************
*
*/

stfull(register int st, register int p)	/* full <st> <player> */
{
	you = (usr+p); you2 = (lstat+p);
	switch(st)
	{
		case STSCORE:	return NO;
		case STSCTG:	return NO;
		case STSTR:	if(you2->strength < you->strength) return NO; break;
		case STDEX:	if(you2->dext < you->dext) return NO; break;
		case STSTAM:	if(you2->stamina < you->stamina) return NO; break;
		case STWIS:	if(you2->wisdom < you->wisdom) return NO; break;
		case STMAGIC:	if(you2->magicpts < you->magicpts) return NO; break;
		case STEXP:	if(you->experience < (rktab+you->rank)->experience) return NO; break;
	}
	return YES;
}

/****** blank.form/empty ******************************************
*
*   NAME
*
*   SYNOPSIS
*
*   FUNCTION
*
*   INPUTS
*
*   RESULT
*
*   EXAMPLE
*
*   NOTES
*
*   BUGS
*
*   SEE ALSO
*
******************************************************************************
*
*/

asetstat(int obj,int stat)
{	register char i,j,x,w,f;

	objtab=obtab+obj;
	x=owner(obj);
	i=lit(loc(obj));		/* WAS the room lit? */
	/* Remove from owners inventory */
	if(x!=-1) { w=(lstat+x)->wield; rem_obj(x,obj); }
	f=STATE->flags & SF_LIT;
	objtab->state=stat;
	if(objtab->flags & OF_SHOWFIRE)
	{
		if(f==0) STATE->flags = STATE->flags & -(1+SF_LIT);
		else STATE->flags = STATE->flags | SF_LIT;
		if(x!=-1) add_obj(x);
		return;	/* Don't need to check for change of lights! */
	}

	if(x!=-1)
	{
		add_obj(x);		/* And put it back again */
		/*== Should check to see if its too heavy now */
		lighting(x,AHERE); (lstat+x)->wield=w;
	}

	if((j=lit(loc(obj))) == i) return;
	if(j==NO) { actionfrom(obj,acp(NOWTOODARK)); sys(NOWTOODARK); }
	else	  { actionfrom(obj,acp(NOWLIGHT)); sys(NOWLIGHT); }
}

awhere(register int obj)
{	register int i,j,found,k; register long *rp;

	found=-1;
	for(i=0; i<nouns; i++)		/* Check all */
	{
		if(stricmp((obtab+obj)->id,(obtab+i)->id)==NULL)
		{
			if(canseeobj(i,Af)==NO) continue;
			if((j=owner(i))!=-1)
			{
				if(lit((lstat+j)->room)==YES)
				{
					if(j!=Af)
					{
						tx("You see "); ans("1m"); tx((usr+j)->name); ans("0;37m"); tx(".\n");
					}
					else tx("There is one in your possesion.\n");
					found++;
				}
				continue;
			}
			rp=(obtab+i)->rmlist;
			for(j=0; j<(obtab+i)->nrooms; j++)
			{
				if(*(rp+j)==-1) continue;
				if(*(rp+j)>=0)
				{
					if(*(rp+j)==-1 || lit(*(rp+j))==NO) continue;
					roomtab=rmtab+*(rp+j); desc_here(2);
				}
				else
				{
					k=-(INS+*(rp+j));
					sprintf(block,"There is one %s something known as %s!\n",obputs[(obtab+k)->putto],(obtab+k)->id);
					tx(block);
				}
				found++;
			}
		}
	}
	if(found==-1) sys(SPELLFAIL);
}

osflag(register int o, register int flag)
{	register int own,l;

	objtab=obtab+o;

	own=owner(o); if(own==-1) l=lit(loc(o)); else rem_obj(own,o);
	STATE->flags = flag;
	if(own != -1) { add_obj(o); lighting(own,AHERE); return; }
	if(lit(loc(o))!=l)
		if(l==YES){ actionfrom(o,acp(NOWTOODARK)); sys(NOWTOODARK); }
		else	  { actionfrom(o,acp(NOWLIGHT)); sys(NOWLIGHT); }
}

/* Set @xx and @xy corresponding to a specific player */

setmxy(register int Flags, register int Them)
{
	if( Them == Af || cansee(Them, Af) == YES )	/* If he can see me */
	{
		ioproc("@me");		strcpy(mxx,ow);
		ioproc("@me the @mr");	strcpy(mxy,ow);
		return;
	}
	if( pROOM(Them) == me2->room )
	{
		switch(Flags)
		{
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
	/* They aren't in the same room */
	switch(Flags)
	{
		case ACTION:
		case EVENT:
			strcpy(mxx,"Someone");
			if(me->rank == ranks-1) strcpy(mxy,"Someone very powerful");
			else strcpy(mxy,"Someone");
			return;
		case TEXTS:
			ioproc("@me");	strcpy(mxx,ow);
			if(me->rank == ranks-1) ioproc("@me the @mr");
			strcpy(mxy,ow);
			return;
		case NOISE:
			strcpy(mxx,"Someone");
			if(me->rank == ranks-1) ioproc("A powerful @gn voice somewhere in the distance");
			else ioproc("A @gn voice in the distance");
			strcpy(mxy,ow);
			return;
		default:
			strcpy(mxx,"Someone"); strcpy(mxy,"Someone");
	}
}
