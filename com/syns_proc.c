
/*
	Routines to process/handle Synonyms
						*/

syn_proc()
{	register char *s,*t; short int no; register short int x;

	err=syns=0;	if(nextc(0)==-1) return;
	fopenw(synsfn); fopenw(synsifn);

	blkget(&datal,&data,0L); s=data;

	do
	{
		do s=sgetl(s,block); while(com(block)==-1);

		tidy(block); if(block[0]==0) continue;
		t=getword(block); t=skipspc(t);

		if((no=isnoun(Word)) < 0)
		{
			if((x=is_verb(Word))==-1)
			{
				printf("\x07!! Invalid verb/noun, \"%s\"...\n",Word);
				err++; continue;
			}
			no=-(2+x);
		}

		while(*t!=0)
		{
			t=getword(t); if(Word[0]==0) break;
			fwrite((char *)&no,1,sizeof(short int),ofp2);
			fprintf(ofp1,"%s%c",Word,0); syns++;
		}
	} while(*s!=0);
	close_ofps(); FreeMem(data,datal);
	data=NULL; datal=NULL;
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
}
