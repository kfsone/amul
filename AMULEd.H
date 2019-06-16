
static struct NewScreen NewScreenStructure = {
	0,0,
	640,200,
	2,
	1,0,
	HIRES,
	CUSTOMSCREEN,
	NULL,
	"=== AMULEd v1.0 Copyright (C) Richard Pike/KingFisher Software, 1991 ===",
	NULL,
	NULL
};

#define NEWSCREENSTRUCTURE NewScreenStructure

static USHORT Palette[] = {
	0x0000,
	0x0FFF,
	0x0887,
	0x0FE1
#define PaletteColorCount 4
};

#define PALETTE Palette

static UBYTE AEAErankSIBuff[10];
static struct StringInfo AEAErankSInfo = {
	AEAErankSIBuff,
	NULL,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors1[] = {
	0,0,
	89,0,
	89,9,
	0,9,
	0,0
};
static struct Border AEBorder1 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors1,
	NULL
};

static struct TextAttr TOPAZ80 = {
	(STRPTR)"topaz.font",
	TOPAZ_EIGHTY,0,0
};
static struct IntuiText AEIText1 = {
	3,0,JAM1,
	-84,1,
	&TOPAZ80,
	"Rank......",
	NULL
};

static struct Gadget AErank = {
	NULL,
	542,46,
	88,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	(APTR)&AEBorder1,
	NULL,
	&AEIText1,
	NULL,
	(APTR)&AEAErankSInfo,
	24,
	NULL
};

static UBYTE AEAEscoreSIBuff[10];
static struct StringInfo AEAEscoreSInfo = {
	AEAEscoreSIBuff,
	NULL,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors2[] = {
	0,0,
	89,0,
	89,9,
	0,9,
	0,0
};
static struct Border AEBorder2 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors2,
	NULL
};

static struct IntuiText AEIText2 = {
	3,0,JAM1,
	-84,1,
	&TOPAZ80,
	"Score.....",
	NULL
};

static struct Gadget AEscore = {
	&AErank,
	362,46,
	88,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	(APTR)&AEBorder2,
	NULL,
	&AEIText2,
	NULL,
	(APTR)&AEAEscoreSInfo,
	23,
	NULL
};

static UBYTE AEAEusernoSIBuff[10];
static struct StringInfo AEAEusernoSInfo = {
	AEAEusernoSIBuff,
	NULL,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors3[] = {
	0,0,
	89,0,
	89,9,
	0,9,
	0,0
};
static struct Border AEBorder3 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors3,
	NULL
};

static struct IntuiText AEIText3 = {
	3,0,JAM1,
	-56,1,
	&TOPAZ80,
	"User #",
	NULL
};

static struct Gadget AEuserno = {
	&AEscore,
	542,16,
	88,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	(APTR)&AEBorder3,
	NULL,
	&AEIText3,
	NULL,
	(APTR)&AEAEusernoSInfo,
	99,
	NULL
};

static UBYTE AEAEplayedSIBuff[10];
static struct StringInfo AEAEplayedSInfo = {
	AEAEplayedSIBuff,
	NULL,
	0,
	10,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors4[] = {
	0,0,
	89,0,
	89,9,
	0,9,
	0,0
};
static struct Border AEBorder4 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors4,
	NULL
};

static struct IntuiText AEIText5 = {
	2,0,JAM1,
	111,0,
	&TOPAZ80,
	"times",
	NULL
};

static struct IntuiText AEIText4 = {
	3,0,JAM1,
	-84,1,
	&TOPAZ80,
	"Played....",
	&AEIText5
};

static struct Gadget AEplayed = {
	&AEuserno,
	88,46,
	88,8,
	NULL,
	RELVERIFY+STRINGRIGHT+LONGINT,
	STRGADGET,
	(APTR)&AEBorder4,
	NULL,
	&AEIText4,
	NULL,
	(APTR)&AEAEplayedSInfo,
	22,
	NULL
};

static UBYTE AEAEpasswordSIBuff[23];
static struct StringInfo AEAEpasswordSInfo = {
	AEAEpasswordSIBuff,
	NULL,
	0,
	23,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors5[] = {
	0,0,
	185,0,
	185,9,
	0,9,
	0,0
};
static struct Border AEBorder5 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors5,
	NULL
};

static struct IntuiText AEIText6 = {
	3,0,JAM1,
	-84,1,
	&TOPAZ80,
	"Password..",
	NULL
};

