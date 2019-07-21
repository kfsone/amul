//
// AMUD/com/CheckDMoves.C	Function to check "Dead MOVE" flags
//

checkdmoves() {
	register int n; struct _ROOM_STRUCT *roomptr;

	fopenr(rooms2fn);	// Open desc. file
	for(n=0,roomptr=rmtab;n<rooms;n++,roomptr++) {
		if(!(roomptr->flags & DMOVE)) continue;
		fseek(ifp,roomptr->desptr,0);
		fread(dmove,IDL,1,ifp);	// Read the DMOVE name
		if(isroom(dmove)==-1) {	// Is it a valid room?
			sprintf(block,"%-9s: invalid DMOVE '%s'\n",
					roomptr->id,dmove);
			tx(block); dchk=-1;
		}
	}
	if(dchk==-1) quit(0*tx("\nCompile failed due to invalid DMOVE flags!\n"));
}
