#include <ctype.h>

#include "buffer.h"
#include "filesystem.h"
#include "sourcefile.h"
#include "tokenizer.h"

#include <h/amul.test.h>

void
_consumeEol(struct Buffer *buf, struct Token *token)
{
    // allow for \r\n, \n and \n\r
    const char first = BufferNext(buf);
    if (!BufferEOF(buf)) {
        const char second = BufferPeek(buf);
        if ((second == '\n' || second == '\r') && second != first)
            BufferSkip(buf);
    }

    token->end = buf->pos;
    token->type = TOKEN_EOL;
}

void
_consumeWhitespace(struct Buffer *buf, struct Token *token)
{
    char c = 0;
	do {
        c = BufferSkip(buf);
    } while (c == ' ' || c == '\t');

	token->end = buf->pos;
    token->type = TOKEN_WHITESPACE;
}

void
_consumeQuote(struct Buffer *buf, struct Token *token)
{
    // grab the first character
    char quote = BufferNext(buf);
    token->start = buf->pos;
    for (;;) {
        const char c = BufferPeek(buf);
        if (c == quote) {
            token->end = buf->pos;
            BufferSkip(buf);
            break;
        }
        if (c == '\n' || c == '\r' || c == 0) {
            token->end = buf->pos;
            break;
        }
        BufferSkip(buf);
    }
    token->type = TOKEN_STRING_LITERAL;
}

void
_consumeComment(struct Buffer *buf, struct Token *token)
{
    for (;;) {
        const char c = BufferSkip(buf);
        if (c == '\n' || c == '\r') {
            _consumeEol(buf, token);
            break;
        }
        if (c == 0)
            break;
    }
    token->end = buf->pos;
    token->type = TOKEN_COMMENT;
}

void
_consumeText(struct Buffer *buf, struct Token *token)
{
    // Ignore the first character so that we don't try to apply the
    // "isalnum" policy to the prefix, this allows '$' for system
    // messages; but check if it an alpha-numeric so we can distinguish
    // '$' from '$10' as SYMBOL vs REGULAR.
    const bool startsAlpha = isalpha(BufferPeek(buf));
    const bool startsNum = isdigit(BufferPeek(buf));
    bool       hasAlpha = startsAlpha;
    bool       hasNum = startsNum;

    token->type = TOKEN_WORD;

    // Now we limit ourselves to alphanumeric with the exception of
    // '=', which we treat as a separator so long as it has no
    // whitespace either side of it.
    for (char c = BufferSkip(buf); c; c = BufferSkip(buf)) {
        if (isalpha(c)) {
            hasAlpha = true;
            continue;
        }
        if (isdigit(c) && c != '-') {
            if (!startsNum && c == '.')
                break;
            hasNum = true;
            continue;
        }
        if (hasAlpha && c == '_')
            continue;
        if (!startsAlpha || c != '=')
            break;
        token->type = TOKEN_LABEL;
        BufferSkip(buf);
        token->end = buf->pos;
        return;
    }

	if (startsNum)
        token->type = TOKEN_NUMBER;
    else if (!startsAlpha)
        token->type = !(hasAlpha || hasNum) ? TOKEN_SYMBOL : TOKEN_IDENTIFIER;
    token->end = buf->pos;
}

error_t
TokenizeParseable(
        struct SourceFile *file, struct Token *tokens, size_t tokensSize, size_t *tokensScanned)
{
    REQUIRE(file);
    REQUIRE(tokens && tokensSize && tokensScanned);
    REQUIRE(file->buffer);

    *tokensScanned = 0;

    // false when we started at end of file
    struct Buffer *buffer = file->buffer;
    if (BufferEOF(buffer))
        return ENOENT;

    const struct Token *tokensStart = tokens;
    struct Token *      endToken = tokens + tokensSize;
    struct Token *      prevToken = NULL;
    const char *        lineStart = buffer->pos;
    bool                endOfBlock = false;

    while (!BufferEOF(buffer) && tokens < endToken && !endOfBlock) {
        tokens->lineOffset = (uint16_t)(buffer->pos - lineStart);
        if (tokens->lineOffset == 0)
            ++file->lineNo;
        tokens->lineNo = file->lineNo;
        tokens->start = buffer->pos;

        switch (BufferPeek(buffer)) {
        case '\r':
        case '\n':
            _consumeEol(buffer, tokens);
			// Potential upgrade to end-of-block
            for (;;) {
                char nextChar = BufferPeek(buffer);
                if (nextChar != '\n' && nextChar != '\r')
                    break;
                ++file->lineNo;
                _consumeEol(buffer, tokens);
                tokens->type = TOKEN_EOB;
            }
            endOfBlock = true;
            lineStart = buffer->pos;
            break;
        case ' ':
        case '\t':
            _consumeWhitespace(buffer, tokens);
            break;
        case '"':
        case '\'':
            _consumeQuote(buffer, tokens);
            lineStart = buffer->pos;
            break;
        case ';':
            // Discard whitespace in-front of comments
            if (prevToken && prevToken->type == TOKEN_WHITESPACE) {
                --tokens;
                prevToken = NULL;
                continue;
            }
            _consumeComment(buffer, tokens);
            break;
        default:
            _consumeText(buffer, tokens);
            break;
        }
        prevToken = tokens;
        ++tokens;
    }

    *tokensScanned = tokens - tokensStart;

    return 0;
}
