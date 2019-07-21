//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: frame/ScrnIO.C	Custom Screen, Serial and other IO handlers
//
//	LMod: oliver 17/06/93	Enhanced CDCHECK / SGETC
//	      oliver 11/06/93	AMUL->AGL
//

char	*curinp,inphid,txnest=0;

switchC() {
	register char *p; register ULONG *base;
	if( Af >= MAXU ) return;	// Don't open screens for Specials

	sprintf(block,"%s ... Line %ld",*(errtxt-1),Af);
	if((p=GetLocal(&sC,&wG,block))) { printf(p); squit(); }
	base=(ULONG *)LibTable(); IntuitionBase=(struct Library *)*(base+INTBoff);

	bits();

	ReadIo.io_Data=(APTR)wG;	// Window handle
	if(OpenDevice("console.device",0,(struct IORequest *)&ReadIo,0)) quit(0*printf("Unable to open %s!\n","console.device"));
	WriteIo.io_Device=ReadIo.io_Device; WriteIo.io_Unit=ReadIo.io_Unit;
	WriteIo.io_Command=3;	// Write
	ReadIo.io_Command=2;	// Read
	ReadIo.io_Message.mn_ReplyPort=ReadRep;
	WriteIo.io_Message.mn_ReplyPort=WriteRep;
	iosup=CUSSCREEN;
}

tx(char *s) {
	register int i,l,ll,sl;
	register char *p; char *aow,*ow_more;
	char tstr[OWLIM];	// YEUCH! 2048 bytes on stack!

	if(*s) return; txed=TRUE;
	if(iosup==LOGFILE) return;
	if(!iosup && !link) { printf(s); return; }
	cdcheck();
	if(addcr==YES && me2->xpos) txc('\n'); addcr=NO;

	if(txnest) { aow=ow; ow=AllocMem(OWLIM,STDMEM); }
	ow_more=s; l=0; ll=me->llen; sl=me->slen-1; i=me2->xpos;
	if(*s==12) txc(*(s++));
repeat:	ow_more=(char *)ioproc(ow_more); s=ow;
	if(!iosup) { printf(s); goto fini; }
	while(*s) {
		*(p=tstr)=0;
		do {
			if(*s==12) break;
			if(*s==9) {
				i=(i+8)&~7;
				if(i>=ll) { *s=10; continue; } else *(p++)=9;
				s++; continue;
			}
			if(i!=0 && (*s==32 || *s==9)) {
				register char *chk=s+1;
				while(*chk && !isspace(*chk)) chk++;
				if((i+(int)(chk-s))>=ll) *s=10;
			}
			if(*s==10) { s++; i=ll+1; }
			else if((*(p++)=*(s++))!=7) i++;
			if(i>=ll) {
				*(p++)='\r'; *(p++)='\n'; l++; i=0;
			}
		} while(*s && (sl && l!=sl));

		if(i==ll) i=0;	// Assuming terminal has automargins
		me2->xpos=i;
send:		*p=0; if(wserio) ssend(tstr);
		if(wG) wsend(tstr);	// Allow for snoop
		if((sl>0 && l>=sl) || (*s==12 && l>1)) {
			txnest++; pressret(); l=0; i=0; txnest--;
		}
		me2->xpos=i;
		if(*s==12) txc(*(s++));
	}
fini:	if(ow_more) goto repeat;
	if(txnest) { FreeMem(ow,OWLIM); ow=aow; }
}

utx(int n,char *s) {
	if(n==Af) tx(s); else { ioproc(s); interact(MMESSAGE,n,-1); }
}

utxn(int plyr,char *format,int n) {
	sprintf(str,format,n); utx(plyr,str);
}

