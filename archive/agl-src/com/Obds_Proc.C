//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Obds_Proc.C	Process ObDescs.Txt (Object Descriptions)
//
//	LMod: oliver 11/06/93	AMUL->AGL. Uses memory instead of disk access
//

obds_proc() {
	char	*p,*s;

	obdes=0;
	fopenw("-ram:ODIDs");	fopenw(obdsfn);
	if(nextc(0)==-1) { close_ofps(); return 0; }
	blkget(&datal,&data,0L); p=data; tx("Proc:");

	do {
		p=skipline(s=p); if(!*s) continue;
		clean_up(s); s=skipspc(s); if(!*s || *s==';') continue;
		getword(skiplead("desc=",s)); if(!Word[0]) continue;
		if(strlen(Word)<3 || strlen(Word)>IDL) {
			error("Invalid ID: %s\n",Word);
			p=skipdata(p); continue;
		}
		strcpy(objdes.id,Word);
		fseek(ofp2,0,2); objdes.descrip=ftell(ofp2);
		fwrite(objdes.id,sizeof(objdes),1,ofp1);
		p=text_proc(p,ofp2,' ');
	} while(*p);
	errabort();
}
