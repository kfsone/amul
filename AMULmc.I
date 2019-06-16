	xdef _match
	output "AMULmc.O"


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
	cmp.b	#' ',(a1)		; Space?
	bne.s	.failed
.end	moveq	#0,d0
	rts
.space	cmp.b	#'_',d1		; Underscore?
	beq.s	.loop
	cmp.b	#' ',d1
	beq.s	.loop
	bra.s	.failed

	even
