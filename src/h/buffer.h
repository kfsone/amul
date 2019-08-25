#ifndef AMUL_BUFFER_H
#define AMUL_BUFFER_H
// Buffer is for consuming bytes from a fixed range in memory.

#include <cstring>

#include "h/amul.type.h"

struct Buffer {
    const char *m_start{nullptr};
    const char *m_cur{nullptr};
    const char *m_end{nullptr};

    constexpr Buffer() noexcept {}
    constexpr Buffer(const char *start, const char *end) noexcept
        : m_start{start}
        , m_cur{start}
        , m_end{end}
    {
    }
    constexpr Buffer(const char *start, size_t len) noexcept
        : Buffer{start, start + len}
    {
    }
    template <size_t Size>
    constexpr Buffer(const char (&str)[Size]) noexcept
        : Buffer{&str[0], &str[0] + Size}
    {
    }

    void Assign(const char *start, size_t length) noexcept
    {
        m_start = start;
        m_cur = start;
        m_end = m_start + length;
    }

    constexpr const char *begin() const noexcept { return m_start; }
    constexpr const char *end() const noexcept { return m_end; }
    constexpr const char *it() const noexcept { return m_cur; }

    constexpr bool   Eof() const noexcept { return it() >= end(); }
    constexpr size_t Size() const noexcept { return end() - begin(); }

    uint8_t Peek() const noexcept { return !Eof() ? *m_cur : 0; }
    uint8_t Read() noexcept { return (!Eof()) ? *(m_cur++) : 0; }
    void    Skip() noexcept { m_cur += !Eof() ? 1 : 0; }

    const char *ReadLine() noexcept {
        if (Eof())
            return nullptr;
        const char* eol = strpbrk(m_cur, "\r\n");
        if (eol) {
            m_cur = eol;
            if (*m_cur == '\r')
                ++m_cur;
            if (*m_cur == '\n')
                ++m_cur;
        } else
            eol = m_end;
        return eol;
    }

    void Close() noexcept { Assign(nullptr, 0); }
};

#endif  // AMUL_BUFFER_H
