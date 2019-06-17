CXBRK()		/* What to do when ^C pressed */
{
	return 0;
}

memfail(register char *s)
{
	txs("** Unable to allocate memory for %s! **\n",s);
	quit();
}

givebackmemory()
{
	if(repbk!=NULL)	  DeletePort(repbk);
	if(reply!=NULL)	  DeletePort(reply);
	if(amul!=NULL)	  FreeMem((char *)amul,(long)sizeof(*amul));
	if(amanrep!=NULL) DeletePort(amanrep);
	if(amanp!=NULL)	  FreeMem((char *)amanp,(long)sizeof(*amanp));
	if(ob!=NULL)	  FreeMem(ob,5000);
	if(ow!=NULL)	  FreeMem(ow,3000);
	if(input!=NULL)	  FreeMem(input,400);
	if(serio!=NULL)	  DeleteExtIO((struct IORequest*)serio);
	if(wserio!=NULL)  DeleteExtIO((struct IORequest*)wserio);
}

fopenr(register char *s)
{
	if(ifp!=NULL) fclose(ifp);
	sprintf(block,"%s%s",dir,s);
	if((ifp=fopen(block,"rb"))==NULL)
	{
		sprintf(spc,"\x07** Error: Can't open \"%s\" for %sding!\n\n",block,"rea");
		tx(spc); quit();
	}
}

fopena(char *s)
{
	if(afp!=NULL) fclose(afp);
	sprintf(block,"%s%s",dir,s);
	if((afp=fopen(block,"rb+"))==NULL)
	{
		sprintf(spc,"\x07** Error: Can't open \"%s\" for %sding!\n\n",block,"appen");
		tx(spc); quit();
	}
}

close_ofps()
{
	if(ofp1!=NULL)fclose(ofp1); if(ofp2!=NULL)fclose(ofp2); if(ofp3!=NULL)fclose(ofp3);
	ofp1=ofp2=ofp3=NULL;
}

kquit(register char *s)
{
	sprintf(block,"@me just dropped carrier.\n"); action(ow,AOTHERS);
	quit();
}

quit()
{
	txs("\n%s exiting.\n\n",vername);
	scrend();					/* custom screen */
	if(link>0 && amul->from>-1) SendIt(MSG_DISCONNECT,0,me->name);
	if(ifp!=NULL)	fclose(ifp);
	if(afp!=NULL)	fclose(afp);
	close_ofps(); givebackmemory();
	exit(0);
}

SendIt(register int t,register int d,register char *p)
{
	if(link==0) return;
	AMt=t; AMd=d; AMp=p;
	PutMsg(port, (struct Message *) amanp);
	WaitPort(amanrep); GetMsg(amanrep);
	Af=AMf; Ad=AMd; Ap=AMp; At=AMt; Ap1=Apx1; Ap2=Apx2; Ap3=Apx3; Ap4=Apx4;
}

pressret()
{	register int l;
	sys(RETURN); Inp(block,0);
	l=strlen(umsgp+*(umsgip+RETURN));
	while(l>0) { txc(8); txc(32); txc(8); l--; }
}

sys(register int n)
{
	tx(umsgp+*(umsgip+n));
}

crsys(register int n)
{
	txc('\n'); tx(umsgp+*(umsgip+n));
}

timeto(register char *s,register long secs)
{	register int x,y;

	if(secs >= 3600)	/* More than an hour */
	{
		x = secs / 3600;	/* Hours */
		y = secs - ( x * 3600 );/* Minutes & seconds */
		if( y < 60 )		/* Upto 1 minute? */
		{
			sprintf(s,"%ld %s, %ld %s",x,(x>1)?"hours":"hour",y,(y>1)?"seconds":"second");
			return;
		}
		y = y / 60;
		sprintf(s,"%ld %s and %ld %s",x,(x>1)?"hours":"hour",y,(y>1)?"minutes":"minute");
		return;
	}
	x = secs / 60; y = secs - ( x * 60 );
	if( x == 0 && y == 0 ) strcpy(s,"now");
	if( x != 0 && y == 0 ) sprintf(s,"%ld %s",x,(x>1)?"minutes":"minute");
	if( x == 0 && y != 0 ) sprintf(s,"%ld %s",y,(y>1)?"seconds":"second");
	if( x != 0 && y != 0 ) sprintf(s,"%ld %s and %ld %s",x,(x>1)?"minutes":"minute",y,(y>1)?"seconds":"second");
}