//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/FileBits.C	Miscellaneous functions
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

CXBRK()		// What to do when ^C pressed
{	return 0;	}

memfail(register char *s) {
	txs("** Unable to allocate memory for %s! **\n",s);
	quit();
}

givebackmemory() {
	if(reply)	DeleteAPort(reply,amanp);
	if(amanrep)	DeleteAPort(amanrep,agl);
	if(ob)		FreeMem(ob,OWLIM);
	if(ow)		FreeMem(ow,OWLIM);
	if(input)	FreeMem(input,400);
	if(serio)	DeleteExtIO((struct IORequest*)serio);
	if(wserio)	DeleteExtIO((struct IORequest*)wserio);
}

fopenr(register char *s) {
	if(ifp) fclose(ifp);
	sprintf(block,"%s%s",dir,s);
	if(!(ifp=fopen(block,"rb"))) {
		sprintf(spc,"\x07** Error: Can't open \"%s\" for %sding!\n\n",block,"rea");
		tx(spc); quit();
	}
}

fopena(char *s) {
	if(afp) fclose(afp);
	sprintf(block,"%s%s",dir,s);
	if(!(afp=fopen(block,"rb+"))) {
		sprintf(spc,"\x07** Error: Can't open \"%s\" for %sding!\n\n",block,"appen");
		tx(spc); quit();
	}
}

close_ofps() {
	if(ofp1)fclose(ofp1); if(ofp2)fclose(ofp2); if(ofp3) fclose(ofp3);
	ofp1=ofp2=ofp3=NULL;
}

kquit(register char *s) {
	if(link && me2->state>=PLAYING) {
		sprintf(block,"@me has just %s.\n",s); action(block,AOTHERS);
	}
	exeunt='K'; quit();
}

quit() {
	if(exeunt!='K') txs("\n%s exiting.\n\n",vername);
	scrend();					// custom screen
	if(link>0 && agl->from>-1) SendIt(MDISCNCT,0,me->name);
	if(ifp)	fclose(ifp);
	if(afp)	fclose(afp);
	close_ofps(); givebackmemory();
	if(AGLBase) CloseLibrary(AGLBase);
	exit(0);
}

pressret() {
	register int l; char	myt1[4];
	sys(RETURN); Inp(myt1,0);
	l=strlen(umsgp+*(umsgip+RETURN));
	while(l>0) { txc(8); txc(32); txc(8); l--; }
}

sys(int n)
{	tx(umsgp+*(umsgip+n));	}

crsys(int n)
{	txc('\n'); tx(umsgp+*(umsgip+n));	}

timeto(char *s,long secs) {
	register int x,y;

	if(secs >= 3600) {	// More than an hour
		x = secs / 3600;	// Hours
		y = secs - ( x * 3600 );// Minutes & seconds
		if( y < 60 ) {		// Upto 1 minute?
			sprintf(s,"%ld %s, %ld %s",x,(x>1)?"hours":"hour",y,(y>1)?"seconds":"second");
			return;
		}
		y = y / 60;
		sprintf(s,"%ld %s and %ld %s",x,(x>1)?"hours":"hour",y,(y>1)?"minutes":"minute");
		return;
	}
	x = secs / 60; y = secs - ( x * 60 );
	if(!x && !y) strcpy(s,"now");
	if( x && !y) sprintf(s,"%ld %s",x,(x>1)?"minutes":"minute");
	if(!x &&  y) sprintf(s,"%ld %s",y,(y>1)?"seconds":"second");
	if( x &&  y) sprintf(s,"%ld %s and %ld %s",x,(x>1)?"minutes":"minute",y,(y>1)?"seconds":"second");
}

