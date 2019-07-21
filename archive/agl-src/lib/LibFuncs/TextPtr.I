; ีอัออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออัอธ ;
; ณþณ   agl.library Functions. Copyright (C) 1993, KingFisher Software   ณþณ ;
; ิอฯออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออฯอพ ;

******* agl.library/TextPtr ********************************************
*
*   NAME
*	TextPtr -- Gets a pointer to a table of error text messages.
*
*   SYNOPSIS
*	table = TextPtr()
*
*	char **TextPtr();
*
*   FUNCTION
*	Returns the address of a list of string pointers which point to the
*	standard error messages.
*
*   EXAMPLE
*	/* Prints the 5th error message */
*	table = TextPtr(); printf("Error: %s.\n",*table+4);
*
*   SEE ALSO
*	See "aglerrors" include for #defines/equates
*
******************************************************************************
*
*

TextPtr
	move.l	_RandSeed,d0		; Fiddle the random seed while we go!
	lea	err_text,a0
	add.l	a0,d0
	ror.l	#3,d0
	addq	#7,d0
	move.l	d0,_RandSeed
	move.l	a0,d0
	rts

	; *PRIVATE* Text ;

characc	dc.b	7,10,"Sorry, that character already exists under another account.",10,0
charlim	dc.b	7,"You already have %ld characters active on this account.",10
	dc.b	"You cannot create any more.",10,0
aglintro	dc.b	10,' >  %s -- /chat for CHAT MODE. /? for settings editor.  <',10,10,0
gochat	dc.b	'** Entering CHAT MODE **',10,10
	dc.b	'To return to normal mode, type "/endchat" at the prompt.',10
	dc.b	'Lines beginning with a "/" are treated as game commands.',10,10,0
usered	dc.b	"	AGLEd v3.0",10,10
	dc.b	"/p	Change password",10
	dc.b	"/lf	LineFeed on/off",10
	dc.b	"/an	ANSI on/off",10
	dc.b	"/x	Screen width",10
	dc.b	"/y	Screen length",10,10,0
usrsets	dc.b	10,"Terminal settings:",10,10
	dc.b	"Screen size is %ld by %ld characters.",10
	dc.b	"ANSI codes are %s.	Add LF after CR is %s.",10,10
	dc.b	"Change settings? (y/N): ",0
deftitle	dc.b	12,10,10," Welcome to @ad!",10,10," There are @po online.	"
	dc.b	"Next reset in @re",10," Last Compiled: @lc	Last Reset: @lr",10,10,0
title	dc.b	"<< AGL >> Copyright (C) 1993, KingFisher Software!",0


	; *PUBLIC* Messages ;

NoAMAN	dc.b	"AMan not running!",10,0
NoPORT	dc.b	"Can't create message port!",10,0
NoMEM	dc.b	"Out of memory!",10,0
NoSCRN	dc.b	"Can't open screen!",10,0
NoWIN	dc.b	"Can't open window!",10,0

	even		; Because it's text

	; Now the table

	dc.l	characc,charlim,aglintro
	dc.l	gochat,usered,usrsets,deftitle,title	; Privates
err_text
	dc.l	NoAMAN,NoPORT,NoMEM,NoSCRN,NoWIN	; Pubs

