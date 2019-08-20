#include <thread>

#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "spinlock.h"


TEST(SpinLockTest, Lock)
{
    SpinLock lock{};
    EXPECT_FALSE(lock.m_lock);
    lock.Lock();
    EXPECT_TRUE(lock.m_lock);
}

TEST(SpinLockTest, Unlock)
{
    SpinLock lock{};
    EXPECT_FALSE(lock.m_lock);
    lock.m_lock = true;
    lock.Unlock();
    EXPECT_FALSE(lock.m_lock);
}

TEST(SpinLockTest, SpinLock)
{
    std::atomic<int> i = 0;
    SpinLock         lock{};
    lock.Lock();
    EXPECT_TRUE(lock.m_lock);

    std::thread fn([&] {
        lock.Lock();
        i = 2;
    });
    EXPECT_TRUE(lock.m_lock);

    std::this_thread::yield();
    i = 1;
    std::this_thread::yield();

    EXPECT_EQ(i, 1);
    lock.Unlock();
    fn.join();
    EXPECT_TRUE(lock.m_lock);
    EXPECT_EQ(i, 2);
}

TEST(SpinLockTest, IsLocked)
{
    SpinLock lock{};
    EXPECT_FALSE(lock.IsLocked());
    lock.Lock();
    EXPECT_TRUE(lock.IsLocked());
}

TEST(SpinLockTest, TryLock)
{
    SpinLock lock{};
    EXPECT_TRUE(lock.TryLock());
    EXPECT_FALSE(lock.TryLock());
    EXPECT_FALSE(lock.TryLock());
    lock.Unlock();
    EXPECT_TRUE(lock.TryLock());
}
