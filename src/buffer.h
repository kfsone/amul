#ifndef AMUL_SRC_BUFFER_H
#define AMUL_SRC_BUFFER_H

// Buffer is for consuming bytes from a fixed range in memory.

#include <h/amul.type.h>

class Buffer
{
  protected:
    const char *m_pos{nullptr};
    const char *m_end{nullptr};
    const char *m_start{nullptr};

  public:
    constexpr Buffer() noexcept {}
    constexpr Buffer(const char *start, const char *end) noexcept
        : m_pos{start}
        , m_end{end}
        , m_start{start}
    {
    }

    constexpr auto   Start() const noexcept { return m_start; }
    constexpr auto   End() const noexcept { return m_end; }
    constexpr auto   Pos() const noexcept { return m_pos; }
    constexpr size_t Size() const noexcept { return m_end - m_start; }
    constexpr bool   Eof() const noexcept { return m_pos >= m_end; }

	void Assign(const char* start, size_t length) noexcept {
		m_start = m_pos = start;
		m_end = start + length;
	}
    char Peek() const noexcept { return !Eof() ? *m_pos : 0; }
    char Read() noexcept { return !Eof() ? *(m_pos++) : 0; }
    void Skip() noexcept { if (!Eof()) ++m_pos; }
    char Next() noexcept { 
        if (!Eof() && ++m_pos < m_end)
            return *m_pos;
        return 0;
    }

	void Close() noexcept
	{
		m_pos = m_end = m_start = nullptr;
	}
};

#endif  // AMUL_SRC_BUFFER_H