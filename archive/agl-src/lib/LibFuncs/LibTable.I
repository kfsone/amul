; ีอัออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออัอธ ;
; ณþณ   agl.library Functions. Copyright (C) 1993, KingFisher Software   ณþณ ;
; ิอฯออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออฯอพ ;

******* agl.library/LibTable ******************************************
*
*   NAME
*	LibTable -- Returns a pointer to a table of library bases.
*
*   SYNOPSIS
*	bases = LibTable()
*	 d0
*
*	struct Library **LibTable( void );
*
*   FUNCTION
*	Returns a pointer to a table of Library Bases, saving you
*	having to test for them. If agl.library is open it follows
*	that these libraries are available. Until you close agl.library
*	you are guaranteed their availability.
*
*   RESULT
*	bases - Pointer to the table of Library Bases. They are:
*
*		bases+0 -> OpenLibrary("dos.library",0L);
*		bases+4 -> OpenLibrary("intuition.library",0L);
*		bases+8 -> OpenLibrary("graphics.library",0L);
*
*   EXAMPLE
*	base=LibTable(); DOSBase=*(base++); IntuitionBase=*(base++);
*	GfxBase=*(base++);
*
*   NOTES
*	More ROM libraries may be added to this list in future.
*
******************************************************************************
*
*

LibTable:
	lea	_DOSBase,a0
	move.l	a0,d0
	rts

