/*
          ####         ###     ### ##     ## ####
         ##  ##         ###   ###  ##     ##  ##            Amiga
        ##    ##        #########  ##     ##  ##            Multi
        ##    ##        #########  ##     ##  ##            User
        ########  ----  ## ### ##  ##     ##  ##            adventure
        ##    ##        ##     ##  ##     ##  ##            Language
       ####  ####      ####   ####  #######  #########


              ****       AMUL2.C......Adventure System      ****
              ****            The Saga Continues!           ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike.


             Secondary/low-level functions, macros and routines
*/

#include "frame/AMULInc.H"

extern	char	ncop[];				/* Conditions */

#include	"frame/Daemons.C"
#include	"frame/FileBits.C"
#include	"frame/Parser.C"

/* Various low-level macros/functions for AMUL... */

owner(register int obj)
{	register int r;

	r=-5-*(obtab+obj)->rmlist;
	if(r>=MAXU || r<0) return -1;
	return r;
}

show_rank(register int p,register int rankn,register int sex)
{
	str[0]=0; make_rank(p,rankn,sex); tx(str);
}

make_rank(register int p,register int rankn,register int sex)
{
	strcat(str," the "); p+=5;
	if(*(lstat+p)->pre!=0) {  strcat(str,(lstat+p)->pre); strcat(str," "); }
	strcat(str,(sex==0)?(rktab+rankn)->male:(rktab+rankn)->female);
	if(*(lstat+p)->post!=0) { strcat(str," "); strcat(str,(lstat+p)->post); }
}

moveto(register int r)		/* Move player to room, testing ligthing! */
{	register int i;

	/*
	  Set the players current lighting to NONE and then test lighting for
	  the room. Then move the player and restore his lighting. Now test
	  again!
                                                                            */
	StopFollow(); me2->flags = me2->flags | PFMOVING;
	lroom=me2->room; i=me2->light; me2->light=0; lighting(Af,AOTHERS);
	me2->room=r; me2->light=i; me2->hadlight=0; lighting(Af,AOTHERS);
	look((rmtab+me2->room)->id, me->rdmode);
	me2->flags = me2->flags & -(1 + PFMOVING);
}

mod(register long n,register long x)	/* n MODulo X */
{	/*== This nolonger freezes the task on the odd ocassion! */
	n=n & 65535;

	if(x==0) return 0;

	if(n<0) return -mod(-n,x);
	if(x<0) x=-x;

	while(n>x) n-=x;
	if(n<0) n+=x;
	return n;
}

rnd()			/*== Return pseudo-random number! */
{	long x;

	time(&x); srand(x); return rand();
}

gotin(register int obj,register int st)	/* Do I own a 'obj' in state 'stat'? */
{	register int i;

	for(i=0; i<nouns; i++)
	{
		if(stricmp((obtab+i)->id,(obtab+obj)->id)==NULL && ((obtab+i)->state==st || st==-1) && owner(i)==Af)
			return YES;
	}
	return NO;
}

	/* The next two commands are ACTIONS but work like conditions */

achecknear(register int obj)	/* Check player is near object, else msg + endparse */
{
	if(nearto(obj)==NO) { txs("I can't see the %s!\n",(obtab+obj)->id); donet=ml+1; return -1; }
	inc=0; return 0;
}

acheckget(register int obj)	/* Check the player can 'get' the object */
{
	if(carrying(obj)!=-1)
	{
		tx("You've already got it!\n");
		donet=ml+1; return -1;
	}
	if(achecknear(obj)==-1) return -1; else inc=1;
	objtab=obtab+obj;
	if((objtab->flags&OF_SCENERY) || (STATE->flags&SF_ALIVE) || objtab->nrooms!=1)
	{
		tx("You can't move that!\n"); donet=ml+1; return -1;
	}
	if((rktab+me->rank)->numobj<=0)
	{
		tx("You can't manage it!\n"); donet=ml+1; return -1;
	}
	if(STATE->weight > (rktab+me->rank)->maxweight)
	{
		tx("You aren't strong enough to lift that!\n"); donet=ml+1; return -1;
	}
	if(me2->numobj + 1 > (rktab+me->rank)->numobj)
	{
		tx("You can't carry any more!"); tx(" You'll have to drop something else first.\n");
		donet=ml+1; return -1;
	}
	if(STATE->weight + me2->weight > (rktab+me->rank)->maxweight)
	{
		tx("It's too heavy."); tx(" You'll have to drop something else first.\n");
		donet=ml+1; return -1;
	}
	inc=0; return 0;
}

look_here(register int f,register int rm)
{	register char il;

	/* Can I see? */
	if(me2->flags & PFBLIND) { list_what(rm,0); return; }
	/* Can we see in here? */
	if((il=lit(me2->room))==NO)
	{
		sys(TOODARK); *(rctab+rm) = *(rctab+rm) & rclr; goto die;
	}
	desc_here(f); list_what(rm,0);
die:	if((roomtab->flags & DEATH) && me->rank!=ranks-1)
	{
		if(dmove[0]==0) strcpy(dmove,(rmtab+lroom)->id);
		akillme(); return; 
	}
	else whohere();
}

