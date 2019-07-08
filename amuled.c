/*
	AMUL User Editor by Oliver Smith
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
						*/

#include "stdio.h"
#include "ctype.h"
#include "fcntl.h"
#include "proto/exec.h"
#include "proto/dos.h"
#include "proto/intuition.h"
#include "proto/graphics.h"
#include "exec/types.h"
#include "exec/io.h"
#include "exec/memory.h"
#include "libraries/dos.h"
#include "intuition/intuition.h"
#include "itools1l.h"
#include "inovatools1.h"
#include "adv:h/amul.defs.h"
#include "adv:h/amul.stct.h"

struct Library *ITBase;

#define	NAMEL		20	/* Length of names	*/

struct NewWindow *mywin,*myreq;	/* Close NewWindow so we can be resident */
struct Window *OpenWindow(),*win;
struct Screen *OpenScreen(),*sc;
struct Task *MyTask,*FindTask();
struct ViewPort vp;
struct RastPort *rp;
struct IntuiMessage *imsg;
struct Menu *mymen;
struct Gadget *gadg;
UWORD  code;
ULONG  class;
struct _PLAYER me,blank;

USHORT	quit_flag,uno,dirn;
FILE	*ifp;
char	dir[64],*advfn="Prof.CMP",*plyrfn="Players Data",block[1024],adname[42];

#define	NGADS 5

/* Gadget numbers */
#define	gUSERNO		99
#define	gNAME		20
#define	gPASSWORD	gNAME+1
#define	gPLAYS		gPASSWORD+1
#define	gSCORE		gPLAYS+1
#define gRANK		gSCORE+1

USHORT gadnum[] =	/* Tells us what order to go around the gadgets */
{
	gNAME,gPASSWORD,gPLAYS,gSCORE,gRANK,gNAME
};

/* Menu Gadget Numbers */
#define	mABOUT		0
#define	mFIND		2
#define	mSAVE		3
#define	mDELETE		4
#define	mFIRST		6
#define	mPREV		7
#define	mNEXT		8
#define	mLAST		9
#define	mQUIT		11

#include "adv:AMULEd.H"

main(int argc, char *argv[])
{	register int i; register char *p;

	MyTask=FindTask(0); MyTask->tc_Node.ln_Name = "AMUL-Ed v1.0";

	if(argc != 2) error("%s Usage error, try: %s <game path>!\n",argv[0]);
	p=argv[1]; strcpy(dir,p);
	if(*(p+strlen(p)-1)!='/' && *(p+strlen(p)-1)!=':') strcat(dir,"/");

	fopenr(advfn); fgets(adname,41,ifp); adname[strlen(adname)-1]=0; closei();

	if(( ITBase = OpenLibrary("inovatools1.library",0)) == NULL)
		error("%s Can't open %s.library\n","InovaTools1");
	if(( GfxBase = OpenLibrary("graphics.library",0)) == NULL)
		error("%s Can't open %s.library\n","Graphics");
	if(( IntuitionBase = OpenLibrary("intuition.library",0)) == NULL)
		error("%s Can't open %s.library\n","Intuition");

	if((sc=OpenScreen( &NewScreenStructure))==NULL) error("%s Can't open %s\n","screen");
	vp = sc->ViewPort; LoadRGB4(&vp,(UWORD *)&Palette,PaletteColorCount);
	if((mywin = DupeNewWindow(&AENewWindowStructure1))==NULL) error("%s Can't open %s\n","window");
	if((myreq = DupeNewWindow(&RENewWindowStructure2))==NULL) error("%s Can't open %s\n","window");
	if((mymen = DupeMenu(&AEMenuList1))==NULL) error("%s Can't open %s\n","window");
	mywin->Screen = sc; myreq->Screen = sc;
	if((win = OpenWindow(mywin)) == NULL) error("%s Can't open %s\n","window");

	/* Set the 'next gadget' pointers */
	gadg=GetGadget(win,gadnum[0]);
	for(i=0; i<NGADS; i++)
	{
		gadg->UserData = (APTR)GetGadget(win,gadnum[i+1]);
		gadg = (struct Gadget *)gadg->UserData;
	}

	moFirst();			/* Read first user */

	SetMenuStrip(win,mymen);	/* attach any Menu */

	do
	{
		imsg = GetIntuiMessage( win );
		code = imsg->Code;				/* MENUNUM */
		gadg = (struct Gadget *) imsg->IAddress;	/* Gadget */
		class= imsg->Class;
		ReplyMsg(imsg);
		if( class == CLOSEWINDOW ) { quit_flag = TRUE; continue; }
		if( class == GADGETUP || class == GADGETDOWN ) { gadg_proc(); continue; }
		if( class == MENUPICK )
		{
			menu_proc(); continue;
		}
		fprintf(stderr,"AMULEd: ** Unknown Intuition message class.\n");
	} while(quit_flag == FALSE);

	ClearMenuStrip(win);
	quit();
}