static struct Gadget AEpassword = {
	&AEplayed,
	88,33,
	184,8,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&AEBorder5,
	NULL,
	&AEIText6,
	NULL,
	(APTR)&AEAEpasswordSInfo,
	21,
	NULL
};

static UBYTE AEAEnameSIBuff[21];
static struct StringInfo AEAEnameSInfo = {
	AEAEnameSIBuff,
	NULL,
	0,
	21,
	0,
	0,0,0,0,0,
	0,
	0,
	NULL
};

static SHORT AEBorderVectors6[] = {
	0,0,
	185,0,
	185,9,
	0,9,
	0,0
};
static struct Border AEBorder6 = {
	-1,-1,
	2,0,JAM1,
	5,
	AEBorderVectors6,
	NULL
};

static struct IntuiText AEIText7 = {
	3,0,JAM1,
	-84,1,
	&TOPAZ80,
	"Name......",
	NULL
};

static struct Gadget AEname = {
	&AEpassword,
	88,20,
	184,8,
	NULL,
	RELVERIFY,
	STRGADGET,
	(APTR)&AEBorder6,
	NULL,
	&AEIText7,
	NULL,
	(APTR)&AEAEnameSInfo,
	20,
	NULL
};

#define AEGadgetList1 AEname

static struct IntuiText AEIText8 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Quit AMUL-Ed",
	NULL
};

static struct MenuItem AEMenuItem12 = {
	NULL,
	0,88,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText8,
	NULL,
	'Q',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText9 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"-------------------",
	NULL
};

static struct MenuItem AEMenuItem11 = {
	&AEMenuItem12,
	0,80,
	152,8,
	ITEMTEXT+HIGHCOMP,
	0,
	(APTR)&AEIText9,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText AEIText10 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"End of File",
	NULL
};

static struct MenuItem AEMenuItem10 = {
	&AEMenuItem11,
	0,72,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText10,
	NULL,
	'=',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText11 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Next User",
	NULL
};

static struct MenuItem AEMenuItem9 = {
	&AEMenuItem10,
	0,64,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText11,
	NULL,
	']',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText12 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Previous User",
	NULL
};

static struct MenuItem AEMenuItem8 = {
	&AEMenuItem9,
	0,56,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText12,
	NULL,
	'[',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText13 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Start of File",
	NULL
};

static struct MenuItem AEMenuItem7 = {
	&AEMenuItem8,
	0,48,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText13,
	NULL,
	'-',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText14 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"-------------------",
	NULL
};

static struct MenuItem AEMenuItem6 = {
	&AEMenuItem7,
	0,40,
	152,8,
	ITEMTEXT+HIGHCOMP+HIGHBOX,
	0,
	(APTR)&AEIText14,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText AEIText15 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Delete User",
	NULL
};

static struct MenuItem AEMenuItem5 = {
	&AEMenuItem6,
	0,32,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText15,
	NULL,
	'D',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText16 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Save User",
	NULL
};

static struct MenuItem AEMenuItem4 = {
	&AEMenuItem5,
	0,24,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText16,
	NULL,
	'S',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText17 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"Find User",
	NULL
};

static struct MenuItem AEMenuItem3 = {
	&AEMenuItem4,
	0,16,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText17,
	NULL,
	'F',
	NULL,
	MENUNULL
};

static struct IntuiText AEIText18 = {
	3,1,COMPLEMENT,
	0,0,
	NULL,
	"-------------------",
	NULL
};

static struct MenuItem AEMenuItem2 = {
	&AEMenuItem3,
	0,8,
	152,8,
	ITEMTEXT+HIGHCOMP+HIGHBOX,
	0,
	(APTR)&AEIText18,
	NULL,
	NULL,
	NULL,
	MENUNULL
};

static struct IntuiText AEIText19 = {
	3,1,COMPLEMENT,
	0,0,
	&TOPAZ80,
	"About AMUL-Ed",
	NULL
};

static struct MenuItem AEMenuItem1 = {
	&AEMenuItem2,
	0,0,
	152,8,
	ITEMTEXT+COMMSEQ+ITEMENABLED+HIGHCOMP,
	0,
	(APTR)&AEIText19,
	NULL,
	'A',
	NULL,
	MENUNULL
};

static struct Menu AEMenu1 = {
	NULL,
	10,0,
	63,0,
	MENUENABLED,
	"Options",
	&AEMenuItem1
};

