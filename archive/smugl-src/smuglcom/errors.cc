/*
 * miscellaneous error handling routines
 * These are mostly here to make proto-ising easier :-(
 */

static const char rcsid[] = "$Id: errors.cc,v 1.5 1997/05/22 02:21:36 oliver Exp $";

#include "fileio.hpp"
#include "libprotos.hpp"
#include "smuglcom.hpp"

#include <cstdarg>

void
error(const char *msg, ...)
{ /* Report a compiler error */
    va_list va;
    if (needcr == TRUE)
        printf("\n");
    needcr = FALSE;
    printf("\007#E#> ");
    va_start(va, msg);
    vprintf(msg, va);
    va_end(va);
    err++;
    if (err > 20)
        errabort();
}

void
warne(const char *msg, ...)
{ /* Report a compiler warning */
    va_list va;

    if (!warn)
        return;
    if (needcr == TRUE)
        printf("\n");
    needcr = FALSE;
    printf(" W > ");
    va_start(va, msg);
    vprintf(msg, va);
    va_end(va);
}

/* "End of Section": Abort if errors were encountered, otherwise tidy up */
void
errabort(void)
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

void
quit(const char *msg, ...)
{ /* Exit with a message */
    if (msg && *msg) {
        va_list va;
        va_start(va, msg);
        vprintf(msg, va);
        va_end(va);
    }
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
