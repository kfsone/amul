#pragma once

// Defines the base classes for Message implementations.

#include "amul.typedefs.h"
#include "msgports.h"

extern thread_local slotid_t t_slotId;
extern thread_local MsgPortPtr t_replyPort;

// Message types

struct DispatchedMessage : public Message {
    DispatchedMessage(MsgPortPtr replyPort=nullptr) noexcept : Message(t_slotId, replyPort){}
	virtual ~DispatchedMessage() noexcept;
};

template<typename ParamType>
struct ParameterizedDispatch : public DispatchedMessage {
    ParamType m_param{};
    ParameterizedDispatch() = default;
    ParameterizedDispatch(const ParamType &param, MsgPortPtr replyPort = nullptr) noexcept
        : DispatchedMessage(replyPort), m_param{ param }
    {
    }
    ParameterizedDispatch(ParamType &&param, MsgPortPtr replyPort = nullptr) noexcept
        : DispatchedMessage(replyPort), m_param{ forward<ParamType>(param) }
    {
    }
	virtual ~ParameterizedDispatch() {}
};
struct ReplyableMessage : public DispatchedMessage {
    ReplyableMessage(MsgPortPtr port = t_replyPort) noexcept : DispatchedMessage(port) {}
	virtual ~ReplyableMessage() noexcept;
};

template<typename ParamType>
struct ParameterizedReplyable : public ReplyableMessage {
    ParamType m_param{};
    ParameterizedReplyable() = default;
    ParameterizedReplyable(ParamType &&param) noexcept
        : ReplyableMessage(), m_param{ forward<ParamType>(param) }
    {
    }
};

