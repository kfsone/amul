#include <map>
#include <memory>
#include <string>

//#include "amigastubs.h"
#include "message.base.h"
#include "message.execfn.h"
#include "msgports.h"
#include "system.h"

SpinLock s_tablesLock{};
SpinLock s_criticalSection{};

using PortTable = std::map<std::string, MsgPortPtr>;
using PortMap = std::map<MsgPort *, MsgPortPtr>;

PortTable s_portTable;
PortMap s_portMap;


// Out-of-line dtor for Message.
Message::~Message() noexcept {}
DispatchMessage::~DispatchMessage() noexcept {}
ReplyableMessage::~ReplyableMessage() noexcept {}
MsgExecuteFn::~MsgExecuteFn() noexcept {}

CriticalSection::CriticalSection() noexcept : SpinGuard(s_criticalSection) {}

MsgPortPtr
FindPort(std::string portName) noexcept
{
    SpinGuard guard(s_tablesLock);
    auto it = s_portTable.find(portName);
    return (it != s_portTable.end()) ? it->second : nullptr;
}

MsgPortPtr
FindPort(slotid_t slotId) noexcept
{
    return FindPort(std::to_string(slotId));
}

MsgPortPtr
CreatePort(std::string portName)
{
    SpinGuard guard(s_tablesLock);

    // Ports don't have to have a name.
    if (!portName.empty()) {
        auto it = s_portTable.find(portName);
        if (it != s_portTable.end())
            return it->second;
    }

    auto port = make_shared<MsgPort>(portName);

    if (!portName.empty())
        s_portTable.emplace(portName, port);
    else
        s_portMap.emplace(port.get(), port);

    return port;
}

void
MsgPort::Close() noexcept
{
    {
        SpinGuard guard{ m_spinLock };
        if (!m_open.exchange(false))
            return;
    }

    SpinGuard guard(s_tablesLock);
    if (m_name.empty()) {
        auto it = s_portMap.find(this);
        if (it != s_portMap.end())
            s_portMap.erase(it);
        return;
    }

    auto it = s_portTable.find(m_name);
    if (it != s_portTable.end() && it->second.get() == this) {
        s_portTable.erase(it);
        return;
    }

    for (auto &&entry : s_portTable) {
        if (entry.second.get() == this) {
            s_portTable.erase(entry.first);
            return;
        }
    }
}

bool
MsgPort::IsReady() noexcept
{
    SpinGuard guard{ m_spinLock };
    return m_msgList.empty() == false;
}

void
MsgPort::Put(MessagePtr &&msg)
{
    SpinGuard guard{ m_spinLock };
    if (m_open)
        m_msgList.emplace_back(move(msg));
}

MessagePtr
MsgPort::Wait()
{
    MessagePtr result{ nullptr };
    for (;;) {
        if (SpinGuard guard{ m_spinLock }; !m_msgList.empty()) {
            std::swap(m_msgList.front(), result);
            m_msgList.pop_front();
            break;
        }
        if (!m_open)
            break;
        YieldCpu();
    }
    return result;
}

// Non Blocking
MessagePtr
MsgPort::Get()
{
    MessagePtr result{ nullptr };
    if (SpinGuard guard{ m_spinLock }; !m_msgList.empty()) {
        std::swap(m_msgList.front(), result);
        m_msgList.pop_front();
    }
    return result;
}

void
ReplyMsg(MessagePtr &&msg)
{
    if (msg->m_replyPort)
        msg->m_replyPort->Put(move(msg));
}

void
MsgPort::Clear() noexcept
{
    SpinGuard guard{ m_spinLock };
    m_msgList.clear();
}
