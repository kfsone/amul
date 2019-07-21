;;
;; AMUDmc.I			Various assembler functions for AMUD
;;

	include	"adv:h/AMUD.I"

	xdef _match,_setvername,_ioproc,_rts

	xref _sprintf,_ow,_esc		; ow = string space

	output "AMUDmc.O"

OWLIM	equ	2048


; Match:
;
;	Compare two strings, allowing for difference of case,
;	and also treating '_' as ' '.

_match
	move.l	4(sp),a0
	move.l	8(sp),a1
.loop:	tst.b	(a0)		; Match #1?
	beq.s	.endit		; go for an ending!
	cmp.b	(a0)+,(a1)+	; A match?
	beq.s	.loop
	move.b	-1(a1),d1		; Get character
	beq.s	.failed
	move.b	-1(a0),d0		; get character
	cmp.b	#' ',d0		; space?
	beq.s	.space
	cmp.b	#'_',d0
	beq.s	.space
	cmp.b	#'a',d0		; lower case?
	blt.s	.lower
	exg.l	d0,d1
.lower	cmp.b	#'Z',d0		; if this is upper case...
	bgt.s	.failed
	cmp.b	#'A',d0		; if its not alpha
	blt.s	.failed
	add.b	#'a'-'A',d0	; make  it lower
	cmp.b	d0,d1
	beq.s	.loop
.failed	moveq	#-1,d0
	rts
.endit	tst.b	(a1)		; End of a1 too?
	beq.s	.end
	cmp.b	#' ',(a1)	; Space?
	bne.s	.failed
.end	moveq	#0,d0
	rts
.space	cmp.b	#'_',d1		; Underscore?
	beq.s	.loop
	cmp.b	#' ',d1
	beq.s	.loop
	bra.s	.failed

;--------------------- ioproc(char *s)
; An important function which searches out escape codes and control codes in
; AMUD text strings.

_ioproc
	move.l	4(sp),a0	; Get pointer #1
	movem.l	a1-a2,-(sp)
	move.l	_ow,a1		; Destination
	move.l	a1,a2		; End of string pointer
	add.l	#OWLIM-80,a2	; Maximum length
	bra.s	.mloop
.mloop1	cmp.b	#10,d0		; Last character return?
	bne.s	.mloop		; check length of string
	cmp.l	a1,a2		; Getting near to end of string?
	bge.s	.finish		; Yes -> return next byte pointer
.mloop	move.b	(a0)+,d0	; Get character
	beq.s	.end		; Null -> end
	cmp.l	a1,a2		; Getting near end?
	cmp.b	#'@',d0		; AMUD ESCape sequence?
	beq.s	.doesc		; process escape
;	cmp.b	#13,d0		; '\r'?			; ** NOT NEEDED? **
;	beq.s	.repcr		; Yes -> replace w/10	; ** NOT NEEDED? **
	cmp.b	#'^',d0		; ESCape process?
	beq.s	.ctrl		; ctrl character
.use	move.b	d0,(a1)+	; no - use it.
	bra.s	.mloop1		; and do next
.end	lea	$0.w,a0		; No 'next byte'
.finish	move.b	#0,(a1)		; end of string
	move.l	a0,d0		; make it the return
	movem.l	(sp)+,a1-a2	; restore a1
	rts
.doesc
	move.l	a1,-(sp)	; esc(s,p)	s = a0, p = a1
	move.l	a0,-(sp)
	jsr	_esc
	move.l	(sp)+,a0	; get them back
	move.l	(sp)+,a1
	tst.b	d0		; esc(s,p)!=NULL
	bne.s	.skip		; not an escape - print instead
	move.b	#'@',d0		; code
	bra.s	.use
.skip	tst.b	(a1)+		; find EOS
	bne.s	.skip
	lea	-1(a1),a1	; ok.
	lea	2(a0),a0	; skip the code too.
	bra.s	.mloop
.ctrl
	move.b	(a0)+,d0	; get control-code
	move.l	a0,-(sp)
	ext.w	d0
	lea	.table,a0	; get our table
	sub.b	#'G',d0		; < 'G'?
	bmi.s	.bad
	cmp.b	#'m'-'G',d0	; beyond max lim?
	bgt.s	.bad
	cmp.b	#'g'-'G',d0
	blt.s	.trylow
	sub.b	#'g'-'G',d0	; lowercase
.trylow	cmp.b	#'M'-'G',d0	; beyond top range?
	ble.s	.good
.bad	moveq	#'K'-'G',d0	; only character not valid
.good	move.b	(a0,d0.w),d0	; Get character
	move.l	(sp)+,a0
	move.b	d0,(a1)+	; store
	bra.s	.mloop

;		 G   H   I   J  ANY  L   M
.table	dc.b	$07,$08,$09,$0a,'^',$0c,$0a

	even

_setvername			; Set version name: setvername(string)
	move.l	4(sp),a0	; destination
	pea	date		; date
	pea	REVISION	; Revision
	pea	VERSION		; Version
	pea	version		; template string
	move.l	a0,-(sp)	; destination
	jsr	_sprintf	; call sprintf
	lea	20(sp),sp	; patch up again
_rts	rts

version	dc.b	"AMUD v%d.%d (%s)",0
date	DATE
	dc.b	0

	even

