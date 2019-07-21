//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: SMsg_Proc.C	System Message (SysMsg.Txt) Processor
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

smsg_proc() {
	register char	*p,*s;	register long id; long pos;

	smsgs=0;
	if(nextc(0)==-1) return;	// Nothing to process!
	fopenw(umsgifn); fopenw(umsgfn);	// Text and index

	blkget(&datal,&data,0L); p=skipspc(data); tx("Proc:");

	do {
		p=skipline(s=p); if(!*s) continue;
		getword(skiplead("msgid=",skipspc(s))); if(!*Word) continue;

		if(Word[0]!='$') {
			error("Invalid SysMsg ID '%s'!\n",Word); p=skipdata(p); continue;
		}
		if(atoi(Word+1)!=smsgs+1) {
			error("Message %s out of sequence!\n",Word); p=skipdata(p); continue;
		}
		id=++smsgs;	// Now copy the text across
		pos=ftell(ofp2); fwrite((char *)&pos,4,1,ofp1);
		p=text_proc(p,ofp2,'\n');
	} while(*p && smsgs<NSMSGS);
	if(smsgs!=NSMSGS) error("%ld message(s) missing!\n\n",NSMSGS-smsgs);
	errabort();
}

