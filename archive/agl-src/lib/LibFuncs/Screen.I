; ีอัออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออัอธ ;
; ณþณ   agl.library Functions. Copyright (C) 1993, KingFisher Software   ณþณ ;
; ิอฯออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออฯอพ ;

******* agl.library/GetScreen ********************************************
*
*   NAME
*	GetLocal -- Allocates a screen and/or window for local use.
*
*   SYNOPSIS
*	error=GetLocal(Screen, Window, Title)
*	 d0              a0      a1      a2
*
*	char *GetLocal(struct Screen **,struct Window **,char *);
*
*   FUNCTION
*	Allocates a custom screen and/or window using the standard AGL
*	screen settings. The screen data should be clipped against the
*	W/Bench on setup. The routine should also contain a LoadRGB4
*	command... If Screen or Window are NULL, either the screen or
*	window will not be allocated. If Title is NULL, the default
*	copyright message will be used as the screen/window titles.
*
*   INPUTS
*	struct Screen **Screen	Address of your screen POINTER
*	struct Window **Window	Address of your window POINTER
*	char           *Title	Pointer to initial window title
*
*   RESULT
*	error is either NULL or a pointer to text describing the error.
*
*   EXAMPLE
*	char *p; struct Screen *sc; struct Window *wi;
*	...
*	if((p=GetLocal(&sc,&wi,"A Test Screen"))!=NULL) {
*		printf("Failed: %s",p); exit(0);
*	}
*	if((p=GetLocal(NULL,&wi,"A WorkBench Window"))) {
*		printf(p); exit(0);
*	}
*
*   NOTES
*	If Screen is NULL, GetLocal will open the Window on the WorkBench
*	screen.
*	The standard intuition CloseScreen/CloseWindow commands can be used
*	to close either screen/window. Alternatively, place them in an
*	IOLine structure and use FreeLine() to dispose of them.
*
*   SEE ALSO
*	AttCon(), RelDevice(), struct IOLine, FreeLine()
*
******************************************************************************
*
*

GetLocal:		; a0=**Screen, a1=**Window, a2=*Title
	movem.l	a3-a6/d1-d7,-(sp)	; Save (probably too many) registers
	movem.l	a0-a2,-(sp)
	move.l	$4.w,a6			; Exec Call
	CALL	Forbid			; So no-one else uses SCREEN structure
	bsr	.SetSizes		; Get WB Size etc
	tst.l	(sp)
	beq.s	.UseWB			; Use workbench if Screen==NULL
.UseCS	lea	Scr,a0
	CALLINT	OpenScreen		; OpenScreen(&Scr)
	tst.l	d0
	bne.s	.ssave
	move.l	#NoSCRN,d0		; "Can't open screen"
	bra	.Fin
.ssave	move.l	(sp),a0			; a0 = **Screen
	move.l	d0,(a0)			; *Screen=OpenScreen(&scr)
	move.l	d0,a0			; a0=screen
	move.l	d0,Win+nw_Screen		; nw.Screen=*Screen
	move.l	Scr+ns_Width,Win+nw_Width
	sub.w	#11,Win+nw_Height
	move.w	#CUSTOMSCREEN,Win+nw_Type
	lea	sc_ViewPort(a0),a0		; Load colour palette
	lea	Palette,a1
	moveq	#PColors,d0
	CALLGRAF	LoadRGB4			; - LoadRGB4(vp,Pal,Count)
	bra.s	.Window
.UseWB	move.l	#0,Win+nw_Screen
	move.w	#WBENCHSCREEN,Win+nw_Type
.Window	tst.l	4(sp)		; Window pointer?
	beq.s	.FinOK		; No -> bye bye!
	lea	Win,a0
	CALLINT	OpenWindow	; OpenWindow(&Win)
	tst.l	d0		; Failed?
	bne.s	.wsave		; no -> do titles
	tst.l	(sp)		; Was there a screen?
	bne.s	.noscrn		; No -> don't try and close
	move.l	(sp),a1
	move.l	(a1),a0
	move.l	#0,(a1)
	CALL	CloseScreen	; Shut screen down
.noscrn	move.l	#NoWIN,d0
	bra.s	.Fin
.wsave	move.l	4(sp),a0	; a0=**Window
	move.l	d0,(a0)		; *Window=OpenWindow(&Win)
	move.l	(a0),a0		; a0=*Window (for titles)
.Titles	move.l	8(sp),a1	; a1 = title
	bne.s	.use		; a1=NULL?
	lea	title,a1		; yes->use default
.use	move.l	a1,a2		; screen title
	CALL	SetWindowTitles	; Set titles
.FinOK	moveq.l	#0,d0		; Clear return
.Fin	move.l	_SysBase,a6	; Back to exec
	CALL	Permit		; Re-enable multi-tasking
	movem.l	(sp)+,a0-a2	; Restore args
	movem.l	(sp)+,a3-a6/d1-d7
	rts

.SetSizes
	sub.l	#sc_SIZEOF,a7	; "push" screen onto stack
	move.l	a7,a0		; address of local variable "sc"
	move.l	#sc_SIZEOF,d0	; sizeof(Screen)
	move.l	#WBENCHSCREEN,d1	; Type: WBench
	lea	$0.w,a1		; get's ignored anyway
	CALLINT	GetScreenData	; Get copy of workbench
	move.w	sc_Height(a7),d0
	cmp.w	Scr+ns_Height,d0
	bge.s	.hok
	move.w	sc_Height(a7),Scr+ns_Height
.hok	move.l	Scr+ns_Width,Win+nw_Width
	sub.w	#11,Win+nw_Height
	lea	sc_SIZEOF(a7),a7	; Restore a7
	rts

	include	"adv:h/Screen.I"	; Screen/Window Definitions

	even
