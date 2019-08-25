#include "h/spinlock.h"

extern void Yield();

bool
SpinLock::TryLock() noexcept
{
    return !m_lock.exchange(true, std::memory_order_acquire);
}

void
SpinLock::Lock() noexcept
{
    for (size_t i = 0; i < 30; ++i) {
        if (TryLock())
            return;
    }
    while (!TryLock()) {
        Yield();
    }
}

void
SpinLock::Unlock() noexcept
{
    m_lock.exchange(false, std::memory_order_release);
}
