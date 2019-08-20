#include <map>
#include <memory>
#include <string>

#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "msgports.h"
#include "system.h"

using PortTable = std::map<string_view, MsgPortPtr>;
using PortMap = std::map<MsgPort *, MsgPortPtr>;

extern PortTable s_portTable;
extern PortMap s_portMap;

TEST(MsgPortTest, Test01_CreatePortAnon)
{
    EXPECT_TRUE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());

    auto port = CreatePort("");
    EXPECT_NE(port, nullptr);
    EXPECT_TRUE(port->m_name.empty());
    EXPECT_TRUE(port->m_msgList.empty());
    EXPECT_FALSE(port->m_spinLock.IsLocked());

    EXPECT_TRUE(s_portTable.empty());
    EXPECT_FALSE(s_portMap.empty());
    EXPECT_EQ(s_portMap.size(), 1);

    auto it = s_portMap.begin();
    EXPECT_EQ(it->first, port.get());
    EXPECT_EQ(it->second.get(), port.get());

    s_portMap.clear();
}

TEST(MsgPortTest, Test01_CreatePortNamed)
{
    EXPECT_TRUE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());

    constexpr const string_view portName = "testing";
    auto port = CreatePort(portName);
    EXPECT_NE(port, nullptr);
    EXPECT_EQ(port->m_name, portName);
    EXPECT_TRUE(port->m_msgList.empty());
    EXPECT_FALSE(port->m_spinLock.IsLocked());

    EXPECT_FALSE(s_portTable.empty());
    EXPECT_TRUE(s_portMap.empty());
    EXPECT_EQ(s_portTable.size(), 1);

    auto it = s_portTable.begin();
    EXPECT_EQ(it->first, portName);
    EXPECT_EQ(it->second.get(), port.get());
    EXPECT_EQ(it, s_portTable.find(portName));

    s_portTable.clear();
}

TEST(MsgPortTest, Test02_DeletePort)
{
    constexpr const string_view portName = "test2";

    auto port1 = CreatePort("");
    EXPECT_NE(port1, nullptr);
    EXPECT_EQ(s_portMap.size(), 1);
    auto port2 = CreatePort(portName);
    EXPECT_NE(port2, nullptr);
    EXPECT_EQ(s_portTable.size(), 1);

    port2->Close();
    EXPECT_EQ(s_portTable.size(), 0);
    EXPECT_EQ(s_portMap.size(), 1);

    port1->Close();
    EXPECT_EQ(s_portTable.size(), 0);
    EXPECT_EQ(s_portMap.size(), 0);
}

TEST(MsgPortTest, Test03_FindPort)
{
    auto first = CreatePort("foo");
    auto second = CreatePort("bar");
    auto third = CreatePort("baz");

    EXPECT_EQ(s_portTable.size(), 3);
    EXPECT_EQ(FindPort("foo").get(), first.get());
    EXPECT_EQ(FindPort("bar").get(), second.get());
    EXPECT_EQ(FindPort("baz").get(), third.get());

	first->Close();
    EXPECT_EQ(FindPort("foo").get(), nullptr);
    EXPECT_EQ(FindPort("bar").get(), second.get());
    EXPECT_EQ(FindPort("baz").get(), third.get());
    second->Close();
    EXPECT_EQ(FindPort("foo").get(), nullptr);
    EXPECT_EQ(FindPort("bar").get(), nullptr);
    EXPECT_EQ(FindPort("baz").get(), third.get());
    third->Close();
    EXPECT_EQ(FindPort("foo").get(), nullptr);
    EXPECT_EQ(FindPort("bar").get(), nullptr);
    EXPECT_EQ(FindPort("baz").get(), nullptr);
}