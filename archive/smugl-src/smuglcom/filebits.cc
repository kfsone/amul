/*
 * filebits -- various file manipulation functions
 */

static const char rcsid[] = "$Id: filebits.cc,v 1.8 1997/05/22 02:21:36 oliver Exp $";

#include "smuglcom.hpp"
#include "libprotos.hpp"

static char *func_get(off_t off);
static size_t filesize(void);

void
close_ofps(void)
    {                           /* Close the common file handles */
    if (ofp1)
	fclose(ofp1);
    if (ofp2)
	fclose(ofp2);
    if (ofp3)
	fclose(ofp3);
    if (ofp4)
	fclose(ofp4);
    if (ofp5)
	fclose(ofp5);
    if (afp)
	fclose(afp);
    ofp1 = ofp2 = ofp3 = ofp4 = ofp5 = afp = NULL;
    }

/* Find the next "real data" in 'ifp'.
** If no data is found and required is non-zero, raises a fatal error */
int
nextc(int required)
    {
    char c;
    do
	{
	while ((c = fgetc(ifp)) != EOF && isspace(c));
	if (c == ';')
	    fgets(block, 1024, ifp);
	}
    while (c != EOF && (c == ';' || isspace(c)));
    if (required == 1 && c == EOF)
	quit("File contains NO data!\n");
    if (c == EOF)
	return -1;
    fseek(ifp, -1, 1);		/* Move back 1 char */
    return 0;
    }

void
fopenw(const char *s)
    {                           /* Open the next free 'ofp' for writing */
    FILE *tfp;
    char *file = datafile(s);
    if (!(tfp = fopen(file, "wb")))
	Err("write", file);
    if (!ofp1)
	ofp1 = tfp;
    else if (!ofp2)
	ofp2 = tfp;
    else if (!ofp3)
	ofp3 = tfp;
    else if (!ofp4)
	ofp4 = tfp;
    else
	ofp5 = tfp;
    }

void
fopena(const char *s)
    {				/* Open file for appending */
    char *file = datafile(s);
    if (afp)
	fclose(afp);
    if (!(afp = fopen(file, "rb+")))
	Err("create", file);
    }

void
fopenr(const char *s)
    {                           /* Open file for reading */
    char *file = datafile(s);
    if (ifp)
	fclose(ifp);
    if (!(ifp = fopen(file, "rb")))
	Err("open", file);
    }

FILE *
rfopen(const char *s)           /* Open file for reading */
    {
    FILE *fp;
    char *file = datafile(s);

    if (!(fp = fopen(file, "rb")))
	Err("open", file);
    return fp;
    }

/* Skip the current 'block' of text on 'ifp'. That is, search for
** the next \n\n terminator */
void
skipblock(void)
    {
    char c, lc;

    lc = 0;
    c = '\n';
    while (c != EOF && !(lc == '\n' && c == lc))
	{
	lc = c;
	c = fgetc(ifp);
	}
    }

/* The string-based equivalent of skipblock */
char *
skipdata(char *p)
    {
    char *s;
    do
	p = skipline(s = p);
    while (*s && *s != 10);
    return p;
    }

/* Reformat a block of text: Remove leading, multiple and trailing spaces,
** replace ambiguous end-of-line terminators with a consistant terminator,
** etc, etc */
void
tidy(char *s)
    {
    char *p = s;
    repspc(s);			/* Clean up whitespace */
    s = skipspc(s);
    if (s != p)
	{
	strcpy(p, s);		/* Remove that annoying leading whitespace */
	s = p;
	}
    s = s + strlen(s) - 1;
    while (isspace(*s))
	*(s--) = 0;
    }

/* get_line(file, into)
 * Read a line of text from a file. Understands line continuations
 * ('+' or '\' at EOL) and then calls 'clean_trim' on the text
 */
void
get_line(FILE * fp, char *into, int limit)
    {
    char *base = into;
    while (fgets(into, limit, fp))
	{
	int l = strlen(into);
	into += (l - 2);	/* Move to possible last char */
	limit -= l;
	if (*into != '\\' && *into != '+')
	    break;
	}
    clean_trim(base);
    }

/* clean_trim(string)
 * "Processes" (ala clean_up) a text string and removes trailing whitespace
 */
void
clean_trim(char *s)
    {
    clean_up(s);
    s = s + strlen(s) - 1;
    while (isspace(*s))
	*(s--) = 0;
    }