desc_here(register int f)
{	register char *p,c;
	fseek(ifp,roomtab->desptr,0L);
	if(roomtab->flags & DMOVE)	/* A dmove room? */
		fgets(dmove,IDL,ifp);
	else dmove[0]=0;

	/* Print short description */
	p=block;
	if(!(roomtab->flags & DEATH)) ans("1m");
	while((c=fgetc(ifp))!=0 && c!=EOF && c!='\n')
	{
		*(p++)=c; *p=0;
		if(p>block+1020)
		{
			tx(block); p=block; *p=0;
		}
	}
	if(p!=block) tx(block);	/* Make sure we dump it! */
	/* If I am the toprank show me the room id too! */
	ans("0;37m");
	if(me->rank == ranks-1) {sprintf(block,"   (%s)",roomtab->id); tx(block); block[0]=0; }
	txc('\n');
	if(c == '\n' && f == RDVB)
	{
		/* Display LONG description! */
		p=block;
		while((c=fgetc(ifp))!=0 && c!=EOF)
		{
			*(p++)=c; *p=0;
			if(p>block+1020)
			{
				tx(block); p=block; *p=0;
			}
		}
		if(p!=block) tx(block);
	}
}

list_what(register int r,register int i)
{	register int o,or,f;

	f=-1;
	if(lit(me2->room)==NO) return sys(TOOMAKE);
	if(me2->flags & PFBLIND) sys(YOURBLIND);
	if(((rmtab+r)->flags & HIDEWY) && i!=0 && me->rank != ranks-1)
	{
		sys(NOWTSPECIAL); /* Wizards can see in hideaways! */
	}
	for(o=0;o<nouns;o++)		/* All objects */
	{
		/* Only let the right people see the object */
		if(canseeobj(o,Af)==NO) continue;
		if(((rmtab+r)->flags & HIDEWY) && (i==0 || (i==1 && me->rank!=ranks-1)) && !((obtab+o)->flags & OF_SCENERY)) continue;
		if(lit(me2->room) == NO && !((obtab+o)->flags & OF_SMELL)) continue;
		obj=*(obtab+o); 
		for(or=0; or<obj.nrooms; or++)
		{
			if(*(obj.rmlist+or)==r && State(o)->descrip>=0)
			{
				if(isOINVIS(o)) ans("3m");
				f++; descobj(o);
				if(isOINVIS(o)) ans("0;37m");
			}
		}
	}
	if(f==-1 && i==1) sys(NOWTSPECIAL);
}
	
descobj(register int Ob)
{
	obj.states=(obtab+Ob)->states+(long)(obtab+Ob)->state;
	if(obj.states->descrip==0) return;
	sprintf(str,desctab+obj.states->descrip,adtab+((obtab+Ob)->adj * (IDL + 1)));
	if( ((obtab+Ob)->flags & OF_SHOWFIRE) && ( obj.states->flags & SF_LIT ) )
	{
		if(*(str+strlen(str)-1)=='\n' || *(str+strlen(str)-1)=='{') { *(str+strlen(str)-1) = 0; }
		if((obtab+Ob)->adj != -1)
			sprintf(spc," The %s %s is on fire.\n",(adtab+( (obtab+Ob)->adj * (IDL + 1))),(obtab+Ob)->id);
		else	sprintf(spc," The %s is on fire.\n",(obtab+Ob)->id); strcat(str,spc);
	}
	if((obtab+Ob)->contains<=0) { tx(str); return; }
	if(*(str+strlen(str)-1)=='\n' || *(str+strlen(str)-1)=='{') { *(str+strlen(str)-1) = 0; }
	strcat(str," ");
	showin(Ob, NO);
}

inflict(register int x, register int s)
{
	you2=lstat+x; if(you2->state != PLAYING) return;
	switch(s)
	{
		case SGLOW:	if(!(you2->flags&PFGLOW)) { you2->flags=(you2->flags|PFGLOW); you2->light++; } break;
		case SINVIS:	you2->flags=you2->flags|PFINVIS; break;
		case SDEAF:	you2->flags=you2->flags|PFDEAF; break;
		case SBLIND:	you2->flags=you2->flags|PFBLIND; break;
		case SCRIPPLE:	you2->flags=you2->flags|PFCRIP; break;
		case SDUMB:	you2->flags=you2->flags|PFDUMB; break;
		case SSLEEP:	you2->flags=you2->flags|PFASLEEP; break;
		case SSINVIS:	you2->flags=you2->flags|PFSINVIS; break;
	}
	calcdext(); lighting(x,AHERE);
}

