#ifndef AMUL_SRC_TRIE_H
#define AMUL_SRC_TRIE_H 1

#ifndef TRIE_TERMINAL_IS_CUSTOM
// Define TRIE_TERMINAL_IS_CUSTOM if you want to override the type used
typedef uint64_t trie_terminal_t;
#endif

struct TrieNode {
	// if terminal is non-null, it identifies that this node represents a
	// discrete word. It may still have children for similar or derived
	// words, e.g. a node representing "node" could have a child representing
	// the word "nodes".
	// beyond null vs non-null, 'terminal' is opaque user-defined.
	union {
		struct TrieNode **children;
		struct TrieNode *next;
	};
	trie_terminal_t	terminal;
	// what letter this represents
	char			letter;
};

// Trie covers characters 32-127
enum {
	MIN_TRIE_CHAR = 32,
	MAX_TRIE_CHAR = 127,
	TRIE_RANGE = MAX_TRIE_CHAR - MIN_TRIE_CHAR
};

struct TrieNodePool {
	struct TrieNodePool	*next;
	struct TrieNode		nodes[];
};

struct Trie {
	struct TrieNode roots[TRIE_RANGE];
	struct TrieNodePool* pools;
	struct TrieNode* freelist;	// Linked by 'abuse of 'next' pointers.
	size_t nodes;
	size_t terminals;
};

// Release a trie and all of it's nodes
void         	CloseTrie(struct Trie *trie);

// Look up a word in the trie. If terminal is not NULL, the word will be
// added if it was not already present. If the word is either present or
// added, returns the terminal of the entry.
trie_terminal_t get_trie_word(struct Trie *trie, const char *string, trie_terminal_t terminal);

// Helper: test for presence of a word in a trie.
static inline
bool is_trie_word(struct Trie *trie, const char* string) {
	return get_trie_word(trie, string, (trie_terminal_t)NULL);
}

// Helper: Get a word without adding it
static inline
trie_terminal_t lookup_trie_word(struct Trie* trie, const char *string) {
	return get_trie_word(trie, string, (trie_terminal_t)NULL);
}

#endif
