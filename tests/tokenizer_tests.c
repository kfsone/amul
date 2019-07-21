#include <h/amul.test.h>
#include <src/buffer.h>
#include <src/tokenizer.h>

extern void _consumeEol(struct Buffer *buf, struct Token *token);
extern void _consumeWhitespace(struct Buffer *buf, struct Token *token);
extern void _consumeQuote(struct Buffer *buf, struct Token *token);
extern void _consumeComment(struct Buffer *buf, struct Token *token);
extern void _consumeText(struct Buffer *buf, struct Token *token);

//////////////////////////////////////////////////////////////////////////////////////////////////
// helper structs

enum { MAX_TEST_TOKENS = 64 };
struct TokenizerTest {
    struct SourceFile sf;
    struct Buffer     buffer;
    struct Token      tokens[MAX_TEST_TOKENS];
    size_t            scanned;
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// helper functions

static struct Buffer
init_buffer(const char *text, struct Token *token)
{
    struct Buffer buffer = {text, text + strlen(text), text};
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
    struct TokenizerTest *tt = calloc(sizeof(struct TokenizerTest), 1);
    assert(tt);
    tt->sf = (struct SourceFile){"source.txt", NULL, &tt->buffer, 0, 0};
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
    struct Buffer b = init_buffer("\nNewline", &token);
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
    struct Buffer b = init_buffer("\rNewline", &token);
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
    struct Buffer b = init_buffer("\n\rNewline", &token);
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
    struct Buffer b = init_buffer("\r\nNewline", &token);
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
    struct Buffer b = init_buffer("\n\nNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lflf(struct TestContext *t)
{
    struct Token  token;
    struct Buffer b = init_buffer("\r\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_crlfcrlf(struct TestContext *t)
{
    struct Token  token;
    struct Buffer b = init_buffer("\n\r\n\rNewline", &token);
    _consumeEol(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 2, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_eol_lfcrlfcr(struct TestContext *t)
{
    struct Token  token;
    struct Buffer b = init_buffer("\r\n\r\nNewline", &token);
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
        struct Buffer b = init_buffer(" ", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_PTR_EQUAL(b.end, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
    {
        struct Token  token;
        struct Buffer b = init_buffer(" i", &token);
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
        struct Buffer b = init_buffer("\t", &token);
        _consumeWhitespace(&b, &token);
        EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
        EXPECT_PTR_EQUAL(b.start + 1, b.pos);
        EXPECT_PTR_EQUAL(b.end, b.pos);
        EXPECT_PTR_EQUAL(b.start, token.start);
        EXPECT_PTR_EQUAL(b.pos, token.end);
    }
    {
        struct Token  token;
        struct Buffer b = init_buffer("\ti", &token);
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
        struct Buffer b = init_buffer("   \t\t\t   \t\t\t", &token);
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
        struct Buffer b = init_buffer("   \t\t\t   \t\t\t\n", &token);
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
        struct Buffer buffer = init_buffer("\"\"", &token);
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
        struct Buffer buffer = init_buffer("\"'\"!", &token);
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
        struct Buffer buffer = init_buffer("\"a' \"!", &token);
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
        struct Buffer buffer = init_buffer("''", &token);
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
        struct Buffer buffer = init_buffer("'\"'!", &token);
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
        struct Buffer buffer = init_buffer("'a\" '!", &token);
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
        struct Buffer buffer = init_buffer("\"...", &token);
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
        struct Buffer buffer = init_buffer("\"...\n", &token);
        _consumeQuote(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_VAL_EQUAL('\n', *buffer.pos);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("\"...\r", &token);
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
        struct Buffer buffer = init_buffer(";", &token);
        _consumeComment(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_COMMENT, token.type);
        EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
        EXPECT_PTR_EQUAL(buffer.start, token.start);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer(";;;;;    hello ", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer(";;;;;    hello \n\r\n", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
        EXPECT_VAL_EQUAL('\n', *token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer(";;;;;    hello \r\n\r\r\n", &token);
        _consumeComment(&buffer, &token);
        EXPECT_PTR_EQUAL(buffer.end - 3, token.end);
        EXPECT_VAL_EQUAL('\r', *token.end);
    }
}

void
test_consume_text_word(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer = init_buffer("abc", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_WORD, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);
}

void
test_consume_text_symbol(struct TestContext *t)
{
    {
        struct Token  token;
        struct Buffer buffer = init_buffer("$", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("=", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("_", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("_=", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("=_", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("= ", &token);
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
        struct Buffer buffer = init_buffer("$bc", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("$1c", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("$c_1", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("=1a2", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("!abc.1", &token);
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
        struct Buffer buffer = init_buffer("1", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end, token.end);
    }

    {
        struct Token  token;
        struct Buffer buffer = init_buffer("123-", &token);
        _consumeText(&buffer, &token);
        EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
        EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    }
}

void
test_consume_text_label(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

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

    EXPECT_ERROR(EINVAL, TokenizeParseable(NULL, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeParseable(&tt->sf, NULL, MAX_TEST_TOKENS, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeParseable(&tt->sf, &tt->tokens[0], 0, &tt->scanned));
    EXPECT_ERROR(EINVAL, TokenizeParseable(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, NULL));

    // It should also want to fail if we pass it a file without a buffer
    tt->sf.buffer = NULL;
    EXPECT_ERROR(EINVAL, TokenizeParseable(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
}

void
test_tokenize_parseable_enoent(struct TestContext *t)
{
    // passing an empty buffer should get us an ENOENT
    struct TokenizerTest *tt = (struct TokenizerTest *)(t->userData);
    EXPECT_ERROR(ENOENT, TokenizeParseable(&tt->sf, &tt->tokens[0], MAX_TEST_TOKENS, &tt->scanned));
}

struct TokenizerTest *
_tokenTest(struct TestContext *t, const char *text, size_t maxTestTokens)
{
    struct TokenizerTest *tt = (struct TokenizerTest *)(t->userData);
    tt->buffer = init_buffer(text, NULL);
    EXPECT_SUCCESS(TokenizeParseable(&tt->sf, &tt->tokens[0], maxTestTokens, &tt->scanned));
    return tt;
}

void
test_tokenize_parseable_eob(struct TestContext *t)
{
	//                                         -1- -2- 3 4 -5- -6-
    struct TokenizerTest *tt = _tokenTest(t, "\n\r\n\r\n\n\n\r\r\n", MAX_TEST_TOKENS);
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
    struct TokenizerTest *tt = _tokenTest(t, "\n", MAX_TEST_TOKENS);
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
}

void
test_tokenize_parseable_eol_lf(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\r", MAX_TEST_TOKENS);
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
}

void
test_tokenize_parseable_eol_crlf(struct TestContext *t)
{
    struct TokenizerTest *tt = _tokenTest(t, "\r\n", MAX_TEST_TOKENS);
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
}

void
test_tokenize_parseable_whitespace(struct TestContext *t)
{
}

void
test_tokenize_parseable_comment(struct TestContext *t)
{
}

void
test_tokenize_parseable_string_lit(struct TestContext *t)
{
}

void
test_tokenize_parseable_label(struct TestContext *t)
{
}

void
test_tokenize_parseable_number(struct TestContext *t)
{
}

void
test_tokenize_parseable_word(struct TestContext *t)
{
}

void
test_tokenize_parseable_identifier(struct TestContext *t)
{
}

void
test_tokenize_parseable_symbol(struct TestContext *t)
{
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

    RUN_TEST(test_tokenize_parseable_failargs);
    RUN_TEST(test_tokenize_parseable_enoent);
    RUN_TEST(test_tokenize_parseable_eob);
    RUN_TEST(test_tokenize_parseable_eol_cr);
    RUN_TEST(test_tokenize_parseable_eol_lf);
    RUN_TEST(test_tokenize_parseable_eol_crlf);
    RUN_TEST(test_tokenize_parseable_whitespace);
    RUN_TEST(test_tokenize_parseable_comment);
    RUN_TEST(test_tokenize_parseable_string_lit);
    RUN_TEST(test_tokenize_parseable_label);
    RUN_TEST(test_tokenize_parseable_number);
    RUN_TEST(test_tokenize_parseable_word);
    RUN_TEST(test_tokenize_parseable_identifier);
    RUN_TEST(test_tokenize_parseable_symbol);

    assert(NUM_TOKEN_TYPES == TOKEN_SYMBOL + 1);
}
