;;
;; AManX.i			Various Assembler functions for AMan
;;
	output	"AManX.o"
	xdef	_setvername,_MyUsage
	xref	_sprintf

	include	"adv:h/AMan.I"

Wait	equ	-$13e
GetMsg	equ	-$174
FreeMem	equ	-$0d2
Signal	equ	-$144

_setvername			; Set version name: setvername(string)
	move.l	4(sp),a0	; destination
	pea	date		; date
	pea	REVISION	; Revision
	pea	VERSION		; Version
	pea	version		; template string
	move.l	a0,-(sp)	; destination
	jsr	_sprintf	; call sprintf
	lea	20(sp),sp	; patch up again
	rts

version	dc.b	"AMan v%d.%d (%s)",0
date	DATE
	dc.b	0

	section	text
_MyUsage
	dc.b	"Usage:",13,13
	dc.b	"\taman [-q] <path>\tLoad & Start game",13
	dc.b	"\taman -k\t\t\tKill game",13
	dc.b	"\taman -r [<n>]\t\tForce reset [in <n> seconds>]",13
	dc.b	"\taman -x <n>\t\teXtend game by <n> seconds",13
	dc.b	"\taman -s<path>\t\tSet path for next session",13,0

	even

	end
	
