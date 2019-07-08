#include "amulcom.includes.h"
#include "h/amul.vars.h"

#include <sys/stat.h>
#if !defined(_MSC_VER)
#    include <unistd.h>
#endif

#include <string>

using namespace AMUL::Logging;
using namespace Compiler;

extern FILE *ifp;
extern FILE *ofp1;
extern FILE *ofp2;
extern FILE *ofp3;
extern FILE *ofp4;
extern FILE *ofp5;
extern FILE *afp;

extern Buffer verbBuffer, mobBuffer, objBuffer;

static size_t ifpFileSize;

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
nextc(bool required)
{
    for (;;) {
        char c = fgetc(ifp);
        if (c == EOF) {
            if (required)
                GetLogger().fatalf("File contains no data");
            return -1;
        }
        if (isspace(c))
            continue;
        if (isCommentChar(c)) {
            fgets(block, 1024, ifp);
            continue;
        }
        ungetc(c, ifp);
        return 0;
    }
}

void
quit()
{
    if (GetContext().m_completed) {
        sprintf(block, "%s%s", dir, Resources::Compiled::gameProfile());
        unlink(block);
    }
    unlink("objh.tmp");
    verbBuffer.free();
    mobBuffer.free();
    objBuffer.free();
    OS::Free(mobp, sizeof(mob) * mobchars);
    OS::Free(rmtab, sizeof(room) * rooms);

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

static size_t
getFileSize(const char *filepath)
{
    struct stat sb {
    };
    int err = stat(filepath, &sb);
    if (err == -1) {
        GetLogger().fatalop("stat", filepath);
    }
    return sb.st_size;
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

    ifpFileSize = getFileSize(fnm);

    if (ifp = fopen(fnm, "rb"); !ifp)
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
    fwritesafe(*rmtab, afp);
}

void
opentxt(const char *filename)
{
    // One a source file (.txt) for reading, and because it's a text file,
    // open it in non-binary mode so that the OS will potentially do eol
    // conversion for us.
    std::string filepath = dir;
    filepath += filename;
    filepath += ".txt";

    ifpFileSize = getFileSize(filepath.c_str());

    ifp = fopen(filepath.c_str(), "r");

    if (!ifp) {
        GetLogger().fatalf("Unable to open file: %s", filepath.c_str());
    }
}

void
skipblock()
{
    char c{'\n'}, lc{0};
    while (c != EOF && !(c == lc && isEol(c))) {
        lc = c;
        c = fgetc(ifp);
    }
}

// trim both ends of a string, and replace tabs with spaces
void
tidy(char *ptr)
{
    char *lastNonSpace = nullptr;
    while (*ptr) {
        if (*ptr == '\t' || *ptr == '\r')
            *ptr = ' ';
        if (!isspace(*ptr))
            lastNonSpace = ptr;
        ++ptr;
    }
    if (lastNonSpace)
        *(lastNonSpace + 1) = 0;
}

int
is_verb(const char *token)
{
    if (verbs == 0) {
        GetLogger().errorf("Tried to look up verb '%s' with 0 verbs", token);
        return -1;
    }
    if (strlen(token) > IDL) {
        GetLogger().errorf("Invalid verb (too long): %s", token);
        return -1;
    }

    if (stricmp(token, verb.id) == 0)
        return (verbs - 1);

    vbptr = vbtab;
    for (int i = 0; i < verbs; i++, vbptr++) {
        if (stricmp(vbptr->id, token) == 0)
            return i;
    }
    return -1;
}

void
Buffer::open(size_t offset)
{
    auto size = filesize();
    // add nul-terminator and round to page size
    m_size = (size + offset + 1 + 4095) & ~4095;
    if (offset > 0)
        m_data = OS::AllocateClear(m_size);
    else
        m_data = OS::Allocate(m_size);
    if (!m_data) {
        GetLogger().fatalf("Out of memory (requested %zu bytes)", m_size);
    }
    auto into = static_cast<char *>(m_data) + offset;
    /// TODO: What if we needed *more* space?
    auto bytes = fread(into, 1, size, ifp);

    // fread does not add a trailing zero, and in-case character conversions
    // were done in-place, we need to add one so that we don't inadvertently
    // see left-over characters from the conversion.
    into[bytes] = '\0';
}

Buffer::~Buffer() { this->free(); }

void
Buffer::free()
{
    if (m_data) {
        OS::Free(m_data, m_size);
        m_data = nullptr;
    }
}

// Return size of current file
int32_t
filesize()
{
    if (!ifp)
        return -1;
    return static_cast<int32_t>(ifpFileSize);
}
