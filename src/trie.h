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
	trie_terminal_t	terminal;
	struct TrieNode **children;
	// what letter this represents
	char			letter;
};

// Trie covers characters 32-127
enum {
	MIN_TRIE_CHAR = 32,
	MAX_TRIE_CHAR = 127,
	TRIE_RANGE = MAX_TRIE_CHAR - MIN_TRIE_CHAR
};

struct Trie {
	struct TrieNode roots[TRIE_RANGE];
	size_t nodes;
	size_t terminals;
};

// Constructor for creating a new Trie instance.
struct Trie 	*NewTrie();
// Release a trie and all of it's nodes
void         	CloseTrie(struct Trie *trie);
// Attempts to add a new word to a trie, returning true if the word was
// not previously registered.
// Returns false if (a) the word was already registered or (b) terminal is NULL
bool         	add_trie_word(struct Trie *trie, const char *word,
						   trie_terminal_t terminal);

// Look up a word in the trie, returns NULL if the word is not registered,
// otherwise returns the terminal value of the word.
trie_terminal_t get_trie_word(struct Trie *trie, const char *string);

#endif
