#include "buffer.h"
#include <gtest/gtest.h>
#include "gtest_aliases.h"

class TestBuffer : public Buffer
{
  public:
    using Buffer::Buffer;
    void IncEnd(uint32_t i = 1) noexcept { m_end += i; }
    void IncPos(uint32_t i = 1) noexcept { m_pos += i; }

    void ResetPos() noexcept { m_pos = m_start; }
};

TEST(BufferTest, NewBuffer)
{
    const Buffer nul_buf{};
    EXPECT_EQ(nul_buf.Start(), nullptr);
    EXPECT_EQ(nul_buf.Pos(), nullptr);
    EXPECT_EQ(nul_buf.End(), nullptr);

    const char   *data = "hello";
    const Buffer ptr_buf{data, data+5};
    EXPECT_EQ(data, ptr_buf.Start());
    EXPECT_EQ(data, ptr_buf.Pos());
    EXPECT_EQ(data + 5, ptr_buf.End());
}

TEST(BufferTest, BufferEof)
{
	const char *data = "a";
    Buffer buffer{data, data+1};
    EXPECT_FALSE(buffer.Eof());
	buffer.Skip();
	EXPECT_TRUE(buffer.Eof());
}

TEST(BufferTest, BufferAssign)
{
	Buffer buffer{};
	const char *text = "hello";
	buffer.Assign(text, 0);
	EXPECT_EQ(text, buffer.Start());
    EXPECT_EQ(text, buffer.Pos());
    EXPECT_EQ(text, buffer.End());

	buffer.Assign(text+1, 4);
    EXPECT_EQ(text+1, buffer.Start());
    EXPECT_EQ(text+1, buffer.Pos());
    EXPECT_EQ(text+5, buffer.End());
}

TEST(BufferTest, BufferPeek)
{
    const char *data{"abc"};
    TestBuffer  buffer{data, data};
    EXPECT_EQ(0, buffer.Peek());
    EXPECT_EQ(&data[0], buffer.Pos());
    buffer.IncEnd();
    EXPECT_EQ('a', buffer.Peek());
    EXPECT_EQ(&data[0], buffer.Pos());
    buffer.IncPos();
    EXPECT_EQ(0, buffer.Peek());
    EXPECT_EQ(&data[1], buffer.Pos());

    // Increment end twice to check things work when end > pos + 1
    buffer.IncEnd(2);
    EXPECT_EQ('b', buffer.Peek());
    EXPECT_EQ(&data[1], buffer.Pos());
    buffer.IncPos();
    EXPECT_EQ('c', buffer.Peek());
    EXPECT_EQ(&data[2], buffer.Pos());
}

TEST(BufferTest, BufferNext)
{
    char   data[] = {'a', 'z', '\0'};
    Buffer buffer = Buffer{data, data};

    // Test with zero-width range
    EXPECT_EQ(0, buffer.Next());
    EXPECT_EQ(buffer.Start(), buffer.Pos());

    buffer = Buffer{data, data + 1};
    EXPECT_EQ('a', buffer.Next());
    EXPECT_EQ(0, buffer.Next());
    EXPECT_EQ(buffer.Start() + 1, buffer.Pos());

    buffer = Buffer{data, data + 3};
    EXPECT_EQ('a', buffer.Next());
    EXPECT_EQ('z', buffer.Next());
    EXPECT_EQ(buffer.Start() + 2, buffer.Pos());
    EXPECT_FALSE(buffer.Eof());
    EXPECT_EQ('\0', buffer.Next());
    EXPECT_EQ(buffer.Start() + 3, buffer.Pos());
    EXPECT_EQ('\0', buffer.Next());
    EXPECT_EQ(buffer.Start() + 3, buffer.Pos());
    EXPECT_TRUE(buffer.Eof());
}

TEST(BufferTest, BufferSkip)
{
    char   data[] = {"abXc"};
    Buffer buffer = Buffer{data, data};

    // Confirm that it honors eof
    buffer.Skip();
    EXPECT_EQ(buffer.Start(), buffer.Pos());

    buffer = Buffer{data, data+4};
    buffer.Skip();
    EXPECT_EQ('b', buffer.Peek());
    buffer.Skip();
    buffer.Skip();
    EXPECT_EQ('c', buffer.Next());
}

TEST(BufferTest, BufferSize)
{
    char   data[16];
    Buffer buffer{data, data};
    EXPECT_EQ(0, buffer.Size());

    buffer = Buffer{data, data + 15};
    EXPECT_EQ(15, buffer.Size());
}

TEST(BufferTest, BufferClose)
{
	const char* text = "hello";
	Buffer buffer { text, text + 5 };
	buffer.Close();
    EXPECT_EQ(buffer.Start(), nullptr);
    EXPECT_EQ(buffer.Pos(), nullptr);
	EXPECT_EQ(buffer.End(), nullptr);
}
