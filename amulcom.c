/*
          ####        ###     ###  ##     ##  ###
         ##  ##        ###   ###   ##     ##  ##            Amiga
        ##    ##       #########   ##     ##  ##            Multi
        ##    ##       #########   ##     ##  ##            User
        ########  ---  ## ### ##   ##     ##  ##            adventure
        ##    ##       ##     ##    ##   ##   ##            Language
       ####  ####     ####   ####   #######   ########


              ****    AMULCOM.C.......Adventure Compiler    ****
              ****               Main Program!              ****

    Copyright (C) Oliver Smith, 1990. Copyright (C) Kingfisher s/w 1990
  Program Designed, Developed and Written By: Oliver Smith & Richard Pike

 Notes:

   When the LOCATE function is installed, aswell as as the 'i' words, we must
  have a user variable, 'located'. This will allow the user to fiddle and
  tinker with the locate function... We will also need a 'setword' so that
  you could write:

   locate id [pref=]c|h|a|e|i [state=]# in=ID|cont=ID|outside|REGardless[/#]

   Last Amendments: 26/08/90   12:30   OS   Installed GameTime= (title.txt)
		    27/09/90   14:52   OS   Enhanced ObjProc.C (uses MC rtns)

									   */

#define	COMPILER 1

#include "h/amulcom.h"
#include "h/amul.defs.h"		/* Defines in one nice file     */
#include "h/amul.incs.h"		/* Include files tidily stored. */
#include "h/amul.vars.h"		/* all INTERNAL variables       */
#include "h/amul.lnks.h"		/* (external) Linkage symbols   */
#include "h/amul.cons.h"		/* Predefined Constants etc     */

/* Compiler specific variables... */

int	dchk;				/* Do we check DMOVE ptrs?	*/
int	dmoves;				/* How many DMOVEs to check?	*/
int	rmn;				/* Current room no.		*/
int	rmrd;				/* Read rooms?			*/
long	readgot,FPos;			/* Used during TT/Lang writes	*/
char	exi=0;				/* Is it okay to exit?		*/
int	warn=1;				/* Display warnings?		*/
char	Word[64],c;			/* For internal use only <grin>	*/
int	err;				/* Error count			*/
int	proc;				/* What we are processing	*/
long	clock;				/* Bits for time etc		*/
long	ohd;				/* Output handle for direct dos	*/
char	*data;				/* Pointer to data buffer	*/
char	*data2;				/* Secondary buffer area	*/
char	*syntab;			/* Synonym table, re-read	*/
char	idt[IDL+1];			/* Temporary ID store		*/
long	datal,datal2,mins;		/* Length of data! & gametime	*/
long	obmem;				/* Size of Objects.TXT		*/
long	vbmem;				/* Size of Lang.Txt		*/
long	invis, invis2;			/* Invisibility Stuff		*/
long	wizstr;				/* Wizards strength		*/
char	*mobdat,*px;			/* Mobile data			*/
long	moblen;				/* Length			*/

struct	Task *mytask,*FindTask();

struct UMSG
{
	char	id[IDL+1];
	long	fpos;
}umsg;

char	fnm[150],was[128],*getword(char *),*skipspc(char *),*sgetl(char *,char *),*skiplead(char *,char *);
char	*skipline(char *);
FILE	*ofp5;

struct _OBJ_STRUCT2 *obtab2,*objtab2,obj2,*osrch,*osrch2;

CXBRK()
{
	tx("\n[42m** Break pressed - memory released, files closed! Tata!\n\n[0m");
	quit();
}

	/*---------------------------------------------------------*/


const char *
getword(const char *from)
{
    char *to = Word;
    *to = 0;
    from = skipspc(from);
    for (const char* end = Word + sizeof(Word) - 1; to < end; ++to, ++from) {
        char c = *to = tolower(*from);
        if (c == ' ' || c == '\t') {
            c = *to = 0;
        }
        if (c == 0) {
            goto broke;
        }
    }

    // overflowed 'Word', add a trailing '\0' and drain remaining characters.
    *to = 0;
    for (;;) {
        switch (*from) {
        case 0:
        case ';':
        case ' ':
        case '\t': goto broke;
        default: ++from;
        }
    }

broke:
    return from;
}


close_ofps()
{
	if(ofp1!=NULL)fclose(ofp1);
	if(ofp2!=NULL)fclose(ofp2);
	if(ofp3!=NULL)fclose(ofp3);
	if(ofp4!=NULL)fclose(ofp4);
	if(ofp5!=NULL)fclose(ofp5);
	if(afp!=NULL)fclose(afp);
	ofp1=ofp2=ofp3=ofp4=ofp5=afp=NULL;
}

nextc(int f)			/* Find the next real stuff in file */
{
	do
	{
		while((c=fgetc(ifp))!=EOF && isspace(c));
		if(c==';' || c=='*') fgets(block,1024,ifp);
		if(c=='*')printf("[3m*%s[0m",block);	/* Print cmts */
	} while(c!=EOF && (c=='*' || c==';' || isspace(c)));
	if(f==1 && c==EOF)
	{
		printf("\nFile contains NO data!\n\n");
		quit();
	}
	if(c==EOF) return -1;
	fseek(ifp,-1,1);	/* Move back 1 char */
	return 0;
}

quit()
{
	if(exi!=1)
	{
		sprintf(block,"%s%s",dir,advfn);
		unlink(block);
	}
	unlink("ram:ODIDs");
	unlink("ram:umsg.tmp");
	unlink("ram:objh.tmp");
	if(mobdat)   FreeMem(mobdat,moblen); mobdat=NULL;
	if(mobp)     FreeMem(mobp,sizeof(mob)*mobchars); mobp=NULL;
	if(rmtab!=0) FreeMem(rmtab,sizeof(room)*rooms); rmtab=NULL;
	if(data!=0)  FreeMem(data, datal); data=NULL;
	if(data2!=0) FreeMem(data2,datal2);data2=NULL;
	if(obtab2!=0) FreeMem(obtab2,obmem); obtab2=NULL;
	if(vbtab!=0) FreeMem(vbtab,vbmem); vbtab=NULL;
	if(ifp!=NULL)fclose(ifp); ifp=NULL;
	close_ofps();
	exit(0);
}

fopenw(char *s)		/* Open file for reading */
{
	FILE *tfp;
	if(*s=='-') strcpy(fnm,s+1);
	else sprintf(fnm,"%s%s",dir,s);
	if((tfp=fopen(fnm,"wb"))==NULL) Err("write",fnm);
	if(ofp1==NULL)ofp1=tfp; else if(ofp2==NULL)ofp2=tfp; else if(ofp3==NULL) ofp3=tfp; else if(ofp4==NULL) ofp4=tfp; else ofp5=tfp;
	return NULL;
}

fopena(char *s)		/* Open file for appending */
{
	if(afp!=NULL) fclose(afp);
	if(*s=='-') strcpy(fnm,s+1);
	else sprintf(fnm,"%s%s",dir,s);
	if((afp=fopen(fnm,"rb+"))==NULL) Err("create",fnm);
	return NULL;
}

fopenr(char *s)		/* Open file for reading */
{
	if(ifp!=NULL) fclose(ifp);
	if(*s!='-') sprintf(fnm,"%s%s",dir,s);
	else strcpy(fnm,s+1);
	if((ifp=fopen(fnm,"rb"))==NULL) Err("open",fnm);
}

Err(char *s,char *t)
{
	printf("## Error!\x07\nCan't %s %s!\n\n",s,t); quit();
}

rfopen(s)		/* Open file for reading */
char *s;
{	FILE *fp;

	if(*s!='-') sprintf(fnm,"%s%s",dir,s);
	else strcpy(fnm,s+1);
	if((fp=fopen(fnm,"rb"))==NULL) Err("open",fnm);
	return (long)fp;
}

ttroomupdate()		/* Update room entries after TT */
{
	fseek(afp,0,0L);
	fwrite(rmtab->id,sizeof(room),rooms,afp);
}

oneword(char *s)		/* Cut one word out of a string */
{
	char *p;
	p=s;
	while(isspace(*s) && *s!=0) s++;
	*p=0; if(*s==0) return 0;
	while(!isspace(*s) && *s!=0) *(p++)=*(s++);
	*p=0;
}

opentxt(char *s)
{
	sprintf(block,"%s%s.TXT",dir,s);
	if((ifp=fopen(block,"rb"))==NULL)
	{
		printf("[33;1m !! Missing file %s !! [0m\n\n",block);
		exit(202);
	}
}

skipblock()
{	char c,lc;

	lc=0; c='\n';
	while(c!=EOF && !(c==lc=='\n')) { lc=c; c=fgetc(ifp); }
}

tidy(char *s)
{
	repspc(s); remspc(s);
loop:	if(isspace(*(s+strlen(s)-1))) {*(s+strlen(s)-1)=0; goto loop;}
}

