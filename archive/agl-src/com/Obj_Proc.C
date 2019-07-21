//
//	AGL - Advanced Games Language (C) Oliver Smith/KFS, 1991-1993
//
//	File: Obj_Proc.C	Process Objects.Txt
//
//	LMod: oliver 20/06/93	Implemented article handling
//	      oliver 11/06/93	AMUL->AGL. Removed "bitset"
//

objs_proc() {
	register char *p,*s; int roomno;

	nouns=adjs=0;

	fopenw(adjfn); close_ofps();	// Create new file
	fopenw(objsfn); fopenw(statfn); fopenw(objrmsfn); fopena(adjfn);

	if(nextc(0)==-1) { close_ofps(); return 0; } // Nothing to process
	cleanget(&obmem,(char **)&obtab2,32*sizeof(obj2)); objtab2=obtab2+32;
	p=skipspc((char *)objtab2); tx("Proc:");

	do {
loop:		p=skipline(s=p); s=skipspc(s); if(!*s) continue;
		Word[IDL]=0; s=getword(skiplead("noun=",s)); Word[IDL+1]=0;
		if(Word[IDL] || !*Word || !Word[1]) {
			error("Invalid ID: %s\n",Word); p=skipdata(p); goto loop;
		}
		obj2.adj=obj2.mobile=-1; obj2.idno=nouns;
		obj2.state=obj2.nrooms=obj2.contains=obj2.flags=obj2.putto=obj2.article=0;
		obj2.rmlist=(long *) ftell(ofp3); strcpy(obj2.id,Word);

		// Get the object flags
		do {
			s=getword(s); if(!*Word) continue;
			if((roomno=isoflag1(Word))!=-1)
				obj2.flags=(obj2.flags | (1<<roomno));
			else {
				if((roomno=isoparm())==-1) {
					error("%s: Invalid paramter '%s'\n",obj2.id,Word);
					break;
				}
				switch(1<<roomno) {
					case OP_ART:	set_art(); break;
					case OP_ADJ:	set_adj(); break;
					case OP_START:	set_start(); break;
					case OP_HOLDS:	set_holds(); break;
					case OP_PUT:	set_put(); break;
					case OP_MOB:	set_mob(); mobs++; break;
					default:
						printf("%s: Invalid parameter '%s'\n",obparms[roomno]);
				}
			}
		} while(*Word);

		// Get the room list
		obj2.initstate=obj2.state; roomno=-2;

rmloop:		p=skipline(s=p); if(!*p) quit(error("%s: Unexpected end of file!\n",obj2.id));
		s=skipspc(s); if(!*s) goto rmloop;
rmsloop:	s=getword(s);
		if(*Word) {
			if((roomno=isloc(Word)) == -1) { roomno=-1; continue; }
			fwrite((char *)&roomno,1,4,ofp3);
			obj2.nrooms++; goto rmsloop;
		}
		if(!obj2.nrooms && roomno==-2) {
			error("%s: No locations!\n",obj2.id);
			p=skipdata(p); continue;
		}
		obj2.nstates=0;
		do {
stloop:			p=skipline(s=p); if(!*s) break;
			s=skipspc(s); if(*s) state_proc(s);
		} while(*p!=0);
		if(!obj2.nstates) state_proc("none");
		else if(obj2.nstates>100) object("amount of states");
		if((long)(obtab2+(nouns)) > (long)s) object("table overflow");
		*(obtab2+(nouns++))=obj2;
	} while(*p);
/*
	close_ofps();
	sort_objs();
*/
	if(!err) fwrite((char *)obtab2,sizeof(obj2),nouns,ofp1);
	errabort();		// Abort if an error
}

statinv(register char *s) {
	error("%s state %ld: %s!\n",obj2.id,obj.nstates+1,s);
}

#define	INCOMPLETE	"Incomplete state line"

