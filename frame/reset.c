void
reset()
{
    SendIt(MDATAREQ, 0, dir); /* Get basic data */

    fopenr(advfn);
    fgets(adname, 41, ifp);
    adname[strlen(adname) - 1] = 0;
    fscanf(ifp, "%ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &rooms,
           &ranks, &verbs, &syns, &nouns, &adjs, &ttents, &umsgs, &word, &mins, &invis, &invis2,
           &minsgo, &mobs, &rscale, &tscale, &mobchars);

    SendIt(MDATAREQ, 1, NULL); /* Get rooms data */
    rooms = amul->data;
    rmtab = (struct room *)amul->ptr;
    rescnt = (short *)amul->p1;

    SendIt(MDATAREQ, 2, NULL); /* Get ranks data */
    ranks = amul->data;
    rktab = (struct rank *)amul->ptr;

    SendIt(MDATAREQ, 3, NULL); /* Get object headers */
    nouns = amul->data;
    obtab = (struct obj *)amul->ptr;

    SendIt(MDATAREQ, 4, NULL); /* Get verbs */
    verbs = amul->data;
    vbtab = (struct verb *)amul->ptr;

    SendIt(MDATAREQ, 5, NULL); /* Get descriptions */
    desctab = amul->ptr;

    SendIt(MDATAREQ, 6, NULL); /* Get room table data */
    ormtab = (long)amul->ptr;

    SendIt(MDATAREQ, 7, NULL); /* Get states! */
    statab = (struct state *)amul->ptr;

    SendIt(MDATAREQ, 8, NULL); /* Get adjectives */
    adtab = amul->ptr;

    SendIt(MDATAREQ, 9, NULL); /* Get travel table */
    ttp = (struct tt *)amul->ptr;

    SendIt(MDATAREQ, 10, NULL); /* Get UMsg Indexes */
    umsgip = (long *)amul->ptr;

    SendIt(MDATAREQ, 11, NULL); /* Get UMsg Text */
    umsgp = amul->ptr;

    SendIt(MDATAREQ, 12, NULL); /* Get TT Params */
    ttpp = (long *)amul->ptr;

    SendIt(MDATAREQ, 13, NULL); /* Get roomcount table */
    rctab = (short *)amul->ptr;

    SendIt(MDATAREQ, 14, NULL); /* Get slot table */
    slottab = (struct vbslot *)amul->ptr;

    SendIt(MDATAREQ, 15, NULL); /* Get vt table */
    vtp = (struct vt *)amul->ptr;

    SendIt(MDATAREQ, 16, NULL); /* Get vtp table */
    vtpp = (long *)amul->ptr;

    SendIt(MDATAREQ, 17, NULL); /* Get syn data */
    synp = amul->ptr;
    synip = (short int *)amul->data;

    SendIt(MDATAREQ, 18, NULL); /* Get last reset & create times */
    lastres = amul->ptr;
    lastcrt = (char *)amul->data;
}
