//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: AGLCom.C		Compiler - Main File
//
//	LMod: oliver 20/06/93	Added set_art()
//	      oliver 11/06/93	AMULCom -> AGLCom
//

#include "adv:com/AGLCom.H"

CXBRK() {
	tx("\nAGLCom: Break!\n[0m"); quit();
}

main(int argc,char *argv) {
	setvername(vername); (mytask=FindTask(0L))->tc_Node.ln_Name = vername;

	printf("\t\t AGL Compiler; %25.25s\n\n",vername);

	ofp1=ofp2=ofp3=ofp4=ofp5=NULL; dir[0]=0; dchk=rmrd=warn=TRUE;
	ohd=Output();

	if(argc>6) usage();	if(argc>1) argue(argc,argv);

	tx(">> Checking .TXT files: "); needcr=TRUE;
	for(c=1; c<TXTFILES; c++) checkf(txtfile[c]);
	checksys(); errabort();
	
	tx("\x1B[1mCOMPILING!\n\x1B[0m");

	section(TF_SYSTEM);	sys_proc();	fclose(ifp);
	if(rmrd) {
		section(TF_ROOMS);	room_proc();	fclose(ifp);
	}
	fopenf(rooms1fn);	//  Check DMOVE ptrs
	if(!rmrd) {
		tx("Load rooms: ");
		fseek(ifp,0,2L); rooms=ftell(ifp)/sizeof(room); rewind(ifp);
	}
	if(!(rmtab=(struct room *)AllocMem(sizeof(room)*rooms,STDMEM)))
		quit(puts("No memory for ROOM ID table!\n"));
	if(fread((char *)rmtab,sizeof(room),rooms,ifp)!=rooms)
		quit(puts("Rooms file corrupt! Aborting!"));
	if(dchk && dmoves) {
		tx("\x0d\x1b[K<<DMOVEs>> Proc:"); checkdmoves(); fclose(ifp);
	}
	section(TF_RANKS);	rank_proc();	fclose(ifp);
	section(TF_SYSMSG);	smsg_proc();	fclose(ifp);
	section(TF_UMSG);	if(umsg_proc()==-1) quit();	fclose(ifp);
	section(TF_MOBILES);	mob_proc1();	fclose(ifp);
	section(TF_OBDESCS);	obds_proc();	fclose(ifp);
	section(TF_OBJECTS);	objs_proc();	fclose(ifp);
	section(TF_LANG);	proc=1; lang_proc();	fclose(ifp);
	section(TF_TRAVEL);	proc=0; trav_proc();	fclose(ifp);
	section(TF_SYNS);	syn_proc();	fclose(ifp);

	tx("\x0d\x1b[K\nCompiled OK\n\n");
	printf("Statistics for %s:\n\n",adname);
	printf("Rooms: %d. Nouns: %d. Verbs: %d.\n",rooms,nouns,verbs);
	fopenw(advfn); time(&clock);
	// Line 1
	fprintf(ofp1,"%s\n",adname);
	putn(rooms); putn(ranks); putn(verbs); putn(syns); putn(nouns);
	putn(adjs); putn(ttents); putn(umsgs); putn(clock); putn(mins);
	putn(invis); putn(invis2); putn(minsgo); putn(mobs); putn(rscale);
	putn(tscale); putn(mobchars);
	// Line 2... System.Txt Options
	fprintf(ofp1,"%ld %ld %ld %ld %c\n%s\n",
		scrx,scry,scrw,scrh,scrm,logname);
	exi=1; 	quit();
}

isrflag(char *s) {		// Check to see if s is a room flag
	int	_x;
	for(_x=0;_x<NRFLAGS;_x++)
		if(!strcmp(s,rflag[_x])) return _x;
	return -1;
}

isroom(char *s) {
	int r;
	roomtab=rmtab;
	for(r=0;r<rooms;r++,roomtab++)
		if(!strcmp(roomtab->id,s)) return r;
	return -1;
}

isoflag1(char *s) {		// Is it a FIXED object flag?
	int i;
	for(i=0;i<NOFLAGS;i++)
		if(!strcmp(obflags1[i],s)) return i;
	return -1;
}

isoparm() {			// Is it an object parameter?
	int i,j;
	for(i=0;i<NOPARMS;i++) {
		if(!strncmp(obparms[i],Word,(j=strlen(obparms[i]))))
			{ strcpy(Word,Word+j); return i; }
	}
	return -1;
}

isoflag2(char *s) {		// Is it a state flag?
	int i;
	for(i=0;i<NSFLAGS;i++)
		if(!strcmp(obflags2[i],s)) return i;
	return -1;
}

set_art() {
	register int i;
	for(i=0; i<NART; i++)
		if(!strcmp(article[i],Word)) {
			obj2.article=i; return;
		}
	object("article type");
}

set_adj() {
	register int i;

	if(strlen(Word)>IDL || strlen(Word)<3) {
		printf("\nInvalid adjective '%s'!\n\n\x07",Word);
		quit();
	}
	if(!adjs) {
		for(i=0;i<IDL+1;i++) dmove[i]=0; strcpy(dmove,Word);
		obj2.adj=0; fwrite(dmove,IDL+1,1,afp); adjs++; return;
	}
	fseek(afp,0L,0);	// Move to beginning
	i=0;
	do {
		if(!fread(dmove,IDL+1,1,afp)) continue;	// Read adj!
		if(!strcmp(Word,dmove)) { obj2.adj=i; return; }
		i++;
	} while(!feof(afp));
	for(i=0;i<IDL+1;i++) dmove[i]=0; strcpy(dmove,Word);
	fseek(afp,0L,2);	// Move to end!
	fwrite(dmove,IDL+1,1,afp);	// Add this adjective
	obj2.adj=adjs++;
}

