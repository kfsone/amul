#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

#include "trie.h"

/*
 * TODO:
 *
 * Allocate letter nodes in bulk, say 1k at a time and organize as
 * sibling -> child -v
 *
 * This version is "good enough"
 */

///////////////////////////////////////////////////////////////////////////////
// Helpers

// is_power_of_2 returns true if `value` is a power of 2, e.g 0, 1, 2, 4, 8...
// It does this by bit-anding value with value-1 and testing for zero. This
// works because a powers of 2 always have only one bit set, and no value
// lower than them can possibly set that bit, but sequent values will share
// at least the nearest power-of-2 bit. E.g 8 == 8 + 0, 9 = 8 + 1
static inline
bool is_power_of_2(int value) { return !(value & (value - 1)); }


///////////////////////////////////////////////////////////////////////////////
// TrieNode methods

// NewTrieNode is the 'constructor' for a TrieNode instance.
struct TrieNode*
NewTrieNode(struct Trie* trie, char c, trie_terminal_t terminal) {
	struct TrieNode* node = calloc(sizeof(struct TrieNode), 1);
	node->letter = c;
	node->terminal = terminal;
	trie->nodes++;
	if (terminal)
		trie->terminals++;
	return node;
}

///////////////////////////////////////////////////////////////////////////////
// Trie methods.

void _new_pool(struct Trie* trie)
{
	static const size_t growthRate = 1000;
	static const size_t poolSize = sizeof(void*) + sizeof(struct TrieNode) * growthRate;
	struct TrieNodePool *pool = calloc(poolSize, 1);
	assert(pool != NULL);
	pool->next = trie->pools;
	trie->pools = pool;
	struct TrieNode** cur = &trie->freelist;
	while (*cur != NULL) {
		cur = &(*cur)->next;
	}
	for (size_t i = 0; i < growthRate; ++i) {
		*cur = &pool->nodes[i];
		cur = &pool->nodes[i].next;
	}
}

// NewTrie is the 'constructor' for a Trie instance, returns an empty Trie.
struct Trie *
NewTrie() {
	struct Trie* trie = calloc(sizeof(struct Trie), 1);
	for (int i = 0; i < TRIE_RANGE; ++i) {
		trie->roots[i].letter = MIN_TRIE_CHAR + i;
	}
	return trie;
}

// CloseTrie is the destructor for a Trie instance which frees all
// associated memory.
void
CloseTrie(struct Trie *trie) {
	/// todo: walk & delete
}

// Internal helper: Allocate or increase child capacity of a given node
struct TrieNode*
_grow_trie_node(struct Trie* trie, struct TrieNode *node, char letter, int childNo)
{
	// the child list is always sized a power of 2. This way we always
	// know the relative capacity. 
	if (is_power_of_2(childNo)) {
		if (childNo == 0) {
			// 2 children: one for the first node and a NULL terminator
			node->children = calloc(sizeof(node), 2);
			printf("%c alloc 2 nodes for %c\n", node->letter, letter);
		} else {
			int new_capacity = childNo << 1;
			node->children = realloc(node->children, sizeof(node) * new_capacity);
			memset(node->children + childNo, 0, sizeof(node) * childNo);
			printf("%c growing to %d nodes for %c\n", node->letter, new_capacity, letter);
		}
	}

	node->children[childNo] = NewTrieNode(trie, letter, (trie_terminal_t)0);
	int cap;
	for (cap = 2; cap < childNo; ++cap)
		;
	for (int i = 0; i < cap; ++i) {
		printf(" %p\n", node->children[i]);
	}

	return node->children[childNo];
}

// Internal helper: traverses the trie to the point where a given
// node should be. If terminal is not zero, nodes will be added so
// we can always reach the insertion point.
trie_terminal_t
get_trie_word(struct Trie *trie, const char *string, trie_terminal_t terminal)
{
	assert(string[0] >= MIN_TRIE_CHAR && string[0] <= MAX_TRIE_CHAR);
	struct TrieNode *nodeCursor = &trie->roots[(*string) - MIN_TRIE_CHAR];
	// We've resolve the first node
	++string;
	while (*string) {
		assert(nodeCursor);
		// always use a ^2 size, that way we don't need to store
		// capacity
		struct TrieNode* child = NULL;
		int childNo = 0;
		if (nodeCursor->children) {
			for (childNo = 0; nodeCursor->children[childNo]; ++childNo) {
				if (nodeCursor->children[childNo]->letter == *string) {
					child = nodeCursor->children[childNo];
					break;
				}
			}
		}
		if (child == NULL) {
			if (!terminal) {
				return (trie_terminal_t)0;
			}
			child = _grow_trie_node(trie, nodeCursor, *string, childNo);
		}
		nodeCursor = child;
		++string;
	}
	if (nodeCursor) {
		if (!terminal) {
			return nodeCursor->terminal;
		}
		if (nodeCursor->terminal && nodeCursor->terminal != terminal) {
			assert(nodeCursor->terminal == terminal);
			return nodeCursor->terminal;
		}
		nodeCursor->terminal = terminal;
		return terminal;
	}
	return 0;
}

