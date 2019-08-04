/*
 * Management of the "vocab" table (an index and actual data)
 *
 * The vocab table has two indexes. The first, vocab_index, is a
 * regular offset index. vocab_index[20] points to the 20th vocab
 * entry. This is the reverse-lookup index (numeric to name).
 * The forward lookup index is a hash-based index, allowing for
 * much faster lookups of names based on hash keys.
 */

static const char rcsid[] = "$Id: vocab.cc,v 1.7 1997/05/22 02:21:44 oliver Exp $";

#include <cstring>
#include <iostream>

#include "fileio.hpp"
#include "includes.hpp"
#include "libprotos.hpp"
#include "protos.hpp"
#include "structs.hpp"
#include "vocab.hpp"

#include "cl_vocab.hpp"

/* Statistical information */
static long hash_allocs;      /* Number of times we alloc'd the hash */
static long str_allocs;       /* Number of times we alloc'd vocab */
static long hash_total_alloc; /* Total hash memory */
static long idx_allocs;       /* Number of reallocs of index memory */

extern char vocifn[], vocfn[];
extern struct VOCAB VC;

/* new_word
 *  p       : Location of the new word (must be null terminated)
 *  need_new: Fail if "TRUE" and word is already in index
 */
vocid_t
new_word(const char *p, int need_new)
{
    vocid_t i;

    /* Make sure there's something to look at */
    if (!p || !*p)
        return -1;

    /* Is this word already in the vocab table? */
    i = is_word(p);
    if (need_new && i != -1)
        return -1;
    else if (i != -1)
        return i;

    /* So let's add it */
    /* First; make sure there's enough memory in this hash slot */
    if (VC.hash_size[hash] % VCHASH_GROW_SIZE == 0) {
        size_t req = (VC.hash_size[hash] + VCHASH_GROW_SIZE) * sizeof(long);
        VC.hash[hash] = (vocid_t *) grow(VC.hash[hash], req, "Resizing Vocab Hash table");
        hash_allocs++;
        hash_total_alloc += VCHASH_GROW_SIZE * sizeof(long);
    }
    /* Now the reverse index */
    if (VC.items % VCREV_GROW_SIZE == 0) {
        size_t req = (VC.items + VCREV_GROW_SIZE) * sizeof(long);
        VC.index = (offset_t *) grow(VC.index, req, "Resizing Vocab Index");
        idx_allocs++;
    }
    /* Next, ensure there's memory in the vocab space */
    if ((VC.cur_vocab + hash_len + 1) >= VC.vocab_alloc) {
        VC.vocab_alloc += VC_GROW_SIZE;
        VC.vocab = (char *) grow(VC.vocab, VC.vocab_alloc, "Resizing Vocab space");
        str_allocs++;
    }
    /* Copy the text into the vocab area */
    strcpy((VC.vocab + VC.cur_vocab), p);
    /* Set the index offset pointer for this hash entry */
    VC.hash[hash][VC.hash_size[hash]++] = VC.items;
    VC.index[VC.items] = VC.cur_vocab;
    /* Increment cur_vocab past the null byte */
    VC.cur_vocab += hash_len + 1;
    if (VC.hash_size[hash] > VC.hash_depth)
        VC.hash_depth = VC.hash_size[hash];
    return VC.items++;
}

void
hash_stats(void)
{ /* Display the hash-table statistics */
    int zeros[2] = { 0, 0 };
    long lowest = VC.hash_depth;
    int at_low = 0, at_high = 0;
    long avg = 0;
    long avg_div = 0;
    for (uint32_t i = 0; i < VOCAB_ROWS; i++) {
        if (VC.hash_size[i] == 0)
            zeros[i % 2]++;
        if (VC.hash_size[i] < lowest) {
            lowest = VC.hash_size[i];
            at_low = 1;
        } else if (VC.hash_size[i] == lowest)
            at_low++;

        /* Don't consider this any further unless it was used */
        if (VC.hash_size[i]) {
            avg += VC.hash_size[i];
            avg_div++;
            if (VC.hash_size[i] == VC.hash_depth) {
                at_high++;
                printf("clumpers: ");
                for (counter_t j = 0; j < VC.hash_depth; j++) {
                    printf("%s, ", word(VC.hash[i][j]));
                }
                printf("\n");
            }
        }
    }

    std::cout << "Vocab Hash Statistics:\n";
    std::cout << " Vocab string space: " << VC.vocab_alloc << "(" << VC.cur_vocab << "), "
              << "Allocs: " << str_allocs << "\n";
    std::cout << " Reverse index allocs: " << idx_allocs << "\n";
    std::cout << " Hash rows: " << static_cast<uint32_t>(VOCAB_ROWS)
              << ", Mem: " << hash_total_alloc << ", Allocs: " << hash_allocs << ", Use: " << lowest
              << "(" << at_low << ")"
              << " - " << VC.hash_depth << "(" << at_high << "), Avg: " << avg / avg_div << "\n";
    std::cout << " Zeros = Even:" << zeros[0] << ", Odd:" << zeros[1] << "\n";
    std::cout << " Entries in the hash: " << VC.items << "\n";
    for (counter_t i = 1; i <= VC.hash_depth; i++) {
        for (uint32_t j = 0; j < VOCAB_ROWS; j++) {
            std::cout << (VC.hash_size[j] >= 1 ? '#' : '.');
        }
        std::cout << "\n";
    }
}

void
save_vocab_index(void)
{ /* Write the vocab indexes to disk */
    int fd;
    long i;
    int j;

    fd = open(datafile(vocifn), O_WRONLY | O_CREAT | O_TRUNC, 0664);
    if (fd == -1)
        Err("write", datafile(vocifn));
    /* First write the number of rows, number of indexes and hash depth */
    i = VOCAB_ROWS;
    write(fd, (char *) &i, sizeof(long));
    write(fd, (char *) &VC.items, sizeof(long));
    write(fd, (char *) &VC.hash_depth, sizeof(long));
    /* Record how large 'vocab' is */
    write(fd, (char *) &VC.cur_vocab, sizeof(long));
    /* Write out the hash row sizes */
    write(fd, (char *) &VC.hash_size, sizeof(long) * VOCAB_ROWS);
    /* Write out the reverse-lookup index */
    write(fd, (char *) VC.index, sizeof(long) * VC.items);
    for (i = 0; i < VOCAB_ROWS; i++) {
        long zero = 0;
        write(fd, VC.hash[i], sizeof(long) * VC.hash_size[i]);
        /* Pad the line out to 'hash_depth' to allow for growth */
        for (j = VC.hash_size[i]; j < VC.hash_depth; j++)
            write(fd, &zero, sizeof(long));
    }
    close(fd);

    /* Now write the actual vocab to disk */
    fd = open(datafile(vocfn), O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1)
        Err("write", datafile(vocfn));
    write(fd, VC.vocab, VC.cur_vocab);
    close(fd);
}
