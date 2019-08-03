/* $Id: consts.hpp,v 1.6 1999/06/11 14:26:08 oliver Exp $
 * String and other constants
 */

#ifndef CONS_H
#define CONS_H 1

#ifdef COMPILER
extern const char *std_flag[];
extern const char *rflag[];
extern const char *rparam[];
extern const char *obflags1[];
extern const char *obparms[];
extern const char *obflags2[];
extern unsigned char syntl[];
#endif
extern const char *syntax[];

/* Text (not really constant, but...) */
extern const char *obputs[];
extern const char *prep[];
extern const char *article[];

extern struct ARGS cond[];
extern struct ARGS action[];

/* All the various file names */
extern char advfn[], plyrfn[], roomsfn[], ranksfn[], ttfn[];
extern char ttpfn[], langfn[], synsifn[];
extern char objsfn[], statfn[], umsgifn[], umsgfn[];
extern char mobfn[], vocifn[], vocfn[], bobfn[];
extern char statsfn[];

#endif /* CONS_H */
