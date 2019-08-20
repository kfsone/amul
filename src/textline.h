#pragma once

#if defined(TEXTLINE_CODE)
#include <string>
#include <string_view>

extern thread_local bool t_needCR;

struct TextLine {
    static thread_local std::string tl_line;
    using print_t = void (*)(string_view) noexcept;
    print_t m_printFn;

    TextLine(print_t printFn) : m_printFn(printFn)
    {
        tl_line = t_needCR ? "\n" : "";
        t_needCR = false;
    }
    ~TextLine()
    {
        m_printFn(tl_line);
        t_needCR = (tl_line.back() != '\n');
    }
};

template<typename T>
TextLine &operator<<(TextLine &tl, const T &value) noexcept;

template<>
TextLine &operator<<<char>(TextLine &tl, const char &value) noexcept
{
    tl.tl_line += value;
    return tl;
}

template<>
TextLine &operator<<<string_view>(TextLine &tl, const string_view &value) noexcept
{
    tl.tl_line += value;
    return tl;
}

template<>
TextLine &operator<<<std::string>(TextLine &tl, const std::string &value) noexcept
{
    tl.tl_line += value;
    return tl;
}

template<typename T>
TextLine &
operator<<(TextLine &tl, const T &value) noexcept
{
    tl.tl_line += std::to_string(value);
    return tl;
}
#endif
