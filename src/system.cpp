#include <memory>

#include <h/amigastubs.h>
#include "msgports.h"
#include "system.h"

// OS/portability functions

#include <map>
#include <memory>
#include <string>

using PortTable = std::map<std::string, std::unique_ptr<MsgPort>>;
using PortMap   = std::map<MsgPort*, std::unique_ptr<MsgPort>>;
static PortTable s_portTable;
static PortMap s_portMap;

MsgPort *FindPort(const char *portName)
{
    auto it = s_portTable.find(portName);
    return (it != s_portTable.end()) ? it->second.get() : nullptr;
}

MsgPort *
CreatePort(const char *portName, uint8_t  priority)
{
	// Ports don't have to have a name.
	if (portName != nullptr && *portName != 0) {
    	if (auto port = FindPort(portName); port)
        	return port;
	}
    auto port = std::make_unique<MsgPort>(portName, priority);
    auto portp = port.get();

	if (portName != nullptr && *portName != 0)
    	s_portTable.emplace(portName, std::move(port));
	else
		s_portMap.emplace(portp, std::move(port));

    return portp;
}

void
DeletePort(MsgPort *port)
{
	if (port->ln_Name == nullptr || *port->ln_Name == 0) {
		auto it = s_portMap.find(port);
		if (it != s_portMap.end())
			s_portMap.erase(it);
		return;
	}

	auto it = s_portTable.find(port->ln_Name);
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

#include <chrono>
#include <thread>

using Tick = std::chrono::duration<int64_t, std::ratio<1, 50>>;

void
Delay(unsigned int ticks)
{
	// Amiga 'ticks' were 1/50th of a second.
	std::this_thread::sleep_for(Tick(ticks));
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
		std::this_thread::yield();
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

void MsgPort::Clear()
{
	SpinGuard guard{m_spinLock};
	m_msgList.clear();
}

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
		std::this_thread::yield();
	}
}

void
SpinLock::Unlock() noexcept
{
	m_lock.exchange(false, std::memory_order_release);
}
