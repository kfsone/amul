#ifndef AMUL_LIB_FILEMAPPING_H
#define AMUL_LIB_FILEMAPPING_H
#pragma once

#include <h/amul.type.h>
#include <string>
#include <string_view>

// Creates a memory mapping of the named (complete) filepath.
// *datap will be populated with the address of the mapping in memory and,
// if 'sizep' is not null, *sizep will be the size of the file.
//
// Returns 0 unless there is an error, in which case it returns the error code.
error_t NewFileMapping(std::string_view filepath, void **datap, size_t *sizep) noexcept;

// Unmaps a file mapping, and clears the pointer to it.
void CloseFileMapping(void **datap, size_t length) noexcept;

///////////////////////////////////////////////////////////////////////////////
// FileMappingImpl provides the business logic of the FileMapping class so
// that FileMapping can be a light-weight template wrapper.
//
// Responsibilities are delegated so that FileMappingImpl deals with the
// abstract data pointer (void*) and bytes the mapping covers (capacity).
// FileMapping itself provides the templated members such as iterators and
// size (which is in units of the type).
//
class FileMappingImpl
{
    std::string m_filepath;   // name of the file or empty for an anonymous mapping
    void *m_void{ nullptr };  // internal pointer to the start of the mapping
    size_t m_size{ 0 };       // number of elements of
    size_t m_capacity{ 0 };   // total size in bytes

  public:
    FileMappingImpl() noexcept = default;

    FileMappingImpl(std::string_view filepath) noexcept : m_filepath(filepath) {}

    // Can't copy, only move/borrow.
    FileMappingImpl(const FileMappingImpl &) = delete;
    FileMappingImpl &operator=(const FileMappingImpl &) = delete;

    ~FileMappingImpl() noexcept = default;

    error_t Open(std::string_view filepath) noexcept
    {
        m_filepath = filepath;
        return Open();
    }

    virtual size_t ElementSize() const noexcept = 0;

    error_t Open() noexcept
    {
        Close();
        if (error_t err = NewFileMapping(filepath(), &m_void, &m_capacity); err != 0) {
            return err;
        }
        m_size = m_capacity / ElementSize();
        return 0;
    }

    void Close() noexcept { CloseFileMapping(&m_void, m_capacity); }

    const std::string &filepath() const noexcept { return m_filepath; }
    size_t size() const noexcept { return m_size; }
    bool empty() const noexcept { return m_size > 0; }
    const void *get() const noexcept { return m_void; }
};

///////////////////////////////////////////////////////////////////////////////
// FileMapping is a container-like encapsulation of a memory mapped view of
// a file or annonymous mapping for sharing memory.
//
template<typename T = char>
class FileMapping final : public FileMappingImpl
{
  public:
    using element_type = const T;
    using const_type = const T;
    using self_type = FileMapping<element_type>;
    using iterator = element_type *;
    using const_iterator = const_type *;

    using FileMappingImpl::FileMappingImpl;

    size_t ElementSize() const noexcept final { return sizeof(element_type); }

    iterator begin() const noexcept { return data(); }
    iterator end() const noexcept { return data() + size(); }
    const_iterator cbegin() const noexcept { return data(); }
    const_iterator cend() const noexcept { return data() + size(); }

    element_type *data() noexcept { return reinterpret_cast<element_type *>(get()); }
    const_type *data() const noexcept { return reinterpret_cast<element_type *>(get()); }

    const T &operator[](size_t idx) const noexcept { return data()[idx]; }
};

#endif  // AMUL_LIB_FILEMAPPING_H
