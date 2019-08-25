#ifndef AMUL_DEMON_INL_H
#define AMUL_DEMON_INL_H

#include <iostream>

static inline std::ostream &operator<<(std::ostream &os, const Demon &demon) noexcept
{
    os << "demon:" << demon.m_id << ":owner:" << demon.m_owner << ":action:" << demon.m_action;
    return os;
}

#endif  // AMUL_DEMON_INL_H
