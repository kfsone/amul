//
// AMUD/com/Obds_Proc.C		Process Object Descriptions file
//

obds_proc() {
	char	lastc;

	obdes=0;
	fclose(fopenw(obdsfn));			// Create file
	if(nextc(0)==-1) return tx("** No entries.\n");
	ofp1=fopenw("-ram:ODIDs"); ofp2=fopenw(obdsfn); tx("Proc:");
	do {
		fgets(block,1024,ifp); clean_trim(block);
		getword(skiplead("desc=",skipspc(block)));
		if(strlen(Word)<3 || strlen(Word)>IDL) {
			error("Invalid ID: %s\n",Word);
			skipblock(); continue;
		}
		strcpy(objdes.id,Word);
		fseek(ofp2,0,2); objdes.descrip=ftell(ofp2);
		fwrite(objdes.id,sizeof(objdes),1,ofp1);
		lastc='\n';
		while((c=fgetc(ifp))!=EOF && !(c=='\n' && lastc=='\n')) {
			if((lastc==EOF || lastc=='\n') && c==9) continue;
			fputc((lastc=c),ofp2);
		};
		fputc(0,ofp2); obdes++; nextc(0);
	} while(c!=EOF);
	errabort();
}