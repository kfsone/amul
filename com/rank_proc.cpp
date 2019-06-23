#include "amulcominc.h"

rank_proc()			/*=* Process RANKS.TXT *=*/
{
	char	*p; int	n;

	nextc(1);
	fopenw(ranksfn);
	putchar('\n');

	ranks=0;n=0;err=0;

	do
	{
		fgets(block,1024,ifp); if(feof(ifp)) continue;
		if(com(block)==-1 || block[0]=='\n') continue;
		tidy(block); if(block[0]==0) continue;
		p=getword(block); if(chkline(p)!=0) continue;
		ranks++; rank.male[0]=0; rank.female[0]=0;
		if(strlen(Word)<3 || strlen(Word)>RANKL)
		{
			printf("!! \x07 Invalid Male Rank: \"%s\"\x07 !!\n",Word);
			quit();
		}
		n=0;
		do
		{
			if(Word[n]=='_') Word[n]=' ';
			rank.male[n]=rank.female[n]=tolower(Word[n]);
			n++;
		} while(Word[n-1]!=0);

		p=getword(p); if(chkline(p)!=0) continue;
		if(strcmp(Word,"=")!=NULL && strlen(Word)<3 || strlen(Word)>RANKL)
		{
			printf("\n!! \x07 Invalid Female Rank: \"%s\"\x07 !!\n",Word);
			quit();
		}
		if(Word[0]!='=')
		{
			n=0;
			do
			{
				if(Word[n]=='_') Word[n]=' ';
				rank.female[n]=tolower(Word[n]);
				n++;
			} while(Word[n-1]!=0);
		}

		p=getword(p); if(chkline(p)!=0) continue;
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.score, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.score=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.strength, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.strength=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number stamina, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.stamina=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min.dexterity, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.dext=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number wisdom, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.wisdom=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number experience, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.experience=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number magic points, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.magicpts=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number max weight carried, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.maxweight=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number max number of objects carried, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.numobj=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid number min. points per kill, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.minpksl=atoi(Word);

		p=getword(p);
		if(!isdigit(Word[0]))
		{
			printf("## Invalid task number, \"%s\"!\n",Word);
			err++; continue;
		}
		rank.tasks=atoi(Word);

		p=skipspc(p); if(*p=='\"') p++;
		strcpy(block,p); p=block;
		while(*p!=0 && *p!='\"') p++;
		*(p++)=0;
		if(p-block>10)	/* Greater than prompt length? */
		{
			printf("\n\"%s\" prompt (rank %d) too long!\n",block,ranks);
			err++; continue;
		}
		if(block[0] == 0) strcpy(rank.prompt,"$ ");
		else strcpy(rank.prompt,block);

		wizstr=rank.strength;
		fwrite(rank.male,sizeof(rank),1,ofp1);
	} while(!feof(ifp));
	if(err!=0)
	{
		printf("\n\n\x07!! Aborting due to %ld errors!\n\n",err);
		quit();
	}
	close_ofps();
}

chkline(char *p)
{
	if(*p==0)
	{
		printf("## Rank line %ld incomplete!!!\n",ranks);
		err++; return 1;
	}
	return 0;
}
