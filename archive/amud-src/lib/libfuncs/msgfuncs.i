; -------------------------------------------------------------------------- ;
; |-|amud.library Message Functions. Copyright (C) KingFisher Software 92|-| ;
; -------------------------------------------------------------------------- ;

******* amud.library/SendMsg ********************************************
*
*   NAME
*	SendMsg -- Send a message and wait for the reply.
*
*   SYNOPSIS
*	SendMsg(Msg,Port,Reply)
*		a0   a1   a2
*
*	void SendMsg(APTR,struct MsgPort *,struct MsgPort *);
*
*   FUNCTION
*	Puts a message to a port and either waits for the reply or for any
*	signal (if Reply==-1)
*
*   INPUTS
*	Msg	Pointer to your message.
*	Port	Address of the message port.
*	Reply	Address of the reply port, or -1 to Wait(-1L);
*
*   RESULT
*	Returns when the message has been replied to, or when Wait(-1L) is
*	interrupted.
*
*   EXAMPLE
*	struct Aport *amud; struct MsgPort *port,*reply;
*	CommsPrep(&port,&reply,&amud);
*		...
*	SendMsg(amud,port,reply);
*		...
*	SendMsg(amud,port,-1);
*
******************************************************************************
*
*

SendMsg

