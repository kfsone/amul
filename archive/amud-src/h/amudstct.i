; -------------------------------------------------------------------------- ;
; |-|   amud structure defs.    Copyright (C) KingFisher Software 1992   |-| ;
; -------------------------------------------------------------------------- ;

;;
;;  Based on AMUD.Stct.H - Version 1, Revision 1
;;

	IFND AMUD_STCT_I
AMUD_STCT_I	SET	1

	IFND EXEC_PORTS_I
	INCLUDE "exec/ports.i"
	ENDC ; EXEC_PORTS_I

 STRUCTURE  AP,MN_SIZE	; strcut Aport
	LONG	AP_TYPE		; Message type
	LONG	AP_FROM		; Which AMUD frame (-1 if not)
	LONG	AP_DATA		; Data field
	LONG	AP_P1		; general pointer 1
	LONG	AP_P2		;    ""     ""    2
	LONG	AP_P3		;    ""     ""    3
	LONG	AP_P4		;    ""     ""    4
	APTR	PTR		; Text pointer (usually)
	LABEL	AP_SIZE

 STRUCTURE  LS,0
	APTR	LS_BUF		; Text output buffer
	APTR	LS_REP		; struct MsgPort *reply
	LONG	LS_FLAGS		; user's flags
	SHORT	LS_ROOM		; current room
	LONG	LS_SCTG		; Scored this game
	USHORT	LS_REC		; PlayerData record No.
	USHORT	LS_NUMOBJ		; Objects carried
	LONG	LS_WEIGHT		; Weight carried
	USHORT	LS_DEXTADJ	; Dexterity adjustment
	USHORT	LS_WIELD		; Current weapon
	BYTE	LS_STATE		; Line state
	BYTE	LS_IOLOCK		; Device lock
	BYTE	LS_SUP		; IO Driver Type
	BYTE	LS_LIGHT		; Current light source count
	BYTE	LS_HADLIGHT	; Previous count
	BYTE	LS_HELPING	; Giving help to
	BYTE	LS_HELPED		; Being helped by
	BYTE	LS_FOLLOWING	; I am following
	BYTE	LS_FOLLOWED	; Is following me
	BYTE	LS_FIGHTING	; Fighting against
	STRUCT	LS_PRE,80		; Pre-rank descript
	STRUCT	LS_POST,80	; Post-rank descript
	APTR	LS_ARR		; Pointer to arrive string
	APTR	LS_DEP		; Pointer to depart string
	LABEL	LS_SIZE


	ENDC	; AMUD_STCT_I
