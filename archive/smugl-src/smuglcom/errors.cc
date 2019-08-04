/*
 * miscellaneous error handling routines
 * These are mostly here to make proto-ising easier :-(
 */

#include "errors.hpp"
#include "fileio.hpp"
#include "libprotos.hpp"
#include "smuglcom.hpp"

/* "End of Section": Abort if errors were encountered, otherwise tidy up */
void
errabort()
{
    if (data) {
        free(data);
        data = NULL;
    }
    if (err)
        quit("\n\n!! Aborting due to %ld errors !!\n\n", (char *) err, 0L, 0L, 0L, 0L);
    close_ofps();
    save_messages();
    needcr = FALSE;
}

 /* Exit with a message */
void
quit()
{
    if (exi != 1) {
        char *p = datafile(advfn);
        unlink(p);
    }
    if (ifp)
        fclose(ifp);
    ifp = NULL;
    if (msgfp)
        fclose(msgfp);
    msgfp = NULL;
    close_ofps();
    exit(0);
}

void
Err(const char *s, const char *t)
{ /* Error function for file operations */
    quit("FATAL ERROR! Can't %s %s!\n", s, t);
}
