// user message definition parser

#include "amulcom.includes.h"

using namespace AMUL::Logging;
using namespace Compiler;

bool
umsg_proc()
{
	char *s;

	umsgs = 0;
	OS::CreateFile("umsg.tmp");
	fopena(umsgifn);
	ofp1 = afp;
	afp = NULL;
	fseek(ofp1, 0, 2L);
	fopena(umsgfn);
	ofp2 = afp;
	afp = NULL;
	fseek(ofp2, 0, 2L);
	fopena("umsg.tmp");
	if (nextc(0) == -1) {
		close_ofps();
		return false;
	} /* None to process */
	blkget(&datal, &data, 0L);
	s = data;

	do {
	loop:
		do
			s = sgetl(s, block);
		while (com(block) == -1 && *s != 0);
		if (*s == 0)
			break;
		tidy(block);
		if (block[0] == 0)
			goto loop;
		striplead("msgid=", block);
		getword(block);
		if (Word[0] == 0)
			goto loop;

		if (Word[0] == '$') {
			GetLogger().errorf("Invalid ID: %s: '$' is reserved for System Messages", Word);
			skipblock();
			goto loop;
		}
		if (strlen(Word) > IDL) {
			GetLogger().errorf("Invalid ID: %s: too long", Word);
			skipblock();
			goto loop;
		}
		umsgs++; /* Now copy the text across */
		strcpy(umsg.id, Word);
		umsg.fpos = ftell(ofp2);
		fwrite(umsg.id, sizeof(umsg), 1, afp);
		fwrite((char *)&umsg.fpos, 4, 1, ofp1);
		do {
			while (*s != 0 && com(s) == -1)
				s = skipline(s);
			if (*s == 0 || *s == 13) {
				*s = 0;
				break;
			}
			if (*s == 9)
				s++;
			if (*s == 13) {
				block[0] = 13;
				continue;
			}
			s = sgetl(s, block);
			if (block[0] == 0)
				break;
			umsg.fpos = strlen(block);
			if (block[umsg.fpos - 1] == '{')
				block[--umsg.fpos] = 0;
			else
				strcat(block + (umsg.fpos++) - 1, "\n");
			fwrite(block, 1, umsg.fpos, ofp2);
		} while (*s != 0 && block[0] != 0);
		fputc(0, ofp2);
	} while (*s != 0);
	close_ofps();

	OS::Free(data, datal);

	GetContext().terminateOnErrors();

	return true;
}

/* Check FP for umsg id! */
int
isumsg(char *s, FILE *fp)
{
	int i;

	if (*s == '$') {
		i = atoi(s + 1);
		if (i < 1 || i > NSMSGS) {
			GetLogger().error("Invalid System Message ID: %s", s);
			return -1;
		}
		return i - 1;
	}
	if (umsgs == 0)
		return -1;
	fseek(fp, 0, 0L); /* Rewind file */
	for (i = 0; i < umsgs; i++) {
		fread(umsg.id, sizeof(umsg), 1, fp);
		if (stricmp(umsg.id, s) == NULL)
			return i + NSMSGS;
	}
	return -1;
}

int
ttumsgchk(char *s)
{
	s = skiplead("msgid=", s);
	s = skiplead("msgtext=", s);
	s = skipspc(s);
	if (*s == '\"' || *s == '\'')
		return msgline(s + 1);
	return chkumsg(s);
}

int
chkumsg(char *s)
{
	int   r;
	FILE *fp;

	if (*s != '$' && umsgs == 0)
		return -1;

	if ((fp = fopen("umsg.tmp", "rb+")) == NULL) {
		GetLogger().fatalf("Unable to access umsg.tmp");
	}
	r = isumsg(s, fp);
	fclose(fp);
	return r;
}

int
msgline(char *s)
{
	FILE *  fp;
	int32_t pos;
	char	c;
	fp = afp;
	afp = NULL;
	fopena(umsgfn);
	fseek(afp, 0, 2L);
	pos = ftell(afp);

	fwrite(s, strlen(s) - 1, 1, afp);
	if ((c = *(s + strlen(s) - 1)) != '{') {
		fputc(c, afp);
		fputc('\n', afp);
	}
	fputc(0, afp);
	fopena(umsgifn);
	fseek(afp, 0, 2L);
	fwrite((char *)&pos, sizeof(int32_t), 1, afp);
	fclose(afp);
	afp = fp;
	return NSMSGS + (umsgs++);
}
