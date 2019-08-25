#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "h/buffer.h"

class TestBuffer : public Buffer
{
  public:
    using Buffer::Buffer;
    void IncEnd(uint32_t i = 1) noexcept { m_end += i; }
    void IncPos(uint32_t i = 1) noexcept { m_cur += i; }

    void ResetPos() noexcept { m_cur = m_start; }
};

TEST(BufferTest, NewBuffer)
{
    const Buffer nul_buf{};
    EXPECT_EQ(nul_buf.begin(), nullptr);
    EXPECT_EQ(nul_buf.it(), nullptr);
    EXPECT_EQ(nul_buf.end(), nullptr);

    const char * data{"hello"};
    const Buffer ptr_buf{data, data + 5};
    EXPECT_EQ(data, ptr_buf.begin());
    EXPECT_EQ(data, ptr_buf.it());
    EXPECT_EQ(data + 5, ptr_buf.end());
}

TEST(BufferTest, BufferEof)
{
    const char *data{"a"};
    Buffer      buffer{data, data + 1};
    EXPECT_FALSE(buffer.Eof());
    buffer.Skip();
    EXPECT_TRUE(buffer.Eof());
}

TEST(BufferTest, BufferAssign)
{
    Buffer      buffer{};
    const char *text{"hello"};
    buffer.Assign(text, 0);
    EXPECT_EQ(text, buffer.begin());
    EXPECT_EQ(text, buffer.it());
    EXPECT_EQ(text, buffer.end());

    buffer.Assign(text + 1, 4);
    EXPECT_EQ(text + 1, buffer.begin());
    EXPECT_EQ(text + 1, buffer.it());
    EXPECT_EQ(text + 5, buffer.end());
}

TEST(BufferTest, BufferPeek)
{
    Buffer buffer{};
    EXPECT_EQ(buffer.Peek(), '\0');
    buffer.Assign("g", 1);
    EXPECT_EQ(buffer.Peek(), 'g');
}

TEST(BufferTest, BufferRead)
{
    char   data[] = {'a', 'z', '\0'};
    Buffer buffer = Buffer{data, data};

    // Test with zero-width range
    EXPECT_EQ(0, buffer.Read());
    EXPECT_EQ(buffer.begin(), buffer.it());

    buffer = Buffer{data, data + 1};
    EXPECT_EQ('a', buffer.Read());
    EXPECT_EQ(0, buffer.Read());
    EXPECT_EQ(buffer.begin() + 1, buffer.it());

    buffer = Buffer{data, data + 3};
    EXPECT_EQ('a', buffer.Read());
    EXPECT_EQ('z', buffer.Read());
    EXPECT_EQ(buffer.begin() + 2, buffer.it());
    EXPECT_FALSE(buffer.Eof());
    EXPECT_EQ('\0', buffer.Read());
    EXPECT_EQ(buffer.begin() + 3, buffer.it());
    EXPECT_EQ('\0', buffer.Read());
    EXPECT_EQ(buffer.begin() + 3, buffer.it());
    EXPECT_TRUE(buffer.Eof());
}

TEST(BufferTest, BufferSkip)
{
    const char *data{"abXc"};
    Buffer      buffer = Buffer{data, data};

    // Confirm that it honors eof
    buffer.Skip();
    EXPECT_EQ(buffer.begin(), buffer.it());

    buffer = Buffer{data, data + 4};
    buffer.Skip();
    EXPECT_EQ('b', buffer.Peek());
    buffer.Skip();
    buffer.Skip();
    EXPECT_EQ('c', buffer.Read());
}

TEST(BufferTest, BufferSize)
{
    const char data[16]{};  // empty buffer
    Buffer     buffer{data, data};
    EXPECT_EQ(0, buffer.Size());

    static_assert(sizeof(data) > 15);
    buffer = Buffer{data, data + 15};
    EXPECT_EQ(15, buffer.Size());
}

TEST(BufferTest, BufferClose)
{
    const char *text = "hello";
    Buffer      buffer{text, text + 5};
    buffer.Close();
    EXPECT_EQ(buffer.begin(), nullptr);
    EXPECT_EQ(buffer.it(), nullptr);
    EXPECT_EQ(buffer.end(), nullptr);
}
