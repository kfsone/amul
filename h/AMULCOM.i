VERSION     EQU 0
REVISION    EQU 8992
DATE    MACRO
        dc.b    '29.5.91'
    ENDM
VERS    MACRO
        dc.b    'AMULCOM 0.8992'
    ENDM
VSTRING MACRO
        dc.b    'AMULCOM 0.8992 (29.5.91)',13,10,0
    ENDM
VERSTAG MACRO
        dc.b    0,'$VER: AMULCOM 0.8992 (29.5.91)',0
    ENDM
