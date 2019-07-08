switchC(int n,char c)
{	register int h; struct NewScreen ns; struct NewWindow nw;

	h=255; ns=NewScreenStructure; nw=PFEWNewWindowStructure1;

	if(toupper(c)=='N') h=200;
	IntuitionBase=OpenLibrary("intuition.library", 0L);
	if (IntuitionBase==0L)
	{
		puts("\nno intuition.library\n"); squit();
	}
	GfxBase=OpenLibrary("graphics.library", 0L);
	if (GfxBase==0L)
	{
		puts("\nunable to open grafix library!\n"); squit();
	}

	nw.Height=h;

	sC=OpenScreen(&ns);	/* open screen if present */
	if(sC==0L)
	{
		puts("Can't open screen!\n"); squit();
	}

	nw.Screen=sC; vP=sC->ViewPort;

	nw.Height=ns.Height;
	wG=OpenWindow(&nw); /* open the window */
	if ( wG==0L )
	{
		puts("open window failed\n"); squit();
	}

	rpG=wG->RPort;	/* get a rastport pointer for the window */
	LoadRGB4(&vP,(USHORT *)&Palette,(long)PaletteColorCount);
	ReadIo.io_Data=(APTR)wG;	/* Window handle */
	bits();

	if(OpenDevice("console.device",0,(struct IORequest *)&ReadIo,0)!=NULL) quit(0*printf("Unable to open %s!\n","console.device"));
	WriteIo.io_Device=ReadIo.io_Device;
	WriteIo.io_Unit=ReadIo.io_Unit;
	WriteIo.io_Command=3;		/* Write */
	ReadIo.io_Command=2;		/* Read */
	ReadIo.io_Message.mn_ReplyPort=ReadRep;
	WriteIo.io_Message.mn_ReplyPort=WriteRep;
	iosup=CUSSCREEN;
}

tx(register char *s)
{	register int i,l; register char *p,*ls,*lp;

	if(iosup==LOGFILE) return;
	if(addcr==YES && needcr==YES) txc('\n'); addcr=NO; needcr=NO;
	if(iosup==0) { printf(s); return; }

	ioproc(s); s=ow; l=0;
	while(*s!=0)
	{
		p=spc;
		i=0; ls=lp=NULL;
		do
		{
			if(*s=='\n') break;
			if(i<79 && (*s==9 || *s==32)) { ls=s; lp=p; }
			if(*s=='\r') { s++; continue; }
			*(p++)=*(s++); i++;
		} while(*s!=0 && *s!='\n' && ( me->llen <8 || i<(me->llen-1)) && *s!=12);

		if(i>0) needcr=YES;
		if( ((me->llen-1) >= 8 && i==(me->llen-1)) && *s!='\n')
		{
			if(*s==' ' || *s==9) s++;
			else if(*s!=0 && ls!=NULL)
			{
				s=ls+1; p=lp+1;
			}
			if(iosup==SERIO) *(p++)='\r';
			*(p++)='\n';
			needcr=NO;
		}
		if(*s=='\n')
		{
			if(iosup==SERIO) *(p++)='\r';
			*(p++)='\n';
			s++; needcr=NO;
		}
		*p=0;
		switch(iosup)
		{
			case SERIO:	ssend(spc); break;
			case CUSSCREEN:	wsend(spc); break;
		}
		l++;
		if(me->slen > 0 && l >= (me->slen) && *s!=12)
		{
			pressret(); l=0;
		}
		if(*s==12) { s++; l=0; }
	}
}

utx(register int n,register char *s)
{
	ioproc(s);
	if(n==Af) tx(s);
	else interact(MMESSAGE,n,-1);
}

utxn(register int plyr,register char *format,register int n)
{
	sprintf(str,format,n); utx(plyr,str);
}

txc(char c)
{
	switch(iosup)
	{
		case 0:	putchar(c); return;
		case LOGFILE: return;
		case SERIO:
			wserio->IOSer.io_Data=(APTR)&c;
			wserio->IOSer.io_Length=1; wserio->IOSer.io_Command=CMD_WRITE;
			DoIO((struct IORequest *)wserio);
			break;
		case CUSSCREEN:
			WriteIo.io_Data=(APTR)&c;
			WriteIo.io_Length=1;
			SendIO((struct IORequest *)&WriteIo);
			return;			/* Dont add line feeds. */
	}
	if(c=='\n') { txc('\r'); needcr=NO; } else needcr=YES;
}

txn(char *format,int n)
{
	sprintf(str,format,n); tx(str);
}

txs(char *format,char *s)
{
	sprintf(str,format,s); tx(str);
}

Inp(char *s,int l)	/* Get to str, and max length l */
{	register char *p; register int c;

	p=s; *p=c=0; forced=0;
	do
	{
		if(ip==0) iocheck(); if(forced!=0) return;
		switch(iosup)
		{
			case LOGFILE:	return '\n'; break;
			case SERIO:	c=sgetc(); break;
			case CUSSCREEN:	c=wgetc(); break;
		}
		if(ip==0) iocheck(); if(forced!=0) return;
		c=c&255;
		if(c==NULL) continue;
		if(l==NULL) return;
		if(c==8)
		{
			if(p>s)
			{
				txc(8); txc(32); txc(8); *(--p)=0;
			}
			continue;
		}
		if(c==10 || c==13)
		{
			c='\n'; *(p++)=0; txc((char)c); continue;
		}
		if(c==27 || c>0 && c<23 || c==me->rchar)
		{
			txc('\n'); tx((rktab+me->rank)->prompt); tx(s); continue;
		}
		if(c==24 || c==23)
		{
			while(p!=s) { txc(8); txc(32); txc(8); p--; }
			*p=0; continue;
		}
		if(c<32 || c>127) continue;
		if(p>=s+l-1) continue;
		*(p++)=(char) c; *p=0; txc((char)c); needcr=YES;
	} while(c!='\n');
	if(isspace( *(s+strlen(s)-1) )) *(s+strlen(s)-1)=0;
	needcr=NO; if(ip==0) iocheck();
}

