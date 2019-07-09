
	* M/C functions for AMUL

	output	"adv:extras.o"

	xref	_block,_strcpy,__ctype,_Write,_ohd,_Word,_printf,_strlen

	xdef	_bitset,_striplead,_tx,_com,_repspc,_remspc,_skipspc
	xdef	_getword,_sgetl,_repcrlf,_skipline,_skiplead

_bitset	move.l	4(sp),d1
bitset			; m/c entry point
	moveq.l	#1,d0
	lsl.l	d1,d0
	rts

_skiplead		; skip text (a0) from (a1)
	move.l	4(sp),a0		; Get parameters
	move.l	8(sp),a1
skiplead		; m/c entry point
	move.l	a1,-(sp)
.loop	tst.b	(a0)			; End of string?
	beq.s	.end			; yes, remove it from BLOCK
	cmp.b	(a0)+,(a1)+		; do they match?
	beq.s	.loop
	move.b	-1(a0),d0
	move.b	-1(a1),d1
	cmp.b	#'a',d0			; match upper and lower
	blt.s	.ok1
	cmp.b	#'z',d0
	bgt.s	.ok1
	add.b	#'A'-'a',d0
.ok1	cmp.b	#'a',d1
	blt.s	.ok2
	cmp.b	#'z',d1
	blt.s	.ok2
	add.b	#'A'-'a',d1
.ok2	cmp.b	d0,d1
	beq.s	.loop
	move.l	(sp),a1
.end	move.l	a1,d0
	lea	4(sp),sp
	rts

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

;com(char *s)
;{
;	if(block[0]==';') return -1;
;	if(block[0]=='*')
;	{
;		tx(s); tx("\n\x1B[0m"); return -1;
;	}
;	return 0;
;}

;opentxt(s)
;char *s;
;{
;	sprintf(block,"%s%s.TXT",dir,s);
;	if((ifp=fopen(block,"rb"))==NULL)
;	{
;		printf("[33;1m !! Missing file %s !! [0m\n\n",block);
;		exit(202);
;	}
;}

_tx			; display a string (using AmiDOS command)
	move.l	4(sp),a0
tx			; m/c entry point
	move.l	a0,a1
	moveq	#0,d0
.loop	tst.b	(a0)+
	bne.s	.loop
.end	sub.l	a1,a0
	move.l	a0,-(sp)
	move.l	a1,-(sp)
	move.l	_ohd,-(sp)
	bsr	_Write
	lea	12(sp),sp
	rts

_com
	move.l	4(sp),a0
com			; m/c entry point
	tst.b	(a0)			; Empty string?
	beq.s	.zed
	cmp.b	#13,(a0)		; Blank line?
	beq.s	.zed
	cmp.b	#-1,(a0)		; EOF?
	beq.s	.zed
	cmp.b	#';',(a0)
	beq.s	.neg
	cmp.b	#'*',(a0)
	bne.s	.zed
	move.l	a0,-(sp)
	lea	rev,a0
	bsr.s	tx
	bsr.s	_tx
	lea	revof,a0
	bsr.s	tx
	move.l	(sp)+,a0
.neg	moveq	#-1,d0
	rts
.zed	moveq	#0,d0
	rts

_repspc			; Replace tabs with spaces
	move.l	4(sp),a0
repspc			; m/c entry point
.loop	tst.b	(a0)			; end of text?
	beq.s	.ret
	cmp.b	#9,(a0)			; tab?
	beq.s	.rep1
	cmp.b	#13,(a0)+
	bne.s	repspc
	move.b	#10,-1(a0)
	bra.s	repspc
.rep1	move.b	#32,(a0)+
	bra.s	.loop
.ret	rts

_repcrlf		; Replace crs with lfs
	move.l	4(sp),a0
repcrlf			; m/c entry point
.loop	tst.b	(a0)			; end of text?
	beq.s	.ret
	cmp.b	#13,(a0)+
	bne.s	repcrlf
	move.b	#10,-1(a0)
	bra.s	repcrlf
.ret	rts

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

_skipspc		; Skip _SPACES_ in a string (a0)
	move.l	4(sp),a0
skipspc	tst.b	(a0)			; EOS?
	beq.s	.ret
	cmp.b	#32,(a0)+		; space?
	beq.s	skipspc
	lea	-1(a0),a0		; move back a step
.ret	move.l	a0,d0
	rts

newline
	tst.b	(a0)			; EOS?
	beq.s	.ret
	cmp.b	#10,(a0)+		; RETURN?
	bne.s	newline
.ret	bra.s	sgetl

_sgetl			; Get string from (a0) to (a1)
	move.l	4(sp),a0		; Get FROM
	move.l	8(sp),a1		; Get TO
	cmp.b	#';',(a0)		; comment line?
	beq.s	newline			; yes -> next line
	cmp.b	#'*',(a0)		; comment line?
	beq.s	newline
	move.b	#0,(a1)
sgetl			; M/C entry point
	move.b	(a0)+,(a1)		; Copy
	beq.s	.rts			; End of string?
	cmp.b	#10,(a1)+		; Carriage return? (blank line)
	bne.s	sgetl
.crlf	clr.b	-1(a1)
.rts	move.l	a0,d0			; Return (char *) a0
	rts

_skipline				; Skip to next cr/lf
	move.l	4(sp),a0
skipline			; M/C Entry point
	tst.b	(a0)			; EOS?
	beq.s	.rts
	cmp.b	#10,(a0)+
	bne.s	skipline
.rts	move.l	a0,d0
	rts


rev	dc.b	27,"[3m",0
revof	dc.b	13,10,27,"[0m",0
