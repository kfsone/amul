#include "amulcominc.h"

void
close_ofps()
{
	if (ofp1 != NULL)
		fclose(ofp1);
	if (ofp2 != NULL)
		fclose(ofp2);
	if (ofp3 != NULL)
		fclose(ofp3);
	if (ofp4 != NULL)
		fclose(ofp4);
	if (ofp5 != NULL)
		fclose(ofp5);
	if (afp != NULL)
		fclose(afp);
	ofp1 = ofp2 = ofp3 = ofp4 = ofp5 = afp = NULL;
}

/* Find the next real stuff in file */
char
nextc(int f)
{
	do {
		while ((c = fgetc(ifp)) != EOF && isspace(c))
			;
		if (c == ';' || c == '*')
			fgets(block, 1024, ifp);
		if (c == '*')
			printf("[3m*%s[0m", block); /* Print cmts */
	} while (c != EOF && (c == '*' || c == ';' || isspace(c)));
	if (f == 1 && c == EOF) {
		printf("\nFile contains NO data!\n\n");
		quit();
	}
	if (c == EOF)
		return -1;
	fseek(ifp, -1, 1); /* Move back 1 char */
	return 0;
}

void
quit()
{
	if (exi != 1) {
		sprintf(block, "%s%s", dir, advfn);
		unlink(block);
	}
	unlink("ram:ODIDs");
	unlink("ram:umsg.tmp");
	unlink("ram:objh.tmp");
	if (mobdat)
		FreeMem(mobdat, moblen);
	mobdat = NULL;
	if (mobp)
		FreeMem(mobp, sizeof(mob) * mobchars);
	mobp = NULL;
	if (rmtab != 0)
		FreeMem(rmtab, sizeof(room) * rooms);
	rmtab = NULL;
	if (data != 0)
		FreeMem(data, datal);
	data = NULL;
	if (data2 != 0)
		FreeMem(data2, datal2);
	data2 = NULL;
	if (obtab2 != 0)
		FreeMem(obtab2, obmem);
	obtab2 = NULL;
	if (vbtab != 0)
		FreeMem(vbtab, vbmem);
	vbtab = NULL;
	if (ifp != NULL)
		fclose(ifp);
	ifp = NULL;
	close_ofps();
	exit(0);
}

/* Open file for reading */
FILE *
fopenw(char *s)
{
	FILE *tfp;
	if (*s == '-')
		strcpy(fnm, s + 1);
	else
		sprintf(fnm, "%s%s", dir, s);
	if ((tfp = fopen(fnm, "wb")) == NULL)
		Err("write", fnm);
	if (ofp1 == NULL)
		ofp1 = tfp;
	else if (ofp2 == NULL)
		ofp2 = tfp;
	else if (ofp3 == NULL)
		ofp3 = tfp;
	else if (ofp4 == NULL)
		ofp4 = tfp;
	else
		ofp5 = tfp;
	return NULL;
}

/* Open file for appending */
FILE *
fopena(char *s)
{
	if (afp != NULL)
		fclose(afp);
	if (*s == '-')
		strcpy(fnm, s + 1);
	else
		sprintf(fnm, "%s%s", dir, s);
	if ((afp = fopen(fnm, "rb+")) == NULL)
		Err("create", fnm);
	return NULL;
}

/* Open file for reading */
FILE *
fopenr(char *s)
{
	if (ifp != NULL)
		fclose(ifp);
	if (*s != '-')
		sprintf(fnm, "%s%s", dir, s);
	else
		strcpy(fnm, s + 1);
	if ((ifp = fopen(fnm, "rb")) == NULL)
		Err("open", fnm);
}

void
Err(char *s, char *t)
{
	printf("## Error!\x07\nCan't %s %s!\n\n", s, t);
	quit();
}

/* Open file for reading */
int32_t
rfopen(char *s)
{
	FILE *fp;

	if (*s != '-')
		sprintf(fnm, "%s%s", dir, s);
	else
		strcpy(fnm, s + 1);
	if ((fp = fopen(fnm, "rb")) == NULL)
		Err("open", fnm);
	return (int32_t)fp;
}

/* Update room entries after TT */
void
ttroomupdate()
{
	fseek(afp, 0, 0L);
	fwrite(rmtab->id, sizeof(room), rooms, afp);
}

void
opentxt(char *s)
{
	sprintf(block, "%s%s.TXT", dir, s);
	if ((ifp = fopen(block, "rb")) == NULL) {
		printf("[33;1m !! Missing file %s !! [0m\n\n", block);
		exit(202);
	}
}

void
skipblock()
{
	char c, lc;

	lc = 0;
	c = '\n';
	while (c != EOF && !(c == lc == '\n')) {
		lc = c;
		c = fgetc(ifp);
	}
}

// trim both ends of a string, and replace tabs with spaces
void
tidy(char *s)
{
	char *p = s;
	while (*p && isspace(*p))
		p++;
	if (!*p)
		return;
	while (*p) {
		if (*p == '\t')
			*p = ' ';
		if (*p != ' ')
			s = p;
		++p;
	}
	*(s + 1) = 0;
}

int
is_verb(char *s)
{
	int i;

	if (verbs == 0 || strlen(s) > IDL) {
		printf("@! illegal verb.\n");
		return -1;
	}

	if (stricmp(s, verb.id) == 0)
		return (verbs - 1);

	vbptr = vbtab;
	for (i = 0; i < verbs; i++, vbptr++) {
		if (stricmp(vbptr->id, s) == 0)
			return i;
	}
	return -1;
}

void
blkget(int32_t *s, char **p, int32_t off)
{
	*s = filesize() + off;
	if ((*p = (char *)OS::Allocate(*s)) == NULL) {
		tx("\x07\n** Out of memory!\n\n");
		close_ofps();
		quit();
	}
	fread((*p) + off, 1, *s, ifp);
	*((*p + *s) - 2) = 0;
	*((*p + *s) - 1) = 0;
}

/* Return size of current file */
int32_t
filesize()
{
	int32_t now, s;

	now = ftell(ifp);
	fseek(ifp, 0, 2L);
	s = ftell(ifp) - now;
	fseek(ifp, now, 0L);
	return s + 2; /* Just for luck! */
}
