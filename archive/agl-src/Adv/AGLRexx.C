/***** AGLRexx.C ***********************************************************
 *
 * NAME
 *	AGLRexx.C -- Forms an AGL scripting system
 *
 * SYNOPSIS
 *	AGLRexx <line> <script> <myroom> <noun1> <noun2> <helper>
 *	to invoke from AGL in Lang.Txt:
 *	script <name> <n1> <n2>
 *
 * FUNCTION
 *	Used by AGL to enable ARexx control of user. See below for commands.
 *
 * INPUTS
 *	line	Is the line number of the user to control
 *	script	Is the name of the script to run
 *	myroom	Current ID of my room
 *	noun1	Taken from 'n1' in the SCRIPT action
 *	noun2	Taken from 'n2' in the SCRIPT action
 *	helper	Name of the player helping you
 *
 * COMMANDS
 *	Commands available are: (uppercase = text, lowercase = type)
 *
 *	DONE			Script execution finished, resume processing
 *	KILLME			Kills you
 *	QUIT [ASK]		Quit; Ask = Ask user first.
 *	SAVE			Save details
 *	LOG text		Write to log file
 *	RESET [time]		Invoke a reset
 *	EXTEND [time]		Extend the game; default is 300 seconds
 *	SWAP path		Determine next session
 *	INPUT [length]		Returns a text string; default len=80 chars
 *	PRINT [text]		Prints text with traling CR (use { to omit)
 *	SHOWFILE filename	Displays a text file
 *	INFORM group what text	group = Announce Group, e.g. HERE/CANSEE
 *				what = ACTION | NOISE
 *				text = string to be sent
 *	INTERACT player		Same as the action
 *	DO verb			Same as the action
 *	CARRYING noun [state]	Same as carrying/gota conditions
 *
 ****************************************************************************/


/* ====== Main Includes ==================================================== */

#include "exec/types.h"				/* Size of variables used by EXEC */
#include "exec/io.h"				/* Message Port structures etc    */
#include "exec/memory.h"			/* AllocMem flags etc		  */
#include "stdio.h"				/* Standard 'C' Include		  */
#include "ctype.h"				/* Character Types		  */


/* ====== ARexx Stuff ====================================================== */

#include "rexx/storage.h"			/* ARexx includes		*/
#include "rexx/rxslib.h"

struct RexxLib *RexxSysBase;			/* ARexx variables		*/
struct MsgPort MyRexxPort;
struct RexxMsg *rmsg;

extern LONG  CheckRexxMsg();			/* ARexx functions		*/
extern LONG  GetRexxMsg();
extern LONG  SetRexxMsg();

/* ====== The Code Commences =============================================== */

main(int argc,char *argv[])
{	register int i;

	/* Go for ARexx, if its around */
	RexxSysBase = (struct RexxLib *)OpenLibrary("rexxsyslib.library",0L);
	^^ move this to AGL.library

	SetupARexx();	/* Setup the ARexx comms */
	SetupAPorts();	/* Setup the AGL comms */

	for (;;) {		/* Infinite loop */
		Wait(-1L);
point1:		if( RexxSysBase!=NULL && (rmptr = (struct RexxMsg *) GetMsg( &MyRexxPort )) != NULL) { if(DoRexx()==1) return; goto point1; }
		// Check for Reset/Died signal from AGL itself //
	}

	Quit();		/* Tidy close-down procedure */
}

SetupARexx() {
	sprintf(portname,"AGL%s",line);
	InitPort(&MyRexxPort,portname);
	AddPort(&MyRexxPort);
}

CloseARexx() {
	if(MyRexxPort) { RemPort(&MyRexxPort); FreePort(&MyRexxPort); }
}


argueit()		/* Break down arguments */
{	register char *p; register int i;

	for(i=15;i;i--) rmsg->rm_Args[i]=""; p=rmsg->rm_Args[0];
	while(*p!=0)
	{
		if(strncmp(p,"TO ",3) == NULL) { p+=3; continue; }
		rmsg->rm_Args[i++]=p; while(*p!=0 && *p!=' ') p++;
		if(*p==' ') { *(p++)=0; while(*p==' ') p++; }
	}
}

rexx_performer()
{	register int i,cmdno;

        debug("received command '%s'\n",rmsg->rm_Args[0]);

	/* Preprate for the return */
	rmsg->rm_Result1 = rmsg->rm_Result2 = 0L;

	if( !CheckRexxMsg(rmsg) ) { debug("invalid REXX context\n"); ReplyMsg(rmsg); return 0; }
	argueit();
	if(stricmp(rmsg->rm_Args[0],"CLOSE")==NULL) { ReplyMsg(rmptr); CloseARexx(); leave(); }
	if(stricmp(rmsg->rm_Args[0],"OPEN")==NULL)  { ReplyMsg(rmptr); return 1; }

	/* Process our command */
	cmdno = -1;
	for( i = 0; i < NCMDS; i++ )
		if(stricmp(rmsg->rm_Args[0],arexxcmd[i])==NULL) { cmdno = i; break; }

	switch(cmdno)
	{
		case 0:		ReIndex(rmsg->rm_Args[1],rmptr->rm_Args[2]); break;
		case 1:		Mask(); break;
		case 2:		ListSecs(); break;
		case 3:		Catalog(0); break;
		case 4:		Catalog(1); break;
		case 5:		Shrink(); ReIndex("A","X"); break;
		default:	rmsg->rm_Result1 = 5L; rmsg->rm_Result2 = 15L;
	}

end:	ReplyMsg(rmsg); return 0;
}

