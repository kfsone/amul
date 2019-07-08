
 /* Daemon processing bizness! */

dpstart(register int d,register int c)	/* Begin PRIVATE daemon! */
{
	dstart(d,c,MDSTART);
}

dgstart(register int d,register int c)	/* Begin GLOBAL daemon! */
{
	dstart(d,c,MGDSTART);
}

dstart(register int d,register int c,register int t)
{
	if(c == 0)
	{	long lv,ld,lr,v,a1,n1,pp,a2,n2;
		v=iverb; lv=lverb; ld=ldir; lr=lroom;
		a1=iadj1; a2=iadj2; n1=inoun1; n2=inoun2; pp=iprep;
		lang_proc(d,0);
		iverb=v; lverb=lv; ldir=ld; lroom=lr;
		iadj1=a1; iadj2=a2; inoun1=n1; inoun2=n2; iprep=pp;
	}
	else
	{
		Apx1=inoun1; Apx2=inoun2; Apx3=wtype[2]; Apx4=wtype[5];
		SendIt(t,d,(char *)c);		/* Inform AMAN... */
	}
}

dbegin(register int d)			/* Force daemon to happen */
{
}

dshow(register int d)
{
	SendIt(MCHECKD,d,NULL);
	if(Ad==-1) { tx("eventually"); return; }
	timeto(block,Ap1); tx(block);
}

dsend(register int p, register int d, register int c)	/* Send a daemon */
{
	if(p==Af)  dpstart(d,c);
	sendex(p,ASTART,d,c,0,0);	/* Tell THEM to start the daemon */
}
