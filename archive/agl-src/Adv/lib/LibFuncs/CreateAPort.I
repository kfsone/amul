; ีอัออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออัอธ ;
; ณþณ   agl.library Functions. Copyright (C) 1993, KingFisher Software   ณþณ ;
; ิอฯออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออฯอพ ;

******* agl.library/CreateAPort ********************************************
*
*   NAME
*	CreateAPort -- Creates an AmigaDOS comms port, priority 0.
*
*   SYNOPSIS
*	port = CreateAPort( Name )
*	 d0		     a0
*
*	struct MsgPort *CreateAPort( char * );
*
*   FUNCTION
*	Creates a message port and setsup various parameters.
*
*   INPUTS
*	Name	Either NULL or the name of the port.
*
*   RESULT
*	port	Address of the port.
*
*   EXAMPLE
*	if((port=CreateAPort("Manager"))==NULL) error("Can't create port!\n");
*	if((port=CreateAPort(NULL))==NULL) error("Can't create ports!\n");
*
*   NOTES
*	Although it bears great similarity to Amiga.Lib's CreatePort, remember
*	it is NOT.
*
******************************************************************************
*
*

CreateAPort	; a0 = Name | NULL, d0 = Port | NULL

	movem.l	a0-a6/d1-d7,-(sp)
	move.l	a0,-(sp)
	pea	$0.w		; long SigBit : Allocated Signal
	pea	$0.w		; long Port   : Port Memory
	move.l	$4.w,a6
				; Firstly allocate a signal for it.
	moveq	#-1,d0		; AllocSignal(-1)
	CALL	AllocSignal
	tst.l	d0		; Failed?
	bmi.s	.ret		; yes -> return NULL
	move.l	d0,4(sp)

	move.l	#MP_SIZE,d0	; AllocMem(MP_SIZE,Attribs)
	move.l	#MEMF_CLEAR+MEMF_PUBLIC,d1
	CALL	AllocMem
	tst.l	d0		; Failed?
	bne.s	.gotmem
	move.l	4(sp),d0
	CALL	FreeSignal	; Release the signal bit
	bra.s	.ret
.gotmem
	move.l	d0,a0		; a0 = struct MsgPort *port
	move.l	d0,(sp)
	move.l	8(sp),LN_NAME(a0)
	move.b	#0,LN_PRI(a0)	; zero priority
	move.b	#NT_MSGPORT,LN_TYPE(a0)
	move.b	#PA_SIGNAL,MP_FLAGS(a0)
	move.l	4(sp),d0
	move.b	d0,MP_SIGBIT(a0)
	lea	$0.w,a1		; Get my task address.
	CALL	FindTask
	move.l	(sp),a1		; Goes here for ensuing
	move.l	d0,MP_SIGTASK(a1)	; Attach port to me.

	tst.l	8(sp)		; if ( name == NULL )
	beq.s	.private		; -> make port private
	CALL	AddPort		; This one isn't
	bra.s	.ret
.private
	lea	MP_MSGLIST(a1),a1	; Pointer to message list
	NEWLIST	a1		; Initialize it...
.ret
	move.l	(sp),d0		; Return value (port address)
	add.l	d0,_RandSeed	; Fiddle it!
	lea	12(sp),sp		; Skip local store
	movem.l	(sp)+,a0-a6/d1-d7
	rts

******* agl.library/DeleteAPort ********************************************
*
*   NAME
*	DeleteAPort -- Disposes of a port created by CreateAPort
*
*   SYNOPSIS
*	DeleteAPort( Port, Msg )
*		    a0    a1
*
*	void DeleteAPort( struct MsgPort *, struct Aport * );
*
*   FUNCTION
*	Deletes a message port that was previously created by CreateAPort.
*
*   INPUTS
*	Port	Address of the port or NULL.
*	Msg	Address of Aport message or NULL.
*
*   EXAMPLE
*	if((port=CreateAPort("Manager"))==NULL) error("Can't create port!\n");
*	DeleteAPort(port);
*
*   NOTES
*	Although it bears great similarity to Amiga.Lib's DeletePort, remember
*	it is NOT.
*
******************************************************************************
*
*

DeleteAPort
	movem.l	a2-a6/d1-d7,-(sp)
	movem.l	a0-a1,-(sp)
	move.l	$4.w,a6		; All calls are exec.
	cmp.l	#0,a0		; Port to delete?
	beq.s	.NoPort		; No -> try message
	tst.l	LN_NAME(a0)	; Check port name
	beq.s	.private		; It's a private one
	move.l	a0,a1
	CALL	RemPort		; Remove it.
	move.l	(sp),a0		; Make sure we have a0
.private
	moveq	#0,d0		; Release the signal
	move.b	MP_SIGBIT(a0),d0
	CALL	FreeSignal

	move.l	(sp),a1		; Release the memory
	move.l	#MP_SIZE,d0
	CALL	FreeMem

.NoPort	tst.l	4(sp)		; Aport message to delete?
	beq.s	.ret		; no -> return

	move.l	4(sp),a1
	move.l	#AP_SIZE,d0
	CALL	FreeMem

.ret	movem.l	(sp)+,a0-a1
	movem.l	(sp)+,a2-a6/d1-d7
	rts
