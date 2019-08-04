/*
 * vocab.cpp -- Vocab manipulation routines
 * Common to both the compiler and the frame
 */

#include "include/vocab.hpp"
#include "include/defines.hpp"
#include "include/includes.hpp"
#include "include/structs.hpp"
#include "include/typedefs.hpp"

struct VOCAB *vc;  // Vocabulary index data
u_long hash;       // Last hash number we used
u_long hash_len;   // Length of last hashed key

extern char g_dir[];
extern char vocifn[];
extern char vocfn[];

// Return a pointer to a given word
const char *
word(vocid_t idx)
{
    if (idx == vocUNKNOWN)
        return "(undef)";
    off_t offset = vc->index[idx];
    if (idx > vc->items) {
        if (offset == -1L)
            return "(undef string)";
        else
            return (const char *) (vc->index[idx]);
    } else
        return (const char *) (vc->vocab + offset);
}

// Calculate the hash of a word
/* Please don't ask how this works. I don't actually know ;-).
 * I've played with various algorithms, and so far this one gives
 * best distribution over a number of test-data samples.
 * The guiding principle is of add, shift and fold; where folding
 * involves a prime number (hence VOCROWS must always be prime,
 * and 13, 5783 and 1049 are fitting-selected primes).
 * '0xe0000000' is a detector for if the shift has moved a word
 * to the top of the long; the algorithm in libg++ uses 0xf...
 * but I found 0xe gives a nicer distribution in my test cases.
 * (toupper(*(p++)) - '!'). This restricts the alphabet. I know
 * I'm only going to be dealing with [0-9a-z_!.-]. Moving the
 * characters to uppercase is just convenient as it clusters
 * everything nicely. Not doing this tended to have a detrimental
 * effect.
 */
u_long
hash_of(const char *p)
{
    hash = 0;
    hash_len = 0;
    while (*p) {
        char c = toupper(*(p++));
        if (c == ' ')
            c = '_';
        hash_len++;
        hash = (hash << 4) + (c - '!');
        if (hash & (unsigned long) (0xe0000000))
            hash = ((hash ^ (hash_len * 13)) % 5783);
    }
    hash += (hash_len * VOCROWS) % 1049;
    return (hash = hash % VOCROWS);
}

// Determine if this word is in the vocab table
vocid_t
is_word(const char *p)
{
    int i, w;
    // Work out which hash position it's in
    hash_of(p);
    for (i = 0; i < vc->hash_size[hash]; i++) {
        const char *p1 = p;
        const char *p2 = word(w = *(vc->hash[hash] + i));
        while ((*p1 && *p1 == *p2) || (*p1 == ' ' && *p2 == '_')) {
            p1++;
            p2++;
        }
        if (!*p1 && !*p2)
            return w;
    }
    // Lastly; check to see if it's in the extras area
    int idx = vc->items;
    for (i = 0; i < vc->extras; i++, idx++) {
        if (vc->index[idx] != -1 && strcmp(p, (const char *) vc->index[idx]) == 0)
            return idx;
    }
    return vocUNKNOWN;
}

// Load the vocab table into memory
void *
read_in_vocab(void *membase)
{
    char tmp[100];
    long vcrows;
    size_t mem;
    int fd;
    int i;

    sprintf(tmp, "%sData/%s", g_dir, vocifn);
    fd = _open(tmp, O_RDONLY);
    if (fd < 0) {
        printf(">> No Vocab Index: %s\n", tmp);
        exit(1);
    }

    try {
        // Use the "number of rows" value as a version ID for files
        int rv = _read(fd, (char *) &vcrows, sizeof(long));
        if (rv < 0)
            throw rv;
        if (vcrows != VOCROWS) {
            fprintf(stderr, ">> Incompatible hashing mechanism in %s - aborted.\n", tmp);
            exit(1);
        }
        // Read the item count
        rv = _read(fd, &vc->items, sizeof(counter_t));
        if (rv < 0)
            throw rv;
        // Now the hash depth
        rv = _read(fd, &vc->hash_depth, sizeof(counter_t));
        if (rv < 0)
            throw rv;
        // The size of 'vocab'
        rv = _read(fd, &vc->cur_vocab, sizeof(size_t));
        if (rv < 0)
            throw rv;
        // Followed by the reverse index
        rv = _read(fd, &vc->hash_size, sizeof(long) * VOCROWS);
        if (rv < 0)
            throw rv;
        // Designate memory for reverse index, including player entries
        mem = vc->items * sizeof(long);
        mem += MAXU * sizeof(long);
        if (membase) {
            vc->index = (off_t *) membase;
            membase = (void *) ((char *) membase + mem);
        } else {
            vc->index = (off_t *) malloc(mem);
            if (vc->index == NULL) {
                printf(">> Out of memory for vocab reverse index\n");
                exit(2);
            }
        }
        bzero((char *) vc->index, mem);
        rv = _read(fd, (char *) vc->index, sizeof(long) * vc->items);
        if (rv < 0)
            throw rv;
        vc->extras = MAXU;
        for (int extras = 0; extras < vc->extras; extras++)
            vc->index[vc->items + extras] = -1;

        // Now provide for and read in each of the hash rows
        for (i = 0; i < VOCROWS; i++) {
            mem = vc->hash_depth * sizeof(long);
            if (membase) {
                vc->hash[i] = (long *) membase;
                bzero(membase, mem);
                membase = (void *) ((char *) membase + mem);
            } else {
                vc->hash[i] = (long *) malloc(mem);
                if (vc->hash[i] == NULL) {
                    printf(">> Out of memory for vocab hash index\n");
                    exit(2);
                }
            }
            rv = _read(fd, vc->hash[i], vc->hash_depth * sizeof(long));
            if (rv < 0)
                throw rv;
        }
        // We're done with the index file
        _close(fd);

        sprintf(tmp, "%sData/%s", g_dir, vocfn);
        fd = _open(tmp, O_RDONLY);
        if (fd == -1) {
            printf(">> No Vocab Data: %s\n", tmp);
            exit(1);
        }
        vc->vocab_alloc = vc->cur_vocab;
        if (membase) {
            vc->vocab = (char *) membase;
            membase = (void *) (vc->vocab + vc->vocab_alloc);
        } else {
            vc->vocab = (char *) malloc((u_long) vc->vocab_alloc);
            if (vc->vocab == NULL) {
                printf(">> Out of memory for vocab data.\n");
                exit(2);
            }
        }
        bzero(vc->vocab, vc->vocab_alloc);
        rv = _read(fd, vc->vocab, (u_long) vc->cur_vocab);
        if (rv < 0)
            throw rv;
        _close(fd);
    } catch (...) {
        _close(fd);
        fprintf(stderr, ">> Read error on %s", tmp);
        exit(1);
    }

    return membase;
}
