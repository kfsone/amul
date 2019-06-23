#include "amulcominc.h"

void
checkdmoves()
{
	int			  n;
	_ROOM_STRUCT *roomptr;

	/*=* Check RF_CEMETERY ptrs *=*/
	fopenr(rooms2fn); /* Open desc. file */
	roomptr = rmtab;
	for (n = 0; n < rooms; n++) {
		if (roomptr->flags & RF_CEMETERY) {
			sprintf(block, "%-9s\x0d", roomptr->id);
			tx(block);
			fseek(ifp, roomptr->desptr, 0);
			fread(dmove, IDL, 1, ifp); /* Read the RF_CEMETERY name */
			if (isroom(dmove) == -1)   /* Is it a valid room? */
			{
				sprintf(block, "%-9s - invalid RF_CEMETERY '%s'...\n", roomptr->id, dmove);
				tx(block);
				dchk = -1;
			}
		}
		roomptr++;
	}
	if (dchk == -1)
		quit(0 * tx("\nCompile failed due to invalid RF_CEMETERY flags!\n"));
}
