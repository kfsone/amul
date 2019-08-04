#ifndef SMUGL_H_VOCAB_H
#define SMUGL_H_VOCAB_H

/*
 * Vocab manipulation routines
 * Common to both the compiler and the frame
 */

#include "typedefs.hpp"

constexpr size_t VCHASH_GROW_SIZE = 6;   // Forward hash growth rate
constexpr size_t VCREV_GROW_SIZE = 256;  // Reverse hash growth rate
constexpr size_t VC_GROW_SIZE = 4096;    // Vocab Table growth rate

enum { vocUNKNOWN = -1 };  // Word we don't recognise

extern struct VOCAB *vc;   // Primary vocab data structure
extern uint32_t hash;      // Last hash number we used
extern uint32_t hash_len;  // Length of last hashed key

extern const char *word(vocid_t);
extern uint32_t hash_of(const char *);
extern vocid_t is_word(const char *);
extern void *read_in_vocab(void *);

#endif  // VOCAB_H
