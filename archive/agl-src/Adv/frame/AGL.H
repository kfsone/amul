//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/AGL.H	Main AGL Include
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

#ifndef FRAME
#define	FRAME	1
#define	PORTS	1
#endif

#ifndef	DEF
#define	DEF	extern
#endif

#include <ADV:H/Defs.H>
#include <ADV:H/Incs.H>
#include <ADV:H/Msgs.H>
#include <ADV:H/Vars.H>

#define	dtx(x)	if(debug) tx(x)
DEF short int	debug;			// Is debug mode on?

DEF struct Screen *sC;
DEF struct Window *wG;
DEF struct IOExtSer *serio,*wserio,*CreateExtIO();
DEF struct Task   *mytask;
DEF struct MsgPort *ReadRep,*WriteRep,*amanrep;
DEF struct IOStdReq ReadIo, WriteIo;
DEF struct Aport *ap,*amanp,*intam;
DEF struct Library *AGLBase;
DEF struct _VERB_STRUCT verb;

// Bitfield flags
DEF short	autoexits	: 1;
DEF short	link		: 1;
DEF short	forced		: 1;
DEF short	addcr		: 1;
DEF short	exeunt		: 1;
DEF short	failed		: 1;
DEF short	inc		: 1;
DEF short	fol		: 1;
DEF short	txed		: 1;
DEF short	more		: 3;
DEF short	died		: 4;

DEF char	MyFlag,*input,str[800],spc[200],mxx[60],mxy[60],wtil[80];
DEF char	iosup,schar;
DEF char	actor,last_him,last_her;
DEF char	*ob,*gp,*ow,*lastres,*lastcrt;
DEF long	iverb,overb,iadj1,inoun1,iadj2,inoun2,lverb,ldir,lroom,wtype[4],word;
DEF long	ml,donet,it,mins;
DEF short int	donev,skip,rset,rclr,ip,csyn;
DEF unsigned short int *rescnt;
DEF FILE	*fp;

DEF	char	**errtxt;	// agl.library text pointer

DEF char	arr[81],dep[81];

#ifdef	AGL1
char	needkill=FALSE;
#include <ADV:Frame/ScrnIO.C>
#include <ADV:Frame/IOBits.C>
#include <ADV:Frame/Proc.C>	// Parser processing routines
#include <ADV:Frame/Cond.C>	// Parser & VT processor
#else
extern char plyrfn[],rooms2fn[];
#endif
