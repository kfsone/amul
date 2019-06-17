reset()
{
	SendIt(MSG_DATA_REQUEST,0,dir);		/* Get basic data */

	fopenr(advfn);
	fgets(adname,41,ifp); adname[strlen(adname)-1]=0;
	fscanf(ifp,"%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld",&rooms,&ranks,&verbs,&syns,&nouns,&adjs,&ttents,&umsgs,&word,&mins,&invis,&invis2,&minsgo,&mobs,&rscale,&tscale,&mobchars);

	SendIt(MSG_DATA_REQUEST,1,NULL);	/* Get rooms data */
	rooms=Ad; rmtab=(struct room *)Ap; rescnt=(short *)Ap1;

	SendIt(MSG_DATA_REQUEST,2,NULL);	/* Get ranks data */
	ranks=Ad; rktab=(struct rank*)Ap;

	SendIt(MSG_DATA_REQUEST,3,NULL);	/* Get object headers */
	nouns=Ad; obtab=(struct obj *)Ap;

	SendIt(MSG_DATA_REQUEST,4,NULL);	/* Get verbs */
	verbs=Ad; vbtab=(struct verb *)Ap;

	SendIt(MSG_DATA_REQUEST,5,NULL);	/* Get descriptions */
	desctab=Ap;

	SendIt(MSG_DATA_REQUEST,6,NULL);	/* Get room table data */
	ormtab=(long)Ap;

	SendIt(MSG_DATA_REQUEST,7,NULL);	/* Get states! */
	statab=(struct state *)Ap;

	SendIt(MSG_DATA_REQUEST,8,NULL);	/* Get adjectives */
	adtab=Ap;

	SendIt(MSG_DATA_REQUEST,9,NULL);	/* Get travel table */
	ttp=(struct tt *)Ap;

	SendIt(MSG_DATA_REQUEST,10,NULL);	/* Get UMsg Indexes */
	umsgip=(long *)Ap;

	SendIt(MSG_DATA_REQUEST,11,NULL);	/* Get UMsg Text */
	umsgp=Ap;

	SendIt(MSG_DATA_REQUEST,12,NULL);	/* Get TT Params */
	ttpp=(long *)Ap;

	SendIt(MSG_DATA_REQUEST,13,NULL);	/* Get roomcount table */
	rctab=(short *)Ap;

	SendIt(MSG_DATA_REQUEST,14,NULL);	/* Get slot table */
	slottab=(struct vbslot *)Ap;

	SendIt(MSG_DATA_REQUEST,15,NULL);	/* Get vt table */
	vtp=(struct vt *)Ap;

	SendIt(MSG_DATA_REQUEST,16,NULL);	/* Get vtp table */
	vtpp=(long *)Ap;

	SendIt(MSG_DATA_REQUEST,17,NULL);	/* Get syn data */
	synp=Ap; synip=(short int *)Ad;

	SendIt(MSG_DATA_REQUEST,18,NULL);	/* Get last reset & create times */
	lastres=Ap; lastcrt=(char *)Ad;
}