cure(register int x, register int s)
{
	you2=lstat+x; if(you2->state != PLAYING) return;
	switch(s)
	{
		case SGLOW:	if(you2->flags&PFGLOW) { you2->flags=(you2->flags&(-1-PFGLOW)); you2->light--; } break;
		case SINVIS:	you2->flags=you2->flags & -(1+PFINVIS); break;
		case SDEAF:	you2->flags=you2->flags & -(1+PFDEAF); break;
		case SBLIND:	you2->flags=you2->flags & -(1+PFBLIND); break;
		case SCRIPPLE:	you2->flags=you2->flags & -(1+PFCRIP); break;
		case SDUMB:	you2->flags=you2->flags & -(1+PFDUMB); break;
		case SSLEEP:	you2->flags=you2->flags & -(1+PFASLEEP); break;
		case SSINVIS:	you2->flags=you2->flags & -(1+PFSINVIS); break;
	}
	calcdext(); lighting(x,AHERE);
}

summon(int plyr)
{
	if((lstat+plyr)->room==me2->room)
	{
		txs(acp(CANTSUMN),(usr+plyr)->name); return;
	}
	interact(MSUMMONED,plyr,me2->room);
}

adestroy(register int obj)
{	register int i;

	Forbid();
	loseobj(obj);
	for(i=0; i<(obtab+obj)->nrooms; i++) *((obtab+obj)->rmlist+i)=-1;
	(obtab+obj)->flags=(obtab+obj)->flags | OF_ZONKED;
	Permit();
}

arecover(register int obj)
{	register int i;

	if(State(obj)->flags & SF_LIT) me2->light++;
	for(i=0; i<(obtab+obj)->nrooms; i++) *((obtab+obj)->rmlist+i)=me2->room;
	(obtab+obj)->flags=(obtab+obj)->flags & -(1+OF_ZONKED);
	lighting(Af,AHERE);
}

refresh()	/* Restore players details! */
{
	if(me->strength<=0) me->strength=(rktab+me->rank)->strength;
	me2->strength=me->strength;
	if(me->stamina<=0)  me->stamina =(rktab+me->rank)->stamina;
	me2->stamina=me->stamina;
	if(me->dext<=0)     me->dext    =(rktab+me->rank)->dext;
	me2->dext=me->dext; me2->dextadj=0;
	if(me->wisdom<=0) me->wisdom=(rktab+me->rank)->wisdom;
	me2->wisdom=me->wisdom;
	if(me->experience<=0)  me->experience =(rktab+me->rank)->experience;
	if(me->magicpts<=0)     me->magicpts    =(rktab+me->rank)->magicpts;
	me2->magicpts=me->magicpts;
	calcdext();
}

zapme()
{	register char *p; register int i;

	Forbid();
	p=(char *)me->name; exeunt=1;
	for(i=0; i<sizeof(usr); i++) *(p++)=0;
	Permit(); save_me(); nohelp();
}

send(register int o,register int to)
{	register short int x,i;

	x=lit(to); loseobj(o); for(i=0; i<objtab->nrooms; i++) *(objtab->rmlist+i)=to;
	if(lit(to)!=x) actionin(to,acp(NOWLIGHT));
}

achange(register int u)
{
	if(u==Af) { me->sex=1-me->sex; sys(CHANGESEX); }
	else sendex(u,ACHANGESEX,u,NONE); /* Tell them to clear up! */
}

/*== Fixed to allow increase/decrease */
newrank(register int plyr,register int r)
{	register int or;

	or=me->rank;

	if((rktab+r)->tasks!=0)
	{
		if((me->tasks & (1<<( (rktab+r)->tasks)-1 )) == NULL)
		{
			sys(NOTASK); return;
		}
	}
			
	me->rank=r; sys(MADERANK);
		
	/* Update Current Line Stats */
	me2->strength += (rktab+r)->strength - (rktab+or)->strength;
	if(me2->strength > (rktab+r)->strength) me2->strength=(rktab+r)->strength;
	me2->stamina  += (rktab+r)->stamina  - (rktab+or)->stamina;
	if(me2->stamina > (rktab+r)->stamina) me2->stamina=(rktab+r)->stamina;
	me2->wisdom   += (rktab+r)->wisdom   - (rktab+or)->wisdom;
	if(me2->wisdom > (rktab+r)->wisdom) me2->wisdom=(rktab+r)->wisdom;
	me->experience += (rktab+r)->experience - (rktab+or)->experience;
	if(me->experience > (rktab+r)->experience) me->experience=(rktab+r)->experience;
	me2->magicpts   += (rktab+r)->magicpts   - (rktab+or)->magicpts;
	if(me2->magicpts > (rktab+r)->magicpts) me2->magicpts=(rktab+r)->magicpts;
	calcdext();

	/* Update File Stats */
	me->strength  =(rktab+r)->strength;
	me->stamina   =(rktab+r)->stamina;
	me->wisdom    =(rktab+r)->wisdom;
	me->dext      =(rktab+r)->dext;
	me->experience+=(rktab+r)->experience-(rktab+or)->experience;
	me->magicpts  =(rktab+r)->magicpts;

	if(r == ranks-1)
	{
		sys(TOPRANK); SendIt(MMADEWIZ,0,me->name);
	}
}

