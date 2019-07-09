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
    rooms = Ad;
    rmtab = (struct room *)Ap;
    rescnt = (short *)Ap1;

    SendIt(MDATAREQ, 2, NULL); /* Get ranks data */
    ranks = Ad;
    rktab = (struct rank *)Ap;

    SendIt(MDATAREQ, 3, NULL); /* Get object headers */
    nouns = Ad;
    obtab = (struct obj *)Ap;

    SendIt(MDATAREQ, 4, NULL); /* Get verbs */
    verbs = Ad;
    vbtab = (struct verb *)Ap;

    SendIt(MDATAREQ, 5, NULL); /* Get descriptions */
    desctab = Ap;

    SendIt(MDATAREQ, 6, NULL); /* Get room table data */
    ormtab = (long)Ap;

    SendIt(MDATAREQ, 7, NULL); /* Get states! */
    statab = (struct state *)Ap;

    SendIt(MDATAREQ, 8, NULL); /* Get adjectives */
    adtab = Ap;

    SendIt(MDATAREQ, 9, NULL); /* Get travel table */
    ttp = (struct tt *)Ap;

    SendIt(MDATAREQ, 10, NULL); /* Get UMsg Indexes */
    umsgip = (long *)Ap;

    SendIt(MDATAREQ, 11, NULL); /* Get UMsg Text */
    umsgp = Ap;

    SendIt(MDATAREQ, 12, NULL); /* Get TT Params */
    ttpp = (long *)Ap;

    SendIt(MDATAREQ, 13, NULL); /* Get roomcount table */
    rctab = (short *)Ap;

    SendIt(MDATAREQ, 14, NULL); /* Get slot table */
    slottab = (struct vbslot *)Ap;

    SendIt(MDATAREQ, 15, NULL); /* Get vt table */
    vtp = (struct vt *)Ap;

    SendIt(MDATAREQ, 16, NULL); /* Get vtp table */
    vtpp = (long *)Ap;

    SendIt(MDATAREQ, 17, NULL); /* Get syn data */
    synp = Ap;
    synip = (short int *)Ad;

    SendIt(MDATAREQ, 18, NULL); /* Get last reset & create times */
    lastres = Ap;
    lastcrt = (char *)Ad;
}