object(char *s) {
	error("%s: Invalid %s, \"%s\"!\n",obj2.id,s,Word);
}

set_start() {
	if(!isdigit(Word[0])) object("start state");
	obj2.state=atoi(Word);
	if(obj2.state<0 || obj2.state>100) object("start state");
}

set_holds() {
	if(!isdigit(Word[0])) object("holds= value");
	obj2.contains=atoi(Word);
	if(obj2.contains<0 || obj2.contains>1000000) object("holds= state");
}

set_put() {
	register int i;

	for(i=0;i<NPUTS;i++)
		if(!strcmp(obputs[i],Word)) { obj2.putto=i; return; }
	object("put= flag");
}

set_mob()
{	register int i;
	for(i=0; i<mobchars; i++) if(!strcmp(Word,(mobp+i)->id)) { obj2.mobile=i; return; }
	object("mobile= flag");
}

usage() {
	printf("Usage:\n	aglcom [-d] [-q] [-r] <adv path>\n\n -d = Don't check DMOVE flags\n -q = Quiet (no warnings)\n -r = Don't recompile rooms.\n"); exit(0);
}

argue(int argc,char *argv[])
{	int n;
	if(!strcmp(argv[1],"-?")) usage();
	for(n=2;n<=argc;n++) {
		if(!strcmp("-d",argv[n-1])){dchk=0; continue;}
		if(!strcmp("-q",argv[n-1])){warn=0; continue;}
		if(!strcmp("-r",argv[n-1])){rmrd=0; continue;}
		strcpy(dir,argv[n-1]);
		if((c=dir[strlen(dir)-1])!='/' && c!=':') strcat(dir,"/");
	}
}

checkf(char *s) {
	sprintf(block,"%s%s.Txt",dir,s);
	if((ifp=fopen(block,"rb"))) fclose(ifp);
	else error("Missing file %s!\n",block);
	ifp=NULL;
}

checksys() {
	int w;
	sprintf(block,"%s%s.Txt",dir,txtfile[TF_SYSTEM]);
	if((ifp=fopen(block,"rb"))) { fclose(ifp); ifp=NULL; return; }
	if(err) return error("Missing file %s!\n",block);
	w=warn; warn=0; warne("Missing file %s!\n",block);
	if(!(ofp1=fopen(block,"wb")))
		quit(error("Unable to create file %s.\n",block));
	warne("Creating DEFAULT %s.Txt file.\n",block);
	fprintf(ofp1,";\n;	Default %s.Txt for AGL\n;	generated by %s (C) Oliver Smith, 1992\n;\n\n",txtfile[TF_SYSTEM],vername);
	fprintf(ofp1,"%s=\"AGL Lands\"\t; Adventure name\n%s=30\t\t; Session Length\n",sysopt[SO_NAME],sysopt[SO_SESH]);
	fprintf(ofp1,"%s=0\t\t; Min. rank to use SuperGo\n",sysopt[SO_MSGO]);
	fprintf(ofp1,"%s=40\t\t; Top rank users lose 40%s%s=50\t\t; At start of game objects lose 50%s",
			sysopt[SO_RSCL],"\% of scaled values\n",sysopt[SO_TSCL],"\% of scaled values\n");
	fprintf(ofp1,"%s=2\t\t; Inv%s2%s",sysopt[SO_SEE1],"isible >= rank "," can see invisibles\n");
	fprintf(ofp1,"%s=3\t\t; V%s3%s",sysopt[SO_SEE2],"isible >= rank "," can see invisibles\n");
	fprintf(ofp1,"\n; --- End of file\n");
	fclose(ofp1); ofp1=NULL; warn=w;
}

iscond(char *s) {
	int i;
	for(i=0; i<NCONDS; i++)	if(!strcmp(conds[i],s)) return i;
	return -1;
}

isact(char *s) {
	int i;
	for(i=0; i<NACTS; i++)	if(!strcmp(acts[i],s)) return i;
	return -1;
}

section(int i) {
	printf("\x0d[K%s:Load:",txtfile[i]); opentxt(txtfile[i]); needcr=TRUE;
}

error(char *s,char *s1,char *s2,char *s3,char *s4,char *s5) {
	if(needcr) tx("\n"); needcr=FALSE;
	printf("\x0d\x07E> "); printf(s,s1,s2,s3,s4,s5); err++;
	if(err>20) quit(printf("\n\x1b[1m>>> Aborting due to %ld errors! <<<\x1b[0m\n\x07",err));
}

warne(char *s,char *s1,char *s2,char *s3,char *s4,char *s5) {
	if(!warn) return;
	if(needcr) tx("\n"); needcr=FALSE;
	printf("\x0dW  "); printf(s,s1,s2,s3,s4,s5);
}

errabort() {
	if(data) { FreeMem(data,datal); data=NULL; datal=NULL; }
	if(err) {
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	close_ofps(); tx("\x0d\[K");
}
