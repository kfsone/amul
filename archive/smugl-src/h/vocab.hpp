#ifndef VOCAB_H
#define VOCAB_H 1

/* $Id: vocab.hpp,v 1.4 1997/11/26 17:26:37 oliver Exp $
 * Vocab manipulation routines
 * Common to both the compiler and the frame
 */

#include "typedefs.hpp"

#define VCHASH_GROW_SIZE 6  /* Forward hash growth rate */
#define VCREV_GROW_SIZE 256 /* Reverse hash growth rate */
#define VC_GROW_SIZE 4096   /* Vocab Table growth rate */

#define vocUNKNOWN -1  // Word we don't recognise

extern struct VOCAB *vc;  /* Primary vocab data structure */
extern uint32_t hash;     /* Last hash number we used */
extern uint32_t hash_len; /* Length of last hashed key */

extern const char *word(vocid_t);
extern uint32_t hash_of(const char *);
extern vocid_t is_word(const char *);
extern void *read_in_vocab(void *);

#endif /* VOCAB_H */