is_verb(char *s)
{	register int i;

	if(verbs==0 || strlen(s) > IDL) { printf("@! illegal verb.\n"); return -1; }

	if(stricmp(s,verb.id)==0) return (verbs-1);

	vbptr=vbtab;
	for(i=0;i<verbs;i++,vbptr++)
	{
		if(stricmp(vbptr->id,s)==0) return i;
	}
	return -1;
}

blkget(long *s,char **p,long off)
{
	*s=filesize()+off;
	if((*p=(char *)AllocMem(*s,MEMF_PUBLIC))==NULL)
	{
		tx("\x07\n** Out of memory!\n\n"); close_ofps(); quit();
	}
	fread((*p)+off,1,*s,ifp); *((*p+*s)-2)=0; *((*p+*s)-1)=0;
	repcrlf((*p)+off);
}

long filesize()			/* Return size of current file */
{	register long now,s;

	now=ftell(ifp); fseek(ifp,0,2L); s=ftell(ifp)-now;
	fseek(ifp,now,0L);
	return s+2;			/* Just for luck! */
}

room_proc()			/*=* Process ROOMS.TXT *=*/
{
	char	lastc,*p,*p2;
	int	n;

	rooms=0;	nextc(1);		/* Skip any headings etc */

	fopenw(rooms1fn);	fopenw(rooms2fn);

	do
	{
		rooms++;
		p=block;
		while((c=fgetc(ifp))!=EOF && !isspace(c))*(p++)=c;
		*p=0;			/* Set null byte */
		striplead("room=",block);
		if(strlen(block)<3 || strlen(block)>IDL)
		{
			printf("!! \x07 Invalid ID: \"%s\"\x07 !!\n",block);
			quit();
		}
		strcpy(room.id,block);
			/*=* Do the flags *=*/
		room.flags=0; room.tabptr=-1; temp[0]=0;
		if(c!='\n')
		{
			fgets(block,1024,ifp);
			p=block;
			n=-1;
			do
			{
				while(isspace(*p) && *p!=0)p++;
				if(*p==0)continue;
				p2=p;
				while(!isspace(*p2) && *p2!=0)p2++;
				*p2=0;
				if(n==0)	/* Get dmove param */
				{
					strcpy(temp,p); dmoves++;
					p=p2+1; n=-1; continue;
				}
				if((n=isrflag(p))==-1)
				{
					printf("\x07'%s' is invalid!\n\n\x07",p);
					quit();
				}
				n-=NRNULL;
				if(n>=0) room.flags=(room.flags | bitset(n));
				p=p2+1;
			}while(*p!=0);
		}

		lastc='\n';
		fseek(ofp2,0,1);room.desptr=ftell(ofp2);n=0;
		if(temp[0]!=0) fwrite(temp,IDL,1,ofp2);	/* save dmove */
		while((c=fgetc(ifp))!=EOF && !(c=='\n' && lastc=='\n'))
		{
			if(lastc=='\n' && c==9) continue;
			fputc((lastc=c),ofp2); n++;
		};
		fputc(0,ofp2);
		fwrite(room.id,sizeof(room),1,ofp1);
		nextc(0);
	}while(c!=EOF);
	close_ofps();
}


checkdmoves()
{
	int n;
	struct _ROOM_STRUCT *roomptr;

	/*=* Check DMOVE ptrs *=*/
	fopenr(rooms2fn);	/* Open desc. file */
	roomptr=rmtab;
	for(n=0;n<rooms;n++)
	{
		if(roomptr->flags & DMOVE)
		{
			sprintf(block,"%-9s\x0d",roomptr->id); tx(block);
			fseek(ifp,roomptr->desptr,0);
			fread(dmove,IDL,1,ifp);	/* Read the DMOVE name */
			if(isroom(dmove)==-1)	/* Is it a valid room? */
			{
				sprintf(block,"%-9s - invalid DMOVE '%s'...\n",roomptr->id,dmove);
				tx(block); dchk=-1;
			}
		}
		roomptr++;
	}
	if(dchk==-1) quit(0*tx("\nCompile failed due to invalid DMOVE flags!\n"));
}

rank_proc()			/*=* Process RANKS.TXT *=*/
{
	char	*p; int	n;

	nextc(1);
	fopenw(ranksfn);
	putchar('\n');

	ranks=0;n=0;err=0;

	do
	{
		fgets(block,1024,ifp); if(feof(ifp)) continue;
		if(com(block)==-1 || block[0]=='\n') continue;
		tidy(block); if(block[0]==0) continue;
		p=getword(block); if(chkline(p)!=0) continue;
		ranks++; rank.male[0]=0; rank.female[0]=0;
		if(strlen(Word)<3 || strlen(Word)>RANKL)
		{
			printf("!! \x07 Invalid Male Rank: \"%s\"\x07 !!\n",Word);
			quit();
		}
		n=0;
		do
		{
			if(Word[n]=='_') Word[n]=' ';
			rank.male[n]=rank.female[n]=tolower(Word[n]);
			n++;
		} while(Word[n-1]!=0);

		p=getword(p); if(chkline(p)!=0) continue;
		if(strcmp(Word,"=")!=NULL && strlen(Word)<3 || strlen(Word)>RANKL)
		{
			printf("\n!! \x07 Invalid Female Rank: \"%s\"\x07 !!\n",Word);
			quit();
		}
		if(Word[0]!='=')
		{
			n=0;
			do
			{
				if(Word[n]=='_') Word[n]=' ';
				rank.female[n]=tolower(Word[n]);
				n++;
			} while(Word[n-1]!=0);
		}

		p=getword(p); if(chkline(p)!=0) continue;
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.score, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.score=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.strength, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.strength=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number stamina, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.stamina=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.dexterity, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.dext=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number wisdom, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.wisdom=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number experience, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.experience=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number magic points, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.magicpts=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number max weight carried, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.maxweight=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number max number of objects carried, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.numobj=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min. points per kill, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.minpksl=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid task number, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.tasks=atoi(Word);

		p=skipspc(p); if(*p=='\"') p++;
		strcpy(block,p); p=block;
		while(*p!=0 && *p!='\"') p++;
		*(p++)=0;
		if(p-block>10)	/* Greater than prompt length? */
		{
			printf("\n\"%s\" prompt (rank %d) too long!\n",block,ranks);
			err++; continue;
		}
		if(block[0] == 0) strcpy(rank.prompt,"$ ");
		else strcpy(rank.prompt,block);

		wizstr=rank.strength;
		fwrite(rank.male,sizeof(rank),1,ofp1);
	} while(!feof(ifp));
	if(err!=0)
	{
		printf("\n\n\x07!! Aborting due to %ld errors!\n\n",err);
		quit();
	}
	close_ofps();
}

