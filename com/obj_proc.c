objs_proc()
{	register char *p,*s; int roomno;

	nouns=adjs=err=0;

	/* Clear files */
	fopenw(adjfn); close_ofps();
	fopenw(objsfn); fopenw(statfn); fopenw(objrmsfn); fopena(adjfn);

	if(nextc(0)==-1) { close_ofps(); return 0; } /* Nothing to process */
	blkget(&obmem,(char **)&obtab2,32*sizeof(obj2)); objtab2=obtab2+32;
	s=(char *)objtab2;

	do
	{
		if(err>30)
		{
			printf("\x07** Maximum number of errors exceeded!\n");
			quit();
		}
		do p=s=sgetl(s,block); while(*s!=0 && (com(block)==-1 || block[0]==0));
		if(*s==0 || block[0]==0) continue;
		tidy(block); if(block[0]==0) continue;
		striplead("noun=",block); p=getword(block);
		if(strlen(Word)<3 || strlen(Word)>IDL)
		{
			printf("## \x07 Invalid ID: \"%s\"\x07 ##\n",Word);
			err++; Word[IDL+1]=0;
		}
		obj2.adj=obj2.mobile=-1; obj2.idno=nouns;
		obj2.state=obj2.nrooms=obj2.contains=obj2.flags=obj2.putto=0;
		obj2.rmlist=(long *) ftell(ofp3); strcpy(obj2.id,Word);

		/* Get the object flags */
		do
		{
			p=getword(p); if(Word[0] == 0) continue;
			if((roomno=isoflag1(Word))!=-1)
				obj2.flags=(obj2.flags | bitset(roomno));
			else
			{
				if((roomno=isoparm())==-1)
				{
					printf("\x07## Invalid parameter '%s'\n",Word);
					err++; continue;
				}
				switch(bitset(roomno))
				{
					case OP_ADJ:	set_adj(); break;
					case OP_START:	set_start(); break;
					case OP_HOLDS:	set_holds(); break;
					case OP_PUT:	set_put(); break;
					case OP_MOB:	set_mob(); mobs++; break;
					default:
						printf("** Internal: Code for object-parameter '%s' missing!\n",obparms[roomno]);
				}
			}
		} while(Word[0] != 0);

		/* Get the room list */

		p=block; *p='+'; *(p+1)=0; roomno=0;
		do
		{
			p=getword(p);
			if(Word[0] == '+')
			{
				do s=sgetl(s,block); while(*s!=0 && block[0]!=0 && com(block)==-1);
				if(*s==0)
				{
					printf("** Unexpected end of file in Objects.TXT!\n");
					quit();
				}
				p=block; Word[0]=' '; continue;
			}
			if(Word[0] == 0) continue;
			if((roomno=isloc(Word)) == -1) { roomno=-1; continue; }
			fwrite((char *)&roomno,1,4,ofp3);
			obj2.nrooms++;
		} while(Word[0] !=0);
		if(obj2.nrooms == 0  &&  roomno == 0)
		{
			printf("\x07!! No rooms listed for object '%s'...\n",obj2.id);
			err++;
		}
		obj2.nstates=0;
		do
		{
			do s=sgetl(s,block); while(block[0]!=0 && com(block)==-1 && block[0]!='\n');
			if(block[0] == 0 || block[0]=='\n') break;
			state_proc(); block[0]='-';
		} while(block[0]!=0 && block[0]!='\n');
		if(obj2.nstates==0 || obj2.nstates>100)
			object("amount of states (i.e. none)");
		if((long)(obtab2+(nouns)) > (long)s) printf("@! table exceeded data\n");
		*(obtab2+(nouns++))=obj2;
	} while(*s!=0);
	if(err!=0)
	{
		printf("\n\n!! Aborting due to %ld errors !!\n\n",err);
		quit();
	}
	/*
	close_ofps();
	sort_objs();
	*/
	fwrite((char *)obtab2,sizeof(obj2),nouns,ofp1);
	close_ofps();
}

/*
sort_objs()
{	register int i,j,k,nts; long *rmtab,*rmptr;

	if(ifp!=NULL) fclose(ifp); ifp=NULL;
	close_ofps(); fopenr(statfn);	blkget(&datal,&data,NULL); fclose(ifp); ifp=NULL;
	close_ofps(); fopenr(objrmsfn); blkget(&datal2,&data2,NULL); fclose(ifp); ifp=NULL;
	close_ofps(); fopenw(objsfn);	fopenw(statfn); fopenw(objrmsfn); fopenw(ntabfn);
	ifp=NULL;

	printf("Sorting Objects...:\r"); objtab2=obtab2; nts=0; k=0;

	statab=(struct _OBJ_STATE *)data; rmtab=(long *)data2;
	for(i=0; i<nouns; i++)
	{
		if(*(objtab2=(obtab2+i))->id==0)
		{
			printf("@! skipping %ld states, %ld rooms.\n",objtab2->nstates,objtab2->nrooms);
			statab += objtab2->nstates;
			rmtab  += objtab2->nrooms;
			continue;
		}
		strcpy(nountab.id,objtab2->id); nts++;
		nountab.num_of=0; osrch=objtab2; statep=statab; rmptr=rmtab;
		for(j=i; j<nouns; j++, osrch++)
		{
			if(*(osrch->id)!=0 && stricmp(nountab.id,osrch->id)==NULL)
			{
				fwrite((char *)osrch,  sizeof(obj),   1,               ofp1);
				fwrite((char *)statep, sizeof(state), osrch->nstates,  ofp2);
				fwrite((char *)rmptr,  sizeof(long),  osrch->nrooms,   ofp3);
				nountab.num_of++; *osrch->id=0; if(osrch!=objtab) k++;
				statep+=osrch->nstates; rmptr+=osrch->nrooms;
				if(osrch==objtab2) { statab=statep; rmtab=rmptr; objtab2++; i++; }
			}
			else statep+=osrch->nstates; rmptr+=osrch->nrooms;
		}
		
		fwrite((char *)&nountab, sizeof(nountab), 1, ofp4);
	}
	printf("%20s\r%ld objects moved.\n"," ",k);
	close_ofps();
	FreeMem(data, datal); FreeMem(data2, datal2); data=data2=NULL;
	fopenr(objsfn); fread((char *)obtab2, sizeof(obj), nouns, ifp);
}
*/

