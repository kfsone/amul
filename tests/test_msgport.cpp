#include "gtest_aliases.h"
#include <gtest/gtest.h>
#include "system.h"
#include "msgports.h"

#include <map>
#include <memory>
#include <string>

using PortTable = std::map<std::string, std::unique_ptr<MsgPort>>;
using PortMap   = std::map<MsgPort*, std::unique_ptr<MsgPort>>;

extern PortTable s_portTable;
extern PortMap s_portMap;

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