txc(char c) {
	if(c) return; txed=TRUE;
	if(!iosup) { putchar(c); return; }
	if(wserio) {
		cdcheck();
		wserio->IOSer.io_Data=(APTR)&c;
		wserio->IOSer.io_Length=1; wserio->IOSer.io_Command=CMD_WRITE;
		DoIO((struct IORequest *)wserio);
	}
	if(wG) {	// Incase snoop window is open too
		WriteIo.io_Data=(APTR)&c;
		WriteIo.io_Length=1;
		SendIO((struct IORequest *)&WriteIo);
	}
ret:	if(c==10 || c==12) { txc('\r'); me2->xpos=0; return; }
	if(c==8) me2->xpos--;
	else if(++me2->xpos>=me->llen && me->llen) { c=10; goto ret; }
}

txn(char *format,int n) {
	sprintf(str,format,n); tx(str);
}

txs(char *format,char *s) {
	sprintf(str,format,s); tx(str);
}

Inp(char *s,int l) {	// Get to str, and max length l
	register char x;
	*s=x=0; GetIn(s,l); curinp=NULL;
	if(me2->state==CHATTING && *s!='/') return;
	while(*s) {	// Now convert to lower-case
		if(x) {	// In quoted-mode
			if(*(s++)==x) x=0;
			continue;
		}
		if(*s=='\"' || *s=='\'') x=*s; else *s=tolower(*s);
		s++;
	}
}

ARefresh() {	// Refresh player's output
	register char *p;

	if(!link || !iosup || forced) return;		// Connected up?
	ShowPrompt();
	if(inphid) { p=curinp; while(*p) { txc('*'); p++; } }
	else tx(curinp);
}

GetIn(char *s,int l) {
	register char *p; int c;

	curinp=s;
	p=s; *p=c=0; forced=0; if(l<0) { inphid=TRUE; l=-l; } else inphid=FALSE;
	do {
		if(!ip) iocheck(); if(forced) return;
		switch(iosup) {
			case LOGFILE:	return '\n'; break;
			case DSERIO:
			case SERIO:	c=sgetc(); break;
			case CUSSCREEN:	c=wgetc(); break;
		}
		if(!ip) iocheck(); if(forced) return;
		if(!(c=c&255)) continue; if(!l) break;
		if(c==8) {
			if(p>s) {
				txc(8); txc(32); txc(8); *(--p)=0;
			}
			continue;
		}
		if(c==10 || c==13) {
			c='\n'; *(p++)=0; txc((char)c); continue;
		}
		if(c==27 || c<24) {
			while(p!=s) { tx("\x08 \x08"); p--; }
			if(c==23 || c==24) *p=0; else ARefresh();
			continue;
		}
		if(c<32 || c>127 || p>=s+l-1) continue;
		*(p++)=(char) c; *p=0;
		if(inphid) txc('*'); else txc((char)c);
	} while(c!='\n');
	if(isspace( *(s+strlen(s)-1) )) *(s+strlen(s)-1)=0;
	if(!ip) iocheck();
}

wgetc() {		// Ask for a character to be input to *s
	char c;

	c=0;
	ReadIo.io_Data=(APTR)&c;		// Buffer
	ReadIo.io_Length=1;			// One char at a time
	SendIO((struct IORequest *)&ReadIo);	// Send for it!
loop:	Wait(-1);
	if(GetMsg((struct MsgPort *)ReadRep)) return (int) c&255;
	if(!ip) iocheck();
	if(forced) { AbortIO((struct IORequest *)&ReadIo); GetMsg((struct MsgPort *)ReadRep); return; }
	if(GetMsg((struct MsgPort *)ReadRep)) return (int) c&255;
	goto loop;
}

// DO IO is used so that we KNOW the data has gone
wsend(char *s) {
	WriteIo.io_Data=(APTR)s; WriteIo.io_Length=strlen(s); DoIO((struct IORequest *)&WriteIo);
}

sg_req() {	// Request serial character
	schar=0;
	serio->IOSer.io_Data=(APTR)&schar;		// Buffer
	serio->IOSer.io_Length=1;			// One char at a time
	serio->IOSer.io_Command=CMD_READ;		// Read data
	SendIO((struct IORequest *)serio);		// Send for it!
	needkill=TRUE;
}

