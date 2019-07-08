#include "amulcom.includes.h"

#include "h/amul.cons.h"  // Predefined Constants etc
#include "h/amul.vars.h"  // all INTERNAL variables

using namespace AMUL::Logging;
using namespace Compiler;

// Compiler specific variables...

int     dmoves;  // How many RF_CEMETERYs to check?
int     rmn;     // Current room no.
int32_t FPos;    // Used during TT/Lang writes
int     proc;    // What we are processing
char *  syntab;  // Synonym table, re-read
int32_t mins;    // Length of data! & gametime
int32_t obmem;   // Size of Objects.TXT
int32_t wizstr;  // Wizards strength
char *  mobdat;  // Mobile data

char Word[WORD_LEN];

char  fnm[150], was[128];
FILE *ofp5;

struct _OBJ_STRUCT2 *obtab2, *objtab2, obj2, *osrch, *osrch2;

int
isroom(const char *name) noexcept
{
    roomtab = rmtab;
    for (int r = 0; r < rooms; r++, roomtab++) {
        if (strcmp(roomtab->id, name) == 0)
            return r;
    }
    return -1;
}

void
object(const char *s)
{
    GetLogger().fatalf(
            "Object #%d '%s' has invalid %s: %s", nouns + 1, obj2.id, s, Word);
}

void
set_start()
{
    if (!isdigit(Word[0]))
        object("start state");
    obj2.state = atoi(Word);
    if (obj2.state < 0 || obj2.state > 100)
        object("start state");
}

void
set_holds()
{
    if (!isdigit(Word[0]))
        object("holds= value");
    obj2.contains = atoi(Word);
    if (obj2.contains < 0 || obj2.contains > 1000000)
        object("holds= state");
}

void
set_put()
{
    for (int i = 0; i < NPUTS; i++) {
        if (stricmp(obputs[i], Word) == NULL) {
            obj2.putto = i;
            return;
        }
    }
    object("put= flag");
}

void
set_mob()
{
    for (int i = 0; i < mobchars; i++) {
        if (stricmp(Word, (mobp + i)->id) == NULL) {
            obj2.mobile = i;
            return;
        }
    }
    object("mobile= flag");
}

void
checkdmoves()
{
    int           n;
    _ROOM_STRUCT *roomptr;

    // Check RF_CEMETERY ptrs
    fopenr(Resources::Compiled::roomDesc());  // Open desc. file
    roomptr = rmtab;
    for (n = 0; n < rooms; n++) {
        if (roomptr->flags & RF_CEMETERY) {
            printf("%-9s\x0d", roomptr->id);
            fseek(ifp, roomptr->desptr, 0);
            fread(dmove, IDL, 1, ifp);  // Read the RF_CEMETERY name
            if (isroom(dmove) == -1) {  // Is it a valid room?
                GetLogger().errorf(
                        "%-9s - invalid dmove: %s", roomptr->id, dmove);
            }
        }
        roomptr++;
    }
    GetContext().terminateOnErrors();
}

int
isprep(const char *s)
{
    for (int i = 0; i < NPREP; i++) {
        if (strcmp(s, prep[i]) == 0)
            return i;
    }
    return -1;
}