wgetc()			/* Ask for a character to be input to *s */
{	char c;

	c=0;
	ReadIo.io_Data=(APTR)&c;		/* Buffer */
	ReadIo.io_Length=1;			/* One char at a time */
	SendIO((struct IORequest *)&ReadIo);		/* Send for it! */
loop:	Wait(-1);
	if(GetMsg((struct MsgPort *)ReadRep) != NULL) return (int) c&255;
	if(ip==0) iocheck();
	if(forced!=0) { AbortIO((struct IORequest *)&ReadIo); GetMsg((struct MsgPort *)ReadRep); return; }
	if(GetMsg((struct MsgPort *)ReadRep) != NULL) return (int) c&255;
	goto loop;
}

wsend(char *s)
{
	WriteIo.io_Data=(APTR)s; WriteIo.io_Length=strlen(s); DoIO((struct IORequest *)&WriteIo);
}

sgetc()			/* Ask for a character to be input to *s */
{	char c;

	c=0;
	cdcheck();
	serio->IOSer.io_Data=(APTR)&c;			/* Buffer */
	serio->IOSer.io_Length=1;			/* One char at a time */
	serio->IOSer.io_Command=CMD_READ;		/* Read data */
	SendIO((struct IORequest *)serio);		/* Send for it! */
loop:	Wait(-1);
	if(GetMsg((struct MsgPort *)ReadRep) != NULL) return (int) c&255;
	if(ip==0) iocheck();
	if(forced!=0) { AbortIO((struct IORequest *)serio); GetMsg((struct MsgPort *)ReadRep); return; }
	if(GetMsg((struct MsgPort *)ReadRep) != NULL) { cdcheck(); return (int) c&255; }
	goto loop;
}

ssend(char *s)
{
	cdcheck();
	wserio->IOSer.io_Data=(APTR)s; wserio->IOSer.io_Command=CMD_WRITE; wserio->IOSer.io_Length=strlen(s);
	DoIO((struct IORequest *)wserio);
	cdcheck();
}

#define	NOCARRIER (serio->io_Status & (1<<5))		/* Carrier detect in io_Status */

/* On the Amiga, CD is ACTIVE when bit 5 is low, CD is INACTIVE when CD is high */
cdcheck()
{
	sercmd(SDCMD_QUERY); if( NOCARRIER ) kquit("dropped carrier!");
}

bits()
{
	/* Create ports for IO */
	ReadRep=CreatePort(0L,0);
	WriteRep=CreatePort(0L,0);
	if(ReadRep==0L || WriteRep==0L)
	{
		puts("\nunable to allocate reply ports.\n");
		squit();
	}
}

squit()
{
	iosup=0; quit();
}

scrend()
{
	if(iosup==SERIO && wserio!=NULL && wserio->IOSer.io_Device!=NULL)
	{
		AbortIO((struct IORequest *)serio);
		AbortIO((struct IORequest *)wserio);
		CloseDevice((struct IORequest *)wserio);
		wserio->IOSer.io_Device=NULL;
	}
	if(ReadIo.io_Device!=NULL) CloseDevice((struct IORequest *)&ReadIo);
	if(ReadRep!=NULL)  DeletePort(ReadRep);
	if(WriteRep!=NULL) DeletePort(WriteRep);
	if(wG!=NULL) CloseWindow(wG);
	if(sC!=NULL) CloseScreen(sC);
	if(GfxBase != 0L) CloseLibrary((struct Library *)GfxBase);
	if(IntuitionBase != 0L) CloseLibrary((struct Library *)IntuitionBase);
}

switchS(char *s,char *p,char *u,char *rts)
{	register long ser_baud,unit,ext;

	if(s!=NULL) ser_baud=atoi(s);
	if(p==NULL) p=SERIALNAME;
	if(u!=NULL && isdigit(*u)) unit=atoi(u); else unit=0;
	if(rts!=NULL && toupper(*rts)=='N') ext=0; else ext=SERF_7WIRE;
	bits();
	serio=CreateExtIO(ReadRep,(long) sizeof(struct IOExtSer));
	wserio=CreateExtIO(WriteRep,(long) sizeof(struct IOExtSer));
	wserio->io_Baud    = ser_baud;
	wserio->io_Status  = 0;
	wserio->io_SerFlags= SERF_SHARED;
	wserio->io_ExtFlags= 0;
	*serio = *wserio;

	if(OpenDevice(p,unit,(struct IORequest *)wserio,0)!=NULL)
	{
		quit(0*printf("Unable to open %s!\n",p));
	}

	/* Copy unit & device info */
	*serio = *wserio; serio->IOSer.io_Message.mn_ReplyPort=ReadRep;
	wserio->IOSer.io_Message.mn_ReplyPort=WriteRep;

	/* Attempt to set the parameters */
	if(ser_baud != 0)
	{
		/* Clear, flush and prepare */
		sercmd(CMD_CLEAR); sercmd(CMD_FLUSH); sercmd(SDCMD_QUERY);

		wserio->io_SerFlags=wserio->io_SerFlags|SERF_SHARED;
		sercmd(SDCMD_SETPARAMS);
		if(wserio->IOSer.io_Error!=NULL) quit(0*printf("Unable to configure %s\n",p));
	}
	*serio = *wserio;
	serio->IOSer.io_Message.mn_ReplyPort=ReadRep;
	iosup=SERIO;
}

sercmd(int cmd)
{
	wserio->IOSer.io_Command=cmd; DoIO((struct IORequest *)wserio);
}
