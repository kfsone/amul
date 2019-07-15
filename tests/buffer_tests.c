#include <h/amul.test.h>
#include <src/buffer.h>

extern struct Buffer s_buffer[];
extern bool          s_bufferInUse[];

void
test_new_buffer(struct TestContext *t)
{
    struct Buffer *buffer = NULL;
    char data[64];
    EXPECT_ERROR(EINVAL, NewBuffer(NULL, 0, NULL));
    EXPECT_ERROR(EINVAL, NewBuffer(data, 0, NULL));
    EXPECT_ERROR(EINVAL, NewBuffer(data, sizeof(data), NULL));
    buffer = (struct Buffer*)data;
    EXPECT_ERROR(EINVAL, NewBuffer(data, sizeof(data), &buffer));

    buffer = NULL;
    EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));

    EXPECT_NOT_NULL(buffer);
    EXPECT_PTR_EQUAL(&data[0], buffer->start);
    EXPECT_PTR_EQUAL(&data[0] + sizeof(data), buffer->end);
    EXPECT_PTR_EQUAL(buffer->start, buffer->pos);

    EXPECT_PTR_EQUAL(buffer, &s_buffer[0]);
    EXPECT_TRUE(s_bufferInUse[0]);

    // Check that for 15 more allocations we continue to get the real buffers
    for (size_t i = 1; i < 16; ++i) {
        buffer = NULL;
        EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));
        EXPECT_PTR_EQUAL(buffer, &s_buffer[i]);
        EXPECT_TRUE(s_bufferInUse[i]);
    }

    // The next should be an allocation
    buffer = NULL;
    EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));
    EXPECT_NOT_NULL(buffer);
    EXPECT_PTR_EQUAL(&data[0], buffer->start);
    EXPECT_PTR_EQUAL(&data[0] + sizeof(data), buffer->end);
    EXPECT_PTR_EQUAL(buffer->start, buffer->pos);

    // Lets check it's not s_buffer[n]
    EXPECT_FALSE(buffer == &s_buffer[16]);

    // Store the allocated buffer for a release test
    t->userData = buffer;
}

void
test_close_buffer(struct TestContext *t)
{
    struct Buffer *buffer = NULL;

    for (size_t i = 0; i < 16; ++i) {
        buffer = &s_buffer[i];
        CloseBuffer(&buffer);
        EXPECT_FALSE(s_bufferInUse[i]);
        EXPECT_NULL(buffer);
    }

    buffer = (struct Buffer *)(t->userData);
    CloseBuffer(&buffer);
    EXPECT_NULL(buffer);

    t->userData = NULL;
}

void
test_buffer_eof(struct TestContext *t)
{
    char          data[8] = {0};
    struct Buffer b = {&data[0], &data[0], NULL};  // start shouldn't matter
    EXPECT_TRUE(BufferEOF(&b));

    // make the available space larger
    ++b.end;
    EXPECT_FALSE(BufferEOF(&b));
    ++b.end;
    EXPECT_FALSE(BufferEOF(&b));
    ++b.pos;
    EXPECT_FALSE(BufferEOF(&b));
    ++b.pos;
    EXPECT_TRUE(BufferEOF(&b));
    ++b.pos;
    EXPECT_TRUE(BufferEOF(&b));
}

void
test_buffer_peek(struct TestContext *t)
{
    char          data[8] = {"abc"};
    struct Buffer b = {&data[0], &data[0], NULL};
    EXPECT_VAL_EQUAL(0, BufferPeek(&b));
    EXPECT_PTR_EQUAL(&data[0], b.pos);
    ++b.end;
    EXPECT_VAL_EQUAL('a', BufferPeek(&b));
    EXPECT_PTR_EQUAL(&data[0], b.pos);
    ++b.pos;
    EXPECT_VAL_EQUAL(0, BufferPeek(&b));
    EXPECT_PTR_EQUAL(&data[1], b.pos);

	// Increment end twice to check things work when end > pos + 1
    b.end += 2;
    EXPECT_VAL_EQUAL('b', BufferPeek(&b));
    EXPECT_PTR_EQUAL(&data[1], b.pos);
    ++b.pos;
    EXPECT_VAL_EQUAL('c', BufferPeek(&b));
    EXPECT_PTR_EQUAL(&data[2], b.pos);
}