state_proc(char *s) {
	register int flag; register char *p;

	state.weight=state.value=state.flags=0; state.descrip=-1;
	if(!strncmp(s,"none",4)) goto write;

	strcpy(block,s);

	// Get the weight of the object
	p=skiplead("weight=",skipspc(block));
	p=getword(block); if(!*p) return statinv(INCOMPLETE);
	if(!isdigit(Word[0]) && Word[0]!='-') return statinv("bad weight= value");
	state.weight=atoi(Word);
	if(obj2.flags & OF_SCENERY) state.weight = wizstr+1;

	// Get the value of it
	p=skipspc(p); p=skiplead("value=",p);
	p=getword(p); if(!*p) return statinv(INCOMPLETE);
	if(!isdigit(Word[0]) && Word[0]!='-') return statinv("bad value= value");
	state.value=atoi(Word);

	// Get the strength of it (hit points)
	p=skipspc(p); p=skiplead("str=",p);
	p=getword(p); if(!*p) return statinv(INCOMPLETE);
	if(!isdigit(Word[0]) && Word[0]!='-') return statinv("bad str= value");
	state.strength=atoi(Word);

	// Get the damage it does as a weapon
	p=getword(skiplead("dam=",skipspc(p))); if(!*p) return statinv(INCOMPLETE);
	if(!isdigit(Word[0]) && Word[0]!='-') return statinv("bad dam= value");
	state.damage=atoi(Word);

	// Description
	p=skiplead("desc=",skipspc(p));
	if(!*p) return statinv(INCOMPLETE);
	if(*p=='\"' || *p=='\'') { text_id(p+1,*p); p=block; }
	else {
		p=getword(p); is_desid();	// Is it valid?
	}
	if(state.descrip==-1) {
		sprintf(temp,"bad desc= ID (%s)",Word); return statinv(temp);
	}
	while(*p) {
		p=getword(p); if(!*Word) break;
		if((flag=isoflag2(Word))==-1) return statinv("bad state flag");
		state.flags=(state.flags | (1<<flag));
	}
write:	fwrite((char *)&state.weight,sizeof(state),1,ofp2);
	obj2.nstates++;
}

is_desid()
{	register int i; register FILE *fp;
	if(!strcmp(Word,"none")) return state.descrip=-2;
	if(!(fp=fopen("ram:ODIDs","rb+"))) Err("open","ram:ODIDs");
	for(i=0;i<obdes;i++) {
		fread(objdes.id,sizeof(objdes),1,fp); state.descrip=objdes.descrip;
		if(!strcmp(Word,objdes.id)) {
			fclose(fp); return;
		}
	}
	fclose(fp); state.descrip=-1;
}

text_id(register char *p,register char c) {
	FILE *fp;

	strcpy(block,p); p=block; while(*p!=c && *p) p++;
	if(*(p-1)=='{') p--; else *(p++)=' ';

	sprintf(temp,"%s%s",dir,obdsfn);	// Open output file
	if(!(fp=fopen(temp,"rb+"))) Err("open",temp);
	fseek(fp,0,2L); state.descrip=ftell(fp); // Get pos
	if(!fwrite(block,ptr-block,1,fp)) { fclose(fp); Err("write",temp); }
	fputc(0,fp); strcpy(block,p); fclose(fp);
}

isnoun(register char *s) {
	register int i;
	objtab2=obtab2;
	if(!strcmp(s,"none")) return -2;
	for(i=0; i<nouns; i++,objtab2++)
		if(!strcmp(s,objtab2->id)) return i;
	return -1;
}

iscont(register char *s) {
	register int i;
	objtab2=obtab2;
	for(i=0; i<nouns; i++,objtab2++)
		if(!strcmp(s,objtab2->id) && objtab2->contains>0) return i;
	return -1;
}

isloc(register char *s)		// Room or container
{	register int i;
	if((i = isroom(s)) != -1) return i;
	if((i = iscont(s)) == -1) {
		if(isnoun(s) == -1)
			error("%s: Invalid start location: '%s'\n",obj2.id,s);
		else
			error("%s: Location \"%s\" not a container!\n",obj2.id,s);
		return -1;
	}
	return -(INS+i);
}

#undef	INCOMPLETE