/* Allocate memory (with surplus) for reading a given file,
** and read the file into the buffer
** 'off' allows you to specify an additional quantity of surplus,
** useful if you intend to re-use the memory for output */
inline static char *
func_get(off_t off)
    {
    char *p;
    size_t size;
    if (!ifp)
	{
	error(">> no input file open for reading!\n");
	errabort();
	}
    size = (((filesize() + off) / 4096) + 1) * 4096;
    p = (char *)grow(NULL, size, "Reading file to memory");
    bzero(p, size);
    fread(p + off, 1, size - off, ifp);
    return p;
    }

/* Wrapper for 'func_get', which tidies up the end-of-line characters
** afterwards. Used for reading files that operate text blocks such
** as the rooms file */
char *
blkget()
    {
    char *p;
    char c, *s;
    p = func_get(0L);
    s = p;
    while ((c = *(s++)))
	{
	if (c == 13)
	    *(s - 1) = EOL;
	}
    return p;
    }

/* Wrapper for func_get, which tidies up the text using 'clean_up'.
** Used for reading any files which don't have text-blocks
*/
char *
cleanget(off_t off)
    {
    char *p;
    p = func_get(off);
    clean_up(p + off);
    return p;
    }

static size_t
filesize()                      /* Return size of current file (ifp) */
    {
    off_t now;
    size_t s;

    now = ftell(ifp);
    fseek(ifp, 0, 2L);
    s = ftell(ifp) - now;
    fseek(ifp, now, 0L);
    return (s + 4);		/* Just for luck! */
    }

/* Take a text block (p), process and output to desftp. Used for
** processing large text areas, e.g. room descriptions or umsgs */
char *
text_proc(char *p, FILE *destfp)
    {
    char overflow = 0;
    char *s;
    long LEN;
    do
	{
	p = skipline(s = p);    /* Next line */
	if (!*s)                /* Blank-line == end of block */
	    break;
        if (overflow)
            {                   /* We have an overflow character */
            fputc(overflow, destfp);
            overflow = 0;
            }
	if (*s == 9 && !*(s + 1))
	    *s = '\n';
	else if (*s == 9)
	    s++;
	LEN = p - s - 1;
	if (*(p - 2) == '{')
	    LEN--;              /* {<EOL> = don't add CRLF */
	else
            overflow = '\n';
	fwrite(s, sizeof(*s), (size_t)LEN, destfp);
	}
    while (*s);
    fputc(0, destfp);           /* Null terminate for easy handling */
    return p;
    }

void
opentxt(const char *s)
    {                           /* Open a game '.txt' file */
    /* Write to 'block' because some external callers want it there */
    sprintf(block, "%s%s.txt", dir, s);
    ifp = fopen(block, "rb");
    if (ifp == NULL)
	quit("## Missing file: %s!\n", block);
    }

/* get_word fetches the next 'word' from 's' into the static buffer
 * 'Word', upto 62 characters
 */
char *
getword(char *s)
    {
    char *dst;
    int bytes;

    *(dst = Word) = 0;		/* Clear Word */
    bytes = 62;

    s = skipspc(s);
    for (s = skipspc(s); *s && *s != SPC && *s != EOL && *s != CMT; bytes--)
	{
	if (bytes > 0)
	    *(dst++) = *(s++);
	}
    if (*s == CMT)
	*s = 0;
    else
	s = skipspc(s);
    *dst = 0;

    return s;
    }

/* skiplead - look for a matching string and return a pointer beyond it,
** if we find a match...
**  e.g. skiplead("goto=", "goto=daytime") returns "daytime" */
char *
skiplead(const char *skip, char *in)
    {
    char *p;
    /* Ignore any leading white space */
    if (*in == SPC)
	p = skipspc(in);
    else
	p = in;
    while (*skip && *skip == *(p++))
	skip++;
    /* In case of a match, *skip will be 0 */
    if (*skip == 0)
	return p;
    return in;
    }

/* skipline -- returns a pointer to the beginning of the next line */
char *
skipline(char *s)
    {
    while (*s)
	{
	if (*s == EOL)		/* End of line */
	    {
	    *s = 0;
	    return (s + 1);	/* Return beginning of *next* line */
	    }
	s++;
	}
    return s;
    }

/* repspc -- tidy up the white space in a text block */
void
repspc(char *s)
    {
    while (*s)
	{
	if (*s == '\t')
	    *s = SPC;
	if (*s == '\r')
	    *s = EOL;
	}
    }