aadd(register int howmuch, register int stat, register int plyr)
{	register int r;
	if(howmuch < 0) return asub(-howmuch, stat, plyr);
	if(howmuch == 0) return;
	if(plyr==Af)
	{
		switch(stat)
		{
			case STSCORE:
				me->score+=howmuch; me2->sctg+=howmuch;
				ans("1m"); utxn(plyr,"(%ld)\n",me->score); ans("0;37m");
				for(r=ranks-1; r>=0; r--)
				{
					if(me->score>=(rktab+r)->score)
					{
						if(me->rank==r) break;
						newrank(plyr,r); break;
					}
				}
				break;
			case STSTR:	me2->strength+=howmuch; break;
			case STSTAM:	me2->stamina+=howmuch;  sprintf(block,"<STAM: %ld/%ld>\n",me2->stamina,me->stamina); ans("1m"); tx(block); ans("0;37m"); break;
			case STDEX:	me2->dextadj+=howmuch; break;
			case STEXP:	me->experience+=howmuch; break;
			case STWIS:	me2->wisdom+=howmuch; break;
			case STMAGIC:	me2->magicpts+=howmuch; break;
		}
	}
	else sendex(plyr,AADD,howmuch,stat,plyr); /* Tell them to clear up! */
}

asub(register int howmuch, register int stat, register int plyr)
{	register int r;
	if(howmuch < 0) return asub(-howmuch, stat, plyr);
	if(howmuch == 0) return;
	if(plyr==Af)
	{
		switch(stat)
		{
			case STSCORE:
				me->score-=howmuch; me2->sctg-=howmuch;
				if(me->score<0) me->score=0;
				ans("1m"); utxn(plyr,"(%ld)\n",me->score); ans("0;37m");
				for(r=0; r<ranks-1; r++)
				{
					if(me->score<(rktab+(r+1))->score && me->rank==r) break;
					if(me->score<(rktab+(r+1))->score)
					{
						newrank(plyr,r); break;
					}
				}
				break;
			case STSTR:	
					me2->strength-=howmuch; if (me2->strength < 0) me2->strength=0; break;
			case STSTAM:	me2->stamina-=howmuch; if (me2->stamina < 0) me2->stamina=0;
					sprintf(block,"\n<STAM: %ld/%ld>\n",me2->stamina,me->stamina); ans("1m"); tx(block); ans("0;37m");
					if(me2->stamina <= 0) { akillme(); died=1; }
					break;
			case STDEX:	me2->dextadj-=howmuch; break;
			case STWIS:	me2->wisdom-=howmuch; if (me2->wisdom < 0) me2->wisdom=0; break;
			case STEXP:	me->experience-=howmuch; if (me->experience < 0) me->experience=0; break;
			case STMAGIC:	me2->magicpts-=howmuch; if (me2->magicpts < 0) me2->magicpts=0; break;
		}
	}
	else sendex(plyr,ASUB,howmuch,stat,plyr); /* Tell them to clear up! */
}

afix(register int stat, register int plyr)
{
	if(plyr==Af)
	{
		switch(stat)
		{
			case STSTR:	me2->strength=((rktab+me->rank)->strength
						*(rktab+me->rank)->maxweight-me2->weight)
						/(rktab+me->rank)->maxweight; break;
			case STSTAM:	me2->stamina=(rktab+me->rank)->stamina; break;
			case STDEX:	me2->dextadj=0; calcdext(); break;
			case STWIS:	me2->wisdom=(rktab+me->rank)->wisdom; break;
			case STEXP:	me->experience=(rktab+me->rank)->experience; break;
			case STMAGIC:	me2->magicpts=(rktab+me->rank)->magicpts; break;
		}
	}
	else sendex(plyr,AFIX,stat,plyr,0); /* Tell them to clear up! */
}

announce(register char *s,register int towho)	/* Loud noises/events */
{	register int i,x;

	for(i=0; i<MAXU; i++)
	{
		/* If the player is deaf, ignore him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & PFDEAF)) continue;
		/*
		   The next line says:
			if this is another player, and he's in another room,
			and the room is a silent room, ignore him.
		*/
		if(i != Af && (lstat+i)->room != me2->room &&	/* --v */
			((rmtab+(lstat+i)->room)->flags & SILENT)) continue;
		x=0;
		switch(towho)
		{
			case AALL:
			case AEVERY1:	x=1; break;
			case AGLOBAL:	if(i != Af) x=1; break;
			case AOTHERS:	if(i == Af) break;
			case AHERE:	if((lstat+i)->room == me2->room) x=1; break;
			case AOUTSIDE:	if((lstat+i)->room != me2->room) x=1; break;
		}
		if(x == 1)
		{
			setmxy(NOISE,i); utx(i,s);
		}
	}
}

announcein(int toroom,char *s)	/* Loud noises/events */
{	register int i;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is deaf, ignore him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & PFDEAF) || (lstat+i)->room!=toroom) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

