;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Extras.I			Extra functions (in assembler) for AMCom
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	output	"adv:extras.o"

	include	"adv:h/AMCom.I"

	xref	_block,_dir,_Write,_ohd,_Word,_sprintf,_ifp,_fopen,_exit
	xref	_fprintf,_ofp1,_fclose

	xdef	_tx,_repspc,_remspc,_skipspc,_clean_up,_opentxt,_usage
	xdef	_getword,_repcrlf,_skipline,_skiplead,_setvername,_putn

	xdef	_SysDefTxt

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _getword(char *s)	Fetch next word from "s" to variable _Word
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_getword
	move.l	4(sp),a0		; getword(char *s)
getword			; m/c entry point
	lea	_Word,a1
	move.b	#0,(a1)
	bsr.s	skipspc
	cmp.b	#';',(a0)		; Start of comment?
	beq.s	.remcmt
	moveq	#62,d0
.loop	move.b	(a0)+,(a1)		; copy byte
	move.b	(a1)+,d1
	beq.s	.ret			; End of string?
	cmp.b	#32,d1			; or a space?
	beq.s	.replc
	dbra	d0,.loop
.replc	move.b	#0,-1(a1)
.loop2	move.b	(a0)+,d0		; now skip-space...
	beq.s	.ret
	cmp.b	#32,d0
	beq.s	.loop2
	cmp.b	#';',d0
	bne.s	.ret
.remcmt	move.b	#0,-(a0)		; Remove and replace with zero
	bra.s	.rts			; leave pointing to ZERO
.ret	lea	-1(a0),a0		; back one
.rts	move.b	#0,(a1)
	move.l	a0,d0
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _skipspc(char *s)	Skip white spaces at "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_skipspc
	move.l	4(sp),a0
skipspc	tst.b	(a0)			; EOS?
	beq.s	.ret
	cmp.b	#32,(a0)+		; space?
	beq.s	skipspc
	lea	-1(a0),a0		; move back a step
.ret	move.l	a0,d0
	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _skiplead(char *w,char *s)	Skips phrase "w" in string "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_skiplead
	move.l	4(sp),a0		; Get parameters
	move.l	8(sp),a1
skiplead		; m/c entry point
	move.l	a1,-(sp)
	cmp.b	#32,(a0)		; space?
	bne.s	.loop			; no, OK.
	bsr.s	skipspc			; skip leading spaces
.loop	tst.b	(a0)			; End of string?
	beq.s	.end			; yes, remove it from BLOCK
	cmp.b	(a0)+,(a1)+		; do they match?
	beq.s	.loop
	move.l	(sp),a1
.end	move.l	a1,d0
	lea	4(sp),sp
	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _opentxt(char *s)	Appends ".TXT" to file name "s" and opens the file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_opentxt			; Open a text file : opentxt(char *name)
	tst.l	_ifp			; File currently open?
	beq.s	.goFile
	move.l	_ifp,-(sp)		; Close the file
	jsr	_fclose	
	lea	4(sp),sp
.goFile	lea	_block,a0		; Destination string
	lea	_dir,a1			; adventure path
.cpDir	move.b	(a1)+,(a0)+		; copy "dir"
	bne.s	.cpDir
	lea	-1(a0),a0		; Drop back a byte
	move.l	4(sp),a1
.cpStr	move.b	(a1)+,(a0)+		; copy string
	bne.s	.cpStr
	move.b	#'.',-1(a0)		; append ".TXT"
	move.b	#'T',(a0)+
	move.b	#'X',(a0)+
	move.b	#'T',(a0)+
	move.b	#0,(a0)+		; NULL byte
	pea	OpRB			; fopen(block,"rb")
	pea	_block			; file name
	jsr	_fopen			; try to open file
	move.l	d0,_ifp			; store in IFP
	bne.s	.openOK			; if success
	pea	OpFail			; "filename" is already on stack
	jsr	_printf
	lea	12(sp),sp		; restore stack from printf+fopen
	pea	$202.w
	jsr	_exit
.openOK	lea	8(sp),sp		; restore stack from fopen
	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _tx(char *s)		Displays a string (using AmigaDOS WRITE command)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_tx
	move.l	4(sp),a0
