	IFND	LIBRARIES_AGL_LIB_I
LIBRARIES_AGL_LIB_I	SET	1
**
**	$Filename: libraries/AGL_Lib.I $
**
**	Library interface offsets for agl.library
**
**	(C) Copyright KingFisher Software D&EG International
**	               All Rights Reserved
**

CALLAGL	MACRO		; Call a "Library Vectory Offset" ?
	move.l	_AGLBase,a6
	jsr	_LVO\1(a6)
	ENDM

AGLNAME	MACRO		; Library Name
	dc.b	"agl.library",0
	ENDM

	IFND	EXEC_LIBRARIES_I
		include	"exec/libraries.i"
	ENDC	; EXEC_LIBRARIES_I

LVODEF	MACRO		; Define _LVO entry
	LIBDEF	_LVO\1
	ENDM


**
**	LVO definitions
**

	LIBINIT

	LVODEF	LibTable		; (-none-)
	LVODEF	TextPtr		; (-none-)
	LVODEF	CommsPrep		; (a0=&port,a1=&reply,a2=&msg)
	LVODEF	CreateAPort	; (a0=Name)
	LVODEF	DeleteAPort	; (a0=Port,a1=Msg)
	LVODEF	Random		; (d0=Max)
	LVODEF	GetLocal		; (a0=&Screen,a1=&Window,a2=Title)

**
**	Library pointer offsets from LibTable return
**
**	Note: These are LONGWORD counts, not BYTE counts.
*	      Use INTBoff*4 for true byte counts.

#define	DOSBoff	0	; DOSBase=*LibTable();
#define	INTBoff	1	; IntuitionBase=*LibTable()+INTBoff*4;
#define	GFXBoff	2	; GfxBase=*LibTable()+GFXBoff*4;

**
**	Error Message Index (for use with TextPtr)
**

NoAMAN	equ	0		* Manager not running
NoPORT	equ	1		* Couldn't create comms port
NoMEM	equ	2		* Out of memory
NoSCRN	equ	3		* Can't open screen
NoWIN	equ	4		* Can't open window

	ENDC	; LIBRARIES_AGL_LIB_I
