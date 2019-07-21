; -------------------------------------------------------------------------- ;
; |-|  agl.library: Main Source. Copyright (C) 1993, KingFisher Software |-| ;
; -------------------------------------------------------------------------- ;

; Most of this source code developed using Libtool!

NULL		equ	0
ownFuncs		equ	6
numOfFuncs	equ	(4+ownFuncs)	; 4=base functions
negsize		equ	numOfFuncs*6

	opt	l+,c+,d+
	output	"agllib.o"

	xdef	_SysBase
	xref	_CreateTask

	incdir	"arg:"
	include	"exec/types.i"		;-----------------------
	include	"exec/funcdef.i"		; Exec library includes
	include	"exec/libraries.i"
	include	"exec/lists.i"
	include	"exec/memory.i"
	include	"exec/initializers.i"
	include	"exec/resident.i"
	include	"exec/exec_lib.i"
	include	"intuition/intuition_lib.i"
	include	"intuition/intuition.i"
	include	"graphics/graphics_lib.i"
	include	"agllib:rev/agl.library.i"
	include	"adv:h/AGL.Defs.I"
	include	"adv:h/AGL.Stct.I"
	include	"adv:h/AGL.Cons.I"
	incdir	"agllib:"

	SECTION	lib,code

CALL	MACRO
	jsr	_LVO\1(a6)
	endm

Return0:
	moveq	#0,d0
	rts

RomTag:
	dc.w	$4AFC
	dc.l	RomTag,endRom
	dc.b	0,VERSION,9,0
	dc.l	LibName,LibIDString,libstartup
endRom

libstartup:
	move.l	a3,-(sp)
	lea	SegList,a3
	move.l	a0,(a3)+
	move.l	a6,(a3)+
	lea	DOSName,a1
	moveq	#0,d0
	CALL	OpenLibrary
	move.l	d0,(a3)+
	lea	IntuiName,a1
	moveq	#0,d0
	CALL	OpenLibrary
	move.l	d0,(a3)+
	lea	GfxName,a1
	moveq	#0,d0
	CALL	OpenLibrary
	move.l	d0,(a3)+
	move.l	$4.w,a1
	move.l	$122(a1),(a3)+		; 'elapsed' from execbase -> randseed
	jsr	InitTrash			; Try and setup trash task
	tst.l	TrashTask			; worked?
	bne.s	.addlib			; yes -> continue
	bsr.s	abe			; Close libraries
	moveq.l	#0,d0			; clear d0
	bra.s	.fail
.addlib	lea	_AGLBase,a1
	movea.l	a1,a3
	CALL	AddLibrary
	move.l	a3,d0
.fail	movea.l	(sp)+,a3
	rts

myOpen:
	addq.w	#1,32(a6)
	Bclr.b	#3,14(a6)
	move.l	a6,d0
	rts

abe	lea	_SysBase,a3
	movea.l	(a3)+,a6
	movea.l	(a3)+,a1
	CALL	CloseLibrary
	movea.l	(a3)+,a1
	CALL	CloseLibrary
	movea.l	(a3),a1
	jmp	_LVOCloseLibrary(a6)

myClose:
	subq.w	#1,32(a6)
	bne.s	zro
	Bset.b	#3,14(a6)
	bne.s	exp
zro	moveq	#0,d0
	rts

myExpunge:
	Bset.b	#3,14(a6)
	move.w	32(a6),d1
	bne.s	zro
exp	bsr.s	abe
	movem.l	d0-d7/a0-a5,-(sp)
	move.l	TrashPort,a0		; Port to delete
	lea	$0.w,a1			; No message
	bsr	DeleteAPort
	move.l	#0,TrashPort
	move.l	TrashTask,a1
	CALL	RemTask			; Remove TrashPot task
	movem.l	(sp)+,d0-d7/a0-a5
	move.l	a3,-(sp)
	lea	-50(a3),a1
	CALL	Remove
	move.l	-16(a3),d0
	lea	-50(a3),a6
	movea.l	(sp)+,a3
	rts

InitTrash
	movem.l	a0-a6/d0-d7,-(sp)		; Save all registers
	lea	TrashName,a0		; TrashPort Name
	jsr	CreateAPort
	tst.l	d0			; Did it work?
	beq.s	.end			; No -> end
	move.l	d0,TrashPort
	pea	$4096.w			; Stack = 1k
	pea	trash_pot			; Routine
	pea	$0.w			; Priority 0
	pea	TrashPot,-(sp)		; Task Name
	jsr	_CreateTask		; -- Add the task
	lea	16(sp),sp			; restore stack
	move.l	d0,TrashTask		; Did it work?
.end	movem.l	(sp)+,a0-a6/d0-d7		; Restore registers
	rts

trash_pot		; The Trash Pot!
	move.l	$4.w,a6			; Use EXECBase
	bra.s	.getmsg			; begin...
.while	moveq.l	#-1,d0			; any signal
	CALL	Wait			; -- Wait for signal
.getmsg	move.l	TrashPort,a0
	CALL	GetMsg			; -- Pick up message
	tst.l	d0			; If no message ...
	beq.s	.while			; continue waiting
	move.l	d0,a1			; Area to free
	move.l	MN_LENGTH(a1),d0		; Memory to free
	CALL	FreeMem			; -- Release message memory
	bra.s	.getmsg			; More messages?

;--- End of library section -------------------------------------------------

	include	"libfuncs/LibTable.I"	; Get libs table
	include	"libfuncs/TextPtr.I"	; Table of err msgs
	include	"libfuncs/CommsPrep.I"	; Prepare for connect
	include	"libfuncs/CreateAPort.I"	; Create/Delete Ports
	include	"libfuncs/Random.I"		; Generate random no.s
	include	"libfuncs/Screen.I"		; Setup a screen

; ---------------------------------------------------------------------------

LIBF	MACRO
	dc.w	$4EF9
	dc.l	\1
	ENDM

LibFuncTable:
	LIBF	GetLocal
	LIBF	Random
	LIBF	DeleteAPort
	LIBF	CreateAPort
	LIBF	CommsPrep
	LIBF	TextPtr
	LIBF	LibTable
	LIBF	Return0
	LIBF	myExpunge
	LIBF	myClose
	LIBF	myOpen
_AGLBase:
	dc.l	0,0
	dc.b	9,0
	dc.l	LibName
	dc.b	6,0
	dc.w	negsize,54,VERSION,REVISION
	dc.l	LibIDString,0
	dc.w	0
SegList	dc.l	0
_SysBase	dc.l	0
_DOSBase	dc.l	0
_IntuitionBase	dc.l	0
_GfxBase	dc.l	0
_RandSeed	dc.l	0		; Random seed


; Various private bits

TrashTask	dc.l	0		; Created Task
TrashPort	dc.l	0		; Trash Port...

DOSName		dc.b	'dos.library',0
IntuiName		dc.b	'intuition.library',0
GfxName		dc.b	'graphics.library',0
LibName:		dc.b	"agl.library",0
LibIDString	VSTRING
ManNam		MANNAM		; Manager Name
TrashName	dc.b	"TrashPort",0
TrashPot	dc.b	"AGL Trash Pot",0

	even

EndCode:	END

