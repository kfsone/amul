//
//            ****    AMCOM.C.........Adventure Compiler    ****
//            ****               Main Program!              ****
//
// Copyright (C) Oliver Smith, 1991-2. Copyright (C) Kingfisher s/w 1991-2
//        Program Designed, Developed and Written By: Oliver Smith.
//

#include "adv:com/AMCom.H"

CXBRK() {			// Ctrl-C handler
	tx("\nAMCOM: BREAK!\n"); quit();
}

main(int argc,char *argv) {
	setvername(vername); (mytask=FindTask(0L))->tc_Node.ln_Name = vername;

	printf("\t\t \x1b[4mAMUD Compiler; %25.25s\x1b[0m\n\n",vername);

	ofp1=ofp2=ofp3=ofp4=ofp5=NULL; dir[0]=0; dchk=rmrd=warn=1;
	ohd=Output();

	if(argc>6) usage();	if(argc>1) argue(argc,argv);

	tx("\n>> Checking .TXT files: "); needcr=TRUE;
	for(c=1; c<TXTFILES; c++) checkf(txtfile[c]);
	checksys(); errabort();
	
	tx("\x1B[1mCOMPILING!\n\x1B[0m");

	section(TF_SYSTEM);	sys_proc();	fclose(ifp);
	if(rmrd) {
		section(TF_ROOMS);	room_proc();	fclose(ifp);
	}
	fopenr(rooms1fn);	//  Check DMOVE ptrs
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
	fopena(umsgfn); fseek(afp,0,2L); msg1=afp; afp=NULL;
	fopena(umsgifn); fseek(afp,0,2L); msg2=afp; afp=NULL;
	section(TF_MOBILES);	mob_proc();	fclose(ifp);
	section(TF_OBDESCS);	obds_proc();	fclose(ifp);
	section(TF_OBJECTS);	objs_proc();	fclose(ifp);
	section(TF_LANG);	proc=1; lang_proc();	fclose(ifp);
	section(TF_TRAVEL);	proc=0; trav_proc();	fclose(ifp);
	section(TF_SYNS);	syn_proc();	fclose(ifp);

	tx("\x0d\x1b[K\nCompiled OK\n\n");
	printf("Statistics for %s:\n\n",adname);
	printf("\t\tRooms: %6d\tVerbs: %6d\tNouns: %6d\n",rooms,verbs,nouns);
	printf("\t\t \x1b[33mTotal items processed:\x1b[0;1m%7d\n\n\x1b[0m",
		rooms+ranks+adjs+verbs+nouns+syns+ttents+umsgs+
		NSMSGS+mobs+mobchars);
	ofp1=fopenw(advfn); time(&clock);
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
	fseek(afp,0L,0);		// Move to beginning
	i=0;
	do {
		if(!fread(dmove,IDL+1,1,afp)) continue;	// Read adj!
		if(!strcmp(Word,dmove)) { obj2.adj=i; return; }
		i++;
	} while(!feof(afp));
	for(i=0;i<IDL+1;i++) dmove[i]=0; strcpy(dmove,Word);
	fseek(afp,0L,2);		// Move to end!
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

set_mob() {
	register int i;
	for(i=0; i<mobchars; i++)
		if(!strcmp(Word,(mobp+i)->id)) { obj2.mobile=i; return; }
	object("mobile= flag");
}

argue(int argc,char *argv[]) {		// Process command line arguments
	int n;
	if(!strcmp(argv[1],"-?")) usage();
	for(n=2;n<=argc;n++) {
		if(!strcmp("-d",argv[n-1])) { dchk=0; continue; }
		if(!strcmp("-q",argv[n-1])) { warn=0; continue; }
		if(!strcmp("-r",argv[n-1])) { rmrd=0; continue; }
		strcpy(dir,argv[n-1]);
		if((c=dir[strlen(dir)-1])!='/' && c!=':') strcat(dir,"/");
	}
}

checkf(char *s) {
	sprintf(block,"%s%s.Txt",dir,s);
	if((ifp=fopen(block,"rb"))) fclose(ifp);
	else error("Missing: file \x1B[33;1m%s!!!\x1B[0m\n",block);
	ifp=NULL;
}

checksys() {
	int w;
	sprintf(block,"%s%s.Txt",dir,txtfile[TF_SYSTEM]);
	if((ifp=fopen(block,"rb"))) { fclose(ifp); ifp=NULL; return; }
	if(err) return error("Missing: file \x1B[33;1m%s!!!\x1B[0m\n",block);
	w=warn; warn=0;
	warne("Missing: file \x1B[33;1m%s!!!\x1B[0m\n",block);
	if(!(ofp1=fopen(block,"wb")))
		quit(error("Unable to create file %s.\n",block));
	warne("Creating DEFAULT %s.Txt file.\n",block);
	fprintf(ofp1,SysDefTxt,
			vername, sysopt[SO_MSGO], sysopt[SO_RSCL],
			"\% of scaled values", sysopt[SO_TSCL],
			"\% of scaled values", sysopt[SO_SEE1],
			"isible >= rank ", " can see invisibles",
			sysopt[SO_SEE2], "isible >= rank ",
			" can see invisibles");
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
	printf("\x0d[K<<%s>> Load:",txtfile[i]); opentxt(txtfile[i]); needcr=TRUE;
}

error(char *s,char *s1,char *s2,char *s3,char *s4,char *s5) {
	if(needcr==TRUE) tx("\n"); needcr=FALSE;
	printf("\x0d\x07\x1b[1m#E#> ");
	printf(s,s1,s2,s3,s4,s5);
	printf("\x1b[0m"); err++;
	if(err>20) {
		printf("\n\x1b[1m>> %ld errors: aborting! <<\x1b[0m\n\x07",err);
		quit();
	}
}

warne(char *s,char *s1,char *s2,char *s3,char *s4,char *s5) {
	if(!warn) return;
	if(needcr==TRUE) tx("\n"); needcr=FALSE;
	printf("\x0d W > "); printf(s,s1,s2,s3,s4,s5);
}

errabort() {
	if(data) { FreeMem(data,datal); data=NULL; datal=NULL; }
	if(err) {
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	close_ofps(); tx("\x0d\[K");
}

/*
higerhalph(char *s1,char *s2) {	// Is S1 higher than S2?
	while(*s1 && *s1==*s2) { s1++; s2++; }
	if(*s1<*s2 || !*s2) return FALSE; return TRUE;
}

sort_objs()
{	int i,j,k,nts; long *rmtab,*rmptr;

	if(ifp) fclose(ifp); ifp=NULL; close_ofps();
	fopenr(statfn);	blkget(&datal,&data,NULL); fclose(ifp); ifp=NULL;
	fopenr(objrmsfn); blkget(&datal2,&data2,NULL); fclose(ifp); ifp=NULL;
	ofp1=fopenw(objsfn); ofp2=fopenw(statfn); ofp3=fopenw(objrmsfn); ofp4=fopenw(ntabfn);

	printf("\x1b[KSorting Objects...:\r"); objtab2=obtab2; nts=0; k=0;

	statab=(struct _OBJ_STATE *)data; rmtab=(long *)data2;
	for(i=0; i<nouns; i++) {
		if(!*(objtab2=(obtab2+i))->id) {
			printf("@! skipping %ld states, %ld rooms.\n",objtab2->nstates,objtab2->nrooms);
			statab += objtab2->nstates;
			rmtab  += objtab2->nrooms;
			continue;
		}
		strcpy(nountab.id,objtab2->id); nts++;
		nountab.num_of=0; osrch=objtab2; statep=statab; rmptr=rmtab;
		for(j=i; j<nouns; j++, osrch++) {
			if(*(osrch->id) && !strcmp(nountab.id,osrch->id)) {
				fwrite((char *)osrch,  sizeof(obj),   1,               ofp1);
				fwrite((char *)statep, sizeof(state), osrch->nstates,  ofp2);
				fwrite((char *)rmptr,  sizeof(long),  osrch->nrooms,   ofp3);
				nountab.num_of++; *osrch->id=0; if(osrch!=objtab) k++;
				statep+=osrch->nstates; rmptr+=osrch->nrooms;
				if(osrch==objtab2) { statab=statep; rmtab=rmptr; objtab2++; i++; }
			}
			else statep+=osrch->nstates; rmptr+=osrch->nrooms;
		}
		fwrite((char *)&nountab, sizeof(nountab), 1, ofp4);
	}
	printf("%20s\r%ld objects moved.\n"," ",k);
	close_ofps();
	FreeMem(data, datal); FreeMem(data2, datal2); data=data2=NULL;
	fopenr(objsfn); fread((char *)obtab2, sizeof(obj), nouns, ifp);
}
*/
