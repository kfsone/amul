;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;	AMan	AGL Manager	Copyright (C) Oliver Smith, 1991-1993
;;
;; AManMC.I			Extra functions for AMan
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


	; Destination object file
	output	"AManMC.o"

	; External functions/objects referenced
	xref	_sprintf,_printf,_quit

	; Functions made accessible externally
	xdef	_setvername,_Usage

	include	"adv:h/AMan.I"


; _setvername	C equivalent:
;
;	setvername(char *s) { sprintf(s,version,VERSION,REVISION,date); }

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

; _Usage	C equivalent
;
;	Usage() { printf(usage,vername); quit(); }

_Usage:	; Display program usage string
	pea	usage
	jsr	_printf			; Print the string
	lea	4(sp),sp
	jmp	_quit			; Exit the program


;; Data


version	dc.b	"AMan v%d.%d (%s)",0
date	DATE
	dc.b	0
usage	dc.b	"AMan usage:",13,13
	dc.b	"	aman [options]",13
	dc.b	"Options:",13
	dc.b	"	-l <n>		Set session length to <n> minutes",13
	dc.b	"	-m <n>		Set maximum user time to <n> minutes",13
	dc.b	"	-p <path>	First adventure path",13
	dc.b	"	-q		Quiet Mode",13
	dc.b	"	-t <path>	Second adventure path",13
	dc.b	"Use AGCon to control run-time behaviour.",13,0
	even

	end

