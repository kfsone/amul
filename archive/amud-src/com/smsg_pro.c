//
// AMUD/com/Smsg_Proc.C		Smsg.Txt (System Messages) processor
//
// Note: System messages currently have to be stored in numerical order, and
// all messages MUST be provided. Ensure that distribution copies of AMUD are
// supplied with a sample of Smsg.Txt

smsg_proc() {
	register char	*p,*s;	register long id; long pos;

	smsgs=0;
	if(nextc(0)==-1) return;		// Nothing to process!
	fopenw(umsgifn); fopenw(umsgfn);	// Text and index

	blkget(&datal,&data,0L); p=skipspc(data); tx("Proc:");

	do {
loop:		p=skipline(s=p); if(!*s) continue;
		s=skipspc(s); if(!*s || *s==';') goto loop;
		s=skiplead("msgid=",s); getword(s);
		if(!*Word) break;

		if(Word[0]!='$') {
			error("Invalid SysMsg ID '%s'!\n",Word); p=skipdata(p); continue;
		}
		if(atoi(Word+1)!=smsgs+1) {
			error("Message %s out of sequence!\n",Word); p=skipdata(p); continue;
		}
		id=++smsgs;			// Now copy the text across
		pos=ftell(ofp2); fwrite((char *)&pos,4,1,ofp1);
		p=text_proc(p,ofp2);
	} while(*p && smsgs<NSMSGS);
	if(smsgs!=NSMSGS) error("%ld message(s) missing!\n\n",NSMSGS-smsgs);
	errabort();
}

