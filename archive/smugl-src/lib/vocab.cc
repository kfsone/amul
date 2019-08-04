/*
 * vocab.cpp -- Vocab manipulation routines
 * Common to both the compiler and the frame
 */

static const char rcsid[] = "$Id: vocab.cc,v 1.7 1999/05/25 13:07:47 oliver Exp $";

#include "vocab.hpp"
#include "defines.hpp"
#include "fileio.hpp"
#include "includes.hpp"
#include "structs.hpp"
#include "typedefs.hpp"

#include "cl_vocab.hpp"

#include <cctype>
#include <cstring>

struct VOCAB *vc;  /* Vocabulary index data */
uint32_t hash;     /* Last hash number we used */
uint32_t hash_len; /* Length of last hashed key */

extern char dir[];
extern char vocifn[];
extern char vocfn[];

/* Return a pointer to a given word */
const char *
word(vocid_t idx)
{
    if (idx == vocUNKNOWN)
        return "(undef)";
    offset_t offset = vc->index[idx];
    if (idx > vc->items) {
        if (offset == -1L)
            return "(undef string)";
        else
            return reinterpret_cast<const char *>(vc->index[idx]);
    } else
        return reinterpret_cast<const char *>(vc->vocab + offset);
}

/* Calculate the hash of a word */
/* Please don't ask how this works. I don't actually know ;-).
 * I've played with various algorithms, and so far this one gives
 * best distribution over a number of test-data samples.
 * The guiding principle is of add, shift and fold; where folding
 * involves a prime number (hence VOCAB_ROWS must always be prime,
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
uint32_t
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
        if (hash & 0xe0000000UL)
            hash = ((hash ^ (hash_len * 13)) % 5783);
    }
    hash += (hash_len * VOCAB_ROWS) % 1049;
    return (hash = hash % VOCAB_ROWS);
}

/* Determine if this word is in the vocab table */
vocid_t
is_word(const char *p)
{
    int i, w;
    /* Work out which hash position it's in */
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
    /* Lastly; check to see if it's in the extras area */
    int idx = vc->items;
    for (i = 0; i < vc->extras; i++, idx++) {
        if (vc->index[idx] != -1 && strcmp(p, reinterpret_cast<string *>(vc->index[idx])) == 0)
            return idx;
    }
    return vocUNKNOWN;
}

/* Load the vocab table into memory */
void *
read_in_vocab(void *membase)
{
    char tmp[100];
    long vcrows;
    size_t mem;
    int fd;

    sprintf(tmp, "%sData/%s", dir, vocifn);
    fd = open(tmp, O_RDONLY);
    if (fd == -1) {
        printf(">> No Vocab Index: %s\n", tmp);
        exit(1);
    }
    // Use the "number of rows" value as a version ID for files
    read(fd, (char *) &vcrows, sizeof(long));
    if (vcrows != VOCAB_ROWS) {
        fprintf(stderr, ">> Incompatible hashing mechanism in %s - aborted.\n", tmp);
        exit(1);
    }
    /* Read the item count */
    read(fd, &vc->items, sizeof(counter_t));
    /* Now the hash depth */
    read(fd, &vc->hash_depth, sizeof(counter_t));
    /* The size of 'vocab' */
    read(fd, &vc->cur_vocab, sizeof(size_t));
    /* Followed by the reverse index */
    read(fd, &vc->hash_size, sizeof(long) * VOCAB_ROWS);
    /* Designate memory for reverse index, including player entries */
    mem = vc->items * sizeof(long);
    mem += MAXU * sizeof(long);
    if (membase) {
        vc->index = (offset_t *) membase;
        membase = (void *) ((char *) membase + mem);
    } else {
        vc->index = (offset_t *) malloc(mem);
        if (vc->index == NULL) {
            printf(">> Out of memory for vocab reverse index\n");
            exit(2);
        }
    }
    bzero((char *) vc->index, mem);
    read(fd, (char *) vc->index, sizeof(long) * vc->items);
    vc->extras = MAXU;
    for (int extras = 0; extras < vc->extras; extras++)
        vc->index[vc->items + extras] = -1;

    /* Now provide for and read in each of the hash rows */
    for (uint32_t i = 0; i < VOCAB_ROWS; i++) {
        mem = vc->hash_depth * sizeof(long);
        if (membase) {
            vc->hash[i] = (vocid_t *) membase;
            bzero(membase, mem);
            membase = (void *) ((char *) membase + mem);
        } else {
            vc->hash[i] = (vocid_t *) malloc(mem);
            if (vc->hash[i] == NULL) {
                printf(">> Out of memory for vocab hash index\n");
                exit(2);
            }
        }
        read(fd, vc->hash[i], vc->hash_depth * sizeof(long));
    }
    /* We're done with the index file */
    close(fd);

    sprintf(tmp, "%sData/%s", dir, vocfn);
    fd = open(tmp, O_RDONLY);
    if (fd == -1) {
        printf(">> No Vocab Data: %s\n", tmp);
        exit(1);
    }
    vc->vocab_alloc = vc->cur_vocab;
    if (membase) {
        vc->vocab = (char *) membase;
        membase = (void *) (vc->vocab + vc->vocab_alloc);
    } else {
        vc->vocab = (char *) malloc(vc->vocab_alloc);
        if (vc->vocab == NULL) {
            printf(">> Out of memory for vocab data.\n");
            exit(2);
        }
    }
    bzero(vc->vocab, vc->vocab_alloc);
    read(fd, vc->vocab, vc->cur_vocab);
    close(fd);

    return membase;
}