tx					; m/c entry point
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _repspc(char *s)	Replaces any tabs with spaces in string "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_repspc
	move.l	4(sp),a0
repspc					; m/c entry point
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _repcrlf(char *s)	Replace carriage returns with linefeeds
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_repcrlf
	move.l	4(sp),a0
repcrlf					; m/c entry point
.loop	tst.b	(a0)			; end of text?
	beq.s	.ret
	cmp.b	#13,(a0)+
	bne.s	repcrlf
	move.b	#10,-1(a0)
	bra.s	repcrlf
.ret	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _remspc(char *s)	REMOVE leading spaces from "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_remspc
	move.l	4(sp),a0
remspc					; m/c entry point
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


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; char *_skipline(char *s)	Find start of next line in "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_skipline
	move.l	4(sp),a0
skipline				; M/C Entry point
	tst.b	(a0)			; EOS?
	beq.s	.rts
	cmp.b	#10,(a0)+
	bne.s	skipline
	clr.b	-1(a0)			; Clear RETURN
	cmp.b	#';',(a0)		; Comment line?
	bne.s	.rts
	clr.b	(a0)+			; Clear & Skip comment character
	bra.s	skipline
.rts	clr.b	-1(a0)			; Clear the previous character
	move.l	a0,d0
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _setvername(char *s)		Puts the current release name into "s"
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_setvername			; Set version name: setvername(string)
	move.l	4(sp),a0	; destination
	pea	date		; date
	pea	REVISION	; Revision
	pea	VERSION		; Version
	pea	version		; template string
	move.l	a0,-(sp)	; destination
	jsr	_sprintf	; call sprintf
	lea	20(sp),sp	; patch up again
ret	rts
version	dc.b	"AMCom v%d.%d (%s)",0
date	DATE
	dc.b	0
	even


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _putn(long n)	Writes a number to "ofp1" in "%ld " format.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_putn
	move.l	4(sp),a0		; Get the number
	move.l	a0,-(sp)
	pea	PutNF			; format mask
	move.l	_ofp1,-(sp)		; file handle
	jsr	_fprintf		; write to file
	lea	12(sp),sp
	rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _usage		Displays program usage/command line options
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_usage
	lea	MyUsage,a0		; string to print
	bsr	_tx			; Display it
	pea	$0.w			; Error Code 0
	jsr	_exit			;; exit(0);
	rts				;; Not realy neccesary!


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; _clean_up(char *s)	Processes "s": case conversion, comments removed etc.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; This function runs through the entire stack of data pointed to by "s" until
;; it encounters a NULL BYTE (ASCIZ) and does a stack of tidying up which makes
;; it easier to process the file. For this purpose a "map table" is used.
;; Instead of saying "if(c==';') abc() else ... if(c=='+') xyz()" we have a
;; table of "operations" for every relevant character. This table contains a
;; single byte instructing the function what to do to different characters. A
;; positive value tells clean_up to REPLACE the original character. A negative
;; value tells it to call a "special" function. A zero tells it to use the
;; original value. Any non-standard characters are replaced with spaces (char
;; 32), any line feeds (char 13) are replaced with carriage returns (char 10)
;; as are commas (allowing multiple commands on a single REAL line).
;; Uppercase letters are converted to lower case. Comments are removed. Two
;; lines can be "joined" by placing a "+" in the LAST character position before
;; the carriage return. And text contained within quotes is left well alone.
;; Because this is assembler, and because it's a well optimised little routine
;; it can handle huge files VERY quickly, and this minor overhead is nothing
;; compared to the constant checks the processing routines would HAVE to do
;; to cater for all these events.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_clean_up		; Clean up string
	move.l	4(sp),a0
clean_up
	moveq	#0,d0
	moveq	#0,d1
cleanlp	lea	conv_table,a1		; Our character table
.loop	move.b	(a0)+,d0		; get byte
	beq.s	ret			; End of string?
	cmp.b	#90,d0			; Top character in table
	bgt.s	.loop
	move.b	(a1,d0.w),d1		; get conversion code
	beq.s	.loop
	bmi	.subrtn
	move.b	d1,-1(a0)
	bra.s	.loop
