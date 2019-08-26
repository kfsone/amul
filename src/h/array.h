#ifndef AMUL_ARRAY_H
#define AMUL_ARRAY_H

#include <utility>

#include "h/amul.type.h"

template<typename ElementType, size_t NumElements>
struct Array {
    using element_t = ElementType;
    static const size_t s_capacity = NumElements;

    element_t m_elements[s_capacity];
    constexpr const element_t *begin() const noexcept { return &m_elements[0]; }
    const element_t *m_end{ begin() + s_capacity };
    constexpr const element_t *end() const noexcept { return m_end; }
    element_t *m_cur{ &m_elements[0] };
    constexpr const element_t *it() const noexcept { return m_cur; }

    void Reset() noexcept { m_cur = begin(); }

    constexpr bool Empty() const noexcept { return it() == begin(); }
    constexpr size_t Size() const noexcept { return it() - begin(); }
    constexpr size_t Capacity() const noexcept { return s_capacity; }

    // Note: no capacity checking
    const element_t &Push(const element_t &element) noexcept
    {
        *m_cur = element;
        return *(m_cur++);
    }
    const element_t &Push(element_t &&element) noexcept
    {
        *m_cur = std::forward<element_t>(element);
        return *(m_cur++);
    }

    void Pop(size_t numElements = 1) noexcept
    {
        numElements = numElements > Size() ? Size() : numElements;
        m_cur -= numElements;
    }

    // You need to make sure the array isn't empty before using this.
    constexpr const element_t *Back() const noexcept { return (m_cur - 1); }
};

#endif  // AMUL_SRC_ARRAY_H
