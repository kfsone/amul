// This may look like C, but it's really -*- C++ -*-
// $Id: parser.hpp,v 1.4 1999/06/11 14:26:45 oliver Exp $
// Protos and definitions for the parser code

// Defines
#define MAX_PHRASE_SIZE 255  // Maximum input size
// Maximum number of components in a phrase:
//  VERB + ADJ + NOUN + PREP + ADJ + NOUN = 6
#define MAX_TOK 6  // Maximum phrase components (tokens)

enum tokType { tokUNK = -1, tokWORD, tokSTRING };
enum slotResult { slotFailed = -1, slotIgnore, slotProcessed };

extern bool do_condition(struct VBTAB *vt, bool lastCond);
extern slotResult do_action(struct VBTAB *vt, bool lastCond);

struct TOKEN {
    tokType type;
    union {
        vocid_t id;  // For a known word
        char *ptr;   // For a text string
    };
};

// Variables
extern vocid_t verb;   // Current verb
extern char input[];   // Current input
extern char phrase[];  // Current 'phrase'

extern TOKEN token[MAX_TOK];  // Details on the tokens
extern int tokens;            // Number of tokens in current phrase

// Functions
extern void sanitise_input(void);
extern void parse(char *s);
extern int tokenise_phrase(char *s);
extern slotResult parse_phrase();