#define AEMenuList1 AEMenu1

static struct IntuiText AEIText20 = {
	3,0,COMPLEMENT,
	166,3,
	&TOPAZ80,
	"<<< Packing User File - Please Wait >>>",
	NULL
};

#define AEIntuiTextList1 AEIText20

static struct NewWindow AENewWindowStructure1 = {
	0,11,
	640,189,
	3,2,
	GADGETDOWN+GADGETUP+REQSET+MENUPICK+CLOSEWINDOW+REQVERIFY,
	BACKDROP+GIMMEZEROZERO+BORDERLESS+ACTIVATE+NOCAREREFRESH,
	&AEname,
	NULL,
	NULL,
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};

static USHORT __chip REImageData1[] = {
	0x07FC,0x0000,0x3C07,0x8000,0x6000,0xC000,0xC000,0x6000,
	0xC000,0x6000,0xC000,0x6000,0x6000,0xC000,0x3C07,0x8000,
	0x07FC,0x0000,0x07FC,0x0000,0x3C07,0x8000,0x6000,0xC000,
	0xC000,0x6000,0xC000,0x6000,0xC000,0x6000,0x6000,0xC000,
	0x3C07,0x8000,0x07FC,0x0000
};

static struct Image REImage1 = {
	0,0,
	19,9,
	2,
	REImageData1,
	0x0003,0x0000,
	NULL
};

static USHORT __chip REImageData2[] = {
	0x07FC,0x0000,0x3C07,0x8000,0x61F0,0xC000,0xC7FC,0x6000,
	0xCFFE,0x6000,0xC7FC,0x6000,0x61F0,0xC000,0x3C07,0x8000,
	0x07FC,0x0000,0x07FC,0x0000,0x3C07,0x8000,0x61F0,0xC000,
	0xC7FC,0x6000,0xCFFE,0x6000,0xC7FC,0x6000,0x61F0,0xC000,
	0x3C07,0x8000,0x07FC,0x0000
};

static struct Image REImage2 = {
	0,0,
	19,9,
	2,
	REImageData2,
	0x0003,0x0000,
	NULL
};

static struct IntuiText REIText21 = {
	1,0,JAM1,
	-80,0,
	&TOPAZ80,
	"Exit Page",
	NULL
};

static struct Gadget REGadget9 = {
	NULL,
	371,0,
	18,8,
	GADGHIMAGE+GADGIMAGE,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&REImage1,
	(APTR)&REImage2,
	&REIText21,
	NULL,
	NULL,
	21,
	NULL
};

static SHORT REBorderVectors7[] = {
	0,0,
	61,0,
	61,10,
	0,10,
	0,0
};
static struct Border REBorder7 = {
	-1,-1,
	3,0,JAM1,
	5,
	REBorderVectors7,
	NULL
};

static struct IntuiText REIText22 = {
	1,0,JAM2,
	20,1,
	NULL,
	"No!",
	NULL
};

static struct Gadget REGadget8 = {
	&REGadget9,
	86,32,
	60,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&REBorder7,
	NULL,
	&REIText22,
	NULL,
	NULL,
	101,
	NULL
};

static SHORT REBorderVectors8[] = {
	0,0,
	61,0,
	61,10,
	0,10,
	0,0
};
static struct Border REBorder8 = {
	-1,-1,
	3,0,JAM1,
	5,
	REBorderVectors8,
	NULL
};

static struct IntuiText REIText24 = {
	1,0,JAM2,
	17,-17,
	NULL,
	"Are you sure?",
	NULL
};

static struct IntuiText REIText23 = {
	1,0,JAM2,
	18,1,
	NULL,
	"Yes",
	&REIText24
};

static struct Gadget REGadget7 = {
	&REGadget8,
	10,32,
	60,9,
	NULL,
	RELVERIFY,
	BOOLGADGET,
	(APTR)&REBorder8,
	NULL,
	&REIText23,
	NULL,
	NULL,
	100,
	NULL
};

#define REGadgetList2 REGadget7

static struct NewWindow RENewWindowStructure2 = {
	242,84,
	156,46,
	0,2,
	GADGETDOWN+GADGETUP,
	ACTIVATE+RMBTRAP+NOCAREREFRESH,
	&REGadget7,
	NULL,
	"=== Save User? ===",
	NULL,
	NULL,
	5,5,
	-1,-1,
	CUSTOMSCREEN
};
