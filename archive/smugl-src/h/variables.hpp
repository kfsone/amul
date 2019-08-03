/* $Id: variables.hpp,v 1.3 1997/04/26 21:43:15 oliver Exp $
 * Standard variables; pointers, counters, etc
 */

#ifndef VARS_H
#define VARS_H 1

#ifndef DEF
#define DEF extern
#endif

/* Text */
extern char dir[];   /* spc for work dir path */
extern char block[]; /* 1k block of spare txt */

/*DEF struct MsgPort *port, *reply;*/
/*DEF struct Aport *amul;*/

#endif /* VARS_H */
