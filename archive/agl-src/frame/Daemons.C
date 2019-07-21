//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/Daemons.C	Daemon handling routines
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

dpstart(register int d,register int c) {	// Start PRIVATE daemon!
	dstart(d,c,MDSTART);
}

dgstart(register int d,register int c) {	// Begin GLOBAL daemon!
	dstart(d,c,MGDSTART);
}

dstart(register int d,register int c,register int t) {
	if(c) {
		Apx1=inoun1; Apx2=inoun2; Apx3=wtype[1]; Apx4=wtype[3];
		SendIt(t,d,(char *)c);		// Inform AMAN...
	} else {
		long lv,ld,lr,v,a1,n1,a2,n2;
		v=iverb; lv=lverb; ld=ldir; lr=lroom;
		a1=iadj1; a2=iadj2; n1=inoun1; n2=inoun2;
		lang_proc(d,0);
		iverb=v; lverb=lv; ldir=ld; lroom=lr;
		iadj1=a1; iadj2=a2; inoun1=n1; inoun2=n2;
	}
}

dbegin(register int d)			// Force daemon to happen
{
}

dshow(register int d) {
	SendIt(MCHECKD,d,NULL);
	if(Ad==-1) { tx("eventually"); return; }
	timeto(block,Ap1); tx(block);
}

dsend(register int p, register int d, register int c) {	// Send a daemon
	if(p==Af)  dpstart(d,c);
	sendex(p,ASTART,d,c,0,0);	// Tell THEM to start the daemon
}
