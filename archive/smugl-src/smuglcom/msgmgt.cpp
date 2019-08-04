/*
 * msgmgt -- manage the message table index
 */

#include <cctype>
#include <cstring>

#include "errors.hpp"
#include "libprotos.hpp"
#include "smuglcom.hpp"

#define GROW_SIZE 256  // Size to grow index by

long *msgitab;  // The message index table
long i_alloc;   // Indexes allocated for
long *msgip;    // Current position in the msg index

struct MSGID {
    long msg;  // Msg number
    char *id;  // The ID
};
struct MSGID *msgidtab;  // The message ID table
long id_alloc;           // ID entries allocated
struct MSGID *msgidp;    // Current position in the index

/* add the message to the index. if 'id' is set, this will be
 * considered a 'umsg'
 *  id   = NULL or id of a 'umsg'
 *  pos  = message offset within the umsg file.
 * returns id of the message added
 */
msgno_t
add_msg(const char *id)
{
    size_t new_alloc;

    if (!msgfp) {  // Haven't opened message file yet
        msgfp = fopen(datafile(umsgfn), "wb");
        if (msgfp == nullptr)
            Err("writ", datafile(umsgfn));
    }

    if (i_alloc % GROW_SIZE == 0) {  // No indexes availble, need to grow
        new_alloc = (i_alloc + GROW_SIZE) * sizeof(*msgitab);
        msgitab = (long *) grow(msgitab, new_alloc, "Reorganising Msg Index Table");
        msgip = msgitab + i_alloc;
    }
    *msgip = ftell(msgfp);

    if (id && *id) {  // Does it have a label?
        if (id_alloc % GROW_SIZE == 0) {
            new_alloc = (id_alloc + GROW_SIZE) * sizeof(*msgidtab);
            msgidtab = (MSGID *) grow(msgidtab, new_alloc, "Reorganising MsgID Table");
            msgidp = msgidtab + id_alloc;
        }
        msgidp->msg = i_alloc;
        msgidp->id = strdup(id);

        id_alloc++;
        msgidp++;
    }

    msgip++;
    return i_alloc++;  // IE i_alloc++; return (i_alloc-1)
}

// Save the current message index table
void
save_messages()
{
    FILE *fp;
    if (!msgitab || !i_alloc)
        return;
    fp = fopen(datafile(umsgifn), "wb");
    if (!fp)
        Err("write", datafile(umsgifn));
    fwrite(msgitab, sizeof(*msgitab), (size_t) i_alloc, fp);
    fclose(fp);
}

/* Determine if 's' is a message id.
** Returns:
**  -2 for 'empty' message (i.e. "none")
**  -1 for no
**  0+ for message id
*/
msgno_t
ismsgid(const char *s)
{
    MSGID *tmpidp;

    // Is it a "no message" id?
    if (!strncmp("none", s, 4) && (!*(s + 4) || !isalnum(*(s + 4))))
        return -2;

    // Is it a system message?
    if (*s == '$') {
        int i;
        i = atoi(s + 1);
        if (i < 1 || i > NSMSGS) {
            error("Invalid SysMsg ID '%s'.\n", s);
            return -1;
        }
        return i - 1;
    }

    // Now search the message ID table
    for (tmpidp = msgidtab; tmpidp < msgidp; tmpidp++) {
        if (!strcmp(tmpidp->id, s))
            return tmpidp->msg;
    }
    return -1;
}
