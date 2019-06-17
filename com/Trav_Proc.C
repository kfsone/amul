/*
     Travel Processing Routines for AMUL, Copyright (C) Oliver Smith, '90
     --------------------------------------------------------------------
  Warning! All source code in this file is copyright (C) KingFisher Software
*/

#include "com/Trav_Func.C"

trav_proc()			/*=* Process TRAVEL.TXT *=*/
{
	register int strip,lines,nvbs,i,ntt,t,r;
	register char *p; register long *l;

	nextc(1);		/* Move to first text */
	fopenw(ttfn); fopenw(ttpfn); fopena(rooms1fn);
	err=ntt=t=0;

	do
	{
loop1:		if(err>30)
		{
			printf("\x07** Maximum number of errors exceeded!\n");
			quit();
		}
		fgets(block,1000,ifp); if(feof(ifp)) continue; tidy(block);
		if(com(block)==-1 || block[0]==0) goto loop1;
		p=block; getword(block); striplead("room=",Word);
		if((rmn=isroom(Word))==-1)
		{
			printf("** Invalid room '%s'!\n",Word);
			err++; skipblock(); goto loop1;
		}
		if(roomtab->tabptr!=-1)
		{
			printf("\x07!! Room \"%s\" defined twice in travel table!\n",roomtab->id);
			err++; skipblock(); goto loop1;
		}
vbloop:		do fgets(block,1000,ifp); while(com(block)==-1);
		if(block[0]==0 || block[0]=='\n')
		{
			/* Only complain if room is not a death room */
			if((roomtab->flags & DEATH)!=DEATH && warn==1)
				printf("## Room \"%s\" has no TT entries!\n",roomtab->id);
			roomtab->tabptr=-2;
			ntt++; continue;
		}
		tidy(block);
		if(!striplead("verb=",block) && !striplead("verbs=",block))
		{
			printf("## Room %s: expected a VERB[S]= entry!\n",roomtab->id);
			err++; goto vbloop;
		}
		lines=0; verb.id[0]=0;
		roomtab->tabptr=t;roomtab->ttlines=0;
vbproc:		/* Process verb list */
		nvbs=0; tt.pptr=(long *)-1;
		l=(long *)temp; p=block;
		/* Break verb list down to verb no.s */
		do
		{
			p=getword(p);
			if(Word[0] == 0) break;
			if((*l=is_verb(Word))==-1)
			{
				printf("\nRoom \"%s\" has invalid verb, \"%s\"...\n",roomtab->id,Word);
				err++;
			}
			l++; nvbs++;
		} while(Word[0]!=0);
		if(nvbs == 0)
		{
			printf("Room \"%s\" has empty verb[s]= line!\n",roomtab->id);
			quit();
		}
		/* Now process each instruction line */
		do
		{
xloop:			strip=0; r=-1;
			block[0]=0; fgets(block,1000,ifp);
			if(feof(ifp)) break;
			if(block[0]==0 || block[0]=='\n') { strip=-1; continue; }
			tidy(block); if(com(block)==-1 || block[0]==0) goto xloop;
			if(striplead("verb=",block) || striplead("verbs=",block))
			{
				strip=1; break;
			}
			p=precon(block);	/* Strip pre-condition opts */
notloop:		p=getword(p);
			if(strcmp(Word,ALWAYSEP)==NULL)
			{
				tt.condition=CALWAYS; tt.action=-(1+AENDPARSE);
				goto write;
			}
			if(strcmp(Word,"not")==NULL || strcmp(Word,"!")==NULL)
			{
				r=-1*r; goto notloop;
			}
notlp2:			if(Word[0]=='!') { strcpy(Word,Word+1); r=-1*r; goto notlp2; }
			if((tt.condition=iscond(Word)) == -1)
			{
				tt.condition=CALWAYS;
				if((tt.action=isroom(Word))!=-1) goto write;
				if((tt.action=isact(Word))==-1)
				{
					printf("\x07Invalid condition, \"%s\", in TT entry for room \"%s\"...\n",
						Word,roomtab->id);
					err++; goto xloop;
				}
				goto gotohere;
			}
			p=skipspc(p);
			if((p=chkcparms(p,tt.condition,ofp2))==NULL) { err++; goto next; }
			if(r==1) tt.condition=-1-tt.condition;
			if(*p==0)
			{
				printf("\x07Room \"%s\" has entry with missing action!\n",roomtab->id);
				err++; goto xloop;
			}
			p=preact(p); p=getword(p);
			if((tt.action=isroom(Word))!=-1) goto write;
			if((tt.action=isact(Word))==-1)
			{
				printf("\x07Invalid action, \"%s\", in TT entry for room \"%s\"...\n",
					Word,(rmtab+rmn)->id);
				err++; goto xloop;
			}
gotohere:		if(tt.action==ATRAVEL)
			{
				printf("## Tried to call TRAVEL action from travel table! (grin)\n");
				err++; goto xloop;
			}
			p=skipspc(p);
			if((p=chkaparms(p,tt.action,ofp2))==NULL) { err++; goto next; }
			tt.action=0-(tt.action+1);
write:			roomtab=rmtab+rmn;
			l=(long *)temp;
			for(i=0; i<nvbs; i++)
			{
				if(i<nvbs-1) tt.pptr=(long *)-2; else tt.pptr=(long *)-1;
				tt.verb=*(l++);
				fwrite((char *)&tt.verb,sizeof(tt),1,ofp1);
				roomtab->ttlines++; t++; ttents++;
			}
			lines++;
next:			strip=0;
		} while(strip==0 && !feof(ifp));
		if(strip==1 && !feof(ifp)) goto vbproc;
		nextc(0);
		ntt++;
	} while(!feof(ifp));
	if(err==0 && ntt!=rooms && warn==1)
	{
		roomtab=rmtab;
		for(i=0; i<rooms; i++,roomtab++)
			if(roomtab->tabptr == -1 && (roomtab->flags & DEATH) != DEATH)
				printf("Room \"%s\" has no TT entry!\n",roomtab->id);
	}
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	ttroomupdate();
	close_ofps();
}
