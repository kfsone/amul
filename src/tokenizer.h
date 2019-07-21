#ifndef AMUL_SRC_TOKENIZER_H
#define AMUL_SRC_TOKENIZER_H

#include <h/amul.type.h>
#include "sourcefile.h"

enum TokenType {
    TOKEN_INVALID,
    TOKEN_EOL,
    TOKEN_WHITESPACE,
    TOKEN_COMMENT,
    TOKEN_STRING_LITERAL,
    TOKEN_LABEL,		// SOMETHING=<next token>
	TOKEN_NUMBER,
    TOKEN_WORD,
	TOKEN_IDENTIFIER,	// begins with a symbol
    TOKEN_SYMBOL,		// one or more non-alpha-numeric characters
};

struct Token {
    enum TokenType type : 16;
    uint16_t       lineNo;
    uint16_t       lineOffset;
    const char *   start;
    const char *   end;
};

extern error_t ScanParseable(
        struct SourceFile *file, struct Token *tokens, size_t tokensSize, size_t *tokensScanned);

#endif  // AMUL_SRC_TOKENIZER_H
