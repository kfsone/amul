#ifndef AMUL_H_STRUCT_INDEX_H
#define AMUL_H_STRUCT_INDEX_H

#include <deque>
#include <string>

template<typename ElementType, typename IdType>
struct StructIndex
{
    using data_type = ElementType;
    using id_type = IdType;
    using self_type = StructIndex<data_type, id_type>;
    using table_type = std::deque<data_type>;

    using iterator = typename table_type::iterator;
    using const_iterator = typename table_type::iterator;

protected:
    table_type m_data;

public:
    id_type Register(data_type &&entry);
    id_type Lookup(const std::string &name) noexcept;
    data_type *Find(const std::string &name) noexcept;
    data_type *Get(id_type id) noexcept;

    size_t Size() const noexcept { return m_data.size(); }
    bool Empty() const noexcept { return m_data.empty(); }

    iterator begin() noexcept { return m_data.begin(); }
    const_iterator cbegin() const noexcept { return m_data.cbegin(); }
    iterator end() noexcept { return m_data.end(); }
    const_iterator cend() const noexcept { return m_data.cend(); }
};

#endif  // AMUL_H_STRUCT_INDEX_H
