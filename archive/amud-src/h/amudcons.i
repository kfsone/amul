; -------------------------------------------------------------------------- ;
; |-|  amud constant definitions Copyright (C) KingFisher Software 1992  |-| ;
; -------------------------------------------------------------------------- ;

;;
;;  Based on AMUD.Cons.H - Version 1, Revision 1
;;

	IFND AMUD_CONS_I
AMUD_CONS_I	SET	1

MANNAM	MACRO
	dc.b	"AMan Port",0
	ENDM

PLYRFN	MACRO
	dc.b	"PlayerData",0
	ENDM

	ENDC	; AMUD_CONS_I