void
test_buffer_peek_nonprint(struct TestContext *t)
{
    char          data[] = {'\x00', '\x01', '\x02', 'a' };
    struct Buffer b = {&data[0], &data[0], &data[0]};
    EXPECT_VAL_EQUAL(0, BufferPeek(&b));
    EXPECT_PTR_EQUAL(b.start, b.pos);
    b.end++;
    EXPECT_VAL_EQUAL(0, BufferPeek(&b));
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);

	// Increment end by 4 so that the 'a' is inside its range
    b.pos = b.start;
    b.end += 4;
    EXPECT_VAL_EQUAL('a', BufferPeek(&b));
    EXPECT_PTR_EQUAL(b.start+3, b.pos);
}

void
test_buffer_next(struct TestContext *t)
{
    char          data[] = {'\x00', '\x01', '\x02', 'a', ' ', 'b', '\0', '\0', 'c', '\0' };
    struct Buffer b = {&data[0], &data[0], &data[0]};

	// Test with zero-width range
    EXPECT_VAL_EQUAL(0, BufferNext(&b));
    EXPECT_PTR_EQUAL(b.start, b.pos);

	// Move the \0 into range
    b.end++;
    EXPECT_VAL_EQUAL(0, BufferNext(&b));
    EXPECT_PTR_EQUAL(b.start + 1, b.pos);

	// Move 0, 1 and 2 into range, terminating at 'a'.
    b.pos = b.start;
    b.end += 2;
    EXPECT_VAL_EQUAL(0, BufferNext(&b));
    EXPECT_PTR_EQUAL(b.start + 3, b.pos);

	// Make the 'a' visible
    b.pos = b.start;
    b.end++;
    EXPECT_VAL_EQUAL('a', BufferNext(&b));
    EXPECT_VAL_EQUAL(0, BufferNext(&b));

	// Make the rest of the string visible inc terminating 0
    b.end += 6;
    EXPECT_VAL_EQUAL(' ', BufferNext(&b));
    EXPECT_VAL_EQUAL('b', BufferNext(&b));
    EXPECT_VAL_EQUAL('c', BufferNext(&b));

	EXPECT_VAL_EQUAL(0, BufferNext(&b));
    EXPECT_TRUE(BufferEOF(&b));
}

void
test_buffer_skip(struct TestContext *t)
{
    char data[] = {"abXcd !"};
    struct Buffer b = {&data[0], &data[0], &data[0]};
    BufferSkip(&b);
    EXPECT_PTR_EQUAL(b.start, b.pos);
    b.end += 7;
    BufferSkip(&b);
    EXPECT_VAL_EQUAL('b', BufferPeek(&b));
    BufferSkip(&b);
    BufferSkip(&b);
    EXPECT_VAL_EQUAL('c', BufferNext(&b));
    BufferSkip(&b);
    EXPECT_VAL_EQUAL(' ', BufferPeek(&b));
    BufferSkip(&b);
    EXPECT_VAL_EQUAL('!', BufferPeek(&b));
    EXPECT_FALSE(BufferEOF(&b));
    BufferSkip(&b);
    EXPECT_TRUE(BufferEOF(&b));
}

void
buffer_tests(struct TestContext *t)
{
    RUN_TEST(test_new_buffer);
    RUN_TEST(test_close_buffer);
    RUN_TEST(test_buffer_eof);
    RUN_TEST(test_buffer_peek);
    RUN_TEST(test_buffer_peek_nonprint);
    RUN_TEST(test_buffer_next);
    RUN_TEST(test_buffer_skip);
}
