#ifndef AMUL_ATOM_H
#define AMUL_ATOM_H

#include <cstddef>

#include "atomtype.h"

struct Atom final {
    const char *m_start;
    AtomType m_type;
    const char *m_end;

  public:
    explicit constexpr Atom(AtomType at, const char *start, size_t len) noexcept
        : m_start{ start }, m_type{ at }, m_end{ start + len }
    {
    }

    explicit constexpr Atom(AtomType at) noexcept : Atom{ at, "", 0 } {}
    constexpr Atom() noexcept : Atom{ A_INVALID } {}
    explicit constexpr Atom(const char *text) noexcept : Atom{ A_PUNCT, text, 1 } {}

    explicit Atom(struct Buffer &buffer) noexcept;

    constexpr const AtomType &Type() const noexcept { return m_type; }
    constexpr char First() const noexcept { return m_end > m_start ? *m_start : 0; }
    constexpr bool operator==(AtomType at) const noexcept { return m_type == at; }
    constexpr bool operator==(char punct) const noexcept
    {
        return m_type == A_PUNCT && *m_start == punct;
    }
    constexpr bool operator!=(AtomType at) const noexcept { return !(operator==(at)); }
    constexpr bool operator!=(char punct) const noexcept { return !(operator==(punct)); }
};

AtomType NextAtomType(struct Buffer &);

#endif  // AMUL_ATOM_H
