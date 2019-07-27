#include <ctype.h>

#include "buffer.h"
#include "filesystem.h"
#include "sourcefile.h"
#include "tokenizer.h"

#include <h/amul.test.h>

void
_consumeEol(Buffer &buffer, Token &token) noexcept
{
    // allow for \r\n, \n and \n\r
    const char first = buffer.Next();
    if (!buffer.Eof()) {
        const char second = buffer.Peek();
        if ((second == '\n' || second == '\r') && second != first)
            buffer.Skip();
    }

    token.end = buffer.Pos();
    token.type = TOKEN_EOL;
}

void
_consumeWhitespace(Buffer &buffer, Token &token) noexcept
{
    char c = 0;
    do {
        c = buffer.Skip();
    } while (c == ' ' || c == '\t');

    token.end = buffer.Pos();
    token.type = TOKEN_WHITESPACE;
}

void
_consumeQuote(Buffer &buffer, Token &token) noexcept
{
    // grab the first character
    const char quote = buffer.Next();
    token.start = buffer.Pos();
    char escaped = false;
    for (;;) {
        const char c = buffer.Peek();
        if (c == quote && !escaped) {
            token.end = buffer.Pos();
            buffer.Skip();
            break;
        }
        if (c == '\n' || c == '\r' || c == 0) {
            token.end = buffer.Pos();
            break;
        }
        escaped = (!escaped && c == '\\');
        buffer.Skip();
    }
    token.type = TOKEN_STRING_LITERAL;
}

void
_consumeComment(Buffer &buffer, Token &token) noexcept
{
    for (;;) {
        const char c = buffer.Skip();
        if (c == '\n' || c == '\r') {
            _consumeEol(buffer, token);
            break;
        }
        if (c == 0)
            break;
    }
    token.end = buffer.Pos();
    token.type = TOKEN_COMMENT;
}

void
_consumeText(Buffer &buffer, Token &token) noexcept
{
    // Ignore the first character so that we don't try to apply the
    // "isalnum" policy to the prefix, this allows '$' for system
    // messages; but check if it an alpha-numeric so we can distinguish
    // '$' from '$10' as SYMBOL vs REGULAR.
    const char first = buffer.Peek();
    const bool startsAlpha = isalpha(first);
    const bool startsNum = isdigit(first) || first == '-' || first == '.';
    bool       hasAlpha = startsAlpha;
    bool       hasNum = isdigit(first);
    bool       hasDot = first == '.';

    token.type = TOKEN_WORD;

    // Now we limit ourselves to alphanumeric with the exception of
    // '=', which we treat as a separator so long as it has no
    // whitespace either side of it.
    for (char c = buffer.Skip(); c; c = buffer.Skip()) {
        if (isalpha(c)) {
            hasAlpha = true;
            continue;
        }
        if (isdigit(c)) {
            hasNum = true;
            continue;
        }
        if (c == '.') {
            if (!startsNum || hasDot)
                break;
            hasDot = true;
            continue;
        }
        if (hasAlpha && c == '_')
            continue;
        if (!startsAlpha || c != '=')
            break;
        token.type = TOKEN_LABEL;
        buffer.Skip();
        token.end = buffer.Pos();
        return;
    }

    if (startsNum && hasNum && !hasAlpha)  // exclude "-" on its own
        token.type = TOKEN_NUMBER;
    else if (!startsAlpha)
        token.type = !(hasAlpha || hasNum) ? TOKEN_SYMBOL : TOKEN_IDENTIFIER;
    token.end = buffer.Pos();
}

struct TokenizerState
{
    SourceFile *file;
    Token *     curToken;
    char        lastChar;
};

error_t
TokenizeParseable(
        SourceFile &file, Token *tokens, size_t tokensSize, size_t *tokensScanned,
        enum TokenType endToken)
{
    REQUIRE(tokens && tokensSize && tokensScanned);

    *tokensScanned = 0;

    // fail when we started at end of file
    Buffer &buffer = file.buffer;
    if (buffer.Eof())
        return ENOENT;

    const Token *tokensStart = tokens;
    Token		*tokensEnd = tokens + tokensSize;
    Token		*prevToken = NULL;
    const char  *lineStart = buffer.Pos();

    while (!buffer.Eof() && tokens < tokensEnd) {
        tokens->lineOffset = (uint16_t)(buffer.Pos() - lineStart);
        if (tokens->lineOffset == 0)
            ++file.lineNo;
        tokens->lineNo = file.lineNo;
        tokens->start = buffer.Pos();

        switch (buffer.Peek()) {
        case '\r':
        case '\n':
            _consumeEol(buffer, *tokens);
            // Potential upgrade to end-of-block
            for (; endToken != TOKEN_EOL;) {
                char nextChar = buffer.Peek();
                if (nextChar != '\n' && nextChar != '\r')
                    break;
                ++file.lineNo;
                _consumeEol(buffer, *tokens);
                tokens->type = TOKEN_EOB;
            }
            lineStart = buffer.Pos();
            break;
        case ' ':
        case '\t':
            _consumeWhitespace(buffer, *tokens);
            // Discard whitespace that occurs in-front of comments.
            if (buffer.Peek() != ';' || endToken == TOKEN_WHITESPACE)
                break;
            tokens->start = buffer.Pos();
            tokens->lineOffset = buffer.Pos() - lineStart;
            /*FALLTHROUGH*/
        case ';':
            _consumeComment(buffer, *tokens);
            lineStart = buffer.Pos();
            break;
        case '"':
        case '\'':
            _consumeQuote(buffer, *tokens);
            break;
        default:
            _consumeText(buffer, *tokens);
            break;
        }
        prevToken = tokens;
        ++tokens;
        if (prevToken->type == endToken)
            break;
    }

    *tokensScanned = tokens - tokensStart;

    return 0;
}
