//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Room_Proc.C	Process Rooms.Txt
//
//	LMod: oliver 11/06/93	AMUL->AGL. Uses memory instead of disk access
//

room_proc() {
	register char	*p,*s;
	register int	n;

	rooms=0;	nextc(1);		// Skip any headings etc

	fopenw(rooms1fn); fopenw(rooms2fn);
	blkget(&datal,&data,0L); p=data; tx("Proc:");

	do {
		p=skipline(s=p); clean_up(s);
		s=getword(skiplead("room=",skipspc(s))); if(!*Word) continue;
		if(strlen(Word)<3 || strlen(Word)>IDL) {
			error("Invalid ID: %s\n",Word);
			p=skipdata(p); continue;
		}
		strcpy(room.id,Word);
		// Do the flags
		room.flags=0; room.tabptr=-1; temp[0]=0; n=-1;
		while(*s) {
			s=getword(s);
			if(!n) {	// Get dmove param
				strcpy(temp,Word); dmoves++; n=-1; continue;
			}
			if((n=isrflag(Word))==-1) {
				error("Invalid room flag '%s'.\n",Word); continue;
			}
			n-=NRNULL;
			if(n>=0) room.flags=(room.flags | (1<<n));
		}

		room.desptr=ftell(ofp2);
		if(*temp) fwrite(temp,IDL,1,ofp2);	// save dmove
		if(!(room.flags&DEATH)) {
			p=skipline(s=p);
			if(*s==9) s++;
			if(*s) fprintf(ofp2,s); fputc('\n',ofp2);
		}
		p=text_proc(p,ofp2,' '); rooms++;
	}while(c!=EOF);
	errabort();
}
