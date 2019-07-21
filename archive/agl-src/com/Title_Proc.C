//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Title_Proc.C	Process System.Txt and Title.Txt
//
//	LMod: oliver 11/06/93	AMUL->AGL
//

#define	SO_NAME		0
#define	SO_SESH		SO_NAME+1
#define	SO_SEE1		SO_SESH+1
#define	SO_SEE2		SO_SEE1+1
#define	SO_MSGO		SO_SEE2+1
#define	SO_RSCL		SO_MSGO+1
#define	SO_TSCL		SO_RSCL+1
#define	SO_SCRN		SO_TSCL+1
#define	SO_FONT		SO_SCRN+1
#define	SO_LOG		SO_FONT+1
#define	SYSOPTS		SO_LOG+1

#define	SOT_STRING	0
#define	SOT_NUMBER	1
#define	SOT_RANK	2
#define	SOT_TIME	3

char *sysopt[SYSOPTS]={
	"name",		"session",	"iseeinvis",	"seeinvis",
	"minsgo",	"rankscale",	"timescale",	"screen",
	"font",		"log"
};

sys_proc()
{	register char *p,*s,*p2;	register int i;

	nextc(1); cleanget(&datal,&data,0L); p=data;
	*adname=0; mins=invis=invis2=minsgo=rscale=tscale=-2; tx("Proc:");
	scrx=scry=scrw=scrh=-1; scrm='n'; strcpy(logname,"AMUL.Log");

	do {
begin:		p=skipline(s=p); if(!*s) continue;
repeat:		s=skipspc(s); if(!*s) goto begin;
		p2=s; while(isalpha(*s)) s++;
		if(!strncmp(p2,"mins",(int)(s-p2))) goto repeat;
		if(*s!='=') { *s=0; error("Unknown phrase \"%s\".\n",p2); continue; }
		*(s++)=0;

		for(i=0; i<SYSOPTS; i++)
			if(!strcmp(p2,sysopt[i])) break;

		if(i==SYSOPTS) {
			error("Unkown option \"%s=\".\n",p2); continue;
		}

		p2=s;		// Now get the option, p2 points to it
		if(*p2=='\"' || *p2=='\'') {
			s++; while(*s && *s!=*p2) s++; p2++;
		}
		else	while(!isspace(*s) && *s) s++;
		if(*s) *(s++)=0;
		switch(i) {		// What type?
			case SO_NAME:			// Adventure name
				if(strlen(p2)>40) {
					warne("Adventure name too long! Truncated.\n");
					*(p2+40)=0;
				}
				strcpy(adname,p2); break;
			case SO_SESH:			// Session time
				if((mins=number(p2))==-1) {
					error("Invalid %s specified (%s).\n","session time",p2); break;
				}
				if(mins<15) {
					warne("Session time cannot be less than 15 minutes.\n"); mins=-1;
				}
				break;
			case SO_SEE1:			// Invisible see invis
				if((invis=number(p2))==-1)
					error("Invalid %s specified (%s).\n","'see invisible' level",p2);
				break;
			case SO_SEE2:			// visible see invis
				if((invis2=number(p2))==-1)
					error("Invalid %s specified (%s).\n","'visible can see invisible' level",p2);
				break;
			case SO_MSGO:
				if((minsgo=number(p2))==-1)
					error("Invalid %s specified (%s).\n","minimum supergo rank",p2);
				break;
			case SO_RSCL:
				if((rscale=number(p2))==-1)
					error("Invalid %s specified (%s).\n","rank-scaling amount",p2);
				break;
			case SO_TSCL:
				if((tscale=number(p2))==-1)
					error("Invalid %s specified (%s).\n","time-scaling amount",p2);
				break;
			case SO_SCRN:		// Screen spec
				scrx=listnum(p2); scry=listnum(p2);
				scrw=listnum(p2); scrh=listnum(p2);
				scrm=tolower(*p2);	// Mode
				if(scrx==-1 || scry==-1 || scrw==-1 || scrh==-1 || (scrm!='n' && scrm!='i'))
					error("Invalid %s specified (%s).\n","screen settings",p2);
				break;
			case SO_FONT:
				warne("'%s' option not implemented.\n",sysopt[i]); break;
			case SO_LOG:
				if(strlen(p2)>40) {
					error("Log-file name too long!\n"); break;
				}
				strcpy(logname,p2); break;
			default:
				error("** Int.err: sysopt[%ld]!\n",i);
		}
		if(*s) goto repeat;
	} while(*p);
	if(!*adname)	error("Missing adventure name.\n");
	if(mins==-2)	{	warne("No %s entry. Default %s enforced: %s.\n",sysopt[SO_SESH],"session time","15 minutes"); mins=15; }
	if(invis==-2)	{	warne("No %s entry. Default %s enforced: %s.\n",sysopt[SO_SEE1],"seeinvis value","100"); invis=100; }
	if(invis2==-2)	{	warne("No %s entry. Default %s enforced: %s.\n",sysopt[SO_SEE2],"iseeinvis value","100"); invis2=100; }
	if(minsgo==-2)		warne("No %s entry. %s%s will be disabled.\n",sysopt[SO_MSGO],"SUPER","GOing");
	if(rscale==-2)		warne("No %s entry. %s%s will be disabled.\n",sysopt[SO_RSCL],"rank","-scaling of values");
	if(tscale==-2)		warne("No %s entry. %s%s will be disabled.\n",sysopt[SO_TSCL],"timed","-scaling of values");

	errabort();		// Abort if an error
}

number(char *s) {
	if(!isdigit(*s)) return -1; return atoi(s);
}

listnum(char *s)
{	char *olds;	long n;

	olds=s;
	while(*s && isdigit(*s)) s++; if(*s) *(s++)=0;
	n=atoi(olds); strcpy(olds,s);
	return n;
}
