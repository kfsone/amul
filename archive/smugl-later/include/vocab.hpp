#pragma once
/*
 * Vocab manipulation routines
 * Common to both the compiler and the frame
 */

#define VCHASH_GROW_SIZE 6   // Forward hash growth rate
#define VCREV_GROW_SIZE 256  // Reverse hash growth rate
#define VC_GROW_SIZE 4096    // Vocab Table growth rate

#define vocUNKNOWN -1  // Word we don't recognise

extern struct VOCAB *vc;  // Primary vocab data structure
extern u_long hash;       // Last hash number we used
extern u_long hash_len;   // Length of last hashed key

extern const char *word(vocid_t);
extern u_long hash_of(const char *);
extern vocid_t is_word(const char *);
extern void *read_in_vocab(void *);
