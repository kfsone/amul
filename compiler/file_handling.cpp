#include "amulcom.includes.h"
#include "h/amul.vars.h"

using namespace AMUL::Logging;
using namespace Compiler;

extern FILE *ifp;
extern FILE *ofp1;
extern FILE *ofp2;
extern FILE *ofp3;
extern FILE *ofp4;
extern FILE *ofp5;
extern FILE *afp;

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

// Find the next real stuff in file
char
nextc(int f)
{
    char c;
    do {
        while ((c = fgetc(ifp)) != EOF && isspace(c))
            ;
        if (isCommentChar(c))
            fgets(block, 1024, ifp);
    } while (c != EOF && (c == '*' || c == ';' || isspace(c)));
    if (f == 1 && c == EOF) {
        printf("\nFile contains NO data!\n\n");
        quit();
    }
    if (c == EOF)
        return -1;
    fseek(ifp, -1, 1);  // Move back 1 char
    return 0;
}

void
quit()
{
    if (GetContext().m_completed) {
        sprintf(block, "%s%s", dir, Resources::Compiled::gameProfile());
        unlink(block);
    }
    unlink("ODIDs.tmp");
    unlink("objh.tmp");
    OS::Free(mobdat, moblen);
    OS::Free(mobp, sizeof(mob) * mobchars);
    OS::Free(rmtab, sizeof(room) * rooms);
    OS::Free(data, datal);
    OS::Free(data2, datal2);
    OS::Free(obtab2, obmem);
    OS::Free(vbtab, vbmem);

    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;

    close_ofps();

    exit(0);
}

// Open file for reading
FILE *
fopenw(const char *s)
{
    FILE *tfp;
    if (*s == '-')
        strcpy(fnm, s + 1);
    else
        sprintf(fnm, "%s%s", dir, s);
    if ((tfp = fopen(fnm, "wb")) == NULL)
        GetLogger().fatalop("write", fnm);
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

// Open file for appending
FILE *
fopena(const char *s)
{
    if (afp != NULL)
        fclose(afp);
    if (*s == '-')
        strcpy(fnm, s + 1);
    else
        sprintf(fnm, "%s%s", dir, s);
    if ((afp = fopen(fnm, "rb+")) == NULL)
        GetLogger().fatalop("create", fnm);
    return NULL;
}

// Open file for reading
FILE *
fopenr(const char *s)
{
    if (ifp != NULL)
        fclose(ifp);
    if (*s != '-')
        sprintf(fnm, "%s%s", dir, s);
    else
        strcpy(fnm, s + 1);
    if ((ifp = fopen(fnm, "rb")) == NULL)
        GetLogger().fatalop("open", fnm);
    return ifp;
}

// Open file for reading
FILE *
rfopen(const char *filename)
{
    if (FILE *fp = fopen(filename, "rb"); fp) {
        strcpy(fnm, filename);
        return fp;
    }
    snprintf(fnm, sizeof(fnm), "%s%s", dir, filename);
    if (FILE *fp = fopen(fnm, "rb"); fp)
        return fp;
    GetLogger().fatalop("open", fnm);
}

// Update room entries after TT
void
ttroomupdate()
{
    fseek(afp, 0, 0L);
    fwrite(rmtab->id, sizeof(room), rooms, afp);
}

void
opentxt(const char *s)
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
    while (c != EOF && !(c == lc && c == '\n')) {
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
is_verb(const char *s)
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
        GetLogger().fatalf("Out of memory (requested %u bytes)", *s);
    }
    fread((*p) + off, 1, *s, ifp);
    *((*p + *s) - 2) = 0;
    *((*p + *s) - 1) = 0;
}

// Return size of current file
int32_t
filesize()
{
    int32_t now, s;

    now = ftell(ifp);
    fseek(ifp, 0, 2L);
    s = ftell(ifp) - now;
    fseek(ifp, now, 0L);
    return s + 2;  // Just for luck
}
