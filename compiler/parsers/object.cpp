#include "amulcom.includes.h"

#include <string>
#include <unordered_map>
#include <vector>

using namespace AMUL::Logging;
using namespace Compiler;

using AdjectiveLookup = std::unordered_map<std::string, int>;
static std::string     adjectiveTable;
static AdjectiveLookup adjectiveIDs;

void
set_adj(std::string token)
{
    if (token.size() < 3)
        GetLogger().errorf("Invalid adjective (too short): %s", token.c_str());
    if (token.size() > IDL)
        GetLogger().errorf("INvalid adjective (too long): %s", token.c_str());

    auto it = adjectiveIDs.find(token);
    if (it != adjectiveIDs.end()) {
        obj.adj = it->second;
        return;
    }

    int id = (int)adjectiveTable.size();
    adjectiveIDs[token] = id;
    adjectiveTable += token;
    adjectiveTable += '\0';
    obj.adj = id;
}

static void
statinv(const char *s)
{
    GetLogger().fatalf("Object #%d: %s: invalid %s state line: %s", nouns + 1, obj2.id, s, block);
}

void
text_id(const char *from, char c)
{
    char *ptr;

    strcpy(block, from);
    char *p = block;
    while (*p != c && *p != 0)
        p++;
    if (*p == 0)
        *(p + 1) = 0;
    *(p++) = '\n';
    if (*(p - 2) == '{')
        ptr = p - 1;
    else
        ptr = p;

    std::string filepath{dir};
    filepath += Resources::Compiled::objDesc();
    FILE *fp = fopen(filepath.c_str(), "rb+");
    if (!fp)
        GetLogger().fatalop("open", filepath.c_str());
    fseek(fp, 0, SEEK_END);
    state.descrip = ftell(fp);  // Get pos
    if (fwrite(&block[0], ptr - block, 1, fp) != 1) {
        fclose(fp);
        GetLogger().fatalop("write", filepath.c_str());
    }
    fputc(0, fp);
    strcpy(block, p);
    fclose(fp);
}

