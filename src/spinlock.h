#ifndef AMUL_SRC_SPINLOCK_H
#define AMUL_SRC_SPINLOCK_H

#include <atomic>
#include <h/amul.type.h>

#ifndef FRIEND_TEST
#    define FRIEND_TEST(...)
#endif

class SpinLock final
{
    std::atomic_bool m_lock{false};
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

  public:
    SpinGuard(SpinLock &spin)
        : m_spin(spin)
    {
        m_spin.Lock();
    }
    ~SpinGuard() { m_spin.Unlock(); }
};

#endif  // AMUL_SRC_SPINLOCK_H