announcefrom(int obj,char *s)	/* Loud noises/events */
{	register int i,o;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is deaf or can see me, ignore him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & PFDEAF) || (lstat+i)->room == me2->room) continue;
		/* Check if the player is NEAR to someone carrying the object */
		if((o=owner(obj))!=-1 && (lstat+o)->room!=(lstat+i)->room) continue;
		if(o==-1 && isin(obj,(lstat+o)->room)==NO) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

objannounce(register int obj,register char *s)	/* Loud noises/events */
{	register int i,o;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is deaf or can see me, ignore him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & PFDEAF)) continue;
		/* Check if the player is NEAR to someone carrying the object */
		if((o=owner(obj))!=-1 && (lstat+o)->room!=(lstat+i)->room) continue;
		if(o==-1 && isin(obj,(lstat+o)->room)==NO) continue;
		setmxy(NOISE,i); utx(i,s);
	}
}

action(char *s,int towho)	/* Quiet actions/notices */
{	register int i,x;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is asleep, or blind, skip him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & (PFBLIND+PFASLEEP))!=0) continue;
		x=0;
		switch(towho)
		{
			case AALL:
			case AEVERY1:	x=1; break;
			case AGLOBAL:	if(i != Af) x=1; break;
			case AOTHERS:	if(i == Af) break;
			case AHERE:	if((lstat+i)->room == me2->room && cansee(i,Af)==YES) x=1; break;
			case AOUTSIDE:	if((lstat+i)->room != me2->room) x=1; break;
		}
		if(x == 1)
		{
			setmxy(ACTION,i); utx(i,s);
		}
	}
}

actionin(int toroom,char *s)	/* Quiet actions/notices */
{	register int i;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is asleep, or blind, skip him */
		if(actor == i || ((lstat+i)->state < PLAYING) || ((lstat+i)->flags & (PFBLIND+PFASLEEP)) || (lstat+i)->room!=toroom) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

actionfrom(int obj,char *s)	/* Quiet actions/notices */
{	register int i,o;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is asleep, or blind, skip him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & (PFBLIND+PFASLEEP)) || (lstat+i)->room == me2->room) continue;
		/* Check if the player is NEAR to someone carrying the object */
		if((o=owner(obj))!=-1) if((lstat+o)->room!=(lstat+i)->room) continue;
		if(o==-1 && isin(obj,(lstat+i)->room)==NO) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

objaction(register int obj,register char *s)	/* Quiet actions/notices */
{	register int i,o;
	for(i=0; i<MAXU; i++)
	{
		/* If the player is asleep, or blind, skip him */
		if(actor == i || ((lstat+i)->state < 2) || ((lstat+i)->flags & (PFBLIND+PFASLEEP))) continue;
		/* Check if the player is NEAR to someone carrying the object */
		if((o=owner(obj))!=-1) if((lstat+o)->room!=(lstat+i)->room) continue;
		if(o==-1 && isin(obj,(lstat+i)->room)==NO) continue;
		setmxy(ACTION,i); utx(i,s);
	}
}

fwait(long n)
{	register int i;

	if(n<1) n=1;
	for(i=0; i<7; i++)
	{
		Delay(n*7); iocheck();
	}
}

ableep(int n)
{	register int i;

	for(i=0;i<n;i++)
	{
		tx(". "); fwait(1);
	}
	txc('\n');
}

lighting(int x, int twho)	/*== twho - tell who! */
{
	if((lstat+x)->light==(lstat+x)->hadlight || !((rmtab+(lstat+x)->room)->flags & DARK)) return;
	if((lstat+x)->light<=0)
	{
		if((lstat+x)->hadlight <= 0) return;
		(lstat+x)->hadlight=(lstat+x)->light=0;
		if(lit((lstat+x)->room)==NO) action(acp(NOWTOODARK),twho);
	}
	else
	{
		if((lstat+x)->hadlight != 0) return;
		if(lit((lstat+x)->room)==NO) action(acp(NOWLIGHT),twho); 
		(lstat+x)->hadlight=(lstat+x)->light;
	}
}

loseobj(register int obj)	/* Remove object from its owners inventory */
{	register int o,i;

	objtab=obtab+obj;

	if((o=owner(obj))!=-1)
	{
		for(i=0; i<objtab->nrooms; i++) *(objtab->rmlist+i)=-1;
		rem_obj(o); lighting(o,AHERE);
	}
}

nohelp()
{	register int i;

	if(me2->helping!=-1) utx(me2->helping,"@me is no-longer able to help you...\n");
	(lstat+me2->helping)->helped--; me2->helping=-1; you2=lstat;
	for(i=0; i<MAXU; i++,you2++)
		if(you2->helping==Af)
		{
			utx(i,"You are no longer able to help @me.\n");
			you2->helping=-1;
		}
	me2->helping=me2->helped=-1;
}

aforce(register int x,register char *cmd)
{
	DoThis( x, cmd, 0 );
}

