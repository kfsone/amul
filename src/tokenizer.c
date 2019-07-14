#include <ctype.h>

#include "tokenizer.h"
#include "buffer.h"
#include "filesystem.h"
#include "sourcefile.h"

#include <h/amul.test.h>

void
_consumeEol(struct Buffer *buf, struct Token *token)
{
    token->type = TOKEN_EOL;

    // allow for \r\n, \n and \n\r
    const char first = BufferNext(buf);
    if (!BufferEOF(buf)) {
        const char second = BufferPeek(buf);
        if ((second == '\r' || second == '\n') && second != first)
            BufferSkip(buf);
    }

    token->end = buf->pos;
}

void
_consumeWhitespace(struct Buffer *buf, struct Token *token)
{
    token->type = TOKEN_WHITESPACE;
    for (char c = ' '; c == ' ' || c == '\t';) {
        c = BufferNext(buf);
    }
    token->end = buf->pos;
}

void
_consumeQuote(struct Buffer *buf, struct Token *token)
{
    token->type = TOKEN_STRING_LITERAL;
    // grab the first character
    char quote = BufferNext(buf);
    token->start = buf->pos;
    for (;;) {
        const char c = BufferPeek(buf);
        if (c == quote) {
            token->end = buf->pos;
            BufferSkip(buf);
            return;
        }
        switch (c) {
        case 0:
        case '\r':
        case '\n':
            token->end = buf->pos;
            return;
        }
    }
}

void
_consumeComment(struct Buffer *buf, struct Token *token)
{
    token->type = TOKEN_COMMENT;
    for (;;) {
        const char c = BufferNext(buf);
        if (c == '\n' || c == '\r' || c == 0)
            break;
    }
    token->end = buf->pos;
}

void
_consumeRegular(struct Buffer *buf, struct Token *token)
{
    // Ignore the first character so that we don't try to apply the
    // "isalnum" policy to the prefix, this allows '$' for system
    // messages; but check if it an alpha-numeric so we can distinguish
    // '$' from '$10' as SYMBOL vs REGULAR.
    bool hasAlNum = isalnum(BufferNext(buf));

    token->type = TOKEN_REGULAR;

    // Now we limit ourselves to alphanumeric with the exception of
    // '=', which we treat as a separator so long as it has no
    // whitespace either side of it.
    for (;;) {
        const char c = BufferNext(buf);
        if (isalnum(c)) {
            hasAlNum = true;
            continue;
        }
        if (c != '=')
            break;
        // Promote x=y to a label but not x=<space>
        char nextc = BufferPeek(buf);
        if (!isalnum(nextc)) {
            switch (nextc) {
            case '_':
                continue;
            // string literals
            case '\'':
            case '\"':
            // system id prefixes
            case '$':  // smsgs
            case '!':  // npc personas
            case '.':  // private verbs
                token->type = TOKEN_LABEL;
                token->end = buf->pos;
                return;
            }
            // Treat it as a terminator.
            break;
        }
    }

    token->type = hasAlNum ? TOKEN_REGULAR : TOKEN_SYMBOL;
    token->end = buf->pos;
}

error_t
ScanParseable(
        struct SourceFile *file, struct Token *tokens, size_t tokensSize, size_t *tokensScanned)
{
    REQUIRE(file);
    REQUIRE(tokens && tokensSize && tokensScanned);

    *tokensScanned = 0;

    // false when we started at end of file
    struct Buffer *buffer = file->buffer;
    if (BufferEOF(buffer))
        return ENOENT;

    // we always start a new line.
    file->lineNo++;

    const struct Token *tokensStart = tokens;
    struct Token *      endToken = tokens + tokensSize;
    struct Token *      prevToken = NULL;
    const char *        lineStart = buffer->pos;

    while (!BufferEOF(buffer) && tokens < endToken) {
        tokens->lineNo = file->lineNo;
        tokens->lineOffset = (uint16_t)(buffer->pos - lineStart);
        tokens->start = buffer->pos;

        switch (BufferPeek(buffer)) {
        case '\r':
        case '\n':
            _consumeEol(buffer, tokens);
            break;
        case ' ':
        case '\t':
            _consumeWhitespace(buffer, tokens);
            break;
        case '"':
        case '\'':
            _consumeQuote(buffer, tokens);
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
            _consumeRegular(buffer, tokens);
            break;
        }
        prevToken = tokens;
        ++tokens;
    }

    *tokensScanned = tokens - tokensStart;

    return 0;
}
