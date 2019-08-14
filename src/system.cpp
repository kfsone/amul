#include <memory>

#include <h/amigastubs.h>
#include "msgports.h"
#include "system.h"

// OS/portability functions

#include <chrono>
#include <map>
#include <memory>
#include <string>
#include <thread>

// Emulate the Amiga's 50hz time
using Tick = std::chrono::duration<int64_t, std::ratio<1, 50>>;

using PortTable = std::map<std::string, std::unique_ptr<MsgPort>>;
using PortMap   = std::map<MsgPort*, std::unique_ptr<MsgPort>>;
PortTable s_portTable;
PortMap s_portMap;

MsgPort *
FindPort(const char *portName)
{
    auto it = s_portTable.find(portName);
    return (it != s_portTable.end()) ? it->second.get() : nullptr;
}

MsgPort *
CreatePort(const char *portName)
{
	if (portName && *portName == 0)
		portName = nullptr;
	// Ports don't have to have a name.
	if (portName != nullptr) {
    	if (auto port = FindPort(portName); port)
        	return port;
	}

    auto port = std::make_unique<MsgPort>(portName);
    auto portp = port.get();

	if (portName != nullptr)
    	s_portTable.emplace(portName, std::move(port));
	else
		s_portMap.emplace(portp, std::move(port));

    return portp;
}

void
DeletePort(MsgPort *port)
{
	if (port->m_name == nullptr) {
		auto it = s_portMap.find(port);
		if (it != s_portMap.end())
			s_portMap.erase(it);
		return;
	}

	auto it = s_portTable.find(port->m_name);
	if (it != s_portTable.end() && it->second.get() == port) {
		s_portTable.erase(it);
		return;
	}

	for (auto&& entry : s_portTable) {
		if (entry.second.get() == port) {
			s_portTable.erase(entry.first);
			return;
		}
	}
}

void
Delay(unsigned int ticks)
{
	// Amiga 'ticks' were 1/50th of a second.
	std::this_thread::sleep_for(Tick(ticks));
}

void
Yield() noexcept
{
	std::this_thread::yield();
}

bool
MsgPort::IsReady() noexcept
{
	SpinGuard guard{m_spinLock};
	return m_msgList.empty() == false;
}

void
MsgPort::Put(MessagePtr&& msg)
{
	SpinGuard guard{m_spinLock};
	m_msgList.emplace_back(std::move(msg));
}

MessagePtr
MsgPort::Wait()
{
	MessagePtr result {nullptr};
	for (;;) {
		if (SpinGuard guard{m_spinLock}; !m_msgList.empty()) {
			std::swap(m_msgList.front(), result);
			m_msgList.pop_front();
			break;
		}
		Yield();
	}
	return result;
}

MessagePtr
MsgPort::Get()  // Non Blocking
{
	MessagePtr result {nullptr};
	if (SpinGuard guard{m_spinLock}; !m_msgList.empty()) {
		std::swap(m_msgList.front(), result);
		m_msgList.pop_front();
	}
	return result;
}

void
ReplyMsg(MessagePtr&& msg)
{
	msg->mn_ReplyPort->Put(std::move(msg));
}

void
MsgPort::Clear()
{
	SpinGuard guard{m_spinLock};
	m_msgList.clear();
}