afight(int plyr)
{
	if(plyr==Af) return;
	if((rmtab+me2->room)->flags&PEACEFUL) { sys(NOFIGHT); return; }
	if((lstat+plyr)->fighting == Af) { txs("You are already fighting %s!\n",(usr+plyr)->name); donet=ml+1; return; }
	if((lstat+plyr)->fighting != -1) { txs("%s is already in a fight!\n",(usr+plyr)->name); donet=ml+1; return; }
	you2=lstat+plyr;
	you2->flags=you2->flags | PFFIGHT; me2->flags=me2->flags | PFFIGHT | PFATTACKER;
	you2->fighting = Af; me2->fighting = plyr;
	Permit();
}

clearfight()
{
	Forbid();
	if(me2->fighting != -1 && me2->fighting != Af)
	{
		finishfight(me2->fighting);
	}
	finishfight(Af);
	Permit();
}

finishfight(int plyr)
{
	you2           = lstat+plyr;
	you2->flags    = you2->flags & (-1-(PFFIGHT | PFATTACKER));
	you2->fighting = -1;
}

acombat()
{			/* Check this out for Stats!!
	To hit:  Str=40 Exp=50 Dext=10
	Defence: Dext=70 Exp=20 Str=10
	No hits: Players level different by 2 double attacks etc.
	Damage:  Str / 10 + weapon.		<--- made this random!

	== Should ALSO go on how crippled a player is... A cripple can't
	strike out at another player! Also, blindness should affect your
	attack/defence. */

	register int aatt, adef, adam, datt, ddef, str;
	int oldstr,minpksl;

	calcdext();

	if(me2->fighting == Af || me2->fighting == -1 || me2->state < PLAYING || me2->stamina <= 0)
	{
		donet=ml+1;	/* End parse */
		finishfight(Af);
		return;		/* Macro : Permit(); return */
	}
		
	you=usr+me2->fighting; you2=lstat+me2->fighting; minpksl=(rktab+you->rank)->minpksl;

	if(you2->state < PLAYING || you2->room != me2->room || you2->stamina <= 0)
	{
		donet=ml+1; finishfight(Af); return;
	}

	if(me2->wield != -1)
	{
		objtab=obtab+me2->wield;
		str = ( 20 * me2->strength ) + STATE->damage;
	}
	else str = ( 20 * me2->strength );

	if(me->dext == 0) aatt=5;
	else aatt=(50*me->experience)/(rktab+ranks-1)->experience+
		(40*me2->strength)/(rktab+ranks-1)->strength+
		(10*me2->dext)/(rktab+ranks-1)->dext;

	if(me->dext == 0) adef=5;
	else adef=(5*me->experience)/(rktab+ranks-1)->experience+
		(15*me2->strength)/(rktab+ranks-1)->strength+
		(80*me2->dext)/(rktab+ranks-1)->dext;

/*	if(me2->flags & PFCRIP)  { aatt = aatt / 5; adef = adef / 10; }
	if(me2->flags & PFBLIND) { aatt = aatt / 2; adef = adef / 4;  } */

	if(you2->wield != -1)
	{
		objtab=obtab+you2->wield;
		str = ( 20 * you2->strength ) + STATE->damage;
	}
	else str = ( 20 * you2->strength );

	if(you->dext == 0) datt=5;
	else datt=(50*you->experience)/(rktab+ranks-1)->experience+
		(40*you2->strength)/(rktab+ranks-1)->strength+
		(10*you2->dext)/(rktab+ranks-1)->dext;

	if(you->dext == 0) ddef=5;
	else ddef=(5*you->experience)/(rktab+ranks-1)->experience+
		(15*you2->strength)/(rktab+ranks-1)->strength+
		(80*you2->dext)/(rktab+ranks-1)->dext;

/*	if(you2->flags & PFCRIP)  { datt = datt / 5; ddef = ddef / 10; }
	if(you2->flags & PFBLIND) { datt = datt / 2; ddef = ddef / 4;  } */

	oldstr=you2->stamina; adam=-1;
	if(mod(rand(),100)<aatt || (ddef <= 0 && mod(rand(),100) < 50))
	{
		if(mod(rand(),100)<ddef)
		{
			if (you2->wield != -1)
			{
				sys(WBLOCK); utx(me2->fighting,acp(WDEFEND));
			}
			else
			{
				sys(BLOCK); utx(me2->fighting,acp(DEFEND));
			}
/*			if((i=isverb("\"block"))!=-1) lang_proc(i,0);	*/
		}
		else
		{
			if (me2->wield != -1)
			{
				sys(WATTACK);
				objtab=obtab+me2->wield;
				adam=(me2->strength/10)+1+mod(rand(),STATE->damage);
				utx(me2->fighting,acp(WHIT));
			}
			else { adam=(me2->strength/10)+1; sys(ATTACK); utx(me2->fighting,acp(AMHIT)); }
			asub(adam,STSTAM,me2->fighting);
/*			if((i=isverb("\"hit"))!=-1) lang_proc(i,0);	*/
		}
	}
	else
	{
		sys(MISSED); utx(me2->fighting,acp(HEMISSED));
/*		if((i=isverb("\"miss"))!=-1) lang_proc(i,0);		*/
	}
	oldstr-=adam;
	if(( me2->flags & PFATTACKER )&& oldstr > 0)
	{
/*		if(me2->helped != -1 && (lstat+me2->helped)->room==me2->room)	Well?	*/

		sendex(me2->fighting,ACOMBAT,-1,0,0); /* Tell them to clear up! */
	}
	if(oldstr <= 0)
	{
		donet=ml+1;	/* End parse */
		tx("You have defeated @pl!\n");
		aadd(minpksl,STSCORE,Af);
		finishfight(Af);
	}
}

