//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: UMsg_Proc.C	Process UMsg.Txt (User Messages)
//
//	LMod: oliver 11/06/93	AMUL->AGL
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
		p=skipline(s=p); if(!*s) continue;
		clean_up(s); s=skipspc(s); if(!*s || *s==';') continue;
		getword(skiplead("msgid=",s)); if(!Word[0]) continue;

		if(Word[0]=='$') {
			error("Invalid ID, '%s'.\n",Word);
			p=skipdata(p); continue;
		}
		if(strlen(Word)>IDL) {
			error("ID \"%s\" too long.\n",Word,IDL);
			p=skipdata(p); continue;
		}
		umsgs++;	// Now copy the text across
		strcpy(umsg.id,Word); umsg.fpos=ftell(ofp2); fwrite(umsg.id,sizeof(umsg),1,umfp);
		fwrite((char *)&umsg.fpos,4,1,ofp1);
		p=text_proc(p,ofp2,'\n');
	} while(*p);
	errabort();
}

ttumsgchk(char *s) {
	s=skiplead("msgid=",s); s=skipspc(s);
	if(*s=='\"' || *s=='\'') return msgline(s+1);
	return chkumsg(s);
}

chkumsg(char *s)
{	register int i; FILE *fp;

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
	FILE *fp; long pos; char c;
	fp=afp; afp=NULL; fopena(umsgfn); fseek(afp,0,2L); pos=ftell(afp);

	fwrite(s,strlen(s)-1,1,afp);
	if((c=*(s+strlen(s)-1))!='{') { fputc(c,afp); fputc('\n',afp); }
	fputc(0,afp);
	fopena(umsgifn); fseek(afp,0,2L); fwrite((char *)&pos,4,1,afp);
	fclose(afp); afp=fp;
	return NSMSGS+(umsgs++);
}
