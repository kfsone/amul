#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

 /* Process ROOMS.TXT */
void
room_proc()
{
	char c;
	char lastc, *p, *p2;
	int  n;

	rooms = 0;
	nextc(1); /* Skip any headings etc */

	fopenw(Resources::Compiled::roomData());
	fopenw(Resources::Compiled::roomDesc());

	do {
		rooms++;
		p = block;
		while ((c = fgetc(ifp)) != EOF && !isspace(c))
			*(p++) = c;
		*p = 0; /* Set null byte */
		striplead("room=", block);
		if (strlen(block) < 3 || strlen(block) > IDL) {
			printf("!! \x07 Invalid ID: \"%s\"\x07 !!\n", block);
			quit();
		}
		strcpy(room.id, block);
		/* Do the flags */
		room.flags = 0;
		room.tabptr = -1;
		temp[0] = 0;
		if (c != '\n') {
			fgets(block, 1024, ifp);
			p = block;
			n = -1;
			do {
				while (isspace(*p) && *p != 0)
					p++;
				if (*p == 0)
					continue;
				p2 = p;
				while (!isspace(*p2) && *p2 != 0)
					p2++;
				*p2 = 0;
				if (n == 0) /* Get dmove param */
				{
					strcpy(temp, p);
					dmoves++;
					p = p2 + 1;
					n = -1;
					continue;
				}
				if ((n = isrflag(p)) == -1) {
					printf("\x07'%s' is invalid!\n\n\x07", p);
					quit();
				}
				n -= NRNULL;
				if (n >= 0)
					room.flags = (room.flags | bitset(n));
				p = p2 + 1;
			} while (*p != 0);
		}

		lastc = '\n';
		fseek(ofp2, 0, 1);
		room.desptr = ftell(ofp2);
		n = 0;
		if (temp[0] != 0)
			fwrite(temp, IDL, 1, ofp2); /* save dmove */
		while ((c = fgetc(ifp)) != EOF && !(c == '\n' && lastc == '\n')) {
			if (lastc == '\n' && c == 9)
				continue;
			fputc((lastc = c), ofp2);
			n++;
		};
		fputc(0, ofp2);
		fwrite(room.id, sizeof(room), 1, ofp1);
		nextc(0);
	} while (c != EOF);
	close_ofps();
}
