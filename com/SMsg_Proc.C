/*
     System Message processing routines for AMUL, (C) KingFisher Software
     --------------------------------------------------------------------

 Notes:

	System messages MUST be listed in order, and MUST all exist! These
      should be supplied with the package, so the user has a set of defaults.
      We could write all the default system messages into AMULCOM, but this
      would simply be a waste of space!

*/

smsg_proc()
{	register char	*s;	register long id; long pos;

	err=smsgs=0;

	if(nextc(0)==-1) return;	/* Nothing to process! */
	fopenw(umsgifn); fopenw(umsgfn);	/* Text and index */

	blkget(&datal,&data,0L); s=data;

	do
	{
loop:		s=sgetl(s,block);
		if(com(block)==-1) goto loop;
		tidy(block); if(block[0]==0) continue;
		striplead("msgid=",block); getword(block);
		if(Word[0]==0) break;

		if(Word[0]!='$')
		{
			printf("\x07\n\n!! Invalid SysMsg ID, '%s'. SysMsgs MUST begin with a '$'!\n",Word);
			quit();
		}
		if(atoi(Word+1)!=smsgs+1)
		{
			printf("\x07\n\n!! Message %s out of sequence!\n\n",Word);
			quit();
		}
		if(smsgs>=NSMSGS)
		{
			printf("\x07\n\n!! Too many System Messages, only require %ld!\n\n",NSMSGS);
			quit();
		}
		id=++smsgs;	/* Now copy the text across */
		pos=ftell(ofp2); fwrite((char *)&pos,4,1,ofp1);
		do
		{
			while(*s!=0 && com(s)==-1) s=skipline(s);
			if(*s==0 || *s==13) { *s=0; break; }
			if(*s==9) s++; if(*s==13) { block[0]=13; continue; }
			s=sgetl(s,block); if(block[0]==0) break;
			pos=strlen(block);
			if(block[pos-1]=='{') block[--pos]=0;
			else strcat(block+(pos++)-1,"\n");
			fwrite(block,1,pos,ofp2);
		} while(*s!=0 && block[0]!=0);
		fputc(0,ofp2);
	} while(*s!=0);
	close_ofps(); FreeMem(data,datal);
	data=NULL; datal=NULL;
}