exits()
{	register int v,i,maxl,x,ac,brk,l; struct _TT_ENT *tp,*otp;
	int c,a; long *pptr;

	roomtab=rmtab+me2->room;
	if(roomtab->tabptr==-1) { tx("There are no visible exits.\n"); return; }

	vbptr=vbtab; x=0; c=tt.condition; a=tt.action; otp=ttabp; pptr=tt.pptr;

	for(v=0; v<verbs; v++,vbptr++)
	{
		if(vbptr->flags & VB_TRAVEL) continue; /* Not a trav verb */

		roomtab=rmtab+me2->room;
		l=-1; maxl=roomtab->ttlines; tp=ttp+roomtab->tabptr; brk=0;
		for(i=0; i<maxl && brk==0; i++)
		{
			ttabp=tp+i; tt.condition=ttabp->condition; tt.pptr=ttabp->pptr;
			if(ttabp->verb==v && (l=cond(ttabp->condition,l))!=-1)
			{
				if(ttabp->action>=0)
				{
					txs("%-10s ",vbptr->id); brk=1;
					roomtab=rmtab+(ttabp->action);
					if(roomtab->flags & DEATH) sys(CERTDEATH);
					else desc_here(RDBF);
					break;
				}
				ac=-1-ttabp->action;
				switch(ac)
				{
					case AKILLME:
						txs("%s: It's difficult to tell...\n",vbptr->id);
					case AENDPARSE:
					case AFAILPARSE:
					case AABORTPARSE:
					case ARESPOND:
						maxl=-1; break;
					case ASKIP:
						i+=TP1;
				}
				if(tt.condition==CANTEP || tt.condition==CALTEP || tt.condition==CELTEP) maxl=-1;
			}
		}
	}
	tt.condition=c; tt.action=a; ttabp->pptr=pptr; ttabp=otp;
}

isaroom(char *s)
{	int r;

	roomtab=rmtab;
	for(r=0;r<rooms;r++,roomtab++)
		if(stricmp(roomtab->id,s)==0) return r;
	return -1;
}

follow(register int x,register char *cmd)
{
	lockusr(x);
	if((intam=(struct Aport *)AllocMem(sizeof(*amul),MEMF_PUBLIC+MEMF_CLEAR))==NULL)
		memfail("comms port");
	IAm.mn_Length = (UWORD) sizeof(*amul); IAf=Af; IAm.mn_Node.ln_Type = NT_MESSAGE; IAm.mn_ReplyPort = repbk; IAt=MFORCE; IAd=1; IAp=cmd;
	PutMsg((lstat+x)->rep,(struct Message *)intam); (lstat+x)->IOlock=-1;
}

log(register char *s)
{
	ioproc(s); s=ow;
	while(*s!=0)
	{
		if(*s=='\n' || *s=='\r') { strcpy(s,s+1); continue; }
		s++;
	}
	SendIt(MLOG,NULL,ow);
}

PutRankInto(char *s)
{
	PutARankInto(s, Af);
}

PutARankInto(char *s,int x)
{	char *p;

	you=(usr+x); you2=(lstat+x);

	if(you2->pre[0]!=0)
	{
		p=you2->pre;
		while(*p!=0) *(s++)=*(p++); *(s++)=' ';
	}
	p=(you->sex==0)?(rktab+you->rank)->male:(rktab+you->rank)->female;
	while(*p!=0) *(s++)=*(p++);
	if(you2->post[0]!=0)
	{
		p=you2->pre;
		p=you2->post; *(s++)=' '; while(*p!=0) *(s++)=*(p++);
	}
	*s=0;
}

akillme()
{
	if(me2->fighting != -1) clearfight();
	iocheck(); me->plays=-1; exeunt=forced=1; donet=ml+1; ml=-1; me2->state=0;
	nohelp(); ans("1m"); sys(DIED); ans("0;37m");
}

show_tasks(register int p)
{
	sprintf(block,"Tasks completed by ");
	if(p!=Af) strcat(block,(usr+p)->name);
	else strcat(block,"you");
	strcat(block,": ");
	if(me->tasks==0) strcat(block,"None.\n");
	else
	{	register int i; char tsk[6];
		tsk[0]=0;
		for(i=0; i<32; i++)
		{
			if((usr+p)->tasks & (1<<i))
			{
				if(tsk[0]==0) sprintf(tsk,"%d",i+1);
				else sprintf(tsk,", %d",i+1);
				strcat(block,tsk);
			}
		}
		strcat(block,".\n");
	}
	tx(block);
}

