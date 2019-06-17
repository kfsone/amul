putc(char c)
{
	putchar(c);
}

tx(register char *s)
{	register int i,l; register char *p,*ls,*lp;

	if(addcr==YES && needcr==YES) txc('\n'); addcr=NO; needcr=NO;

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
			*(p++)='\n';
			needcr=NO;
		}
		if(*s=='\n')
		{
			*(p++)='\n';
			s++; needcr=NO;
		}
		*p=0;
		putc(spc);
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
	else interact(MSG_MESSAGE,n,-1);
}

utxn(register int plyr,register char *format,register int n)
{
	sprintf(str,format,n); utx(plyr,str);
}

txc(char c)
{
	putchar(c);
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

#define	NOCARRIER (serio->io_Status & (1<<5))		/* Carrier detect in io_Status */

/* On the Amiga, CD is ACTIVE when bit 5 is low, CD is INACTIVE when CD is high */
cdcheck()
{
	sercmd(SDCMD_QUERY); if( NOCARRIER ) kquit("dropped carrier!");
}

bits()
{
	/* Create ports for IO */
}

squit()
{
	quit();
}

scrend()
{
	/* close down io devices */
}
