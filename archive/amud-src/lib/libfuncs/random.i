; -------------------------------------------------------------------------- ;
; |-|   amud.library Functions. Copyright (C) KingFisher Software 1992   |-| ;
; -------------------------------------------------------------------------- ;

******* amud.library/Random ********************************************
*
*   NAME
*	Random -- Generates a mod'ed random number.
*
*   SYNOPSIS
*	num = Random( Max )
*	d0            d0
*
*	int Random( int );
*
*   FUNCTION
*	Generates a random number based between 0 and Max. Each call to
*	this affects the random seed used by all other Random() callers.
*
*   INPUTS
*	Max - Maxmimum value for return.
*
*   RESULT
*	val - A random number basesd on the Elapsed field of ExecBase.
*
*   EXAMPLE
*	x = Random(100);	/* Generates a no. between 0-100 */
*
******************************************************************************
*
*

Random:	
	movem.l	d1/a6,-(sp)
	tst.l	d0		; Is it 0?
	beq.s	.ret		; Yes - return 0
	bpl.s	.positive
.negative	neg.l	d0		; Negate it.
	bsr.s	Random		; Get the random number
	neg.l	d0		; And return a negative value.
	rts
.positive
	move.l	_RandSeed,d1	; Get random seed
	add.l	4(sp),d1	; Grab some junk
	add.l	8(sp),d1	; to throw the seed.
	sub.l	12(sp),d1	; Putrify it!
	move.l	$4.w,a6
	subq	#3,d1
	add.l	a7,d1
	move.l	d1,_RandSeed	; Incase others try it
	ror.l	#3,d1		; Confuse it
	rol.w	#2,d1		; Really mess it up!
	add.l	$122(a6),d1	; Throw it with 'elapsed' again
	move.l	d1,_RandSeed	; Again, confuse others!
	and.l	#$ffff,d1		; only use lower word of seed
	divu	d0,d1		; Divide d0/d1
	swap	d1		; lower word = remainder
	move.w	d1,d0
	ext.l	d0
.ret	movem.l	(sp)+,d1/a6
	rts

