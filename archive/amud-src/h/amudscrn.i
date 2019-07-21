;;
;; AMUD.Scrn.H -- Custom screen & window definitions for AMUD screens etc
;;

	IFND	GRAPHICS_VIEW_I
	INCLUDE	"graphics/view.i"
	ENDC

Scr
	dc.w	0,0,640,260,1		; X, Y, W, H, Depth
	dc.b	0,1			; Detail, Block (pens)
	dc.w	V_HIRES,CUSTOMSCREEN	; View Modes, Screen Type
	dc.l	0,0,0,0			; Font, Title, Gadgets & Bitmap

Win
	dc.w	0,11,0,0		; X,Y,W,H
	dc.b	0,1			; Detail, Block
	dc.l	0			; IDCMP flags
	dc.l	ACTIVATE+BACKDROP+GIMMEZEROZERO+RMBTRAP+BORDERLESS+NOCAREREFRESH
	dc.l	0,0,0			; Gadgets, checkmarks, title
	dc.l	0			; Screen pointer
	dc.l	0			; custom bitmap
	dc.w	-1,-1,-1,-1		; Min. window settings
	dc.w	CUSTOMSCREEN

Palette	dc.w	$000,$fff		; Palette colours
PColors	equ	2

