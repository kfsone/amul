#include <h/amul.test.h>
#include <src/buffer.h>

class TestBuffer : public Buffer
{
  public:
    using Buffer::Buffer;
    void IncEnd(uint32_t i = 1) noexcept { m_end += i; }
    void IncPos(uint32_t i = 1) noexcept { m_pos += i; }

    void ResetPos() noexcept { m_pos = m_start; }
};

void
test_new_buffer(struct TestContext *t)
{
    const Buffer nul_buf{};
    EXPECT_NULL(nul_buf.Start());
    EXPECT_NULL(nul_buf.Pos());
    EXPECT_NULL(nul_buf.End());

    const char   *data = "hello";
    const Buffer ptr_buf{data, strchr(data, 0)};
    EXPECT_PTR_EQUAL(data, ptr_buf.Start());
    EXPECT_PTR_EQUAL(data, ptr_buf.Pos());
    EXPECT_PTR_EQUAL(data + 1, ptr_buf.End());
}

void
test_buffer_eof(struct TestContext *t)
{
    char       data[8] = {'a'};
    Buffer buffer{data, data};
    EXPECT_FALSE(buffer.Eof());
	buffer.Skip();
	EXPECT_TRUE(buffer.Eof());
}

void
test_buffer_assign(struct TestContext *t)
{
	Buffer buffer{};
	const char *text = "hello";
	buffer.Assign(text, 0);
	EXPECT_PTR_EQUAL(text, buffer.Start());
    EXPECT_PTR_EQUAL(text, buffer.Pos());
    EXPECT_PTR_EQUAL(text, buffer.End());

	buffer.Assign(text+1, 4);
    EXPECT_PTR_EQUAL(text+1, buffer.Start());
    EXPECT_PTR_EQUAL(text+1, buffer.Pos());
    EXPECT_PTR_EQUAL(text+5, buffer.End());
}

void
test_buffer_peek(struct TestContext *t)
{
    const char *data{"abc"};
    TestBuffer  buffer{data, data + 3};
    EXPECT_VAL_EQUAL(0, buffer.Peek());
    EXPECT_PTR_EQUAL(&data[0], buffer.Pos());
    buffer.IncEnd();
    EXPECT_VAL_EQUAL('a', buffer.Peek());
    EXPECT_PTR_EQUAL(&data[0], buffer.Pos());
    buffer.IncPos();
    EXPECT_VAL_EQUAL(0, buffer.Peek());
    EXPECT_PTR_EQUAL(&data[1], buffer.Pos());

    // Increment end twice to check things work when end > pos + 1
    buffer.IncEnd(2);
    EXPECT_VAL_EQUAL('b', buffer.Peek());
    EXPECT_PTR_EQUAL(&data[1], buffer.Pos());

    buffer.IncPos();
    EXPECT_VAL_EQUAL('c', buffer.Peek());
    EXPECT_PTR_EQUAL(&data[2], buffer.Pos());
}

void
test_buffer_next(struct TestContext *t)
{
    char   data[] = {'a', 'z', '\0'};
    Buffer buffer = Buffer{data, data};

    // Test with zero-width range
    EXPECT_VAL_EQUAL(0, buffer.Next());
    EXPECT_PTR_EQUAL(buffer.Start(), buffer.Pos());

    buffer = Buffer{data, data + 1};
    EXPECT_VAL_EQUAL('a', buffer.Next());
    EXPECT_VAL_EQUAL(0, buffer.Next());
    EXPECT_PTR_EQUAL(buffer.Start() + 1, buffer.Pos());

    buffer = Buffer{data, data + 3};
    EXPECT_VAL_EQUAL('a', buffer.Next());
    EXPECT_VAL_EQUAL('z', buffer.Next());
    EXPECT_PTR_EQUAL(buffer.Start() + 2, buffer.Pos());
    EXPECT_FALSE(buffer.Eof());
    EXPECT_VAL_EQUAL('\0', buffer.Next());
    EXPECT_PTR_EQUAL(buffer.Start() + 3, buffer.Pos());
    EXPECT_VAL_EQUAL('\0', buffer.Next());
    EXPECT_PTR_EQUAL(buffer.Start() + 3, buffer.Pos());
    EXPECT_TRUE(buffer.Eof());
}

void
test_buffer_skip(struct TestContext *t)
{
    char   data[] = {"abXc"};
    Buffer buffer = Buffer{data, data};

    // Confirm that it honors eof
    buffer.Skip();
    EXPECT_PTR_EQUAL(buffer.Start(), buffer.Pos());

    buffer = Buffer{data, data};
    buffer.Skip();
    EXPECT_VAL_EQUAL('b', buffer.Peek());
    buffer.Skip();
    buffer.Skip();
    EXPECT_VAL_EQUAL('c', buffer.Next());
}

void
test_buffer_size(struct TestContext *t)
{
    char   data[16];
    Buffer buffer{data, data};
    EXPECT_VAL_EQUAL(0, buffer.Size());

    buffer = Buffer{data, data + 15};
    EXPECT_VAL_EQUAL(15, buffer.Size());
}

void
test_buffer_close(struct TestContext *t)
{
	const char* text = "hello";
	Buffer buffer { text, text + 5 };
	buffer.Close();
    EXPECT_NULL(buffer.Start());
    EXPECT_NULL(buffer.Pos());
	EXPECT_NULL(buffer.End());
}

void
buffer_tests(struct TestContext *t)
{
    RUN_TEST(test_new_buffer);
    RUN_TEST(test_buffer_eof);
    RUN_TEST(test_buffer_assign);
    RUN_TEST(test_buffer_peek);
    RUN_TEST(test_buffer_next);
    RUN_TEST(test_buffer_skip);
    RUN_TEST(test_buffer_size);
	RUN_TEST(test_buffer_close);
}