chkline(char *p)
{
	if(*p==0)
	{
		printf("## Rank line %ld incomplete!!!\n",ranks);
		err++; return 1;
	}
	return 0;
}
obds_proc()
{	char	lastc;

	obdes=0;err=0;
	fopenw(obdsfn); close_ofps();		/* Create file */
	if(nextc(0)==-1) return tx("** No Long Object Descriptions!\n");
	fopenw("-ram:ODIDs");	fopenw(obdsfn);
	do
	{
		fgets(block,1024,ifp); tidy(block);
		striplead("desc=",block); getword(block);
		if(strlen(Word)<3 || strlen(Word)>IDL)
		{
			printf("!! \x07 Invalid ID: \"%s\"\x07 !!\n",Word);
			printf("@! note: strlen(Word)=%ld\n",strlen(Word));
			err++; skipblock(); continue;
		}
		strcpy(objdes.id,Word);
		fseek(ofp2,0,2); objdes.descrip=ftell(ofp2);
		fwrite(objdes.id,sizeof(objdes),1,ofp1);
		lastc='\n';
		while((c=fgetc(ifp))!=EOF && !(c=='\n' && lastc=='\n'))
		{
			if((lastc==EOF || lastc=='\n') && c==9) continue;
			fputc((lastc=c),ofp2);
		};
		fputc(0,ofp2);
		obdes++; nextc(0);
	} while(c!=EOF);
	if(err!=0)
	{
		printf("\n\n\x07!! Aborting due to %ld errors!\n\n",err);
		quit();
	}
	close_ofps();
}
objs_proc()
{	register char *p,*s; int roomno;

	nouns=adjs=err=0;

	/* Clear files */
	fopenw(adjfn); close_ofps();
	fopenw(objsfn); fopenw(statfn); fopenw(objrmsfn); fopena(adjfn);

	if(nextc(0)==-1) { close_ofps(); return 0; } /* Nothing to process */
	blkget(&obmem,(char **)&obtab2,32*sizeof(obj2)); objtab2=obtab2+32;
	s=(char *)objtab2;

	do
	{
		if(err>30)
		{
			printf("\x07** Maximum number of errors exceeded!\n");
			quit();
		}
		do p=s=sgetl(s,block); while(*s!=0 && (com(block)==-1 || block[0]==0));
		if(*s==0 || block[0]==0) continue;
		tidy(block); if(block[0]==0) continue;
		striplead("noun=",block); p=getword(block);
		if(strlen(Word)<3 || strlen(Word)>IDL)
		{
			printf("## \x07 Invalid ID: \"%s\"\x07 ##\n",Word);
			err++; Word[IDL+1]=0;
		}
		obj2.adj=obj2.mobile=-1; obj2.idno=nouns;
		obj2.state=obj2.nrooms=obj2.contains=obj2.flags=obj2.putto=0;
		obj2.rmlist=(long *) ftell(ofp3); strcpy(obj2.id,Word);

		/* Get the object flags */
		do
		{
			p=getword(p); if(Word[0] == 0) continue;
			if((roomno=isoflag1(Word))!=-1)
				obj2.flags=(obj2.flags | bitset(roomno));
			else
			{
				if((roomno=isoparm())==-1)
				{
					printf("\x07## Invalid parameter '%s'\n",Word);
					err++; continue;
				}
				switch(bitset(roomno))
				{
					case OP_ADJ:	set_adj(); break;
					case OP_START:	set_start(); break;
					case OP_HOLDS:	set_holds(); break;
					case OP_PUT:	set_put(); break;
					case OP_MOB:	set_mob(); mobs++; break;
					default:
						printf("** Internal: Code for object-parameter '%s' missing!\n",obparms[roomno]);
				}
			}
		} while(Word[0] != 0);

		/* Get the room list */

		p=block; *p='+'; *(p+1)=0; roomno=0;
		do
		{
			p=getword(p);
			if(Word[0] == '+')
			{
				do s=sgetl(s,block); while(*s!=0 && block[0]!=0 && com(block)==-1);
				if(*s==0)
				{
					printf("** Unexpected end of file in Objects.TXT!\n");
					quit();
				}
				p=block; Word[0]=' '; continue;
			}
			if(Word[0] == 0) continue;
			if((roomno=isloc(Word)) == -1) { roomno=-1; continue; }
			fwrite((char *)&roomno,1,4,ofp3);
			obj2.nrooms++;
		} while(Word[0] !=0);
		if(obj2.nrooms == 0  &&  roomno == 0)
		{
			printf("\x07!! No rooms listed for object '%s'...\n",obj2.id);
			err++;
		}
		obj2.nstates=0;
		do
		{
			do s=sgetl(s,block); while(block[0]!=0 && com(block)==-1 && block[0]!='\n');
			if(block[0] == 0 || block[0]=='\n') break;
			state_proc(); block[0]='-';
		} while(block[0]!=0 && block[0]!='\n');
		if(obj2.nstates==0 || obj2.nstates>100)
			object("amount of states (i.e. none)");
		if((long)(obtab2+(nouns)) > (long)s) printf("@! table exceeded data\n");
		*(obtab2+(nouns++))=obj2;
	} while(*s!=0);
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	/*
	close_ofps();
	sort_objs();
	*/
	fwrite((char *)obtab2,sizeof(obj2),nouns,ofp1);
	close_ofps();
}

/*
sort_objs()
{	register int i,j,k,nts; long *rmtab,*rmptr;

	if(ifp!=NULL) fclose(ifp); ifp=NULL;
	close_ofps(); fopenr(statfn);	blkget(&datal,&data,NULL); fclose(ifp); ifp=NULL;
	close_ofps(); fopenr(objrmsfn); blkget(&datal2,&data2,NULL); fclose(ifp); ifp=NULL;
	close_ofps(); fopenw(objsfn);	fopenw(statfn); fopenw(objrmsfn); fopenw(ntabfn);
	ifp=NULL;

	printf("Sorting Objects...:\r"); objtab2=obtab2; nts=0; k=0;

	statab=(struct _OBJ_STATE *)data; rmtab=(long *)data2;
	for(i=0; i<nouns; i++)
	{
		if(*(objtab2=(obtab2+i))->id==0)
		{
			printf("@! skipping %ld states, %ld rooms.\n",objtab2->nstates,objtab2->nrooms);
			statab += objtab2->nstates;
			rmtab  += objtab2->nrooms;
			continue;
		}
		strcpy(nountab.id,objtab2->id); nts++;
		nountab.num_of=0; osrch=objtab2; statep=statab; rmptr=rmtab;
		for(j=i; j<nouns; j++, osrch++)
		{
			if(*(osrch->id)!=0 && stricmp(nountab.id,osrch->id)==NULL)
			{
				fwrite((char *)osrch,  sizeof(obj),   1,               ofp1);
				fwrite((char *)statep, sizeof(state), osrch->nstates,  ofp2);
				fwrite((char *)rmptr,  sizeof(long),  osrch->nrooms,   ofp3);
				nountab.num_of++; *osrch->id=0; if(osrch!=objtab) k++;
				statep+=osrch->nstates; rmptr+=osrch->nrooms;
				if(osrch==objtab2) { statab=statep; rmtab=rmptr; objtab2++; i++; }
			}
			else statep+=osrch->nstates; rmptr+=osrch->nrooms;
		}
		
		fwrite((char *)&nountab, sizeof(nountab), 1, ofp4);
	}
	printf("%20s\r%ld objects moved.\n"," ",k);
	close_ofps();
	FreeMem(data, datal); FreeMem(data2, datal2); data=data2=NULL;
	fopenr(objsfn); fread((char *)obtab2, sizeof(obj), nouns, ifp);
}
*/

statinv(register char *s)
{
	printf("\nObject #%d \"%s\" has invalid %s state line!\n",nouns+1,obj2.id,s,block);
	quit();
}

