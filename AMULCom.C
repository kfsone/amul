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

#include "h/AMULCom.H"
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

#include "com/filebits.c"

#include "COM/Room_Proc.C"
#include "COM/CheckDMoves.C"
#include "COM/Rank_Proc.C"
#include "COM/ObDs_Proc.C"
#include "COM/Obj_Proc.C"
#include "COM/Title_Proc.C"
#include "COM/Trav_Proc.C"
#include "COM/Lang_Proc.C"
#include "COM/UMsg_Proc.C"
#include "COM/SMsg_Proc.C"
#include "COM/Syns_Proc.C"
#include "COM/Mob_Proc.C"

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
