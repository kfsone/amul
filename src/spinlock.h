#ifndef AMUL_SPINLOCK_H
#define AMUL_SPINLOCK_H

#include <atomic>

#include "typedefs.h"

#ifndef FRIEND_TEST
#define FRIEND_TEST(...)
#endif

class SpinLock final
{
    std::atomic_bool m_lock{ false };
    FRIEND_TEST(SpinLockTest, Lock);
    FRIEND_TEST(SpinLockTest, Unlock);
    FRIEND_TEST(SpinLockTest, SpinLock);

  public:
    SpinLock() {}
    ~SpinLock() { m_lock = true; }

    SpinLock(const SpinLock &) = delete;
    SpinLock(SpinLock &&) = delete;
    SpinLock &operator=(const SpinLock &) = delete;
    SpinLock &operator=(SpinLock &&) = delete;

    bool IsLocked() const noexcept { return m_lock; }
    bool TryLock() noexcept;
    void Lock() noexcept;
    void Unlock() noexcept;
};

class SpinGuard
{
    SpinLock &m_spin;
    bool m_held{ false };

  public:
    SpinGuard(SpinLock &spin) noexcept : m_spin(spin) { Acquire(); }
    ~SpinGuard() noexcept { Release(); }
    void Acquire() noexcept
    {
        if (!m_held) {
            m_spin.Lock();
            m_held = true;
        }
    }
    void Release() noexcept
    {
        if (m_held) {
            m_spin.Unlock();
            m_held = false;
        }
    }
};

class CriticalSection final : public SpinGuard
{
  public:
    CriticalSection() noexcept;
};

#endif  // AMUL_SPINLOCK_H