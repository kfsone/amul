#include <h/amul.test.h>
#include <src/buffer.h>
#include "testing.h"

extern Buffer s_buffer[];
extern bool   s_bufferInUse[];

TestCase test_new_buffer{"buffer", "test_new_buffer", [](TestContext &t) {
    Buffer *buffer = nullptr;
    char data[64];
    EXPECT_ERROR(EINVAL, NewBuffer(nullptr, 0, nullptr));
    EXPECT_ERROR(EINVAL, NewBuffer(data, 0, nullptr));
    EXPECT_ERROR(EINVAL, NewBuffer(data, sizeof(data), nullptr));
    buffer = (struct Buffer*)data;
    EXPECT_ERROR(EINVAL, NewBuffer(data, sizeof(data), &buffer));

    buffer = nullptr;
    EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));

    EXPECT_NOT_NULL(buffer);
    EXPECT_PTR_EQUAL(&data[0], buffer->start);
    EXPECT_PTR_EQUAL(&data[0] + sizeof(data), buffer->end);
    EXPECT_PTR_EQUAL(buffer->start, buffer->pos);

    EXPECT_PTR_EQUAL(buffer, &s_buffer[0]);
    EXPECT_TRUE(s_bufferInUse[0]);

    // Check that for 15 more allocations we continue to get the real buffers
    for (size_t i = 1; i < 16; ++i) {
        buffer = nullptr;
        EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));
        EXPECT_PTR_EQUAL(buffer, &s_buffer[i]);
        EXPECT_TRUE(s_bufferInUse[i]);
    }

    // The next should be an allocation
    buffer = nullptr;
    EXPECT_SUCCESS(NewBuffer(data, sizeof(data), &buffer));
    EXPECT_NOT_NULL(buffer);
    EXPECT_PTR_EQUAL(&data[0], buffer->start);
    EXPECT_PTR_EQUAL(&data[0] + sizeof(data), buffer->end);
    EXPECT_PTR_EQUAL(buffer->start, buffer->pos);

    // Lets check it's not s_buffer[n]
    EXPECT_FALSE(buffer == &s_buffer[16]);

    // Store the allocated buffer for a release test
    t.userData = buffer;
}};

TestCase test_close_buffer{"buffer", "test_close_buffer", [](TestContext &t) {
    struct Buffer *buffer = nullptr;

    for (size_t i = 0; i < 16; ++i) {
        buffer = &s_buffer[i];
        CloseBuffer(&buffer);
        EXPECT_FALSE(s_bufferInUse[i]);
        EXPECT_NULL(buffer);
    }

    buffer = (struct Buffer *)(t.userData);
    CloseBuffer(&buffer);
    EXPECT_NULL(buffer);

	// Lastly, call CloseBuffer with nullptr.
	CloseBuffer(&buffer);
	CloseBuffer(nullptr);


    t.userData = nullptr;
}};

void
test_buffer_eof(TestContext &t)
{
    char          data[8] = {0};
    struct Buffer b;
    b.start = b.end = b.pos = &data[0];
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
test_buffer_peek(TestContext &t)
{
    char          data[8] = {"abc"};
    struct Buffer b = {&data[0], &data[0], nullptr};
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
test_buffer_peek_nonprint(TestContext &t)
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
test_buffer_next(TestContext &t)
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
test_buffer_skip(TestContext &t)
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
test_buffer_size(TestContext &t)
{
    char data[16];
    struct Buffer buf = {&data[0], &data[0], &data[0]};
    EXPECT_VAL_EQUAL(0, BufferSize(&buf));
    buf.end += 15;
    EXPECT_VAL_EQUAL(15, BufferSize(&buf));
}

void
buffer_tests(TestContext &t)
{
    RUN_TEST(test_new_buffer);
    RUN_TEST(test_close_buffer);
    RUN_TEST(test_buffer_eof);
    RUN_TEST(test_buffer_peek);
    RUN_TEST(test_buffer_peek_nonprint);
    RUN_TEST(test_buffer_next);
    RUN_TEST(test_buffer_skip);
    RUN_TEST(test_buffer_size);
}
