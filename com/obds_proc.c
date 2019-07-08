obds_proc()
{	char	lastc;

	obdes=0;err=0;
	fopenw(obdsfn); close_ofps();		/* Create file */
	if(nextc(0)==-1) return tx("** No Long Object Descriptions!\n");
	fopenw("-ram:ODIDs");	fopenw(obdsfn);
	do
	{
		fgets(block,1024,ifp); tidy(block);
		striplead("desc=",block); getword(block);
		if(strlen(Word)<3 || strlen(Word)>IDL)
		{
			printf("!! \x07 Invalid ID: \"%s\"\x07 !!\n",Word);
			printf("@! note: strlen(Word)=%ld\n",strlen(Word));
			err++; skipblock(); continue;
		}
		strcpy(objdes.id,Word);
		fseek(ofp2,0,2); objdes.descrip=ftell(ofp2);
		fwrite(objdes.id,sizeof(objdes),1,ofp1);
		lastc='\n';
		while((c=fgetc(ifp))!=EOF && !(c=='\n' && lastc=='\n'))
		{
			if((lastc==EOF || lastc=='\n') && c==9) continue;
			fputc((lastc=c),ofp2);
		};
		fputc(0,ofp2);
		obdes++; nextc(0);
	} while(c!=EOF);
	if(err!=0)
	{
		printf("\n\n\x07!! Aborting due to %ld errors!\n\n",err);
		quit();
	}
	close_ofps();
}
