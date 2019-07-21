; ีอัออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออัอธ ;
; ณþณ   agl.library Functions. Copyright (C) 1993, KingFisher Software   ณþณ ;
; ิอฯออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออฯอพ ;

******* agl.library/CommsPrep ********************************************
*
*   NAME
*	CommsPrep -- Prepares ports etc for an AMan connection.
*
*   SYNOPSIS
*	errmsg = CommsPrep(Port,Reply,Msg)
*	  d0		a0    a1   a2
*
*	char *CommsPrep(struct MsgPort **,struct MsgPort **,struct Aport **)
*
*   FUNCTION
*	Initialises various structures required for an AMan session. Any
*	pointer set to NULL will be skipped.
*
*   INPUTS
*	Port	Pointer to your AMan Message Port pointer.
*	Reply	Pointer to your AMan Reply Port pointer.
*	Msg	Pointer to a (struct Aport *)pointer.
*
*   RESULT
*	*Port	:= pointer to AMan communication port
*	*Reply	:= pointer to a new reply port
*	*Msg	:= pointer to a new (struct Aport) object
*	errmsg	NULL if success, else a pointer to a string that describes
*		the reason for the error.
*
*   EXAMPLE
*	struct MessagePort *Write,*Reply;
*	struct Aport *msg;
*	char	*err;
*	  ...
*	if((err=CommsPrep(&Write,&Reply,&msg))!=NULL) {
*		printf("Error: %s\n",err); quit();
*	}
*
******************************************************************************
*
*

CommsPrep
	movem.l	d1-d7,-(sp)
	movem.l	a0-a6,-(sp)
	move.l	$4.w,a6		; All following calls to exec
	moveq	#0,d0		; Clear return
.Port
	cmp.l	#0,a0		; a0 - Port
	beq.s	.Reply		; No, do reply instead.
	lea	ManNam,a1
	CALL	FindPort		;* Find Manager Port
	lea	NoAMAN,a0		; No,  return error.
	tst.l	d0		; OK?
	beq.s	.Reta0		; No -> Error
	move.l	(sp),a0		; &Port
	move.l	d0,(a0)		; *Port = FindPort(ManNam);
.Reply
	tst.l	4(sp)		; a1 - Reply
	beq.s	.Msg
	lea	$0.w,a0		; Call CreateAPort(NULL)
	jsr	CreateAPort	;* Create the port.
	lea	NoPORT,a0
	tst.l	d0		; OK?
	beq.s	.Reta0		; No -> error
	move.l	4(sp),a0		; &Reply
	move.l	d0,(a0)		; Reply = CreateAPort(0L);
.Msg
	tst.l	8(sp)		; a2 - Msg
	beq.s	.Ret
	move.l	#AP_SIZE,d0
	move.l	#MEMF_PUBLIC+MEMF_CLEAR,d1
	CALL	AllocMem		;* AllocMem(size,attribs)
	tst.l	d0		; OK?
	beq.s	.DelPort		; No -> error
	move.l	8(sp),a0		; &Msg
	move.l	d0,(a0)		; Msg = AllocMem(x,y)
	move.l	d0,a0
	move.b	#NT_MESSAGE,LN_TYPE(a0)
	clr.l	MN_REPLYPORT(a0)	; msg.mn_replyport=0
	lea	TrashPort,a1
	tst.l	4(sp)		; Do we know the reply port?
	beq.s	.leave
	move.l	4(sp),a1		; &Reply
.leave	move.l	(a1),MN_REPLYPORT(a0)	; msg.mn_replyport=reply
	move.w	#AP_SIZE,MN_LENGTH(a0)
	move.l	#-1,AP_FROM(a0)	; msg->from = -1
	lea	$0.w,a0
.Reta0
	move.l	a0,d0		; Return value of A0 in D0
.Ret
	add.l	d0,_RandSeed	; Confuse random seed
	movem.l	(sp)+,a0-a6
	movem.l	(sp)+,d1-d7
	rts
.DelPort
	move.l	4(sp),a1
	move.l	(a1),a0
	clr.l	(a1)
	lea	$0.w,a1
	jsr	DeleteAPort	; DeleteAPort(*Reply,NULL)
	lea	NoMEM,a0
	bra.s	.Reta0

	even