dropall(register int torm)		/* Drop everything to a room */
{	register int i;

	for(i=0; i<nouns && me2->numobj>0; i++)
		if(*(obtab+i)->rmlist == -(5+Af)) adrop(i,torm,NO);
	me2->numobj=0;
}

invent(register int plyr)
{	register int i,pr,j; register char *p;

	p=block+strlen(block);

	strcpy(p,"carrying "); *(p+=9)=0;
	if((lstat+plyr)->numobj==0) { strcat(p,"nothing.\n"); tx(block); return; }
	objtab=obtab; j=0; pr = -(5+plyr);
	for(i=0; i<nouns; i++,objtab++)
		if(*objtab->rmlist==pr && canseeobj(i,Af)==YES)
		{ 
			if(j++!=0) strcat(p,", ");
			strcat(p,objtab->id);
			if(objtab->flags&OF_INVIS) strcat(p," (hidden)");
		}
	if(j==0) strcat(p,"nothing.\n");
	else strcat(p,".\n");
	tx(block);
}

ascore(register int type)
{	
	calcdext();

	if(type == TYPEV)
	{
		sprintf(block,"Recorded details:		%s\n\n",vername); tx(block);
		tx("Name: @m! Sex  : @gn		Played   : @gp times\n");
		ioproc("@mr");
		txs("Rank: %-20s  Score: @sc points	This game: @sg points\n",ow);
		sprintf(block,"Strength: @sr/%ld. Stamina: @st/%ld. Dexterity %ld/%ld.\n",
			(rktab+me->rank)->strength, (rktab+me->rank)->stamina, me2->dext, (rktab+me->rank)->dext);
		tx(block);
		sprintf(block,"Magic Points: @mg/%ld. Wisdom:  @wi.\n",(rktab+me->rank)->magicpts);
		tx(block);

		sprintf(block,"\nCurrent Info:\nObjects Carried: %ld/%ld,	Weight Carried: %ld/%ldg\n",
			me2->numobj, (rktab+me->rank)->numobj, me2->weight, (rktab+me->rank)->maxweight);
		tx(block);
		tx("Following: @mf.	");
		if(me2->helping!=-1) tx("Helping: @fr.  ");
		if(me2->helped!=-1)  tx("Helped by: @he.");
		if(me2->helping!=-1 || me2->helped!=-1) txc('\n');
		/*== Current weapon */
		if(me2->wield != -1) txs("Currently wielding: %s.\n",(obtab+me2->wield)->id);
		show_tasks(Af);
	}
	else
	{
		txs("Score: @sc. ",ow);
		sprintf(block,"Strength: @sr/%ld. Stamina: @st/%ld. Dexterity: %ld/%ld. Magic: @mg/%ld\n",
			(rktab+me->rank)->strength, (rktab+me->rank)->stamina, me2->dext, (rktab+me->rank)->dext, (rktab+me->rank)->magicpts);
		tx(block);
	}
}

calcdext()
{
	me2->dext=(rktab+me->rank)->dext;

	if(me2->flags & PFSITTING) me2->dext=me2->dext/2;
	else if(me2->flags & PFLYING)  me2->dext=me2->dext/3;
	if((LightHere==NO) || (me2->flags & PFBLIND)) me2->dext=me2->dext/5;

	me2->dext-=((me2->dext/10)-(((me2->dext/10)*((rktab+me->rank)->maxweight-(me2->weight)))/(rktab+me->rank)->maxweight));

	if (me2->flags & PFINVIS) me2->dext+=(me2->dext/3);
	if (me2->flags & PFSINVIS) me2->dext+=(me2->dext/2);
	if(me->flags&PFCRIP) me2->dext=0;
	me2->dext+=me2->dextadj;
}

toprank()
{
	int i;
	for(i=0; i<ranks-1; i++)
	{
		if((rktab+i)->tasks!=0)
		{
			me->tasks=me->tasks | (1<<((rktab+i)->tasks-1));
		}
	}
	aadd((rktab+ranks-1)->score-me->score+1, STSCORE, Af);
}

damage(register int obj,register int howmuch)
{
	objtab=obtab+obj;
	if(objtab->flags&OF_SCENERY) return;
	STATE->strength-=howmuch;
	if(STATE->strength<0 && (objtab->flags&OF_SHOWFIRE)) {txs("The %s has burnt away.",objtab->id); *objtab->rmlist=-1;}
	if(STATE->strength<0 && (!(objtab->flags&OF_SHOWFIRE))) {txs("The %s has just broken.",objtab->id); *objtab->rmlist=-1;}
}

repair(register int obj,register int howmuch)
{
	objtab=obtab+obj;
	if(objtab->flags&OF_SCENERY) return;
	STATE->strength+=howmuch;
}
