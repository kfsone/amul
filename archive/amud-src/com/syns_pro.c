//
// AMUD/com/Syns_Proc.C		Process Syns.Txt (Synonyms/Pseudonyms)
//

syn_proc() {
	register char *p,*s; short int no; register short int x;

	syns=0;	if(nextc(0)==-1) return;
	fopenw(synsfn); fopenw(synsifn);

	cleanget(&datal,&data,0L); p=skipspc(data); tx("Proc:");

	do {
loop:		p=skipline(s=p); if(!*s) continue;
		s=getword(s); if(!*Word) goto loop;

		if((no=isnoun(Word)) < 0) {
			if((x=is_verb(Word))==-1) {
				error("Invalid verb/noun: \"%s\".\n",Word); continue;
			}
			no=-(2+x);
		}

		do {
			s=getword(s); if(!*Word) break;
			fwrite((char *)&no,1,sizeof(short int),ofp2);
			fprintf(ofp1,"%s%c",Word,0); syns++;
		} while(*s);
	} while(*p);
	errabort();
}
