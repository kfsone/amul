#ifndef AMUL_SRC_TOKENIZER_H
#define AMUL_SRC_TOKENIZER_H

#include "sourcefile.h"
#include <h/amul.type.h>

enum TokenType {
    TOKEN_INVALID,
    TOKEN_EOB,         // End of a block (paragraph), i.e. \n{2,}
    TOKEN_EOL,         // End of a line
    TOKEN_WHITESPACE,  // tab or space
    TOKEN_COMMENT,
    TOKEN_STRING_LITERAL,
    TOKEN_LABEL,  // SOMETHING=<next token>
    TOKEN_NUMBER,
    TOKEN_WORD,
    TOKEN_IDENTIFIER,  // begins with a symbol
    TOKEN_SYMBOL,      // one or more non-alpha-numeric characters

    NUM_TOKEN_TYPES,
};

struct Token {
    enum TokenType type : 16;
    uint16_t       lineOffset;
    uint32_t       lineNo;
    const char *   start;
    const char *   end;
};

// Handle a block of structure text (as opposed to a block of raw text)
extern error_t TokenizeParseable(
        struct SourceFile *file, struct Token *tokens, size_t tokensSize, size_t *tokensScanned,
        enum TokenType endToken);

#define TokenizeLine(file, tokens, tokensSize, tokensScanned)                                      \
    TokenizeParseable(file, tokens, tokensSize, tokensScanned, TOKEN_EOL)

#define TokenizeBlock(file, tokens, tokensSize, tokensScanned)                                     \
    TokenizeParseable(file, tokens, tokensSize, tokensScanned, TOKEN_EOB)

#endif  // AMUL_SRC_TOKENIZER_H
