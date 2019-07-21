//
// AMUD/com/Room_Proc.C		Process Rooms.Txt
//

room_proc() {
	register char	lastc,*p;
	register int	n;

	rooms=0;	nextc(1);		// Skip any headings etc

	fopenw(rooms1fn);	fopenw(rooms2fn); tx("Proc:");

	do {
		rooms++;
loop:		fgets(block,1000,ifp); clean_trim(block);
		p=skiplead("room=",block); p=getword(p);
		if(strlen(Word)<3 || strlen(Word)>IDL) {
			error("Invalid ID: %s\n",Word); skipblock(); goto end;
		}
		strcpy(room.id,Word);
		// Process flags
		room.flags=0; room.tabptr=-1; temp[0]=0; n=-1;
		while(*p) {
			p=getword(p);
			if(!n) {	// Get dmove param 
				strcpy(temp,Word); dmoves++; n=-1; continue;
			}
			if((n=isrflag(Word))==-1) {
				error("Invalid room flag '%s'.\n",Word); continue;
			}
			n-=NRNULL;
			if(n>=0) room.flags=(room.flags | 1<<n);
		}

		lastc='\n';
		fseek(ofp2,0,1);room.desptr=ftell(ofp2);
		if(*temp) fwrite(temp,IDL,1,ofp2);	// save dmove
		fgets(block,80,ifp);	// Get short desc
		if(*block=='\n') goto descend; n=FALSE;
		fprintf(ofp2,block);
		while((c=fgetc(ifp))!=EOF && !(c=='\n' && lastc=='\n')) {
			if(c==9) continue;
			if(lastc=='\n') { if(n) fputc(32,ofp2); n=TRUE; }
			if((lastc=c)!='\n') fputc(c,ofp2);
		}
descend:	fputc(0,ofp2); fwrite(room.id,sizeof(room),1,ofp1);
end:		nextc(0);
	}while(c!=EOF);
	errabort();
}
