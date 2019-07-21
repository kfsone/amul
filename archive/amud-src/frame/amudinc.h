//
// AMUD/frame/AMUDInc.H		Includes for AMUD frame modules
//

#ifndef FRAME		// If #defines aren't set
#define	FRAME	1
#define	PORTS	1
#endif

#ifndef	DEF		// DEF is only set for the "variables included" module
#define	DEF	extern
#endif

#include <ADV:H/AMUD.Defs.H>		// Various includes
#include <ADV:H/AMUD.Incs.H>
#include <ADV:H/AMUD.Msgs.H>
#include <ADV:H/AMUD.Vars.H>

#define	dtx(x)	if(debug) tx(x)
DEF short int	debug;			// Is debug mode on?

DEF struct Screen *sC;			// AMIGADOS
DEF struct Window *wG;			// AMIGADOS
DEF struct IOExtSer *serio,*wserio,*CreateExtIO();	// AMIGADOS
DEF struct Task   *mytask;		// AMIGADOS
DEF struct MsgPort *ReadRep,*WriteRep,*amanrep;	// AMIGADOS (communications)
DEF struct IOStdReq ReadIo, WriteIo;	// AMIGADOS
DEF struct Aport *ap,*amanp,*intam;	// AMIGADOS
DEF struct Library *AMUDBase;		// AMIGADOS
DEF struct _VERB_STRUCT verb;

DEF char	MyFlag,*input,str[800],spc[200],mxx[60],mxy[60],wtil[80];
DEF char	iosup;
DEF char	actor,last_him,last_her,autoexits,needcr;
DEF char	*ob,*gp,*ow,*lastres,*lastcrt;
DEF char	link,inc,forced,addcr,fol,exeunt,more,failed,died;
DEF char	schar;
DEF long	iverb,overb,iadj1,inoun1,iadj2,inoun2,lverb,ldir,lroom,wtype[4],word;
DEF long	ml,donet,it;
DEF long	mins;
DEF short int	donev,skip,rset,rclr,ip,csyn;
DEF unsigned short int *rescnt;
DEF FILE	*fp;

DEF	char	**errtxt;	// amud.library text pointer

DEF char	arr[81],dep[81];

#ifdef	AMUD1
char	needkill=FALSE;
#include <ADV:Frame/ScrnIO.C>
#include <ADV:Frame/IOBits.C>
#include <ADV:Frame/Proc.C>	// Parser processing routines
#include <ADV:Frame/Cond.C>	// Parser & VT processor
#else
extern char plyrfn[],rooms2fn[];
#endif
