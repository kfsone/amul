//
// AMUD/com/Umsg_Proc.C		Process user messages (Umsg.Txt)
//

umsg_proc() {
	register char *p,*s;

	umsgs=0;
	fopenw("-ram:umsg.tmp"); close_ofps();
	fopena(umsgifn); ofp1=afp; afp=NULL; fseek(ofp1,0,2L);
	fopena(umsgfn);  ofp2=afp; afp=NULL; fseek(ofp2,0,2L);
	fopena("-ram:umsg.tmp"); umfp=afp; afp=NULL;
	if(nextc(0)==-1) { close_ofps(); return 0; }	// None to process
	blkget(&datal,&data,0L); p=data; tx("Proc:");

	do {
loop:		p=skipline(s=p); if(!*s) continue;
		clean_up(s); s=skipspc(s); if(!*s || *s==';') goto loop;
		s=skiplead("msgid=",s); getword(s);
		if(Word[0]==0) goto loop;

		if(Word[0]=='$') {
			error("Invalid ID, '%s'.\n",Word);
			p=skipdata(p); goto loop;
		}
		if(strlen(Word)>IDL) {
			error("ID \"%s\" too long.\n",Word,IDL);
			p=skipdata(p); goto loop;
		}
		umsgs++;	// Now copy the text across
		strcpy(umsg.id,Word); umsg.fpos=ftell(ofp2); fwrite(umsg.id,sizeof(umsg),1,umfp);
		fwrite((char *)&umsg.fpos,4,1,ofp1);
		p=text_proc(p,ofp2);
	} while(*p);
	errabort();
}

ttumsgchk(char *s) {
	s=skiplead("msgid=",s); s=skipspc(s);
	if(*s=='\"' || *s=='\'') return msgline(s+1);
	return chkumsg(s);
}

chkumsg(char *s) {
	register int i;

	if(!strncmp("none",s,4) && (!*(s+4) || !isalnum(*(s+4)))) return -2;
	if(*s!='$' && !umsgs) return -1;

	if(*s=='$') {
		i=atoi(s+1);
		if(i<1 || i>NSMSGS) {
			error("Invalid SysMsg ID '%s'.\n",s); return -1;
		}
		return i-1;
	}
	if(!umsgs) return -1;
	fseek(umfp,0,0L);	// Rewind file
	for(i=0; i<umsgs; i++) {
		fread(umsg.id,sizeof(umsg),1,umfp);
		if(!strcmp(umsg.id,s)) return i+NSMSGS;
	}
	return -1;
}

msgline(char *s) {
	long pos; char c;
	fseek(msg1,0,2L); pos=ftell(msg1); fwrite(s,strlen(s)-1,1,msg1);
	if((c=*(s+strlen(s)-1))!='{') { fputc(c,msg1); fputc('\n',msg1); }
	fputc(0,msg1);
	fseek(msg2,0,2L); fwrite((char *)&pos,4,1,msg2);
	return NSMSGS+(umsgs++);
}
