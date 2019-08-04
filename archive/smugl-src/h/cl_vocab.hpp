#ifndef SMUGL_H_CL_VOCAB_H
#define SMUGL_H_CL_VOCAB_H 1

#include "typedefs.hpp"

constexpr uint32_t VOCAB_ROWS = 1499;  // Prime number

struct VOCAB {
    counter_t items;                // Number of items in table
    offset_t *index;                // Reverse lookup index (offsets in vocab)
    int32_t hash_size[VOCAB_ROWS];  // Size of (items in) each hash-slot
    vocid_t *hash[VOCAB_ROWS];      // Forward hash
    char *vocab;                    // Vocab text
    counter_t hash_depth;           // Greatest number of entries in a hash slot
    size_t cur_vocab;               // Vocab string space (bytes) in use
    size_t vocab_alloc;             // Vocab string space (bytes) allocated
    counter_t extras;               // Extra vocab entries for players, etc
};

#endif  // SMUGL_H_CL_VOCAB_H
