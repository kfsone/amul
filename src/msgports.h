#ifndef AMUL_MSGPORTS_H
#define AMUL_MSGPORTS_H

#include <atomic>
#include <list>
#include <memory>

#include "amigastubs.h"
#include "spinlock.h"
#include "typedefs.h"

struct MsgPort;

using MessagePtr = unique_ptr<struct Message>;
using MsgPortPtr = shared_ptr<struct MsgPort>;

struct Message {
    Message(slotid_t sender, MsgPortPtr replyPort) noexcept
        : m_sender(sender), m_replyPort(replyPort)
    {
    }
    virtual ~Message() = default;

    // ReplyPort is only valid in an original message. Do not copy it.
    constexpr Message(const Message &msg) noexcept : m_sender(msg.m_sender), m_replyPort(nullptr) {}

    virtual void Dispatch() = 0;

    // Identifies the source of the message
    enum : slotid_t { SLOT_Server = -1 };
    slotid_t m_sender{ SLOT_Server };

    // If the sender wants a reply, send to this port.
    /// TODO: We can demote this to a special case; the Amiga needed this because the sender was
    // responsible for freeing the message.
    MsgPortPtr m_replyPort{ nullptr };
};

struct MsgPort {
    using MsgList = std::list<MessagePtr>;

    const string_view m_name{ "" };

    /// TODO: Just using simple spinlocks for now
    SpinLock m_spinLock{};
    std::atomic_bool m_open{ true };
    MsgList m_msgList;

  public:
    MsgPort() = default;
    MsgPort(string_view name) : m_name(name) {}
    ~MsgPort() noexcept { Close(); }

    void Put(MessagePtr &&ptr);
    bool IsReady() noexcept;
    bool IsLocked() const noexcept { return m_spinLock.IsLocked(); }

    MessagePtr Get();   // non-blocking
    MessagePtr Wait();  // blocking
    void Clear() noexcept;
    void Close() noexcept;
};

MsgPortPtr CreatePort(string_view portName = "");
MsgPortPtr FindPort(string_view portName) noexcept;
MsgPortPtr FindPort(slotid_t slotId) noexcept;

void ReplyMsg(MessagePtr &&msg);

#endif  // AMUL_MSGPORTS_H