///////////////////////////////////////////////////////////////////////////////
// Unit tests

void
test_node(struct TrieNode *node, trie_terminal_t terminal, char letter,
		  bool has_child1, bool has_child2)
{
	assert(node->terminal == terminal);
	assert(node->letter == letter);
	if (!has_child1) {
		assert(node->children == NULL);
		return;
	}
	if (has_child1)
		assert(node->children[0] != NULL);
	if (has_child2)
		assert(node->children[1] != NULL);
	else
		assert(node->children[1] == NULL);
	if (has_child2)
		assert(node->children[2] == NULL);
}


void
test_trie()
{
	struct Trie* trie = NewTrie();
	struct TrieNode* cur = &trie->roots['h' - MIN_TRIE_CHAR];
	struct TrieNode* aNode = &trie->roots['a' - MIN_TRIE_CHAR];
	struct TrieNode* safeNode = &trie->roots['H' - MIN_TRIE_CHAR];

	// check the empty trie.
	test_node(cur, 0, 'h', false, false);
	test_node(aNode, 0, 'a', false, false);
	test_node(safeNode, 0, 'H', false, false);

	// test some string lookups on an empty trie fail but make no mods
	assert(!get_trie_word(trie, "a", 0));
	assert(!get_trie_word(trie, "ant", 0));
	assert(!get_trie_word(trie, "h", 0));
	assert(!get_trie_word(trie, "hi", 0));
	test_node(aNode, 0, 'a', false, false);
	test_node(cur, 0, 'h', false, false);
	test_node(safeNode, 0, 'H', false, false);
	///TODO: memcmp the entire list?

	// try adding a simple 2-character string
	assert(get_trie_word(trie, "hi", 123) == 123);

	test_node(aNode, 0, 'a', false, false);
	test_node(cur, 0, 'h', true, false);
	test_node(cur->children[0], 123, 'i', false, false);
	test_node(safeNode, 0, 'H', false, false);

	// repeat the get_trie_word tests to confirm no unexpected results
	assert(!get_trie_word(trie, "a", 0));
	assert(!get_trie_word(trie, "ant", 0));
	assert(!get_trie_word(trie, "H", 0));
	assert(get_trie_word(trie, "hi", 0) == 123);
	assert(!get_trie_word(trie, "hint", 0));
	assert(!get_trie_word(trie, "hist", 0));

	// add a string that will be child of the first but leaving
	// an intermediate non-terminal node
	assert(get_trie_word(trie, "hint", 8) == 8);

	assert(!get_trie_word(trie, "ant", 0));
	assert(get_trie_word(trie, "hi", 0) == 123);
	assert(!get_trie_word(trie, "hin", 0));
	assert(get_trie_word(trie, "hint", 0) == 8);
	assert(!get_trie_word(trie, "hist", 0));

	test_node(cur, 0, 'h', true, false);
	test_node(cur->children[0], 123, 'i', true, false);
	test_node(cur->children[0]->children[0], 0, 'n', true, false);
	test_node(cur->children[0]->children[0]->children[0], 8, 't', false, false);

	// branch on a non-terminal
	assert(get_trie_word(trie, "hist", 777) == 777);

	test_node(cur, 0, 'h', true, false);
	test_node(cur->children[0], 123, 'i', true, true);
	test_node(cur->children[0]->children[0], 0, 'n', true, false);
	test_node(cur->children[0]->children[1], 0, 's', true, false);
	test_node(cur->children[0]->children[0]->children[0], 8, 't', false, false);
	test_node(cur->children[0]->children[1]->children[0], 777, 't', false, false);

	assert(!get_trie_word(trie, "a", 0));
	assert(!get_trie_word(trie, "ant", 0));
	assert(!get_trie_word(trie, "H", 0));
	assert(get_trie_word(trie, "hi", 0) == 123);
	assert(get_trie_word(trie, "hint", 0) == 8);
	assert(get_trie_word(trie, "hist", 0) == 777);
	assert(!get_trie_word(trie, "history", 0));

	// test adding a single character word
	assert(get_trie_word(trie, "a", 999) == 999);

	// previous tests should still be true
	test_node(aNode, 999, 'a', false, false);
	test_node(cur, 0, 'h', true, false);
	test_node(cur->children[0], 123, 'i', true, true);
	test_node(cur->children[0]->children[0], 0, 'n', true, false);
	test_node(cur->children[0]->children[1], 0, 's', true, false);
	test_node(cur->children[0]->children[0]->children[0], 8, 't', false, false);
	test_node(cur->children[0]->children[1]->children[0], 777, 't', false, false);

	assert(get_trie_word(trie, "a", 0) == 999);
	assert(!get_trie_word(trie, "ant", 0));
	assert(!get_trie_word(trie, "H", 0));
	assert(get_trie_word(trie, "hi", 0) == 123);
	assert(get_trie_word(trie, "hint", 0) == 8);
	assert(get_trie_word(trie, "hist", 0) == 777);
	assert(!get_trie_word(trie, "history", 0));
}


int main () {
	test_trie();
}
