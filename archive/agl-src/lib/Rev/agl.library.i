VERSION		EQU	0
REVISION	EQU	3
DATE	MACRO
		dc.b	'24.10.92'
	ENDM
VERS	MACRO
		dc.b	'agl.library 0.3'
	ENDM
VSTRING	MACRO
		dc.b	'agl.library 0.3 (24.10.92)',13,10,0
	ENDM
VERSTAG	MACRO
		dc.b	0,'$VER: agl.library 0.3 (24.10.92)',0
	ENDM
