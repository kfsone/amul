#include "gtest_aliases.h"
#include "msgports.h"
#include "system.h"
#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>

using PortTable = std::map<std::string, std::unique_ptr<MsgPort>>;
using PortMap = std::map<MsgPort *, std::unique_ptr<MsgPort>>;

extern PortTable s_portTable;
extern PortMap   s_portMap;

TEST(MsgPortTest, Test01_CreatePortAnon)
{
    EXPECT_TRUE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());

    auto port = CreatePort(nullptr);
    EXPECT_NE(port, nullptr);
    EXPECT_EQ(port->m_name, nullptr);
    EXPECT_TRUE(port->m_msgList.empty());
    EXPECT_FALSE(port->m_spinLock.IsLocked());

    EXPECT_TRUE(s_portTable.empty());
    EXPECT_FALSE(s_portMap.empty());
    EXPECT_EQ(s_portMap.size(), 1);

    auto it = s_portMap.begin();
    EXPECT_EQ(it->first, port);
    EXPECT_EQ(it->second.get(), port);

    s_portMap.clear();
}

TEST(MsgPortTest, Test01_CreatePortNamed)
{
    EXPECT_TRUE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());

    constexpr const char *portName = "testing";
    auto                  port = CreatePort(portName);
    EXPECT_NE(port, nullptr);
    EXPECT_EQ(port->m_name, portName);
    EXPECT_TRUE(port->m_msgList.empty());
    EXPECT_FALSE(port->m_spinLock.IsLocked());

    EXPECT_FALSE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());
    EXPECT_EQ(s_portTable.size(), 1);

    auto it = s_portTable.begin();
    EXPECT_EQ(it->first, portName);
    EXPECT_EQ(it->second.get(), port);
    EXPECT_EQ(it, s_portTable.find(portName));

    s_portTable.clear();
}

TEST(MsgPortTest, Test02_DeletePort)
{
    constexpr const char *portName = "test2";

    auto port1 = CreatePort(nullptr);
    EXPECT_NE(port1, nullptr);
    EXPECT_EQ(s_portMap.size(), 1);
    auto port2 = CreatePort(portName);
    EXPECT_NE(port2, nullptr);
    EXPECT_EQ(s_portTable.size(), 1);

    DeletePort(port2);
    EXPECT_EQ(s_portTable.size(), 0);
    EXPECT_EQ(s_portMap.size(), 1);

    DeletePort(port1);
    EXPECT_EQ(s_portTable.size(), 0);
    EXPECT_EQ(s_portMap.size(), 0);
}

TEST(MsgPortTest, Test03_FindPort)
{
    constexpr const char *portName = "testx";
    CreatePort("foo");
    CreatePort("bar");
    auto myPort = CreatePort(portName);
    CreatePort("baz");

    EXPECT_EQ(s_portTable.size(), 4);
    EXPECT_EQ(FindPort(portName), myPort);

    s_portTable.clear();
}

TEST(MsgPortTest, Test04_Clear)
{
    MsgPort port{"foo"};
    port.m_msgList.push_back(nullptr);
    port.m_msgList.push_back(nullptr);
    EXPECT_EQ(port.m_msgList.size(), 2);
    port.Clear();
    EXPECT_EQ(port.m_msgList.size(), 0);
}

TEST(MsgPortTest, Test05_Put)
{
    MsgPort port{"foo"};
    port.Put(nullptr);
    EXPECT_FALSE(port.m_msgList.empty());
}

TEST(MsgPortTest, Test06_IsReady)
{
    MsgPort port{"foo"};
    EXPECT_FALSE(port.IsReady());
    port.Put(nullptr);
    EXPECT_TRUE(port.IsReady());
}

TEST(MsgPortTest, Test07_IsLocked)
{
    MsgPort port{"foo"};
    EXPECT_FALSE(port.IsLocked());
    port.m_spinLock.Lock();
    EXPECT_TRUE(port.IsLocked());
}

TEST(MsgPortTest, Test08_Get)
{
    MsgPort port{"foo"};
    EXPECT_FALSE(port.IsReady());
    port.Put(std::make_unique<Message>(nullptr, 200););
    EXPECT_TRUE(port.IsReady());
    auto msgp = port.Get();
    EXPECT_NE(msgp.get(), nullptr);
    EXPECT_EQ(msgp->mn_ReplyPort, nullptr);
    EXPECT_EQ(msgp->mn_Length, 200);
    EXPECT_FALSE(port.IsLocked());
    EXPECT_FALSE(port.IsReady());
}

TEST(MsgPortTest, Test09_Wait)
{
    int        op{0};
    SpinLock   lock{};
    MsgPort    port{"foo"};
    MessagePtr msgp{nullptr};

    port.m_spinLock.Lock();

    std::thread fn1([&]() {
        std::this_thread::yield();
        port.Put(std::make_unique<Message>(nullptr, 42));
        op = 2;
    });
    std::thread fn2([&]() {
        msgp = Wait();
        std::this_thread::yield();
        op = 3;
    }) std::this_thread::yield();

    op = 1;
    port.m_spinLock.Unlock();

    EXPECT_NE(msgp.get(), nullptr);
    EXPECT_FALSE(port.m_spinLock.IsLocked());
    EXPECT_EQ(msgp->mn_Length, 42);
    EXPECT_EQ(op, 3);
    EXPECT_FALSE(port.IsReady());

    fn1.join();
    fn2.join();
}

TEST(MsgPortTest, Test10_Order)
{
    MsgPort  port{"foo"};
    SpinLock lock{};
    lock.Lock();

    std::thread fn([&]() {
        lock.Lock();
        port.Put(nullptr, 10);
        port.Put(nullptr, 20);
        lock.Unlock();
    });

    lock.Unlock();
    auto msgp = port.Wait();
    std::this_thread::yield();

    // Should be the second waiting
    EXPECT_TRUE(port.IsReady());
    EXPECT_NE(msgp.get(), nullptr);
    EXPECT_EQ(msgp->mn_Length, 10);
    msgp = port.Get();
    EXPECT_FALSE(port.IsReady());
    EXPECT_NE(msgp.get(), nullptr);
    EXPECT_EQ(msgp->mn_Length, 20);
}