fopenr(char *s)
{
	if(ifp!=NULL) fclose(ifp);
	sprintf(block,"%s%s",dir,s);
	if((ifp=fopen(block,"rb+"))==NULL)
		error("\x07%s Cannot open file %s for reading!\n",block);
}

quit()
{
	if(win)	CloseWindow(win); win=NULL;
	if(sc)	CloseScreen(sc); sc=NULL;
	if(mymen) DeleteMenu(mymen);
	if(myreq) DeleteNewWindow(myreq); myreq=NULL;
	if(mywin) DeleteNewWindow(mywin); mywin=NULL;
	if(ITBase) CloseLibrary(ITBase); ITBase=NULL;
	if(GfxBase) CloseLibrary(GfxBase); GfxBase=NULL;
	if(IntuitionBase) CloseLibrary(IntuitionBase); IntuitionBase=NULL;
	if(ifp) closei();
	exit(0);
}

/* -------------------------------------------------------------------------- */
/* ------------------------- Gadget Functions ------------------------------- */
/* -------------------------------------------------------------------------- */

gadg_proc()
{
	switch(gadg->GadgetID)
	{
		case gUSERNO:	SetNo(); Update(); ActivateGadget( GetGadget(win, gNAME), win, NULL ); break;
		case gNAME:	gfString(me.name);     break;
		case gPASSWORD:	gfString(me.passwd);   break;
		case gPLAYS:	me.plays = gfNumber(); break;
		case gSCORE:	me.score = gfNumber(); break;
		case gRANK:	me.rank = gfNumber();  break;
	}
	if( gadg->UserData != NULL ) ActivateGadget( (struct Gadget *)gadg->UserData, win, NULL );
	else ActivateGadget( GetGadget(win, gNAME), win, NULL );
}

menu_proc()
{
	switch(ITEMNUM(code))
	{
		case mQUIT:	moQuit();  break;
		case mFIRST:	moFirst(); break;
		case mPREV:	moPrev();  break;
		case mNEXT:	moNext();  break;
		case mLAST:	moLast();  break;
		case mSAVE:	moSave();  break;
		case mDELETE:	moDelete();break;
	}
}

gfString(char *s)	/* Gadget Function: Process a String Info input */
{	struct StringInfo *siptr;
	siptr = (struct StringInfo *) gadg->SpecialInfo;
	strcpy(s,siptr->Buffer);
}

gfNumber()	/* Gadget Function: Process a NUMERIC String Info input */
{
	return GetStringInfoNumber(win, gadg->GadgetID, 10);
}

moQuit()
{
	if(request("== Exit AMULEd? ==")!=FALSE) quit_flag = TRUE;
}

moFirst()		/* Menu Option: FIRST */
{
	uno = 1; dirn = 1; User();
}

moNext()
{
	dirn = 1; uno++; User();
}

moPrev()
{
	dirn = -1; if(uno==1) return; else uno--; User();
}

moLast()
{
	dirn = -1;
	fopenr(plyrfn); fseek(ifp,0,2); uno=ftell(ifp)/sizeof(me);
	User();
}