void
state_proc()
{
    int flag;

    state.weight = state.value = state.flags = 0;
    state.descrip = -1;

    tidy(block);
    if (block[0] == 0)
        return;

    // Get the weight of the object
    const char *p = getWordAfter("weight=", block);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("weight value on");
    state.weight = atoi(Word);
    if (obj2.flags & OF_SCENERY)
        state.weight = wizstr + 1;

    // Get the value of it
    p = skipspc(p);
    p = getWordAfter("value=", p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("value entry on");
    state.value = atoi(Word);

    // Get the strength of it (hit points)
    p = skipspc(p);
    p = getWordAfter("str=", p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("strength entry on");
    state.strength = atoi(Word);

    // Get the damage it does as a weapon
    p = skipspc(p);
    p = getWordAfter("dam=", p);
    if (*p == 0)
        statinv("incomplete");
    if (!isdigit(Word[0]) && Word[0] != '-')
        statinv("damage entry on");
    state.damage = atoi(Word);

    // Description
    p = skipspc(p);
    p = skiplead("desc=", p);
    if (*p == 0)
        statinv("incomplete");
    if (*p == '\"' || *p == '\'') {
        text_id(p + 1, *p);
        p = block;
    } else {
        p = getword(p);
        state.descrip = getObjDescID(Word);
    }
    if (state.descrip == -1) {
        std::string error = "desc= ID ";
        error += Word;
        error += " on";
        statinv(error.c_str());
    }
    while (*p != 0) {
        p = getword(p);
        if (Word[0] == 0)
            break;
        if ((flag = isoflag2(Word)) == -1)
            statinv("flag on");
        state.flags = (state.flags | bitset(flag));
    }
    fwritesafe(state.weight, ofp2);
    obj2.nstates++;
}

int
isnoun(const char *s)
{
    int i;

    objtab2 = obtab2;
    if (stricmp(s, "none") == NULL)
        return -2;
    for (i = 0; i < nouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == NULL)
            return i;
    return -1;
}

int
iscont(const char *s)
{
    int i;

    objtab2 = obtab2;
    for (i = 0; i < nouns; i++, objtab2++)
        if (stricmp(s, objtab2->id) == NULL && objtab2->contains > 0)
            return i;
    return -1;
}

// Test if something is a location (a room or a container)
int
isloc(const char *subject)
{
    int i;

    if ((i = isroom(subject)) != -1)
        return i;
    if ((i = iscont(subject)) == -1) {
        if (isnoun(subject) == -1)
            GetLogger().errorf("Invalid object start location: %s", subject);
        else
            GetLogger().errorf("Tried to start '%s' in non-container: %s", obj2.id, subject);
        return -1;
    }

    return -(INS + i);
}

Buffer objBuffer{};

void
objs_proc()
{
    int roomno;

    nouns = adjs;

    // Clear files
    fopenw(Resources::Compiled::objData());
    fopenw(Resources::Compiled::objState());
    fopenw(Resources::Compiled::objLoc());

    if (nextc(false) == -1) {
        close_ofps();
        return;
    }  // Nothing to process
    objBuffer.open(32 * sizeof(obj2));
    obtab2 = static_cast<_OBJ_STRUCT2 *>(objBuffer.m_data);
    objtab2 = obtab2 + 32;
    const char *s = reinterpret_cast<const char *>(objtab2);
    const char *p = nullptr;

    adjectiveTable.reserve(512);

    do {
        GetContext().checkErrorCount();
        do {
            p = s = extractLine(s, block);
        } while (*s != 0 && (isCommentChar(block[0]) || block[0] == 0));
        if (*s == 0 || block[0] == 0)
            continue;
        tidy(block);
        if (block[0] == 0)
            continue;
        p = getWordAfter("noun=", block);
        if (strlen(Word) < 3 || strlen(Word) > IDL) {
            GetLogger().errorf("Invalid object ID: '%s'", Word);
            Word[IDL + 1] = 0;
        }
        obj2.adj = obj2.mobile = -1;
        obj2.idno = nouns;
        obj2.state = 0;
        obj2.nrooms = 0;
        obj2.contains = 0;
        obj2.flags = 0;
        obj2.putto = 0;
        obj2.rmlist = (int32_t *)(uintptr_t)ftell(ofp3);
        strcpy(obj2.id, Word);

        // Get the object flags
        do {
            p = getword(p);
            if (Word[0] == 0)
                continue;
            if ((roomno = isoflag1(Word)) != -1)
                obj2.flags = (obj2.flags | bitset(roomno));
            else {
                if ((roomno = isoparm()) == -1) {
                    GetLogger().errorf("Invalid object parameter: %s", Word);
                    continue;
                }
                switch (bitset(roomno)) {
                case OP_ADJ: set_adj(Word); break;
                case OP_START: set_start(); break;
                case OP_HOLDS: set_holds(); break;
                case OP_PUT: set_put(); break;
                case OP_MOB:
                    set_mob();
                    mobs++;
                    break;
                default:
                    printf("** Internal: Code for object-parameter '%s' "
                           "missing!\n",
                           obparms[roomno]);
                }
            }
        } while (Word[0] != 0);

        // Get the room list

        // Fake continuation line
        block[0] = '+';
        block[1] = '\0';
        p = block;
        roomno = 0;
        do {
            p = getword(p);
            if (Word[0] == '+') {
                do
                    s = extractLine(s, block);
                while (*s != 0 && block[0] != 0 && isCommentChar(block[0]));
                if (*s == 0) {
                    GetLogger().fatal("Unexpected end of Objects.TXT file");
                }
                p = block;
                Word[0] = ' ';
                continue;
            }
            if (Word[0] == 0)
                continue;
            if ((roomno = isloc(Word)) == -1) {
                roomno = -1;
                continue;
            }
            fwritesafe(roomno, ofp3);
            obj2.nrooms++;
        } while (Word[0] != 0);
        if (obj2.nrooms == 0 && roomno == 0) {
            GetLogger().errorf("No locations specified for object: %s", obj2.id);
        }
        obj2.nstates = 0;
        do {
            do
                s = extractLine(s, block);
            while (isCommentChar(block[0]));
            if (block[0] == 0 || block[0] == '\n')
                break;
            state_proc();
            block[0] = '-';
        } while (block[0] != 0 && block[0] != '\n');
        if (obj2.nstates == 0 || obj2.nstates > 100)
            object("amount of states (i.e. none)");
        if ((uintptr_t)(obtab2 + (nouns)) > (uintptr_t)s)
            printf("@! table exceeded data\n");
        *(obtab2 + (nouns++)) = obj2;
    } while (*s != 0);

    GetContext().terminateOnErrors();

#if defined(AMUL_SORT_OBJS)
    close_ofps();
    sort_objs();
#endif

    fwrite(obtab2, sizeof(obj2), nouns, ofp1);
    close_ofps();

    // save the adjective table
    fopenw(Resources::Compiled::adjTable());
    fwrite(adjectiveTable.data(), 1, adjectiveTable.size(), ofp1);
    close_ofps();
}

#if defined(AMUL_SORT_OBJS)
void
sort_objs()
{
    int      i, j, k, nts;
    int32_t *rmtab, *rmptr;

    if (ifp != NULL)
        fclose(ifp);
    ifp = NULL;
    close_ofps();
    fopenr(Resources::Compiled::objState());
    blkget(&datal, &data, NULL);
    fclose(ifp);
    ifp = NULL;
    close_ofps();
    fopenr(Resources::Compiled::objLoc());
    blkget(&datal2, &data2, NULL);
    fclose(ifp);
    ifp = NULL;
    close_ofps();
    fopenw(Resources::Compiled::objData());
    fopenw(Resources::Compiled::objState());
    fopenw(Resources::Compiled::objLoc());
    fopenw(Resources::Compiled::nounTable());
    ifp = NULL;

    printf("Sorting Objects...:\r");
    objtab2 = obtab2;
    nts = 0;
    k = 0;

    statab = (struct _OBJ_STATE *)data;
    rmtab = (int32_t *)data2;
    for (i = 0; i < nouns; i++) {
        if (*(objtab2 = (obtab2 + i))->id == 0) {
            printf("@! skipping %ld states, %ld rooms.\n", objtab2->nstates, objtab2->nrooms);
            statab += objtab2->nstates;
            rmtab += objtab2->nrooms;
            continue;
        }
        strcpy(nountab.id, objtab2->id);
        nts++;
        nountab.num_of = 0;
        osrch = objtab2;
        statep = statab;
        rmptr = rmtab;
        for (j = i; j < nouns; j++, osrch++) {
            if (*(osrch->id) != 0 && stricmp(nountab.id, osrch->id) == NULL) {
                fwritesafe(*osrch, ofp1);
                fwrite(statep, sizeof(state), osrch->nstates, ofp2);
                fwrite(rmptr, sizeof(int32_t), osrch->nrooms, ofp3);
                nountab.num_of++;
                *osrch->id = 0;
                if (osrch != objtab)
                    k++;
                statep += osrch->nstates;
                rmptr += osrch->nrooms;
                if (osrch == objtab2) {
                    statab = statep;
                    rmtab = rmptr;
                    objtab2++;
                    i++;
                }
            } else
                statep += osrch->nstates;
            rmptr += osrch->nrooms;
        }

        fwritesafe(nountab, ofp4);
    }
    printf("%20s\r%ld objects moved.\n", " ", k);
    close_ofps();
    OS::Free(data, datal);
    OS::Free(data2, datal2);
    fopenr(Resources::Compiled::objData());
    fread((char *)obtab2, sizeof(obj), nouns, ifp);
}
#endif
