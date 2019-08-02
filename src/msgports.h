#ifndef AMUL_SRC_MSGPORTS_H
#define AMUL_SRC_MSGPORTS_H

#include "spinlock.h"
#include <atomic>
#include <deque>
#include <h/amigastubs.h>
#include <memory>

struct MsgPort;

struct Message : public Node {
    constexpr Message(MsgPort *replyPort, size_t size)
        : Node(NT_MESSAGE)
        , mn_ReplyPort(replyPort)
        , mn_Length(size)
    {
    }
    constexpr Message(size_t size)
        : Message(nullptr, size)
    {
    }

    MsgPort *mn_ReplyPort;  // message reply port
    size_t   mn_Length;     // total message length, in bytes
                            // (include the size of the Message
                            // structure in the length)
};

using MessagePtr = std::unique_ptr<Message>;

struct MsgPort {
    using MsgList = std::deque<MessagePtr>;

    const char *m_name;
    MsgList     m_msgList;
    /// TODO: Just using simple spinlocks for now
    SpinLock m_spinLock;

  public:
    MsgPort(const char *name)
        : m_name(name)
    {
    }

    void Put(MessagePtr &&ptr);
    bool IsReady() noexcept;
    bool IsLocked() const noexcept { return m_spinLock.IsLocked(); }

    MessagePtr Get();   // non-blocking
    MessagePtr Wait();  // blocking
    void       Clear();
};

struct Aport : public Message {
    Aport(MsgPort *replyPort = nullptr, uint32_t type_ = 0, int32_t from_ = -1, int64_t data_ = 9)
        : Message(replyPort, sizeof(Aport))
        , type(type_)
        , from(from_)
        , data(data_)
    {
    }

    uint32_t type{0};
    int32_t  from{-1};
    int64_t  data{0};
    int64_t  p1{0}, p2{0}, p3{0}, p4{0};
    void *   opaque{nullptr};
};

// mp_Flags: Port arrival actions (PutMsg)
#define PF_ACTION 3   // Mask
#define PA_SIGNAL 0   // Signal task in mp_SigTask
#define PA_SOFTINT 1  // Signal SoftInt in mp_SoftInt/mp_SigTask
#define PA_IGNORE 2   // Ignore arrival

MsgPort *FindPort(const char *portName);
MsgPort *CreatePort(const char *portName);
void     DeletePort(MsgPort *);

void ReplyMsg(MessagePtr &&msg);

#endif  // AMUL_SRC_MSGPORTS_H
