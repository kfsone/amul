//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: h/Screen.H	Custom Screen Definitions (from PFE! Hehe)
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

#define	ns NewScreenStructure
#define	nw NewWindowStructure1

struct NewScreen NewScreenStructure = {
	0,0,	// screen XY origin relative to View
	0,0,	// screen width and height
	1,	// screen depth (number of bitplanes)
	0,1,	// detail and block pens
	HIRES,	// display modes for this screen
	CUSTOMSCREEN,	// screen type
	NULL,	// pointer to default screen font
	NULL,	// No screen title
	NULL,	// first in list of custom screen gadgets
	NULL	// pointer to custom BitMap structure
};

#define NEWSCREENSTRUCTURE NewScreenStructure

struct NewWindow NewWindowStructure1 = {
	0,11,	// window XY origin relative to TopLeft of screen
	0,0,	// window width and height
	0,1,	// detail and block pens
	NULL,	// IDCMP flags
	ACTIVATE+BACKDROP+GIMMEZEROZERO+RMBTRAP+BORDERLESS+NOCAREREFRESH,	// other window flags
	NULL,	// first gadget in gadget list
	NULL,	// custom CHECKMARK imagery
	NULL,	// window title
	NULL,	// custom screen pointer
	NULL,	// custom bitmap
	-1,-1,	// minimum width and height
	-1,-1,	// maximum width and height
	CUSTOMSCREEN	// destination screen type
};

USHORT Palette[] =
{
	0x0000,	// color #0
	0x0FFF	// color #1
#define PaletteColorCount 2
};

#define PALETTE Palette
