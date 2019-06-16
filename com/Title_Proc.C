title_proc()
{
	nextc(1);	fgets(block,1000,ifp); repspc(block); remspc(block);
	if(!striplead("name=",block))
	{
		tx("Invalid title.txt; missing 'name=' line!\n");
		quit();
	}
	block[strlen(block)-1]=0;	/* Remove \n */
	if(strlen(block)>40)
	{
		block[40]=0;
		printf("Adventure name too long!            \nTruncated to %40s...\n",block);
	}
	strcpy(adname,block);
	fgets(block,1000,ifp); repspc(block);
	mins=getno("gametime=");
	if(mins<15) { tx("!! Minimum game time of 15 minutes inforced!\n"); mins=15; }

	fgets(block,1000,ifp); repspc(block);
	invis=getno("invisible="); remspc(block); getword(block);
	if(!isdigit(Word[0]))
	{
		printf("## Invalid rank for visible players to see other invisible players/objects.\n");
		err++;
	}
	else invis2=atoi(Word);

	fgets(block,1000,ifp); repspc(block);
	minsgo=getno("min sgo=");

	/*-* Get the Scaleing line. *-*/
	fgets(block,1000,ifp); repspc(block);
	rscale=getno("rankscale=");		/* Process RankScale= */
	tscale=getno("timescale=");		/* Process TimeScale= */

	readgot=ftell(ifp);

	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
}

getno(char *s)
{	char *p;
	remspc(block);
	if(!striplead(s,block))
	{
		printf("## Missing %s entry!\n",s); err++; return -1;
	}
	p=getword(block); strcpy(block,p);
	if(!isdigit(Word[0]))
	{
		printf("## Invalid %s entry...\n",s); err++; return -1;
	}
	return atoi(Word);
}
