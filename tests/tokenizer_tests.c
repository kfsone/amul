#include <h/amul.test.h>
#include <src/buffer.h>
#include <src/tokenizer.h>

extern void _consumeEol(struct Buffer *buf, struct Token *token);
extern void _consumeWhitespace(struct Buffer *buf, struct Token *token);
extern void _consumeQuote(struct Buffer *buf, struct Token *token);
extern void _consumeComment(struct Buffer *buf, struct Token *token);
extern void _consumeText(struct Buffer *buf, struct Token *token);

struct Buffer
init_buffer(const char *text, struct Token *token)
{
    struct Buffer buffer = {text, text + strlen(text), text};
    token->type = (enum TokenType)0;
    token->start = text;
    token->end = NULL;
    return buffer;
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
    struct Token  token;
    struct Buffer b = init_buffer(" ", &token);
    _consumeWhitespace(&b, &token);
    EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.end, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);

    b = init_buffer(" i", &token);
    _consumeWhitespace(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_VAL_EQUAL('i', *b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_whitespace_single_tab(struct TestContext *t)
{
    struct Token  token;
    struct Buffer b = init_buffer("\t", &token);
    _consumeWhitespace(&b, &token);
    EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_PTR_EQUAL(b.end, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);

    b = init_buffer("\ti", &token);
    _consumeWhitespace(&b, &token);
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);
    EXPECT_VAL_EQUAL('i', *b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
}

void
test_consume_whitespace_multi(struct TestContext *t)
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

    b = init_buffer("   \t\t\t   \t\t\t\n", &token);
    _consumeWhitespace(&b, &token);
    EXPECT_VAL_EQUAL(TOKEN_WHITESPACE, token.type);
    EXPECT_PTR_EQUAL(b.start + 12, b.pos);
    EXPECT_PTR_EQUAL(b.end - 1, b.pos);
    EXPECT_PTR_EQUAL(b.start, token.start);
    EXPECT_PTR_EQUAL(b.pos, token.end);
    EXPECT_VAL_EQUAL('\n', *token.end);
}

void
test_consume_quote_dq(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("\"\"", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, token.type);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_VAL_EQUAL('"', *token.start);

    buffer = init_buffer("\"'\"!", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_VAL_EQUAL('"', *token.end);
    EXPECT_VAL_EQUAL('!', *buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
    EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
    EXPECT_VAL_EQUAL('\'', *token.start);

    buffer = init_buffer("\"a' \"!", &token);
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

void
test_consume_quote_sq(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("''", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_STRING_LITERAL, token.type);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_VAL_EQUAL('\'', *token.start);

    buffer = init_buffer("'\"'!", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_VAL_EQUAL('\'', *token.end);
    EXPECT_VAL_EQUAL('!', *buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
    EXPECT_PTR_EQUAL(buffer.end - 1, buffer.pos);
    EXPECT_VAL_EQUAL('"', *token.start);

    buffer = init_buffer("'a\" '!", &token);
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

void
test_consume_quote_eol(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("\"...", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end, token.end);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_VAL_EQUAL(0, *token.end);
    EXPECT_VAL_EQUAL(0, *buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start + 1, token.start);
    EXPECT_PTR_EQUAL(buffer.end, token.end);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);

    buffer = init_buffer("\"...\n", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    EXPECT_VAL_EQUAL('\n', *buffer.pos);

    buffer = init_buffer("\"...\r", &token);
    _consumeQuote(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
    EXPECT_VAL_EQUAL('\r', *buffer.pos);
}

void
test_consume_comment(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer(";", &token);
    _consumeComment(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_COMMENT, token.type);
    EXPECT_PTR_EQUAL(buffer.end, buffer.pos);
    EXPECT_PTR_EQUAL(buffer.start, token.start);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer(";;;;;    hello ", &token);
    _consumeComment(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer(";;;;;    hello \n\r", &token);
    _consumeComment(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
    EXPECT_VAL_EQUAL('\n', *token.end);

    buffer = init_buffer(";;;;;    hello \r\n", &token);
    _consumeComment(&buffer, &token);
    EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
    EXPECT_VAL_EQUAL('\r', *token.end);
}

void
test_consume_text_word(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("abc", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_WORD, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);
}

void
test_consume_text_symbol(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("$", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("=", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("_", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("_=", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);

    buffer = init_buffer("=_", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);

    buffer = init_buffer("= ", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_SYMBOL, token.type);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
}

void
test_consume_text_identifier(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("$bc", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("$1c", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("$c_1", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("=1a2", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("!abc.1", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_IDENTIFIER, token.type);
    EXPECT_PTR_EQUAL(buffer.end - 2, token.end);
}

void
test_consume_text_number(struct TestContext *t)
{
    struct Token  token;
    struct Buffer buffer;

    buffer = init_buffer("1", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
    EXPECT_PTR_EQUAL(buffer.end, token.end);

    buffer = init_buffer("123-", &token);
    _consumeText(&buffer, &token);
    EXPECT_VAL_EQUAL(TOKEN_NUMBER, token.type);
    EXPECT_PTR_EQUAL(buffer.end - 1, token.end);
}

void
test_scan_parseable(struct TestContext *t)
{
	///TODO: Implement
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

    RUN_TEST(test_scan_parseable);
}