state_proc()
{	register int flag; register char *p;

	state.weight=state.value=state.flags=0; state.descrip=-1;

	tidy(block); if(block[0]==0) return;

	/* Get the weight of the object */
	striplead("weight=",block);
	p=getword(block); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("weight value on");
	state.weight=atoi(Word);
	if(obj2.flags & OF_SCENERY) state.weight = wizstr+1;

	/* Get the value of it */
	p=skipspc(p); striplead("value=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("value entry on");
	state.value=atoi(Word);

	/* Get the strength of it (hit points)*/
	p=skipspc(p); striplead("str=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("strength entry on");
	state.strength=atoi(Word);

	/* Get the damage it does as a weapon*/
	p=skipspc(p); striplead("dam=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("damage entry on");
	state.damage=atoi(Word);

	/* Description */
	p=skipspc(p); striplead("desc=",p);
	if(*p==0) statinv("incomplete");
	if(*p=='\"' || *p=='\'') { text_id(p+1,*p); p=block; }
	else
	{
		p=getword(p); is_desid();	/* Is it valid? */
	}
	if(state.descrip==-1)
	{
		sprintf(temp,"desc= ID (%s) on",Word); statinv(temp);
	}
	while(*p!=0)
	{
		p=getword(p); if(Word[0]==0) break;
		if((flag=isoflag2(Word))==-1) statinv("flag on");
		state.flags=(state.flags | bitset(flag));
	}
	fwrite((char *)&state.weight,sizeof(state),1,ofp2);
	obj2.nstates++;
}

is_desid()
{	register int i; register FILE *fp;
	if(stricmp(Word,"none")==NULL) return state.descrip=-2;
	if((fp=fopen("ram:ODIDs","rb+"))==NULL) Err("open","ram:ODIDs");
	for(i=0;i<obdes;i++)
	{
		fread(objdes.id,sizeof(objdes),1,fp); state.descrip=objdes.descrip;
		if(stricmp(Word,objdes.id)==0)
		{
			fclose(fp); return;
		}
	}
	fclose(fp); state.descrip=-1;
}

text_id(register char *p,register char c)
{	char *ptr; FILE *fp;

	strcpy(block,p); p=block;
	while(*p!=c && *p!=0) p++;
	if(*p==0) *(p+1)=0;
	*(p++)='\n';
	if(*(p-2) == '{') ptr=p-1; else ptr=p;

	sprintf(temp,"%s%s",dir,obdsfn);	/* Open output file */
	if((fp=fopen(temp,"rb+"))==NULL) Err("open",temp);
	fseek(fp,0,2L); state.descrip=ftell(fp); /* Get pos */
	if(fwrite(block,ptr-block,1,fp)!=1) { fclose(fp); Err("write",temp); }
	fputc(0,fp); strcpy(block,p); fclose(fp);
}

isnoun(register char *s)
{	register int i;

	objtab2=obtab2;
	if(stricmp(s,"none")==NULL) return -2;
	for(i=0; i<nouns; i++,objtab2++)
		if(stricmp(s,objtab2->id)==NULL) return i;
	return -1;
}

iscont(register char *s)
{	register int i;

	objtab2=obtab2;
	for(i=0; i<nouns; i++,objtab2++)
		if(stricmp(s,objtab2->id)==NULL && objtab2->contains>0) return i;
	return -1;
}

isloc(register char *s)		/* Room or container */
{	register int i;

	if((i = isroom(s)) != -1) return i;
	if((i = iscont(s)) == -1)
	{
		if(isnoun(s) == -1)
			printf("\x07## Invalid object start location, '%s'...\n",s);
		else
			printf("\x07## Tried to start '%s' in non-container '%s'!\n",obj2.id,s);
		err++; return -1;
	}

	return -(INS+i);
}
title_proc()
{
	nextc(1);	fgets(block,1000,ifp); repspc(block); remspc(block);
	if(!striplead("name=",block))
	{
		tx("Invalid title.txt; missing 'name=' line!\n");
		quit();
	}
	block[strlen(block)-1]=0;	/* Remove \n */
	if(strlen(block)>40)
	{
		block[40]=0;
		printf("Adventure name too long!            \nTruncated to %40s...\n",block);
	}
	strcpy(adname,block);
	fgets(block,1000,ifp); repspc(block);
	mins=getno("gametime=");
	if(mins<15) { tx("!! Minimum game time of 15 minutes inforced!\n"); mins=15; }

	fgets(block,1000,ifp); repspc(block);
	invis=getno("invisible="); remspc(block); getword(block);
	if(!isdigit(Word[0]))
	{
		printf("## Invalid rank for visible players to see other invisible players/objects.\n");
		err++;
	}
	else invis2=atoi(Word);

	fgets(block,1000,ifp); repspc(block);
	minsgo=getno("min sgo=");

	/*-* Get the Scaleing line. *-*/
	fgets(block,1000,ifp); repspc(block);
	rscale=getno("rankscale=");		/* Process RankScale= */
	tscale=getno("timescale=");		/* Process TimeScale= */

	readgot=ftell(ifp);

	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
}

getno(char *s)
{	char *p;
	remspc(block);
	if(!striplead(s,block))
	{
		printf("## Missing %s entry!\n",s); err++; return -1;
	}
	p=getword(block); strcpy(block,p);
	if(!isdigit(Word[0]))
	{
		printf("## Invalid %s entry...\n",s); err++; return -1;
	}
	return atoi(Word);
}
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
/*
     Travel Processing Routines for AMUL, Copyright (C) Oliver Smith, '90
     --------------------------------------------------------------------
  Warning! All source code in this file is copyright (C) KingFisher Software
*/


trav_proc()			/*=* Process TRAVEL.TXT *=*/
{
	register int strip,lines,nvbs,i,ntt,t,r;
	register char *p; register long *l;

	nextc(1);		/* Move to first text */
	fopenw(ttfn); fopenw(ttpfn); fopena(rooms1fn);
	err=ntt=t=0;

	do
	{
loop1:		if(err>30)
		{
			printf("\x07** Maximum number of errors exceeded!\n");
			quit();
		}
		fgets(block,1000,ifp); if(feof(ifp)) continue; tidy(block);
		if(com(block)==-1 || block[0]==0) goto loop1;
		p=block; getword(block); striplead("room=",Word);
		if((rmn=isroom(Word))==-1)
		{
			printf("** Invalid room '%s'!\n",Word);
			err++; skipblock(); goto loop1;
		}
		if(roomtab->tabptr!=-1)
		{
			printf("\x07!! Room \"%s\" defined twice in travel table!\n",roomtab->id);
			err++; skipblock(); goto loop1;
		}
vbloop:		do fgets(block,1000,ifp); while(com(block)==-1);
		if(block[0]==0 || block[0]=='\n')
		{
			/* Only complain if room is not a death room */
			if((roomtab->flags & DEATH)!=DEATH && warn==1)
				printf("## Room \"%s\" has no TT entries!\n",roomtab->id);
			roomtab->tabptr=-2;
			ntt++; continue;
		}
		tidy(block);
		if(!striplead("verb=",block) && !striplead("verbs=",block))
		{
			printf("## Room %s: expected a VERB[S]= entry!\n",roomtab->id);
			err++; goto vbloop;
		}
		lines=0; verb.id[0]=0;
		roomtab->tabptr=t;roomtab->ttlines=0;
vbproc:		/* Process verb list */
		nvbs=0; tt.pptr=(long *)-1;
		l=(long *)temp; p=block;
		/* Break verb list down to verb no.s */
		do
		{
			p=getword(p);
			if(Word[0] == 0) break;
			if((*l=is_verb(Word))==-1)
			{
				printf("\nRoom \"%s\" has invalid verb, \"%s\"...\n",roomtab->id,Word);
				err++;
			}
			l++; nvbs++;
		} while(Word[0]!=0);
		if(nvbs == 0)
		{
			printf("Room \"%s\" has empty verb[s]= line!\n",roomtab->id);
			quit();
		}
		/* Now process each instruction line */
		do
		{
xloop:			strip=0; r=-1;
			block[0]=0; fgets(block,1000,ifp);
			if(feof(ifp)) break;
			if(block[0]==0 || block[0]=='\n') { strip=-1; continue; }
			tidy(block); if(com(block)==-1 || block[0]==0) goto xloop;
			if(striplead("verb=",block) || striplead("verbs=",block))
			{
				strip=1; break;
			}
			p=precon(block);	/* Strip pre-condition opts */
notloop:		p=getword(p);
			if(strcmp(Word,ALWAYSEP)==NULL)
			{
				tt.condition=CALWAYS; tt.action=-(1+AENDPARSE);
				goto write;
			}
			if(strcmp(Word,"not")==NULL || strcmp(Word,"!")==NULL)
			{
				r=-1*r; goto notloop;
			}
notlp2:			if(Word[0]=='!') { strcpy(Word,Word+1); r=-1*r; goto notlp2; }
			if((tt.condition=iscond(Word)) == -1)
			{
				tt.condition=CALWAYS;
				if((tt.action=isroom(Word))!=-1) goto write;
				if((tt.action=isact(Word))==-1)
				{
					printf("\x07Invalid condition, \"%s\", in TT entry for room \"%s\"...\n",
						Word,roomtab->id);
					err++; goto xloop;
				}
				goto gotohere;
			}
			p=skipspc(p);
			if((p=chkcparms(p,tt.condition,ofp2))==NULL) { err++; goto next; }
			if(r==1) tt.condition=-1-tt.condition;
			if(*p==0)
			{
				printf("\x07Room \"%s\" has entry with missing action!\n",roomtab->id);
				err++; goto xloop;
			}
			p=preact(p); p=getword(p);
			if((tt.action=isroom(Word))!=-1) goto write;
			if((tt.action=isact(Word))==-1)
			{
				printf("\x07Invalid action, \"%s\", in TT entry for room \"%s\"...\n",
					Word,(rmtab+rmn)->id);
				err++; goto xloop;
			}
gotohere:		if(tt.action==ATRAVEL)
			{
				printf("## Tried to call TRAVEL action from travel table! (grin)\n");
				err++; goto xloop;
			}
			p=skipspc(p);
			if((p=chkaparms(p,tt.action,ofp2))==NULL) { err++; goto next; }
			tt.action=0-(tt.action+1);
write:			roomtab=rmtab+rmn;
			l=(long *)temp;
			for(i=0; i<nvbs; i++)
			{
				if(i<nvbs-1) tt.pptr=(long *)-2; else tt.pptr=(long *)-1;
				tt.verb=*(l++);
				fwrite((char *)&tt.verb,sizeof(tt),1,ofp1);
				roomtab->ttlines++; t++; ttents++;
			}
			lines++;
next:			strip=0;
		} while(strip==0 && !feof(ifp));
		if(strip==1 && !feof(ifp)) goto vbproc;
		nextc(0);
		ntt++;
	} while(!feof(ifp));
	if(err==0 && ntt!=rooms && warn==1)
	{
		roomtab=rmtab;
		for(i=0; i<rooms; i++,roomtab++)
			if(roomtab->tabptr == -1 && (roomtab->flags & DEATH) != DEATH)
				printf("Room \"%s\" has no TT entry!\n",roomtab->id);
	}
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	ttroomupdate();
	close_ofps();
}
/* Lang.TXT processor */

lang_proc()			/*=* Process LANG.TXT *=*/
{
	register char	lastc,*p,*p2,*s1,*s2;
	/* n=general, cs=Current Slot, s=slot, of2p=ftell(ofp2) */
	register int	n,cs,s,r;
	unsigned long	of2p,of3p;

	err=0; verbs=0; nextc(1);
	fopenw(lang1fn); close_ofps(); fopena(lang1fn); ofp1=afp; afp=NULL;
	fopenw(lang2fn); fopenw(lang3fn); fopenw(lang4fn);

	blkget(&vbmem,(char **)&vbtab,64*(sizeof(verb))); vbptr=vbtab+64;
	s1=(char *)vbptr; vbptr=vbtab;
	of2p=ftell(ofp2); of3p=ftell(ofp3); FPos=ftell(ofp4);

	do
	{
		if(err>30)
		{
			printf("\x07** Maximum number of errors exceeded!\n");
			quit();
		}
		verbs++; p=block;
loop:		do { s1=sgetl((s2=s1),block); *(s1-1)=0; } while(com(block)==-1 && *s1!=0);
		if(*s1==0) { verbs--; break; }
		tidy(block); if(block[0]==0) goto loop;
		p=skiplead("verb=",block); p=getword(p);
		if(Word[0]==0)
		{
			printf("!! \x07 verb= line without a verb!\n"); goto loop;
		}
		if(strlen(Word)>IDL)
		{
			printf("\x07 Invalid verb ID: \"%s\"",Word); err++;
			do { s1=sgetl((s2=s1),block); *(s1-1)=0; } while(*s1!=0 && block[0]!=0);
			if(*s1==0) break;
			goto loop;
		}

		strcpy(verb.id,Word);

		p2=verb.sort;
		*(p2++)=-1; *(p2++)='C'; *(p2++)='H'; *(p2++)='A'; *(p2++)='E';
		*(p2++)=-1; *(p2++)='C'; *(p2++)='H'; *(p2++)='A'; *(p2++)='E';

		verb.flags=VB_TRAVEL; if(*p==0 || *p==';' || *p=='*') goto noflags;
		p=getword(p);
		if(strcmp("travel",Word)==NULL)
		{
			verb.flags=0; p=getword(p);
		}
		if(strcmp("dream",Word)==NULL)
		{
			verb.flags+=VB_DREAM; p=getword(p);
		}
		if(Word[0]==0 || Word[0]==';' || Word[0]=='*') goto noflags;

		if(chae_proc(Word,verb.sort)==-1) goto noflags;
		p=getword(p); if(Word[0]!=0 && Word[0]!=';' && Word[0]!='*') chae_proc(Word,verb.sort2);

noflags:	verb.ents=0; verb.ptr=(struct _SLOTTAB *)of2p;

stuffloop:	do { s2=s1; s1=sgetl(s1,block); *(s1-1)=0; } while(*s1!=0 && block[0]!=0 && com(block)==-1);
		if(*s1==0 || block[0]==0)
		{
			if(verb.ents==0 && (verb.flags & VB_TRAVEL)) printf("!! \x07Verb \"%s\" has no entries!\n",verb.id);
			goto write;
		}

		tidy(block); if(block[0]==0) goto stuffloop;

		if((p=skiplead("syntax=",block))==block)
		{
			vbprob("no syntax= line",s2); goto stuffloop;
		}

		/* Syntax line loop */
synloop:	setslots(WNONE); verb.ents++; p=skiplead("verb",p);
		p2=getword(p); p2=skipspc(p2);

		/* If syntax line is 'syntax=verb any' or 'syntax=none' */
		if(*p2==0 && strcmp("any",Word)==NULL)
		{
			setslots(WANY); goto endsynt;
		}
		if(*p2==0 && strcmp("none",Word)==NULL)
		{
			setslots(WNONE); goto endsynt;
		}

sp2:		/*=* Syntax line processing *=*/
		p=skipspc(p); if(*p==';' || *p=='|' || *p=='*') goto endsynt;
		Word[0]=0; p=getword(p);
		if(Word[0]==0) goto endsynt;
		if((n=iswtype(Word))==-3)
		{
			sprintf(block,"Invalid phrase, '%s', on syntax line!",Word);
			vbprob(block,s2); goto commands;
		}
		if(Word[0]==0) { s=WANY; goto skipeq; }

		/*=* First of all, eliminate illegal combinations *=*/
		if(n==WNONE || n==WANY)
		{	/* you cannot say none=fred any=fred etc */
			sprintf(block,"Tried to defined %s= on syntax line",syntax[n]);
			vbprob(block,s2);
			goto endsynt;
		}
		if(n==WPLAYER && strcmp(Word,"me")!=NULL && strcmp(Word,"myself")!=NULL)
		{
			vbprob("Tried to specify player other than self",s2);
			goto endsynt;
		}

		/* Now check that the 'tag' is the correct type of word */

		s=-1;
		switch(n)
		{
		    case WADJ:
			/* Need ISADJ() - do TT entry too */
		    case WNOUN: s=isnoun(Word); break;
		    case WPREP: s=isprep(Word); break;
		    case WPLAYER:
			if(strcmp(Word,"me")==NULL || strcmp(Word,"myself")==NULL) s=-3; break;
		    case WROOM:	s=isroom(Word); break;
		    case WSYN:	printf("!! Syn's not supported at this time!\n"); s=WANY;
		    case WTEXT:	s=chkumsg(Word); break;
		    case WVERB:	s=is_verb(Word); break;
		    case WCLASS: s=WANY;
		    case WNUMBER:
				if(Word[0]=='-') s=atoi(Word+1);
				else s=atoi(Word);
		    default:	 printf("** Internal error! Invalid W-type!\n");
		}

		if(n==WNUMBER && s>100000 || -s>100000)
		{
			sprintf(fnm,"Invalid number, %ld",s); vbprob(fnm,s2);
		}
		if(s==-1 && n!=WNUMBER)
		{
			sprintf(fnm,"Invalid setting, '%s' after %s=",Word,syntax[n+1]);
			vbprob(fnm,s2);
		}
		if(s==-3 && n==WNOUN) s=-1;

skipeq:		/*=* (Skipped the equals signs) *=*/
		/* Now fit this into the correct slot */
		cs=1;			/* Noun1 */
		switch(n)
		{
		    case WADJ:
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[4]!=WNONE)
			{
				vbprob("Invalid NOUN NOUN ADJ combination",s2);
				n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[3]!=WNONE)
			{
				vbprob("Invalid NOUN ADJ NOUN ADJ combination",s2);
				n=-5; break;
			}
			if(vbslot.wtype[0]!=WNONE && vbslot.wtype[3]!=WNONE)
			{
				vbprob("More than two adjectives on a line",s2);
				n=-5; break;
			}
			if(vbslot.wtype[0]!=WNONE) cs=3;
			else cs=0;
			break;
		    case WNOUN:
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[4]!=WNONE)
			{
				vbprob("Invalid noun arrangement",s2);
				n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE) cs=4;
			break;
		    case WPREP:
			if(vbslot.wtype[2]!=WNONE)
			{
				vbprob("Invalid PREP arrangement",s2);
				n=-5; break;
			}
			cs=2; break;
		    case WPLAYER:
		    case WROOM:
		    case WSYN:
		    case WTEXT:
		    case WVERB:
		    case WCLASS:
		    case WNUMBER:
			if(vbslot.wtype[1]!=WNONE && vbslot.wtype[4]!=WNONE)
			{
				sprintf(block,"No free noun slot for '%s' entry",
					syntax[n+1]);
				vbprob(block,s2);
				n=-5; break;
			}
			if(vbslot.wtype[1]!=WNONE) cs=4;
			break;
		}
		if(n==-5) goto sp2;
		/* Put the bits into the slots! */
		vbslot.wtype[cs]=n; vbslot.slot[cs]=s;
		goto sp2;

endsynt:	vbslot.ents=0; vbslot.ptr=(struct _VBTAB *)of3p;

commands:	lastc='x'; proc=0;

		do { s2=s1; s1=sgetl(s1,block); *(s1-1)=0; } while(*s1!=0 && block[0]!=0 && com(block)==-1);
		if(block[0]==0 || *s1==0) { lastc=1; goto writeslot; }
		tidy(block);
		if((p=skiplead("syntax=",block))!=block) { lastc=0; goto writeslot; }
		if(*p==0) goto commands;

		vbslot.ents++; r=-1; vt.pptr=(long *)FPos;

		/* Process the condition */
notloop:	p=precon(p); p=getword(p);

		/*=* always endparse *=*/
		if(strcmp(Word,ALWAYSEP)==NULL)
		{
			vt.condition=CALWAYS; vt.action=-(1+AENDPARSE);
			goto writecna;
		}
		if(strcmp(Word,"not")==NULL || (Word[0]=='!' && Word[1]==0))
		{
			r=-1*r; goto notloop;
		}

		/*=* If they forgot space between !<condition>, eg !toprank *=*/
notlp2:		if(Word[0]=='!') { strcpy(Word,Word+1); r=-1*r; goto notlp2; }

		if((vt.condition=iscond(Word)) == -1)
		{
			proc=1;
			if((vt.action=isact(Word)) == -1)
			{
				if((vt.action=isroom(Word))!=-1) { vt.condition=CALWAYS; goto writecna; }
				sprintf(block,"Invalid condition, '%s'",Word); vbprob(block,s2);
				proc=0; goto commands;
			}
			vt.condition=CALWAYS;
			goto doaction;
		}
		p=skipspc(p); proc=1;
		if((p=chkcparms(p,vt.condition,ofp4))==NULL) { err++; goto commands; }
		if(*p==0)
		{
			if((vt.action=isact(conds[vt.condition]))==-1)
			{
				vbprob("Missing action",s2); goto commands;
			}
			vt.action=0-(vt.action+1); vt.condition=CALWAYS; goto writecna;
		}
		if(r==1) vt.condition=-1-vt.condition;
		p=preact(p); p=getword(p);
		if((vt.action=isact(Word))==-1)
		{
			if((vt.action=isroom(Word))!=-1) goto writecna;
			sprintf(block,"Invalid action, '%s'",Word); vbprob(block,s2);
			goto commands;
		}
doaction:	p=skipspc(p);
		if((p=chkaparms(p,vt.action,ofp4))==NULL) { err++; goto commands; }
		vt.action=0-(vt.action+1);

writecna:	/* Write the C & A lines */
		fwrite((char *)&vt.condition,sizeof(vt),1,ofp3); proc=0; of3p+=sizeof(vt);
		goto commands;

writeslot:	fwrite(vbslot.wtype,sizeof(vbslot),1,ofp2); proc=0; of2p+=sizeof(vbslot);
		if(lastc>1) goto commands;
		if(lastc==0) goto synloop;

		lastc='\n';
write:		fwrite(verb.id,sizeof(verb),1,ofp1); proc=0;
		*(vbtab+(verbs-1))=verb;
		if((long)(vbtab+(verbs-1)) > (long)s1) printf("@! table overtaking s1\n");
	} while(*s1!=0);
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	close_ofps();
}

chae_proc(register char *f,register char *t)	/* From and To */
{	register int n;

	if(*f<'0' || *f>'9' && *f!='?') { chae_err(); return -1; }

	if(*f=='?') { *(t++)=-1; f++; }
	else
	{
		n=atoi(f); while(isdigit(*f) && *f!=0) f++;
		if(*f==0) { chae_err(); return -1; }
		*(t++)=(char) n;
	}

	for(n=1; n<5; n++)
	{
		if(*f=='c' || *f=='h' || *f=='a' || *f=='e') { *(t++)=toupper(*f); f++; }
		else { chae_err(); return -1; }
	}

	return 0;
}
	
chae_err(char *p)
{
	printf("\x07## Invalid '#CHAE' flags, \"%s\" in verb %s.\n",Word,verb.id); err++;
}

setslots(unsigned char i)		/* Set the VT slots */
{
	vbslot.wtype[0]=WANY; vbslot.wtype[1]=i; vbslot.wtype[2]=i; vbslot.wtype[3]=WANY; vbslot.wtype[4]=i;
	vbslot.slot[0]=vbslot.slot[1]=vbslot.slot[2]=vbslot.slot[3]=vbslot.slot[4]=WANY;
}

iswtype(char *s)		/* Is 'text' a ptype */
{	int i;

	for(i=0; i<nsynts; i++)
	{
		if(strcmp(s,syntax[i])==NULL) { *s=0; return i-1; }
		if(strncmp(s,syntax[i],syntl[i])!=NULL) continue;
		if(*(s+syntl[i])!='=') continue;
		strcpy(s,s+syntl[i]+1); return i-1;
	}
	return -3;
}

vbprob(char *s,char *s2)	/* Declare a PROBLEM, and which verb its in! */
{
	printf("## Verb %s: line '%s'\n%s!\n",verb.id,s2,s); err++;
}

/* Note about matching actuals...

Before agreeing a match, remember to check that the relevant slot isn't
set to NONE.
Variable N is a wtype... If the phrases 'noun', 'noun1' or 'noun2' are used,
instead of matching the phrases WTYPE with n, match the relevant SLOT with
n...

So, if the syntax line is 'verb text player' the command 'tell noun2 text'
will call isactual with *s=noun2, n=WPLAYER.... is you read the 'actual'
structure definition, 'noun2' is type 'WNOUN'. WNOUN != WPLAYER, HOWEVER
the slot for noun2 (vbslot.wtype[4]) is WPLAYER, and this is REALLY what the
user is refering too.							     */
actualval(char *s,int n)	/* Get actual value! */
{	register int i;

	if(n!=-70 && (*s=='?' || *s=='%' || *s=='^' || *s=='~' || *s=='\`'))
	{
		if(n!=WNUMBER) return -1;
		if(*s=='~') return RAND0+atoi(s+1);
		if(*s=='\`') return RAND1+atoi(s+1);
		i=actualval(s+1,-70); if(i==-1) return -1;
		if((i&IWORD)==0) return -1;
		if(*s=='?') return OBVAL+i;
		if(*s=='%') return OBDAM+i;
		if(*s=='^') return OBWHT+i;
		if(*s=='*') return OBLOC+i;
		if(*s=='#') return PRANK+i;
		return -1;
	}
	if(!isalpha(*s)) return -2;
	for(i=0; i<NACTUALS; i++)
	{
		if(stricmp(s,actual[i].name)!=NULL) continue;
		/* If its not a slot label, and the wtypes match, we's okay! */
		if(!(actual[i].value&IWORD))
			return (actual[i].wtype==n || n==-70) ? actual[i].value:-1;

		/* Now we know its a slot label... check which: */
		switch(actual[i].value-IWORD)
		{
			case IVERB:		/* Verb */
				if(n==PVERB || n==PREAL) return actual[i].value;
				return -1;
			case IADJ1:		/* Adjective #1 */
				if(vbslot.wtype[0]==n) return actual[i].value;
				if(*(s+strlen(s)-1) != '1' && vbslot.wtype[3]==n) return IWORD+IADJ2;
				if(n==PREAL) return actual[i].value;
				return -1;
			case INOUN1:		/* noun 1 */
				if(vbslot.wtype[1]==n) return actual[i].value;
				if(*(s+strlen(s)-1) != '1' && vbslot.wtype[4]==n) return IWORD+INOUN2;
				if(n==PREAL) return actual[i].value;
				return -1;
			case IADJ2:
				return (vbslot.wtype[3]==n || n==-70) ? actual[i].value:-1;
			case INOUN2:
				return (vbslot.wtype[4]==n || n==-70) ? actual[i].value:-1;
			default:
				return -1;	/* Nah... Guru instead 8-) */
		}
	}
	return -2;		/* It was no actual! */
}
umsg_proc()
{	register char *s;

	err=umsgs=0;
	fopenw("-ram:umsg.tmp"); close_ofps();
	fopena(umsgifn); ofp1=afp; afp=NULL; fseek(ofp1,0,2L);
	fopena(umsgfn);  ofp2=afp; afp=NULL; fseek(ofp2,0,2L);
	fopena("-ram:umsg.tmp");
	if(nextc(0)==-1) { close_ofps(); return 0; }	/* None to process */
	blkget(&datal,&data,0L); s=data;

	do
	{
loop:		do s=sgetl(s,block); while(com(block)==-1 && *s!=0);
		if(*s==0) break;
		tidy(block); if(block[0]==0) goto loop;
		striplead("msgid=",block); getword(block);
		if(Word[0]==0) goto loop;

		if(Word[0]=='$')
		{
			printf("Invalid ID, '%s'. ",Word);
			printf("'$' is reserved for System messages!\n");
			err++; skipblock(); goto loop;
		}
		if(strlen(Word)>IDL)
		{
			printf("Invalid ID, '%s'. ",Word);
			printf("Length exceeds %d characters.\n",IDL);
			err++; skipblock(); goto loop;
		}
		umsgs++;	/* Now copy the text across */
		strcpy(umsg.id,Word); umsg.fpos=ftell(ofp2); fwrite(umsg.id,sizeof(umsg),1,afp);
		fwrite((char *)&umsg.fpos,4,1,ofp1);
		do
		{
			while(*s!=0 && com(s)==-1) s=skipline(s);
			if(*s==0 || *s==13) { *s=0; break; }
			if(*s==9) s++; if(*s==13) { block[0]=13; continue; }
			s=sgetl(s,block); if(block[0]==0) break;
			umsg.fpos=strlen(block);
			if(block[umsg.fpos-1]=='{') block[--umsg.fpos]=0;
			else strcat(block+(umsg.fpos++)-1,"\n");
			fwrite(block,1,umsg.fpos,ofp2);
		} while(*s!=0 && block[0]!=0);
		fputc(0,ofp2);
	} while(*s!=0);
	close_ofps(); FreeMem(data,datal);
	data=NULL; datal=NULL;
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
}

isumsg(char *s, FILE *fp)	/* Check FP for umsg id! */
{	register int i;

	if(*s=='$')
	{
		i=atoi(s+1);
		if(i<1 || i>NSMSGS)
		{
			printf("\x07\n!! Invalid System Message ID, '%s'!\n\n",s);
			quit();
		}
		return i-1;
	}
	if(umsgs==0) return -1;
	fseek(fp,0,0L);		/* Rewind file */
	for(i=0; i<umsgs; i++)
	{
		fread(umsg.id,sizeof(umsg),1,fp);
		if(stricmp(umsg.id,s)==NULL) return i+NSMSGS;
	}
	return -1;
}

ttumsgchk(register char *s)
{
	s=skiplead("msgid=",s); s=skiplead("msgtext=",s); s=skipspc(s);
	if(*s=='\"' || *s=='\'') return msgline(s+1);
	return chkumsg(s);
}

chkumsg(char *s)
{	int r; FILE *fp;

	if(*s!='$' && umsgs==0) return -1;

	if((fp=fopen("ram:umsg.tmp","rb+"))==NULL)
	{
		printf("Unable to re-access ram:umsg.tmp!\n"); quit();
	}
	r=isumsg(s,fp);
	fclose(fp);
	return r;
}

msgline(char *s)
{
	FILE *fp; long pos; char c;
	fp=afp; afp=NULL; fopena(umsgfn); fseek(afp,0,2L); pos=ftell(afp);

	fwrite(s,strlen(s)-1,1,afp);
	if((c=*(s+strlen(s)-1))!='{') { fputc(c,afp); fputc('\n',afp); }
	fputc(0,afp);
	fopena(umsgifn); fseek(afp,0,2L); fwrite((char *)&pos,sizeof(long),1,afp);
	fclose(afp); afp=fp;
	return NSMSGS+(umsgs++);
}
/*
     System Message processing routines for AMUL, (C) KingFisher Software
     --------------------------------------------------------------------

 Notes:

	System messages MUST be listed in order, and MUST all exist! These
      should be supplied with the package, so the user has a set of defaults.
      We could write all the default system messages into AMULCOM, but this
      would simply be a waste of space!

*/

smsg_proc()
{	register char	*s;	register long id; long pos;

	err=smsgs=0;

	if(nextc(0)==-1) return;	/* Nothing to process! */
	fopenw(umsgifn); fopenw(umsgfn);	/* Text and index */

	blkget(&datal,&data,0L); s=data;

	do
	{
loop:		s=sgetl(s,block);
		if(com(block)==-1) goto loop;
		tidy(block); if(block[0]==0) continue;
		striplead("msgid=",block); getword(block);
		if(Word[0]==0) break;

		if(Word[0]!='$')
		{
			printf("\x07\n\n!! Invalid SysMsg ID, '%s'. SysMsgs MUST begin with a '$'!\n",Word);
			quit();
		}
		if(atoi(Word+1)!=smsgs+1)
		{
			printf("\x07\n\n!! Message %s out of sequence!\n\n",Word);
			quit();
		}
		if(smsgs>=NSMSGS)
		{
			printf("\x07\n\n!! Too many System Messages, only require %ld!\n\n",NSMSGS);
			quit();
		}
		id=++smsgs;	/* Now copy the text across */
		pos=ftell(ofp2); fwrite((char *)&pos,4,1,ofp1);
		do
		{
			while(*s!=0 && com(s)==-1) s=skipline(s);
			if(*s==0 || *s==13) { *s=0; break; }
			if(*s==9) s++; if(*s==13) { block[0]=13; continue; }
			s=sgetl(s,block); if(block[0]==0) break;
			pos=strlen(block);
			if(block[pos-1]=='{') block[--pos]=0;
			else strcat(block+(pos++)-1,"\n");
			fwrite(block,1,pos,ofp2);
		} while(*s!=0 && block[0]!=0);
		fputc(0,ofp2);
	} while(*s!=0);
	close_ofps(); FreeMem(data,datal);
	data=NULL; datal=NULL;
}


/*
	Routines to process/handle Synonyms
						*/

syn_proc()
{	register char *s,*t; short int no; register short int x;

	err=syns=0;	if(nextc(0)==-1) return;
	fopenw(synsfn); fopenw(synsifn);

	blkget(&datal,&data,0L); s=data;

	do
	{
		do s=sgetl(s,block); while(com(block)==-1);

		tidy(block); if(block[0]==0) continue;
		t=getword(block); t=skipspc(t);

		if((no=isnoun(Word)) < 0)
		{
			if((x=is_verb(Word))==-1)
			{
				printf("\x07!! Invalid verb/noun, \"%s\"...\n",Word);
				err++; continue;
			}
			no=-(2+x);
		}

		while(*t!=0)
		{
			t=getword(t); if(Word[0]==0) break;
			fwrite((char *)&no,1,sizeof(short int),ofp2);
			fprintf(ofp1,"%s%c",Word,0); syns++;
		}
	} while(*s!=0);
	close_ofps(); FreeMem(data,datal);
	data=NULL; datal=NULL;
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
}
/* Mobiles.Txt Processor */

	/*=* Pass 1: Indexes mobile names *=*/

mob_proc1()
{	register char	*p,*s1,*s2; register long n;

	err=0; mobchars=0;
	fopenw(mobfn); if(nextc(0)==-1) return;

	blkget(&moblen,&mobdat,0L); p=mobdat; repspc(mobdat);

	do
	{
ldo:		while(*p!=0 && *p!='!') p=skipline(p); if(*p==0) break;
		p=sgetl(p,block);
		mobchars++; s1=getword(block+1);
		strcpy(mob.id,Word);
		do
		{
			s1=skipspc(s1); if(*s1==0 || *s1==';') break;
			if((s2=skiplead("dead=",s1))!=s1)
			{
				s1=getword(s2); mob.dead=atoi(Word); continue;
			}
			if((s2=skiplead("dmove=",s1))!=s1)
			{
				s1=getword(s2); mob.dmove=isroom(Word);
				if(mob.dmove==-1)
				{
					printf("## Mobile '%s': invalid DMove '%s'.\n",mob.id,Word);
					err++;
				}
				continue;
			}
		} while(*s1!=0 && *s1!=';' && Word[0]!=0);

		p=sgetl(p,block); tidy(block); s1=block; mob.dmove=-1;

		if((s2=skiplead("speed=",s1))==s1) { mobmis("speed="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.speed = atoi(Word);
		if((s2=skiplead("travel=",s1))==s1) { mobmis("travel="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.travel = atoi(Word);
		if((s2=skiplead("fight=",s1))==s1) { mobmis("speed="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.fight = atoi(Word);
		if((s2=skiplead("act=",s1))==s1) { mobmis("act="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.act = atoi(Word);
		if((s2=skiplead("wait=",s1))==s1) { mobmis("wait="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.wait = atoi(Word);
		if( mob.travel+mob.fight+mob.act+mob.wait != 100 )
		{
			printf("## Mobile '%s': Travel+Fight+Act+Wait <> 100%! Please check!\n",mob.id); err++;
		}
		if((s2=skiplead("fear=",s1))==s1) { mobmis("fear="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.fear = atoi(Word);
		if((s2=skiplead("attack=",s1))==s1) { mobmis("attack="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.attack = atoi(Word);
		if((s2=skiplead("hitpower=",s1))==s1) { mobmis("hitpower="); continue; }
		s1=getword(s2); s1=skipspc(s1); mob.hitpower = atoi(Word);

		px=p;
		if((n=getmobmsg("arrive="))==-1) continue; mob.arr=n;
		if((n=getmobmsg("depart="))==-1) continue; mob.dep=n;
		if((n=getmobmsg("flee="))==-1) continue; mob.flee=n;
		if((n=getmobmsg("strike="))==-1) continue; mob.hit=n;
		if((n=getmobmsg("miss="))==-1) continue; mob.miss=n;
		if((n=getmobmsg("dies="))==-1) continue; mob.death=n;
		p=px;

		fwrite(mob.id,sizeof(mob),1,ofp1);
	} while(*p!=0);

	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	close_ofps();
	if(mobchars!=0)
	{
		if((mobp=(struct _MOB_ENT *)AllocMem(sizeof(mob)*mobchars,MEMF_PUBLIC))==NULL)
		{
			printf("### FATALY OUT OF MEMORY!\n"); quit();
		}
		fopena(mobfn); fread((char *)mobp,sizeof(mob)*mobchars,1,afp); close_ofps();
	}
}

mobmis(register char *s)
{
	printf("## Mobile '%s': missing %s field.\n",mob.id,s); err++;
	skipblock();
}

badmobend()
{
	return -1;
}

/*=* Fetch mobile message line *=*/
getmobmsg(register char *s)
{	register char *q; register int n;

loop:	if(com(px)==-1) { px=skipline(px); goto loop; }
	if(*px==0 || *px==13 || *px==10) { err++; printf("## Mobile '%s': Unexpected end of mobile!\n"); return -1; }
	px=skipspc(px); if(*px==0 || *px==13 || *px==10) goto loop;

	if((q=skiplead(s,px))==px) { mobmis(s); err++; return -1; }
	if(toupper(*q)=='N') { px=skipline(px); return -2; }
	n=ttumsgchk(q); px=skipline(px); if(n==-1) { printf("## Mobile '%s': Bad text on '%s' line!\n",mob.id,s); err++; }
	return n;
}
	/*=* Pass 2: Indexes commands mobiles have access to *=*/

/*mob_proc2()
{*/

	/*---------------------------------------------------------*/

main(argc,argv)			/*=* Main Program *=*/
int argc;
char *argv[];
{
	sprintf(vername,"AMULCom v%d.%03d (%8s)",VERSION,REVISION,DATE);
	mytask=FindTask(0L); mytask->tc_Node.ln_Name = vername;

	puts("\x0C\n  [33mAMUL  Multi-User Games Language Copyright (C) KingFisher Software, 1991[0m\n");
	printf("                 [4mAMUL Compiler; %s[0m\n\n",vername);

	ofp1=NULL;	ofp2=NULL;	ofp3=NULL;	dir[0]=0;
	dchk=rmrd=1;	ohd=Output();

	/* Check we have correct no. of parameters */

	if(argc>6)
	{
		puts("!! Error !!\n\n Usage:\n   amulcom <game path>\n");
		exit(0);
	}
	if(argc>1) argue(argc,argv);

	/* Check the files/directories */

	tx("\n%% Checking existence of files\x0d");
	checkf("Title.TXT"); checkf("Rooms.TXT"); checkf("Ranks.TXT");
	checkf("Obdescs.TXT"); checkf("Objects.TXT"); checkf("Lang.TXT");
	checkf("Travel.TXT"); checkf("SysMsg.TXT"); checkf("UMsg.TXT");
	checkf("Reset.TXT"); checkf("Syns.TXT"); checkf("Mobiles.TXT");
	tx("## All .TXT files located.    \n\n\x1B[1mCompiling game files...\n\x1B[0m");

	tx("%% TITLE....:\x0d");
	opentxt("TITLE");	title_proc();	fclose(ifp);
	dmoves=0;
	if(rmrd==1)
	{
		tx("%% ROOMS....:\x0d");
		opentxt("ROOMS");	room_proc();	fclose(ifp);
	}
	fopenr(rooms1fn);	/*=* Check DMOVE ptrs *=*/
	if(rmrd==0)
	{
		fseek(ifp,0,2L); rooms=ftell(ifp)/sizeof(room); rewind(ifp);
	}
	if((rmtab=(struct room *) AllocMem(sizeof(room)*rooms,MEMF_PUBLIC))==NULL)
	{
		puts("No memory for ROOM ID table!\n"); quit();
	}
	if(fread((char *)rmtab,sizeof(room),rooms,ifp)!=rooms)
	{
		puts("Failed to get right number of rooms from files! Aborting!");
		quit();
	}
	if(dchk!=0 || dmoves!=0)
	{
		tx("%% DMOVEs...:\x0d"); checkdmoves(); fclose(ifp);
	}
	tx("%% RANKS....:\x0d");
	opentxt("RANKS");	rank_proc();	fclose(ifp);
	tx("%% SYSMSG...:\x0d");
	opentxt("SysMsg");	smsg_proc();	fclose(ifp);
	if(smsgs!=NSMSGS)
	{
		printf("\x07!! %ld System message(s) missing!\n\n",NSMSGS-smsgs);
		quit();
	}
	tx("%% UMSG.....:\x0d");
	opentxt("UMSG");	if(umsg_proc()==-1) quit(); else fclose(ifp);
	tx("%% MOBILES..:\x0d");
	opentxt("MOBILES");	mob_proc1();	fclose(ifp);
	tx("%% OBDESCS..:\x0d");
	opentxt("OBDESCS");	obds_proc();	fclose(ifp);
	tx("%% OBJECTS..:\x0d");
	opentxt("OBJECTS");	objs_proc();	fclose(ifp);
	tx("%% LANG.....:\x0d");
	opentxt("LANG");	lang_proc();	fclose(ifp); proc=0;
	tx("%% TRAVEL...:\x0d");
	opentxt("TRAVEL");	trav_proc();	fclose(ifp);
	tx("%% SYNS.....:\x0d");
	opentxt("SYNS");	syn_proc();	fclose(ifp);

	tx("Execution finished normally\n\n");
	printf("Statistics for %s:\n\n",adname);
	printf("		Rooms: %6d	Ranks: %6d	Nouns: %6d\n",rooms,ranks,nouns);
	printf("		Adj's: %6d	Verbs: %6d	Syns : %6d\n",adjs,verbs,syns);
	printf("		T.T's: %6d	Umsgs: %6d	SMsgs: %6d\n",ttents,umsgs,NSMSGS);
	printf("		 [33mTotal items processed:[0;1m%7d\n\n[0m",rooms+ranks+adjs+verbs+nouns+syns+ttents+umsgs+NSMSGS+mobs+mobchars);
	fopenw(advfn); time(&clock);
	fprintf(ofp1,"%s\n%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",adname,rooms,ranks,verbs,syns,nouns,adjs,ttents,umsgs,clock,mins,invis,invis2,minsgo,mobs,rscale,tscale,mobchars);
	opentxt("TITLE"); fseek(ifp,readgot,0);
	do
	{
		block[0]=0; readgot=fread(block,1,1020,ifp); fwrite(block,readgot,1,ofp1);
	}while(readgot==1020);
	exi=1;
	quit();
}

	/*---------------------------------------------------------*/

isrflag(s)			/* Check to see if s is a room flag */
char *s;
{
	int	_x;
	for(_x=0;_x<NRFLAGS;_x++)
		if(strcmp(s,rflag[_x])==0) return _x;
	return -1;
}

isroom(char *s)
{
	int r;
	roomtab=rmtab;
	for(r=0;r<rooms;r++)
		if(strcmp(roomtab->id,s)==0) return r;
		else roomtab++;
	return -1;
}

isoflag1(char *s)		/* Is it a FIXED object flag? */
{
	int i;
	for(i=0;i<NOFLAGS;i++)
		if(strcmp(obflags1[i],s)==NULL) return i;
	return -1;
}

isoparm()			/* Is it an object parameter? */
{
	int i;
	for(i=0;i<NOPARMS;i++)
		if(striplead(obparms[i],Word)) return i;
	return -1;
}

isoflag2(char *s)		/* Is it a state flag? */
{
	int i;
	for(i=0;i<NSFLAGS;i++)
		if(strcmp(obflags2[i],s)==NULL) return i;
	return -1;
}

set_adj()
{	register int i;

	if(strlen(Word)>IDL || strlen(Word)<3)
	{
		printf("\nInvalid adjective '%s'!\n\n\x07",Word);
		quit();
	}
	if(adjs==0)
	{
		for(i=0;i<IDL+1;i++) dmove[i]=0; strcpy(dmove,Word);
		obj2.adj=0; fwrite(dmove,IDL+1,1,afp); adjs++; return;
	}
	fseek(afp,0L,0);	/* Move to beginning */
	i=0;
	do
	{
		if(fread(dmove,IDL+1,1,afp)!=1) continue;/* Read adj! */
		if(strcmp(Word,dmove)==NULL) { obj2.adj=i; return; }
		i++;
	} while(!feof(afp));
	for(i=0;i<IDL+1;i++) dmove[i]=0; strcpy(dmove,Word);
	fseek(afp,0L,2);	/* Move to end! */
	fwrite(dmove,IDL+1,1,afp);	/* Add this adjective */
	obj2.adj=adjs++;
}

object(char *s)
{
	printf("\nObject #%d \"%s\" has invalid %s, '%s'!\n",nouns+1,obj2.id,s,Word);
	quit();
}

set_start()
{
	if(!isdigit(Word[0])) object("start state");
	obj2.state=atoi(Word);
	if(obj2.state<0 || obj2.state>100) object("start state");
}

set_holds()
{
	if(!isdigit(Word[0])) object("holds= value");
	obj2.contains=atoi(Word);
	if(obj2.contains<0 || obj2.contains>1000000) object("holds= state");
}

set_put()
{	register int i;

	for(i=0;i<NPUTS;i++)
		if(stricmp(obputs[i],Word)==NULL) { obj2.putto=i; return; }
	object("put= flag");
}

set_mob()
{	register int i;
	for(i=0; i<mobchars; i++) if(stricmp(Word,(mobp+i)->id)==NULL) { obj2.mobile=i; return; }
	object("mobile= flag");
}

argue(argc,argv)
int argc; char *argv[];
{
	int n;
	for(n=2;n<=argc;n++)
	{
		if(strcmp("-d",argv[n-1])==0){dchk=0; continue;}
		if(strcmp("-q",argv[n-1])==0){warn=0; continue;}
		if(strcmp("-r",argv[n-1])==0){rmrd=0; continue;}
		strcpy(dir,argv[n-1]);
		if((c=dir[strlen(dir)-1])!='/' && c!=':') strcat(dir,"/");
	}
}

checkf(char *s)
{
	sprintf(block,"%s%s",dir,s);
	if((ifp=fopen(block,"rb"))==NULL)
	{
		printf("Missing: file \x1B[33;1m%s!!!\x1B[0m\n\n",block);
		exit(202);
	}
	fclose(ifp);ifp=NULL;
}

iscond(register char *s)
{	int i;
	for(i=0; i<NCONDS; i++)
		if(strcmp(conds[i],s)==0) return i;
	return -1;
}

isact(register char *s)
{	int i;
	for(i=0; i<NACTS; i++)
		if(strcmp(acts[i],s)==0) return i;
	return -1;
}

isprep(register char *s)
{	int i;
	for(i=0; i<NPREP; i++)
		if(strcmp(s,prep[i])==0) return i;
	return -1;
}