statinv(register char *s)
{
	printf("\nObject #%d \"%s\" has invalid %s state line!\n",nouns+1,obj2.id,s,block);
	quit();
}

state_proc()
{	register int flag; register char *p;

	state.weight=state.value=state.flags=0; state.descrip=-1;

	tidy(block); if(block[0]==0) return;

	/* Get the weight of the object */
	striplead("weight=",block);
	p=getword(block); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("weight value on");
	state.weight=atoi(Word);
	if(obj2.flags & OF_SCENERY) state.weight = wizstr+1;

	/* Get the value of it */
	p=skipspc(p); striplead("value=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("value entry on");
	state.value=atoi(Word);

	/* Get the strength of it (hit points)*/
	p=skipspc(p); striplead("str=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("strength entry on");
	state.strength=atoi(Word);

	/* Get the damage it does as a weapon*/
	p=skipspc(p); striplead("dam=",p);
	p=getword(p); if(*p==0) statinv("incomplete");
	if(!isdigit(Word[0]) && Word[0]!='-') statinv("damage entry on");
	state.damage=atoi(Word);

	/* Description */
	p=skipspc(p); striplead("desc=",p);
	if(*p==0) statinv("incomplete");
	if(*p=='\"' || *p=='\'') { text_id(p+1,*p); p=block; }
	else
	{
		p=getword(p); is_desid();	/* Is it valid? */
	}
	if(state.descrip==-1)
	{
		sprintf(temp,"desc= ID (%s) on",Word); statinv(temp);
	}
	while(*p!=0)
	{
		p=getword(p); if(Word[0]==0) break;
		if((flag=isoflag2(Word))==-1) statinv("flag on");
		state.flags=(state.flags | bitset(flag));
	}
	fwrite((char *)&state.weight,sizeof(state),1,ofp2);
	obj2.nstates++;
}

is_desid()
{	register int i; register FILE *fp;
	if(stricmp(Word,"none")==NULL) return state.descrip=-2;
	if((fp=fopen("ram:ODIDs","rb+"))==NULL) Err("open","ram:ODIDs");
	for(i=0;i<obdes;i++)
	{
		fread(objdes.id,sizeof(objdes),1,fp); state.descrip=objdes.descrip;
		if(stricmp(Word,objdes.id)==0)
		{
			fclose(fp); return;
		}
	}
	fclose(fp); state.descrip=-1;
}

text_id(register char *p,register char c)
{	char *ptr; FILE *fp;

	strcpy(block,p); p=block;
	while(*p!=c && *p!=0) p++;
	if(*p==0) *(p+1)=0;
	*(p++)='\n';
	if(*(p-2) == '{') ptr=p-1; else ptr=p;

	sprintf(temp,"%s%s",dir,obdsfn);	/* Open output file */
	if((fp=fopen(temp,"rb+"))==NULL) Err("open",temp);
	fseek(fp,0,2L); state.descrip=ftell(fp); /* Get pos */
	if(fwrite(block,ptr-block,1,fp)!=1) { fclose(fp); Err("write",temp); }
	fputc(0,fp); strcpy(block,p); fclose(fp);
}

isnoun(register char *s)
{	register int i;

	objtab2=obtab2;
	if(stricmp(s,"none")==NULL) return -2;
	for(i=0; i<nouns; i++,objtab2++)
		if(stricmp(s,objtab2->id)==NULL) return i;
	return -1;
}

iscont(register char *s)
{	register int i;

	objtab2=obtab2;
	for(i=0; i<nouns; i++,objtab2++)
		if(stricmp(s,objtab2->id)==NULL && objtab2->contains>0) return i;
	return -1;
}

isloc(register char *s)		/* Room or container */
{	register int i;

	if((i = isroom(s)) != -1) return i;
	if((i = iscont(s)) == -1)
	{
		if(isnoun(s) == -1)
			printf("\x07## Invalid object start location, '%s'...\n",s);
		else
			printf("\x07## Tried to start '%s' in non-container '%s'!\n",obj2.id,s);
		err++; return -1;
	}

	return -(INS+i);
}
