//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AGLCom.H		AGL Compiler Main Include
//
//	LMod: oliver 11/06/93	Created
//

#define	COMPILER 1		// Defines who I am
#define	DEF			// All variables are INTERNAL

#include "adv:h/defs.h"		// Includes
#include "adv:h/incs.h"
#include "adv:h/vars.h"
#include "adv:h/data.h"

// Compiler specific variables...

short	rmn;			// Current room no.
long	FPos;			// Used during TT/Lang writes
char	Word[64];		// For internal use only <grin>
char	temp[180];		// temporary string
short	err;			// Error count
short	proc;			// What we are processing
long	clock;			// Bits for time etc
long	ohd;			// Output handle for direct dos
char	*data,*data2;		// Pointer to data buffer
char	*syntab;		// Synonym table, re-read
long	datal,datal2,mins;	// Length of data & gametime
long	obmem,vbmem;		// Size of Objects.TXT and Lang.Txt
long	wizstr;			// Wizards strength
char	*mobdat,*px;		// Mobile data
long	moblen;			// Length
short	scrx,scry,scrw,scrh;	// Screen settings (def=-1)
char	scrm;			// Screen mode (N=Norm, I=Intlc

char	warn,needcr,exi,rmrd,c,dchk,dmoves;

struct	Task *mytask,*FindTask();	// AmigaDOS

struct UMSG { char id[IDL+1]; long fpos; }umsg;

char	*getword(char *),*skipspc(char *),*sgetl(char *,char *);
char	*skiplead(char *,char *),*skipline(char *);
extern char	*SysDefTxt;		// String from extras.o
FILE	*ofp5,*msg1,*msg2;

struct _OBJ_STRUCT2 *obtab2,*objtab2,obj2,*osrch,*osrch2;

#include "adv:com/FileBits.C"		// Compiler sub-includes
#include "adv:com/Room_Proc.C"
#include "adv:com/CheckDMoves.C"
#include "adv:com/Rank_Proc.C"
#include "adv:com/ObDs_Proc.C"
#include "adv:com/Obj_Proc.C"
#include "adv:com/Title_Proc.C"
#include "adv:com/Trav_Proc.C"
#include "adv:com/Lang_Proc.C"
#include "adv:com/UMsg_Proc.C"
#include "adv:com/SMsg_Proc.C"
#include "adv:com/Syns_Proc.C"
#include "adv:com/Mob_Proc.C"

// Text File Definitions

#define	TF_SYSTEM	0		// Numbers of the text files
#define	TF_ROOMS	TF_SYSTEM+1
#define	TF_RANKS	TF_ROOMS+1
#define	TF_SYSMSG	TF_RANKS+1
#define	TF_UMSG		TF_SYSMSG+1
#define	TF_MOBILES	TF_UMSG+1
#define	TF_OBDESCS	TF_MOBILES+1
#define	TF_OBJECTS	TF_OBDESCS+1
#define	TF_LANG		TF_OBJECTS+1
#define	TF_TRAVEL	TF_LANG+1
#define	TF_SYNS		TF_TRAVEL+1
#define	TXTFILES	TF_SYNS+1	// No. of text files

char *txtfile[TXTFILES] = {		// Text file names
	"SYSTEM", "ROOMS", "RANKS", "SYSMSG", "UMSG", "MOBILES",
	"OBDESCS", "OBJECTS", "LANG", "TRAVEL", "SYNS"
};

