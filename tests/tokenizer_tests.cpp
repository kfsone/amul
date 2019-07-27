#include <h/amul.test.h>
#include <src/buffer.h>
#include <src/tokenizer.h>

extern void _consumeEol(Buffer &buf, struct Token *token);
extern void _consumeWhitespace(Buffer &buf, struct Token *token);
extern void _consumeQuote(Buffer &buf, struct Token *token);
extern void _consumeComment(Buffer &buf, struct Token *token);
extern void _consumeText(Buffer &buf, struct Token *token);

//////////////////////////////////////////////////////////////////////////////////////////////////
// helper structs

enum { MAX_TEST_TOKENS = 64 };
struct TokenizerTest {
    struct SourceFile sf;
    Buffer     buffer;
    struct Token      tokens[MAX_TEST_TOKENS];
    size_t            scanned;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// helper functions

static Buffer
init_buffer(const char *text, struct Token *token)
{
    Buffer buffer {text, text + strlen(text)};
    if (token) {
        token->type = (enum TokenType)0;
        token->start = text;
        token->end = NULL;
    }
    return buffer;
}

void
_tearUpTokenizerTest(struct TestContext *t)
{
    assert(t->userData == NULL);
    struct TokenizerTest *tt = (TokenizerTest*)calloc(sizeof(struct TokenizerTest), 1);
    assert(tt);
    tt->sf = {"source.txt", NULL, &tt->buffer, 0, 0};
    t->userData = (void *)tt;
}

void
_tearDownTokenizerTest(struct TestContext *t)
{
    assert(t->userData);
    free(t->userData);
    t->userData = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// test the _consumeEol function

void
test_consume_eol_cr(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\nNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_VAL_EQUAL('N', *b.pos);
    EXPECT_VAL_EQUAL(TOKEN_EOL, token.type);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lf(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_VAL_EQUAL('N', *b.pos);
    EXPECT_VAL_EQUAL(TOKEN_EOL, token.type);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_crlf(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\n\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 2, b.pos);
    EXPECT_VAL_EQUAL('N', *b.pos);
    EXPECT_VAL_EQUAL(TOKEN_EOL, token.type);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lfcr(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\r\nNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 2, b.pos);
    EXPECT_VAL_EQUAL('N', *b.pos);
    EXPECT_VAL_EQUAL(TOKEN_EOL, token.type);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_crcr(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\n\nNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lflf(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\r\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_crlfcrlf(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\n\r\n\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 2, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lfcrlfcr(struct TestContext *t)
{
    struct Token  token;
    Buffer b = init_buffer("\r\n\r\nNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 2, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// test the _consumeWhitespace function

void
test_consume_whitespace_single_space(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer b = init_buffer(" ", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_PTR_EQUAL(b.end, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
    {
        struct Token  token;
        Buffer b = init_buffer(" i", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_VAL_EQUAL('i', *b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
}

void
test_consume_whitespace_single_tab(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer b = init_buffer("\t", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_PTR_EQUAL(b.end, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
    {
        struct Token  token;
        Buffer b = init_buffer("\ti", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_VAL_EQUAL('i', *b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
}

void
test_consume_whitespace_multi(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer b = init_buffer("   \t\t\t   \t\t\t", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 12, b.pos);
        EXPECT_PTR_EQUAL(b.end, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
        EXPECT_VAL_EQUAL(0, *token.end);
    }
    {
        struct Token  token;
        Buffer b = init_buffer("   \t\t\t   \t\t\t\n", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 12, b.pos);
        EXPECT_PTR_EQUAL(b.end - 1, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
        EXPECT_VAL_EQUAL('\n', *token.end);
    }
}

void
test_consume_quote_dq(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("\"\"", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_VAL_EQUAL('"', *token.start);
    }
    {
        struct Token  token;
        Buffer buffer = init_buffer("\"'\"!", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_VAL_EQUAL('"', *token.end);
        EXPECT_VAL_EQUAL('!', *buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
        EXPECT_VAL_EQUAL('\'', *token.start);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("\"a' \"!", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_VAL_EQUAL('"', *token.end);
        EXPECT_VAL_EQUAL('!', *buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
        EXPECT_VAL_EQUAL('a', *token.start);
    }
}

void
test_consume_quote_sq(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("''", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_VAL_EQUAL('\'', *token.start);
    }
    {
        struct Token  token;
        Buffer buffer = init_buffer("'\"'!", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_VAL_EQUAL('\'', *token.end);
        EXPECT_VAL_EQUAL('!', *buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
        EXPECT_VAL_EQUAL('"', *token.start);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("'a\" '!", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_VAL_EQUAL('\'', *token.end);
        EXPECT_VAL_EQUAL('!', *buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
        EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
        EXPECT_VAL_EQUAL('a', *token.start);
    }
}

void
test_consume_quote_eol(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("\"...", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_VAL_EQUAL(0, *token.end);
        EXPECT_VAL_EQUAL(0, *buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("\"...\n", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_VAL_EQUAL('\n', *buffer.pos);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("\"...\r", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_VAL_EQUAL('\r', *buffer.pos);
    }
}

void
test_consume_comment(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer(";", &token);
        _consumeComment(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_COMMENT, token.type);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start, token.start);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer(";;;;;    hello ", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer(";;;;;    hello \n\r\n", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_VAL_EQUAL('\n', *token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer(";;;;;    hello \r\n\r\r\n", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 3, token.end);
        EXPECT_VAL_EQUAL('\r', *token.end);
    }
}

void
test_consume_text_word(struct TestContext *t)
{
    struct Token  token;
    Buffer buffer = init_buffer("abc", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_WORD, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);
}

void
test_consume_text_symbol(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("$", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("=", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("_", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("_=", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("=_", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("= ", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }
}

void
test_consume_text_identifier(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("$bc", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("$1c", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("$c_1", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("=1a2", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("!abc.1", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
    }
}

void
test_consume_text_number(struct TestContext *t)
{
    {
        struct Token  token;
        Buffer buffer = init_buffer("1", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("123-", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("-123", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("12.3", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer(".101", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        Buffer buffer = init_buffer("-.001", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }
}

void
test_consume_text_label(struct TestContext *t)
{
    struct Token  token;
    Buffer buffer;

    buffer = init_buffer("abc=xyz", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_LABEL, token.type);
    EXPECT_PTR_EQUAL(buffer.start, token.start);
    EXPECT_PTR_EQUAL(buffer.start + 4, token.end);
    EXPECT_VAL_EQUAL('x', *token.end);
}

void
test_tokenize_parseable_failargs(struct TestContext *t)
{
    struct TokenizerTest *tt = (struct TokenizerTest *)(t->userData);

    EXPECT_ERROR(EINVAL, TokenizeLine(NULL, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeLine(&tt->sf, NULL, MAX_TEST_TOKENS, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeLine(&tt->sf, &tt->tokens[0], 0, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeLine(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, NULL));

    // It should also want to fail if we pass it a file without a buffer
    tt->sf.buffer = NULL;
    EXPECT_ERROR(EINVAL, TokenizeLine(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
}

void
test_tokenize_parseable_enoent(struct TestContext *t)
{
    // passing an empty buffer should get us an ENOENT
    struct TokenizerTest *tt = (struct TokenizerTest *)(t->userData);
    EXPECT_ERROR(ENOENT, TokenizeLine(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
}

struct TokenizerTest *
_tokenTest(struct TestContext *t, const char *text, size_t maxTestTokens, enum TokenType endToken)
{
    struct TokenizerTest *tt = (struct TokenizerTest *)(t->userData);
    tt->sf.lineNo = 0;
    tt->scanned = 0;
    tt->buffer = init_buffer(text, NULL);
    EXPECT_SUCCESS(
            TokenizeParseable(&tt->sf, &tt->tokens[0], maxTestTokens, &tt->scanned, endToken));
    return tt;
}

void
test_tokenize_parseable_eob(struct TestContext *t)
{
    //                                         -1- -2- 3 4 -5- -6-
    struct TokenizerTest *tt = _tokenTest(t, "\n\r\n\r\n\n\n\r\r\n", MAX_TEST_TOKENS, TOKEN_EOB);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_PTR_EQUAL(tt->buffer.end, tt->buffer.pos);
    EXPECT_VAL_EQUAL(0, *tt->buffer.pos);

    struct Token *token = &tt->tokens[0];
    EXPECT_VAL_EQUAL(TOKEN_EOB, token->type);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);
    EXPECT_PTR_EQUAL(tt->buffer.start, token->start);
    EXPECT_PTR_EQUAL(tt->buffer.end, token->end);
    EXPECT_VAL_EQUAL(0, *token->end);

    EXPECT_VAL_EQUAL(6, tt->sf.lineNo);
}

void
test_tokenize_parseable_eol_cr(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\n", MAX_TEST_TOKENS, TOKEN_EOB);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    struct Token *token = &tt->tokens[0];
    EXPECT_VAL_EQUAL(TOKEN_EOL, token->type);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);
    EXPECT_PTR_EQUAL(tt->buffer.start + 1, tt->buffer.end);
    EXPECT_PTR_EQUAL(tt->buffer.end, tt->buffer.pos);
    EXPECT_VAL_EQUAL(0, *tt->buffer.pos);
    EXPECT_PTR_EQUAL(tt->buffer.start, token->start);
    EXPECT_PTR_EQUAL(tt->buffer.end, token->end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);  // We're not *on* line 2 yet
}

void
test_tokenize_parseable_eol_lf(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\r", MAX_TEST_TOKENS, TOKEN_EOB);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    struct Token *token = &tt->tokens[0];
    EXPECT_VAL_EQUAL(TOKEN_EOL, token->type);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);
    EXPECT_PTR_EQUAL(tt->buffer.start + 1, tt->buffer.end);
    EXPECT_PTR_EQUAL(tt->buffer.end, tt->buffer.pos);
    EXPECT_VAL_EQUAL(0, *tt->buffer.pos);
    EXPECT_PTR_EQUAL(tt->buffer.start, token->start);
    EXPECT_PTR_EQUAL(tt->buffer.end, token->end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_eol_crlf(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\r\n", MAX_TEST_TOKENS, TOKEN_EOB);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    struct Token *token = &tt->tokens[0];
    EXPECT_VAL_EQUAL(TOKEN_EOL, token->type);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);
    EXPECT_PTR_EQUAL(tt->buffer.start + 2, tt->buffer.end);
    EXPECT_PTR_EQUAL(tt->buffer.end, tt->buffer.pos);
    EXPECT_VAL_EQUAL(0, *tt->buffer.pos);
    EXPECT_PTR_EQUAL(tt->buffer.start, token->start);
    EXPECT_PTR_EQUAL(tt->buffer.end, token->end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_eol_break(struct TestContext *t)
{
    // Test that when endToken is TOKEN_EOL we only consume one eol
    struct TokenizerTest *tt = _tokenTest(t, "\r\n\r\n", MAX_TEST_TOKENS, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    struct Token *token = &tt->tokens[0];
    EXPECT_VAL_EQUAL(TOKEN_EOL, token->type);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);
    EXPECT_PTR_EQUAL(tt->buffer.start + 2, tt->buffer.pos);
    EXPECT_VAL_EQUAL('\r', *tt->buffer.pos);
    EXPECT_PTR_EQUAL(tt->buffer.start, token->start);
    EXPECT_PTR_EQUAL(tt->buffer.start + 2, token->end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_whitespace(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "  \t   X", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_VAL_EQUAL(0, tt->tokens[0].lineOffset);
    EXPECT_VAL_EQUAL(' ', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL('X', *tt->tokens[0].end);
}

void
test_tokenize_parseable_comment_no_eol(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "; Who knows", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_COMMENT, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_VAL_EQUAL(0, tt->tokens[0].lineOffset);
    EXPECT_VAL_EQUAL(';', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL(0, *tt->tokens[0].end);

    // Comments should consume whitespace ahead of them.
    tt = _tokenTest(t, "\t\t \t; Who knows", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_COMMENT, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(';', *tt->tokens[0].start);
}

void
test_tokenize_parseable_comment_eol(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "; Who knows\n", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_COMMENT, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_VAL_EQUAL(0, tt->tokens[0].lineOffset);
    EXPECT_VAL_EQUAL(';', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL(0, *tt->tokens[0].end);
    EXPECT_VAL_EQUAL('\n', *(tt->tokens[0].end - 1));
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_string_lit_single_wclose(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "'This is \"test\" text'.", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_VAL_EQUAL(0, tt->tokens[0].lineOffset);
    EXPECT_VAL_EQUAL('T', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL('\'', *tt->tokens[0].end);
    EXPECT_VAL_EQUAL('t', *(tt->tokens[0].end - 1));
    EXPECT_VAL_EQUAL('.', *tt->buffer.pos);
}

void
test_tokenize_parseable_string_lit_double_wclose(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\"This is 'test' text\".", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_VAL_EQUAL(0, tt->tokens[0].lineOffset);
    EXPECT_VAL_EQUAL('T', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL('"', *tt->tokens[0].end);
    EXPECT_VAL_EQUAL('t', *(tt->tokens[0].end - 1));
    EXPECT_VAL_EQUAL('.', *tt->buffer.pos);
}

void
test_tokenize_parseable_string_lit_single_weol(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "'This is \"test\" text\n", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL('T', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL('\n', *tt->tokens[0].end);
    EXPECT_VAL_EQUAL('t', *(tt->tokens[0].end - 1));
    EXPECT_VAL_EQUAL('\n', *tt->buffer.pos);
}

void
test_tokenize_parseable_string_lit_double_weol(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\"This is 'test' text\n", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL('T', *tt->tokens[0].start);
    EXPECT_VAL_EQUAL('\n', *tt->tokens[0].end);
    EXPECT_VAL_EQUAL('t', *(tt->tokens[0].end - 1));
    EXPECT_VAL_EQUAL('\n', *tt->buffer.pos);
}

void
test_tokenize_parseable_string_lit_escaped(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "'This is \\'test\\' text'.", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start + 1, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.end - 2, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.end - 1, tt->buffer.pos);
}

void
test_tokenize_parseable_label(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "thing1=thing2=thing3", 1, TOKEN_LABEL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_LABEL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.start + 7, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.pos, tt->tokens[0].end);
}

void
test_tokenize_parseable_number(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "123.5 ", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_NUMBER, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.start + 5, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.pos, tt->tokens[0].end);

    tt = _tokenTest(t, "-123.5 ", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_NUMBER, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.start + 6, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.pos, tt->tokens[0].end);

    tt = _tokenTest(t, "-000.000 ", 2, TOKEN_NUMBER);
    EXPECT_VAL_EQUAL(TOKEN_NUMBER, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(' ', *tt->tokens[0].end);
}

void
test_tokenize_parseable_word(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "hello_world ", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_WORD, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.end - 1, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.pos, tt->tokens[0].end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_identifier(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "$123 ", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.end - 1, tt->tokens[0].end);
    EXPECT_PTR_EQUAL(tt->buffer.pos, tt->tokens[0].end);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);

    tt = _tokenTest(t, ":a_b_c/d ", 1, TOKEN_IDENTIFIER);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL('/', *tt->tokens[0].end);
}

void
test_tokenize_parseable_symbol(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "$! ", 1, TOKEN_EOL);
    EXPECT_VAL_EQUAL(1, tt->scanned);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, tt->tokens[0].type);
    EXPECT_VAL_EQUAL(1, tt->tokens[0].lineNo);
    EXPECT_PTR_EQUAL(tt->buffer.start, tt->tokens[0].start);
    EXPECT_PTR_EQUAL(tt->buffer.start + 1, tt->tokens[0].end);
    EXPECT_VAL_EQUAL('!', *tt->buffer.pos);
    EXPECT_VAL_EQUAL(1, tt->sf.lineNo);
}

void
test_tokenize_parseable_block(struct TestContext *t)
{
    const char *text =          // this should be 6 tokens:
            "\t\t ; comment\n"  // comment (swallows eol)
            "hello="            // label
            "$world"            // identifier
            " \t "              // whitespace
            "flag"              // word
            "\n\n\r\n"          // EOB
            "Xignore me\n\n"
            ;
    struct TokenizerTest *tt = _tokenTest(t, text, MAX_TEST_TOKENS, TOKEN_EOB);
    EXPECT_VAL_EQUAL(6, tt->scanned);

    struct Token *token = tt->tokens;
    EXPECT_VAL_EQUAL(TOKEN_COMMENT, token->type);
    EXPECT_VAL_EQUAL(';', *token->start);
    EXPECT_VAL_EQUAL(1, token->lineNo);
    EXPECT_VAL_EQUAL(3, token->lineOffset);

	++token;
    EXPECT_VAL_EQUAL(TOKEN_LABEL, token->type);
    EXPECT_VAL_EQUAL('h', *token->start);
    EXPECT_VAL_EQUAL(2, token->lineNo);
    EXPECT_VAL_EQUAL(0, token->lineOffset);

	++token;
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token->type);
    EXPECT_VAL_EQUAL('$', *token->start);
    EXPECT_VAL_EQUAL(2, token->lineNo);
    EXPECT_VAL_EQUAL(6, token->lineOffset);

	++token;
    EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token->type);
    EXPECT_VAL_EQUAL(' ', *token->start);
    EXPECT_VAL_EQUAL(2, token->lineNo);
    EXPECT_VAL_EQUAL(12, token->lineOffset);

	++token;
    EXPECT_VAL_EQUAL(TOKEN_WORD, token->type);
    EXPECT_VAL_EQUAL('f', *token->start);
    EXPECT_VAL_EQUAL(2, token->lineNo);
    EXPECT_VAL_EQUAL(15, token->lineOffset);

	++token;
    EXPECT_VAL_EQUAL(TOKEN_EOB, token->type);
    EXPECT_VAL_EQUAL('\n', *token->start);
    EXPECT_VAL_EQUAL(2, token->lineNo);
    EXPECT_VAL_EQUAL(19, token->lineOffset);

	EXPECT_VAL_EQUAL(4, tt->sf.lineNo);
    EXPECT_VAL_EQUAL('X', *tt->buffer.pos);
}

void
tokenizer_tests(struct TestContext *t)
{
    RUN_TEST(test_consume_eol_cr);
    RUN_TEST(test_consume_eol_lf);
    RUN_TEST(test_consume_eol_crlf);
    RUN_TEST(test_consume_eol_lfcr);

    RUN_TEST(test_consume_eol_crcr);
    RUN_TEST(test_consume_eol_lflf);
    RUN_TEST(test_consume_eol_crlfcrlf);
    RUN_TEST(test_consume_eol_lfcrlfcr);

    RUN_TEST(test_consume_whitespace_single_space);
    RUN_TEST(test_consume_whitespace_single_tab);
    RUN_TEST(test_consume_whitespace_multi);

    RUN_TEST(test_consume_quote_dq);
    RUN_TEST(test_consume_quote_sq);
    RUN_TEST(test_consume_quote_eol);

    RUN_TEST(test_consume_comment);

    RUN_TEST(test_consume_text_word);
    RUN_TEST(test_consume_text_symbol);
    RUN_TEST(test_consume_text_identifier);
    RUN_TEST(test_consume_text_number);
    RUN_TEST(test_consume_text_label);

    t->tearUp = _tearUpTokenizerTest;
    t->tearDown = _tearDownTokenizerTest;

    /// TODO:#define EXPECT_TOKEN(lineNo, lineOffset, startChar, endChar, endPos)

    RUN_TEST(test_tokenize_parseable_failargs);
    RUN_TEST(test_tokenize_parseable_enoent);
    RUN_TEST(test_tokenize_parseable_eob);
    RUN_TEST(test_tokenize_parseable_eol_cr);
    RUN_TEST(test_tokenize_parseable_eol_lf);
    RUN_TEST(test_tokenize_parseable_eol_crlf);
    RUN_TEST(test_tokenize_parseable_eol_break);
    RUN_TEST(test_tokenize_parseable_whitespace);
    RUN_TEST(test_tokenize_parseable_comment_no_eol);
    RUN_TEST(test_tokenize_parseable_comment_eol);
    RUN_TEST(test_tokenize_parseable_string_lit_single_wclose);
    RUN_TEST(test_tokenize_parseable_string_lit_double_wclose);
    RUN_TEST(test_tokenize_parseable_string_lit_single_weol);
    RUN_TEST(test_tokenize_parseable_string_lit_double_weol);
    RUN_TEST(test_tokenize_parseable_string_lit_escaped);
    RUN_TEST(test_tokenize_parseable_label);
    RUN_TEST(test_tokenize_parseable_number);
    RUN_TEST(test_tokenize_parseable_word);
    RUN_TEST(test_tokenize_parseable_identifier);
    RUN_TEST(test_tokenize_parseable_symbol);
    RUN_TEST(test_tokenize_parseable_block);

    assert(NUM_TOKEN_TYPES == TOKEN_SYMBOL + 1);
}