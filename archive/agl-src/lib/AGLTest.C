/**
 *
 * "agl.library" demo #1	(C) KingFisher Software D&EG International 1992
 *
 *                   A l l   R i g h t s   R e s e r v e d
 *
 * Demonstrates using agl.library using the #pragmas. We love Lattice 'C'!
 *
**/

#include <stdio.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <agllib:proto/AGL.H>			/* #pragmas */
#include <adv:h/AGL.Defs.H>			/* AGL #defines. */
#include <adv:h/AGL.Stct.H>			/* Structure defs */
#include <intuition/intuition.h>

#include <agllib:libraries/AGL.H>		/* prototypes */

struct Screen *sc; struct Window *win;
struct Library *AGLBase,*IntuitionBase,*GfxBase,*DOSBase,*OpenLibrary();
struct MsgPort *port,*reply;
struct Aport *agl;
char	**errtxt;

main()
{	struct Library **base; register char *p; register int i;

	if(!(AGLBase=OpenLibrary(AGLNAME,0L))) {
		printf("Unable to open agl.library.\n");
		exit(0);
	}

test1:	printf("1] Test the LibTable() command.\n"); base=LibTable();
	printf(" >  Using openlibrary: DOSBase=%lx, IntuitionBase=%lx, GfxBase=%lx.\n",
		DOSBase=OpenLibrary("dos.library",0L),
		IntuitionBase=OpenLibrary("intuition.library",0L),
		GfxBase=OpenLibrary("graphics.library",0L));
	CloseLibrary(DOSBase); CloseLibrary(IntuitionBase); CloseLibrary(GfxBase);

	printf(" >  using table: DOSBase=%lx, IntuitionBase=%lx, GfxBase=%lx.\n\n",
		*(base++),*(base++),*(base++));

test2:	printf("2] Check out the TextPtr() function!\n"); errtxt=TextPtr();
	printf(" >  TextPtr=%lx.\n",errtxt);
	printf(" >   1: %s",*(errtxt));
	printf(" >   2: %s",*(errtxt+1));
	printf(" >   3: %s",*(errtxt+2));

test3:	printf("3] Check out the CommsPrep() function!\n");

	if((p=CommsPrep(&port,&reply,&agl))!=NULL) {
		printf(" *  Failed : %s",p); goto test4;
	}
	printf(" >  port=%lx, reply=%lx, agl=%lx.\n",port,reply,agl);
	DeleteAPort(reply,agl);

test4:	printf("%c] Check CreateAPort/DeleteAPort (%s Port)... ",'4',"Public");

	if(!(port=CreateAPort("Test Port"))) {
		printf("* Failed!\n"); goto test5;
	}
	printf("Got %lx.\n",port);
	DeleteAPort(port,0L);
test5:	printf("%c] Check CreateAPort/DeleteAPort (%s Port)... ",'5',"Private");
	if((port=CreateAPort(0L))==NULL) {
		printf("* Failed!\n"); goto test6;
	}
	printf("Got %lx.\n",port);
	DeleteAPort(port,0L);
test6:	printf("6] Check the Random() functions...\n >  ");
	for(i=0; i<9; i++) printf("%ld, ",Random(100)); printf("%ld.\n",Random(100));
test7:	printf("7] Check the GetLocal() function... ");
	if((p=GetLocal(&sc,&win,"This is a test screen!"))) {
		printf("* Failed! %s",p); goto test8;
	}
	Delay(200);
	printf("Successful (win=%x, sc=%x).\n",win,sc);
	CloseWindow(win); CloseScreen(sc);
test8:	CloseLibrary(AGLBase);
	printf("\n\/*-- End of code --*\/\n");
}
