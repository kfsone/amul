//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: FileBits.C	Miscellaneous AGLCom functions
//
//	LMod: oliver 11/06/93	AMUL -> AGL
//

close_ofps() {
	if(ofp1)	fclose(ofp1);	if(ofp2)	fclose(ofp2);
	if(ofp3)	fclose(ofp3);	if(ofp4)	fclose(ofp4);
	if(ofp5)	fclose(ofp5);	if(afp)		fclose(afp);
	ofp1=ofp2=ofp3=ofp4=ofp5=afp=NULL;
}

nextc(int f) {		// Finds first "real" character in a file
	do {
		while((c=fgetc(ifp))!=EOF && isspace(c));
		if(c==';') fgets(block,1024,ifp);
	} while(c!=EOF && (c==';' || isspace(c)));
	if(f==1 && c==EOF) {
		error("File contains NO data!\n"); quit();
	}
	if(c==EOF) return -1;
	fseek(ifp,-1,1);	/* Move back 1 char */
	return 0;
}

quit() {		// Exit politely
	if(exi!=1) {
		sprintf(block,"%s%s",dir,advfn); unlink(block);
	}
	if(ifp)		fclose(ifp); ifp=NULL;
	if(umfp)	fclose(umfp); umfp=NULL;
	if(msg1)	fclose(msg1); msg1=NULL;
	if(msg2)	fclose(msg2); msg2=NULL;
	unlink("ram:ODIDs"); unlink("ram:umsg.tmp"); unlink("ram:objh.tmp");
	if(mobdat)	FreeMem(mobdat,moblen); mobdat=NULL;
	if(mobp)	FreeMem(mobp,sizeof(mob)*mobchars); mobp=NULL;
	if(rmtab)	FreeMem(rmtab,sizeof(room)*rooms); rmtab=NULL;
	if(data)	FreeMem(data, datal); data=NULL;
	if(data2)	FreeMem(data2,datal2);data2=NULL;
	if(obtab2)	FreeMem(obtab2,obmem); obtab2=NULL;
	if(vbtab)	FreeMem(vbtab,vbmem); vbtab=NULL;
	close_ofps();	exit(0);
}

// The "fopen" functions open a file on the first available IO channel.

fopenw(char *s) {	// Open file for reading
	FILE *tfp;
	if(*s=='-') strcpy(temp,s+1);
	else sprintf(temp,"%s%s",dir,s);
	if(!(tfp=fopen(temp,"wb"))) Err("write",temp);
	if(!ofp1)ofp1=tfp; else if(!ofp2)ofp2=tfp; else if(!ofp3) ofp3=tfp; else if(!ofp4) ofp4=tfp; else ofp5=tfp;
	return NULL;
}

fopena(char *s) {	// Open file for appending
	if(afp) fclose(afp);
	if(*s=='-') strcpy(temp,s+1);
	else sprintf(temp,"%s%s",dir,s);
	if(!(afp=fopen(temp,"rb+"))) Err("append",temp);
	return NULL;
}

fopenr(char *s) {	// Open file for reading
	if(ifp) fclose(ifp);
	if(*s=='-') strcpy(temp,s+1);
	else sprintf(temp,"%s%s",dir,s);
	if(!(ifp=fopen(temp,"rb"))) Err("read",temp);
	return NULL;
}

Err(char *s,char *t) {
	error("FATAL ERROR! Can't %s %s!\n",s,t); quit();
}

rfopen(char *s) {	// Open file for reading
	FILE *fp;

	if(*s!='-') sprintf(temp,"%s%s",dir,s);
	else strcpy(temp,s+1);
	if(!(fp=fopen(temp,"rb"))) Err("open",temp);
	return (long)fp;
}

ttroomupdate() {	// Update room entries after TT
	if(err) return; fseek(afp,0,0L);
	fwrite(rmtab->id,sizeof(room),rooms,afp);
}

skipblock() {
	char c,lc;

	lc=0; c='\n';
	while(c!=EOF && !(c==lc=='\n')) { lc=c; c=fgetc(ifp); }
}

char *skipdata(register char *p) {
	register char *s;
	do p=skipline(s=p); while(*s && *s!=10); return p;
}

tidy(char *s) {
	repspc(s); remspc(s); s=s+strlen(s)-1; while(isspace(*s)) *(s--)=0;
}

clean_trim(char *s) {
	clean_up(s); s=s+strlen(s)-1; while(isspace(*s)) *(s--)=0;
}

is_verb(char *s) {
	register int i;
	if(!verbs) return -1;
	vbptr=vbtab;
	for(i=0;i<verbs;i++,vbptr++) {
		if(!strcmp(vbptr->id,s)) return i;
	}
	if(!strcmp(verb.id,s)) return verbs-1;
	return -1;
}

// func_get : allocates memory for a text file and reads the file into it
func_get(long *s,char **p,long off) {
	*s=(((filesize()+off)/4096)+1)*4096;
	if(!(*p=(char *)AllocMem(*s,STDMEM))) {
		tx("\x07\n** Out of memory!\n\n"); close_ofps(); quit();
	}
	fread((*p)+off,1,*s,ifp); *((*p+*s)-2)=0; *((*p+*s)-1)=0;
}

// blkget : calls func_get and replaces $0D$ with $0A$
blkget(long *s,char **p,long off) {
	func_get(s,p,off); repcrlf((*p)+off);
}

// cleanget : calls func_get and uses the "clean" function to process it
cleanget(long *s,char **p,long off) {
	func_get(s,p,off); clean_up((*p)+off);
}

long filesize() {	// Return size of current file
	register long now,s;

	now=ftell(ifp); fseek(ifp,0,2L); s=ftell(ifp)-now;
	fseek(ifp,now,0L);
	return s+2;			// Just for luck!
}

char *text_proc(register char *p,FILE *destfp,char lastc) {
	register char *s; register long LEN;
	do {
txtloop:	p=skipline(s=p); if(!*s) break;
		if(*s==9) s++; if(!*s) { fputc('\n',destfp); goto txtloop; }
		LEN=p-s;
		if(*(p-1)=='{') LEN=p-s-2;
		else *(p-1)=(*p=='\n')?lastc:' '; 
		fwrite(s,1,LEN,destfp);
	} while(*s);
	fputc(0,destfp); return p;
}