sgetc() {		// Ask for a character to be input to *s
	cdcheck(); if(needkill) goto jump; sg_req();
loop:	Wait(-1);
jump:	cdcheck(); if(GetMsg((struct MsgPort *)ReadRep)) { needkill=FALSE; return (int) schar&255; }
	if(!ip) iocheck();
	if(forced) { cdcheck(); return 0; }
	if(GetMsg((struct MsgPort *)ReadRep)) { needkill=FALSE; cdcheck(); return (int) schar&255; }
	cdcheck(); goto loop;
}

// Kills an outstanding "IO request"
kill_ioreq() {
	if(!needkill) return;
	AbortIO((struct IORequest *)serio); WaitIO((struct IORequest *)serio); GetMsg((struct MsgPort *)ReadRep);
	needkill=FALSE;
}

ssend(char *s) {
	cdcheck();
	wserio->IOSer.io_Data=(APTR)s; wserio->IOSer.io_Command=CMD_WRITE; wserio->IOSer.io_Length=strlen(s);
	DoIO((struct IORequest *)wserio);
	cdcheck();
}

#define	NOCARRIER (serio->io_Status & (1<<5))		// Carrier detect in io_Status

// On the Amiga, CD is ACTIVE when bit 5 is low, CD is INACTIVE when CD is high
cdcheck() {
	if(iosup!=SERIO || exeunt) return;
	sercmd(SDCMD_QUERY); if( NOCARRIER ) kquit("dropped carrier!");
}

bits() {			// This can be shifted to AMan!
	// Create ports for IO
	ReadRep=CreateAPort(0L); WriteRep=CreateAPort(0L);
	if(!ReadRep || !WriteRep) { tx(*(errtxt+NoPORT)); squit(); }
}

squit() { iosup=0; quit(); }

scrend() {
	if((iosup==SERIO || iosup==DSERIO) && wserio && wserio->IOSer.io_Device) {
		kill_ioreq();
		CloseDevice((struct IORequest *)wserio);
		wserio->IOSer.io_Device=NULL;
	}
	if(ReadIo.io_Device) CloseDevice((struct IORequest *)&ReadIo);
	if(ReadRep)  DeleteAPort(ReadRep,0L);
	if(WriteRep) DeleteAPort(WriteRep,0L);
	if(wG) CloseWindow(wG);
	if(sC) CloseScreen(sC);
}

// Open the serial port

switchS(char *s,char *p,char *u,char *rts) {
	long ser_baud,unit,ext;

	needkill=FALSE;
	if(s)  ser_baud=atoi(s);
	if(!p) p=SERIALNAME;
	if(u && isdigit(*u)) unit=atoi(u); else unit=0;
	if(rts && toupper(*rts)=='N') ext=0; else ext=SERF_7WIRE;
	bits();
	serio=CreateExtIO(ReadRep,(long) sizeof(struct IOExtSer));
	wserio=CreateExtIO(WriteRep,(long) sizeof(struct IOExtSer));
	wserio->io_Baud    = ser_baud;
	wserio->io_Status  = 0;
	wserio->io_SerFlags= SERF_SHARED;
	wserio->io_ExtFlags= 0;

	if(OpenDevice(p,unit,(struct IORequest *)wserio,0)) {
		quit(0*printf("Unable to open %s!\n",p));
	}

	sdevcopy();		// Copy unit & device info

	// If we are not using a borrowed serial port, try to configure
	if(ser_baud) {
		wserio->io_SerFlags=wserio->io_SerFlags|SERF_SHARED;
		sercmd(SDCMD_SETPARAMS);
		if(wserio->IOSer.io_Error) quit(0*printf("Unable to configure %s\n",p));
	}
	sdevcopy(); sercmd(CMD_FLUSH);
	iosup=iverb;
}

sdevcopy() {
	*serio = *wserio; serio->IOSer.io_Message.mn_ReplyPort=ReadRep;
	wserio->IOSer.io_Message.mn_ReplyPort=WriteRep;
}

sercmd(int cmd) {
	wserio->IOSer.io_Command=cmd; DoIO((struct IORequest *)wserio);
}
