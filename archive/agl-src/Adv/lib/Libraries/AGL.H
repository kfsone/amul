/*
  --------------------------------------------------------------------------
  |-| agl.library include file. Copyright (C) 1993, KingFisher Software! |-|
  --------------------------------------------------------------------------*/

#define	AGLNAME	"agl.library"

struct Library **LibTable();
char	**TextPtr();
char	*CommsPrep(struct MsgPort **,struct MsgPort **,struct Aport **);
struct MsgPort *CreateAPort(char *);
void	DeleteAPort(struct MgPort*, struct Aport *);
int	Random(int);
char	*GetLocal(struct Screen **,struct Window **,char *);

extern	struct Library *AGLBase;

/*
	Library pointer offsets from LibTable return
*/

#define	DOSBoff	0		/* DOSBase=LibTable()+0; */
#define	INTBoff	1		/* IntuitionBase=LibTable()+1; */
#define	GFXBoff	2		/* GfxBase=LibTable()+2; */

/*
	Error text offsets for use with TextPtr()
*/

#define	NoAMAN		0		/* Manager not running */
#define	NoPORT		1		/* Couldn't create comms port */
#define	NoMEM		2		/* Out of memory */
#define	NoSCRN		3		/* Can't open screen */
#define	NoWIN		4		/* Can't open window */

	/* -- end of AGL_Lib.H -- */
