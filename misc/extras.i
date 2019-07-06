
	* M/C functions for AMUL

	output	"extras.o"

	xref	_block,_strcpy,__ctype,_Write,_Word,_printf,_strlen

	xdef	_bitset,_striplead,,_com,_repspc,_remspc,_skipspc
	xdef	_getword,_sgetl,

_striplead		; strip text (a0) from text
	move.l	4(sp),a0		; Get parameters
	move.l	8(sp),a1
	bsr.s	skiplead		; Skip the header
	cmp.l	8(sp),d0		; No change?
	beq.s	.end
	move.l	8(sp),a0
.loop2	move.b	(a1)+,(a0)+
	bne.s	.loop2
	moveq	#-1,d0
	rts
.end	moveq	#0,d0
	rts

_remspc			; Chop leading spaces from (a0)
	move.l	4(sp),a0
remspc			; m/c entry point
	move.l	a0,a1
.loop	tst.b	(a0)			; end of string?
	beq.s	.ret
	cmp.b	#32,(a0)+
	beq.s	.loop
	lea	-1(a0),a0
.loop2	move.b	(a0)+,(a1)
	tst.b	(a1)+			; zero?
	bne.s	.loop2
.ret	rts

_getword		; Get first word from (a0)
	move.l	4(sp),a0		; getword(_CHAR *P);
getword			; m/c entry point
	lea	_Word,a1
	move.b	#0,(a1)
	bsr.s	skipspc
	moveq	#62,d0
.loop	move.b	(a0)+,(a1)		; copy byte
	move.b	(a1)+,d1
	beq.s	.ret			; End of string?
	cmp.b	#32,d1			; or a space?
	beq.s	.repspc			; yes, remove it, and return
	cmp.b	#'A',d1
	blt.s	.next
	cmp.b	#'Z',d1
	bgt.s	.next
	add.b	#'a'-'A',-1(a1)		; force lower!
.next	dbra	d0,.loop
.loop2	move.b	(a0)+,d0		; find EOS
	beq.s	.ret
	cmp.b	#';',d0
	beq.s	.repspc
	cmp.b	#'*',d0
	beq.s	.repspc
	cmp.b	#32,d0
	bne.s	.loop2
.repspc	move.b	#0,-1(a1)
.ret	move.l	a0,d0			; return end of string ptr
	move.b	#0,(a1)
	subq.l	#1,d0
	rts

rev	dc.b	27,"[3m",0
revof	dc.b	13,10,27,"[0m",0
