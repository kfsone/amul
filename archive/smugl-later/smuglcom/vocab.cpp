/*
 * Management of the "vocab" table (an index and actual data)
 *
 * The vocab table has two indexes. The first, vocab_index, is a
 * regular offset index. vocab_index[20] points to the 20th vocab
 * entry. This is the reverse-lookup index (numeric to name).
 * The forward lookup index is a hash-based index, allowing for
 * much faster lookups of names based on hash keys.
 */

#include "include/includes.hpp"
#include "include/structs.hpp"
#include "include/vocab.hpp"
#include "smuglcom/protos.hpp"
#include "include/libprotos.hpp"
#include "include/fderror.hpp"

/* Statistical information */
static long hash_allocs;		/* Number of times we alloc'd the hash */
static long str_allocs;			/* Number of times we alloc'd vocab */
static long hash_total_alloc;	/* Total hash memory */
static long idx_allocs;			/* Number of reallocs of index memory */

extern char vocifn[], vocfn[];
extern struct VOCAB VC;

/* new_word
 *  p       : Location of the new word (must be null terminated)
 *  need_new: Fail if "true" and word is already in index
 */
vocid_t
new_word(const char *p, bool need_new)
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
	if (VC.hash_size[hash] % VCHASH_GROW_SIZE == 0)
	{
		size_t  req = (VC.hash_size[hash] + VCHASH_GROW_SIZE) * sizeof(long);

		VC.hash[hash] = (long *) grow(VC.hash[hash], req, "Resizing Vocab Hash table");
		hash_allocs++;
		hash_total_alloc += VCHASH_GROW_SIZE * sizeof(long);
	}
	/* Now the reverse index */
	if (VC.items % VCREV_GROW_SIZE == 0)
	{
		size_t  req = (VC.items + VCREV_GROW_SIZE) * sizeof(long);

		VC.index = (off_t *) grow(VC.index, req, "Resizing Vocab Index");
		idx_allocs++;
	}
	/* Next, ensure there's memory in the vocab space */
	if ((VC.cur_vocab + hash_len + 1) >= VC.vocab_alloc)
	{
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
{								/* Display the hash-table statistics */
	int     i, j;
	int     zeros[2] = { 0, 0 };
	long    lowest = VC.hash_depth;
	int     at_low = 0, at_high = 0;
	long    avg = 0;
	long    avg_div = 0;

	for (i = 0; i < VOCROWS; i++)
	{
		if (VC.hash_size[i] == 0)
			zeros[i % 2]++;
		if (VC.hash_size[i] < lowest)
		{
			lowest = VC.hash_size[i];
			at_low = 1;
		}
		else if (VC.hash_size[i] == lowest)
			at_low++;

		/* Don't consider this any further unless it was used */
		if (VC.hash_size[i])
		{
			avg += VC.hash_size[i];
			avg_div++;
			if (VC.hash_size[i] == VC.hash_depth)
			{
				at_high++;
				printf("clumpers: ");
				for (j = 0; j < VC.hash_depth; j++)
				{
					printf("%s, ", word(VC.hash[i][j]));
				}
				printf("\n");
			}
		}
	}

	printf("Vocab Hash Statistics:\n");
	printf(" Vocab String Space: %d(%d), Allocs: %ld\n", VC.vocab_alloc, VC.cur_vocab, str_allocs);
	printf(" Reverse Index Allocs: %ld\n", idx_allocs);
	printf(" Hash Rows: %d, Mem: %ld, Allocs: %ld, Use: %ld(%d) - %ld(%d) Avg: %ld\n",
		   VOCROWS, hash_total_alloc, hash_allocs, lowest, at_low, VC.hash_depth, at_high, avg / avg_div);
	printf(" Zeros = Even:%d, Odd:%d\n", zeros[0], zeros[1]);
	printf(" Entries in the Hash: %ld\n", VC.items);
	for (i = 1; i <= VC.hash_depth; i++)
	{
		for (j = 0; j < VOCROWS; j++)
		{
			if (VC.hash_size[j] >= i)
				putchar('#');
			else
				putchar('.');
		}
		putchar('\n');
	}
}

void
save_vocab_index(void)
{								/* Write the vocab indexes to disk */
	int     fd;
	long    i;
	int     j;

	fd = _open(datafile(vocifn), O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd == -1)
		Err("write", datafile(vocifn));
	/* First write the number of rows, number of indexes and hash depth */
	i = VOCROWS;
	if ( _write(fd, (char *) &i, sizeof(long)) < (ssize_t)sizeof(long) )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	if ( _write(fd, (char *) &VC.items, sizeof(long)) < (ssize_t)sizeof(long) )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	if ( _write(fd, (char *) &VC.hash_depth, sizeof(long)) < (ssize_t)sizeof(long) )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	/* Record how large 'vocab' is */
	if ( _write(fd, (char *) &VC.cur_vocab, sizeof(long)) < (ssize_t)sizeof(long) )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	/* Write out the hash row sizes */
	if ( _write(fd, (char *) &VC.hash_size, sizeof(long) * VOCROWS) < (ssize_t)sizeof(long) * VOCROWS )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	/* Write out the reverse-lookup index */
	if ( _write(fd, (char *) VC.index, sizeof(long) * VC.items) < (ssize_t)sizeof(long) * VC.items )
		throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	for (i = 0; i < VOCROWS; i++)
	{
		long    zero = 0;
		if ( _write(fd, VC.hash[i], sizeof(long) * VC.hash_size[i]) < (ssize_t)sizeof(long) * VC.hash_size[i] )
			throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
		/* Pad the line out to 'hash_depth' to allow for growth */
		for (j = VC.hash_size[i]; j < VC.hash_depth; j++)
			if ( _write(fd, &zero, sizeof(long)) < (ssize_t)sizeof(long) )
				throw Smugl::FDWriteError(datafile(vocifn), errno, fd) ;
	}
	_close(fd);

	/* Now write the actual vocab to disk */
	fd = _open(datafile(vocfn), O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (fd == -1)
		Err("write", datafile(vocfn));
	if ( _write(fd, VC.vocab, VC.cur_vocab) < (ssize_t)VC.cur_vocab )
		throw Smugl::FDWriteError(datafile(vocfn), errno, fd) ;
	_close(fd);
}