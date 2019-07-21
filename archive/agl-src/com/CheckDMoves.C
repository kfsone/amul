//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: CheckDMoves.C	Checks that the "dmove" flags are valid
//
//	LMod: oliver 11/06/93	AMUL -> AGL
//

checkdmoves() {
	register int n; struct _ROOM_STRUCT *roomptr;

	fopenr(rooms2fn);
	roomptr=rmtab;
	for(n=0;n<rooms;n++) {
		if(roomptr->flags & DMOVE) {
			fseek(ifp,roomptr->desptr,0);
			fread(dmove,IDL,1,ifp);	// Read the DMOVE name
			if(isroom(dmove)==-1) {	// Is it a valid room?
				sprintf(block,"%-9s: invalid DMOVE '%s'\n",
					roomptr->id,dmove);
				tx(block); dchk=-1;
			}
		}
		roomptr++;
	}
	if(dchk==-1)
		quit(0*tx("\nCompile failed due to invalid DMOVE flags!\n"));
}
