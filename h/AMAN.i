VERSION     EQU 0
REVISION    EQU 590
DATE    MACRO
        dc.b    '29.5.91'
    ENDM
VERS    MACRO
        dc.b    'AMAN 0.590'
    ENDM
VSTRING MACRO
        dc.b    'AMAN 0.590 (29.5.91)',13,10,0
    ENDM
VERSTAG MACRO
        dc.b    0,'$VER: AMAN 0.590 (29.5.91)',0
    ENDM