/*

	Actual definitions
	------------------


These are pretty important... The 'actuals' allow the end user to refer to
variable elements of the adventure... Such as 'noun1', 'adj2', 'room of object
the box', 'current state of door', 'no. of player carrying' etc...

*/

/* Actual flags */

#define	IWORD		0x00100000		/* one of users words	*/
#define	MEPRM		0x00200000		/* user parameter	*/
#define	OBVAL		0x00400000		/* Object weight	*/
#define	OBDAM		0x00800000		/* Object damage	*/
#define	OBWHT		0x01000000		/* Object weight	*/
#define RAND0		0x02000000		/* Range 0 to <num>	*/
#define	RAND1		0x04000000		/* <num/2> to <num*1.5>	*/
#define	OBLOC		0x08000000		/* Location of obj	*/
#define	PRANK		0x10000000		/* Rank of player	*/

/* 'i' words */
#define	IVERB		0
#define	IADJ1		1
#define	INOUN1		2
#define	IPREP		3
#define	IADJ2		4
#define	INOUN2		5

/* meprms */
#define	LOCATE		0			/* Value of locate	*/
#define	SELF		1			/* My user no.		*/
#define	HERE		2			/* My room		*/
#define	RANK		3			/* # of my rank		*/
#define	FRIEND		4			/* Who I am helping	*/
#define	HELPER		5			/* People helping me	*/
#define	ENEMY		6			/* Who I'm fighting	*/
#define	WEAPON		7			/* My current weapon	*/
#define	SCORE		8			/* My score		*/
#define	SCTG		9			/* My score this game	*/
#define	STR		10			/* My strength		*/
#define	LASTROOM	11			/* Last room I was at!	*/
#define	LASTDIR		12			/* Last direction I did	*/
#define	LASTVB		13			/* Last verb I did!	*/

/* Numbers */
#define LESS		0x40000000		/* <Beyond max lim>	*/
#define MORE		0x80000000		/* <Beyond max lim>	*/
#define	NONE		0,0,0

#ifdef COMPILER
#define NACTUALS	33			/* No. of actual names	*/
typedef struct ACTUAL		/* Structure for actual data */
{
	char	*name;				/* Name of actual	*/
	int	value;				/* The effective no.	*/
  short int	wtype;				/* Wtype of word	*/
};

struct ACTUAL actual[NACTUALS]=
{
	"verb",	  IWORD+IVERB,	TC_VERB,
	"adj",	  IWORD+IADJ1,	TC_ADJ,	  "adj1",   IWORD+IADJ1, TC_ADJ,
	"noun",	  IWORD+INOUN1, TC_NOUN,	  "noun1",  IWORD+INOUN1,TC_NOUN,
	"player", IWORD+INOUN1, TC_PLAYER,  "player1",IWORD+INOUN1,TC_PLAYER,
	"text",	  IWORD+INOUN1, TC_TEXT,	  "text1",  IWORD+INOUN1,TC_TEXT,
	"room",	  IWORD+INOUN1, TC_ROOM,	  "room1",  IWORD+INOUN1,TC_ROOM,
	"number", IWORD+INOUN1, WC_NUMBER,  "number1",IWORD+INOUN1,WC_NUMBER,
	"prep",	  IWORD+IPREP,	TC_PREP,
	"adj2",	  IWORD+IADJ2,	TC_ADJ,
	"noun2",  IWORD+INOUN2, TC_NOUN,
	"player2",IWORD+INOUN2, TC_PLAYER,
	"text2",  IWORD+INOUN2, TC_TEXT,
	"room2",  IWORD+INOUN2, TC_ROOM,
	"locate", MEPRM+LOCATE, TC_NOUN,
	"me",	  MEPRM+SELF,	TC_PLAYER,
	"here",	  MEPRM+HERE,	TC_ROOM,
	"myrank", MEPRM+RANK,	WC_NUMBER,
	"friend", MEPRM+FRIEND,	TC_PLAYER,
	"helper", MEPRM+HELPER,	TC_PLAYER,
	"enemy",  MEPRM+ENEMY,	TC_PLAYER,
	"weapon", MEPRM+WEAPON,	TC_NOUN,
	"myscore",MEPRM+SCORE,	WC_NUMBER,
	"mysctg", MEPRM+SCTG,	WC_NUMBER,
	"mystr",  MEPRM+STR,	WC_NUMBER,
	"lastroom",MEPRM+LASTROOM,TC_ROOM,
	"lastdir",MEPRM+LASTDIR,TC_VERB,
	"lastverb",MEPRM+LASTVB,TC_VERB
};
#endif