moSave()
{
	if(request("=== Save User? ===")==FALSE) return;
	fopenr(plyrfn);
	fseek(ifp,(uno-1)*sizeof(me),0); fwrite(me.name,sizeof(me),1,ifp);
	closei();
}

moDelete()
{
	if(request("== Delete User? ==")==FALSE) return;
	fopenr(plyrfn);
	fseek(ifp,(uno-1)*sizeof(me),0); fwrite(blank.name,sizeof(me),1,ifp);
	closei(); userpack(); User();
}

userpack()
{	char file[80]; FILE *ofp;

	PrintIText(win->RPort,&AEIntuiTextList1,0,0);

	fopenr(plyrfn);

	sprintf(block,"%s%s.new",dir,plyrfn); unlink(block); /* Incase */
	if((ofp=fopen(block,"ab+"))==NULL) { DisplayBeep(NULL); return; }
	/* ifp must be open already */
	rewind(ifp);
	do
	{
		fread(me.name,sizeof(me),1,ifp); if(me.name[0]==0 || feof(ifp)) continue;
		fwrite(me.name,sizeof(me),1,ofp);
	} while(!feof(ifp));
	fclose(ofp); closei();
	sprintf(block,"%s%s",dir,plyrfn);
	strcpy(file,block); strcat(file,".bak"); unlink(file); Rename(block,file);
	strcpy(file,block); strcat(file,".new"); Rename(file,block);
	fopenr(plyrfn);
	fseek(ifp,0,2); if(uno > ftell(ifp)/sizeof(me)) uno--; rewind(ifp);
	closei();

	PrintIText(win->RPort,&AEIntuiTextList1,0,0);
}

request(register char *title)
{	struct Window *req; struct Gadget *gad;

	ClearIntuiMessages(win); myreq->Title = title;
	if((req=FlashyOpenWindow(myreq))==NULL) { DisplayBeep(sc); return FALSE; }
	do { imsg = GetIntuiMessage(req); } while(imsg->Class != GADGETUP && imsg->Class != GADGETDOWN);
	gad=(struct Gadget *)imsg->IAddress;
	ReplyMsg(imsg); ClearIntuiMessages(req); FlashyCloseWindow(req);
	return (gad->GadgetID == 100) ? TRUE : FALSE;
}

SetNo()
{
	SetStringInfoNumber( win, gUSERNO,   ftell(ifp)/sizeof(me), 10 );
}

Update()
{
	RefreshGadgets( GetGadget(win, gNAME), win, NULL );
}

User()			/* Set the current user according to "me" */
{	register int i;

	fopenr(plyrfn);
loop:	fseek(ifp,(uno-1)*sizeof(me),0); if(feof(ifp) || fread(me.name,sizeof(me),1,ifp)!=1 && uno!=1) { dirn=-1; uno--; goto loop; }
	for(i=strlen(me.name); i<NAMEL+1; i++) me.name[i]=0;
	for(i=strlen(me.passwd); i<23; i++) me.passwd[i]=0;
	if(me.name[0]==0) { uno+=dirn; goto loop; }
	setstringinfo( gNAME, me.name );
	setstringinfo( gPASSWORD, me.passwd );
	SetStringInfoNumber( win, gUSERNO,   (uno=ftell(ifp)/sizeof(me)), 10 );
	SetStringInfoNumber( win, gPLAYS,    me.plays, 10 );
	SetStringInfoNumber( win, gSCORE,    me.score, 10 );
	SetStringInfoNumber( win, gRANK,     me.rank,  10 );
	Update(); closei();
}

setstringinfo(register int no,register char *s)
{	struct StringInfo *siptr;
	siptr = (struct StringInfo *)GetGadget(win,no)->SpecialInfo;
	strcpy(siptr->Buffer,s);
}

error(char *s,char *s1,char *s2,char *s3,char *s4,char *s5)
{
	fprintf(stderr,s,"AMUL-Ed error: ",s1,s2,s3,s4,s5); quit();
}

closei()
{
	fclose(ifp); ifp=NULL;
}