.subrtn	ext.w	d1
	move.l	(a1,d1.w),a1		; address of routine
	jmp	(a1)			; Call routine
cfcase
	add.b	#'a'-'A',d0		; Shift case
	move.b	d0,-1(a0)		; Go to next character
	bra.s	cleanlp
cfskcoma			; Skip a comma
	cmp.b	#10,(a0)		; Next character a CR?
	beq.s	.comacr
	cmp.b	#13,(a0)		; or a LF?
	beq.s	.comacr
	move.b	#10,-1(a0)		; No - so COMMA is valid!
	bra.s	cleanlp
.comacr	move.b	#32,-1(a0)		; Replace with a space!
	bra.s	cleanlp
cfskquo				; Skip quotes ( original in d0 )
	move.b	(a0)+,d1
	beq.s	cfend			; NULL -> end
	cmp.b	d1,d0			; End quote?
	beq.s	cleanlp
	cmp.b	#10,(a0)		; CR?
	beq.s	cleanlp
	cmp.b	#13,(a0)		; LF?
	bne.s	cfskquo			; Neither - try next character
cfend	lea	-1(a0),a0		; Char 13 replaced with char 10
	bra.s	cleanlp
cfplus				; Process "+"
	cmp.b	#10,(a0)		; if the "+" is the last character on
	beq.s	.cr			; a line, we remove the carriage return
	cmp.b	#13,(a0)		; so that the processing routines think
	bne.s	cleanlp			; it is part of the same line
.cr	move.b	#$20,-1(a0)		; Remember: also replace the "+" with
	move.b	#$20,(a0)		; a space, else confusion!
	bra.s	cleanlp
cfskcmnt			; Remove comment text
	move.b	#32,-1(a0)		; Clear the ';'
.loop	move.b	(a0)+,d1
	beq.s	cfend
	cmp.b	#10,d1
	beq	cleanlp
	cmp.b	#13,d1
	beq.s	cfend
	move.b	#32,-1(a0)
	bra.s	.loop

c1	equ	-4		; Different 'cleanup' functions
c2	equ	-8		; These values are offsets from the address
c3	equ	-12		; "conv_table", i.e., "c1" infers
c4	equ	-16		; "conv_table - 4", i.e. the pointer to
c5	equ	-20		; "cfskquo"

	dc.l	cfplus,cfskcmnt,cfcase,cfskcoma,cfskquo
conv_table
	dc.b	32,32,32,32,32,32,32,0,0,32,0,32,0,10		; 0-13( -cr)
	dc.b	32,32,32,32,32,32,32,32,32,32,32,32,32,0	;14-27( -esc)
	dc.b	32,32,32,32,0,0,c1,0,0,0,0,c1,0,0		;28-41( -))
	dc.b	0,c5,c2,0,0,0,0,0,0,0,0,0,0,0			;42-55(*-7)
	dc.b	0,0,0,c4,0,0,0,0,0
	dc.b	c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3,c3

OpFail	dc.b	27,"[33;1m!! Missing File %s !!",27,"[0m",13,13,0
OpRB	dc.b	"rb",0
PutNF	dc.b	"%ld ",0

MyUsage				; Usage Text
	dc.b	"Usage: amcom [<options>] <path>",13
	dc.b	"Options:",13
	dc.b	" -d = Don't check DMove flags",13
	dc.b	" -q = Quiet (no warnings)",13
	dc.b	" -r = Don't recompile rooms",13,0

	section	text

_SysDefTxt			; Default for System.Txt
	dc.b	";",13,";",9,"Default System.Txt for AMUD",13
	dc.b	";",9,"generated by %s (C) Oliver Smith, 1993.",13
	dc.b	";",13,13,'name="AMUD Game"',9,"; Adventure name",13
	dc.b	"session=30",9,9"; Session Length",13
	dc.b	"%s=30",9,9,"; Session Length",13
	dc.b	"%s=0",9,9,"; Min. rank to use SuperGo",13
	dc.b	"%s=40",9,9,"; Top rank users lose 40%s",13
	dc.b	"%s=50",9,9,"; At start of game objects lose 50%s",13
	dc.b	"%s=2",9,9,"; Inv%s2%s",13,"%s=3",9,9,"; V%s3%s",13,13

	even

	end
