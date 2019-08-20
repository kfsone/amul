#pragma once

// Defines the base classes for Message implementations.

#include "msgports.h"
#include "typedefs.h"

extern thread_local slotid_t t_slotId;
extern thread_local MsgPortPtr t_replyPort;

// Message types

struct DispatchMessage : public Message {
    DispatchMessage(MsgPortPtr replyPort=nullptr) noexcept : Message(t_slotId, replyPort){};
};

template<typename ParamType>
struct ParameterizedDispatch : public DispatchMessage {
    ParamType m_param{};
    ParameterizedDispatch(const ParamType &param, MsgPortPtr replyPort = nullptr) noexcept
        : DispatchMessage(replyPort), m_param{ param }
    {
    }
    ParameterizedDispatch(ParamType &&param, MsgPortPtr replyPort = nullptr) noexcept
        : DispatchMessage(replyPort), m_param{ forward<ParamType>(param) }
    {
    }
};
struct ReplyableMessage : public DispatchMessage {
    ReplyableMessage(MsgPortPtr port = t_replyPort) noexcept : DispatchMessage(port) {}
};

template<typename ParamType>
struct ParameterizedReplyable : public ReplyableMessage {
    ParamType m_param{};
    ParameterizedReplyable(ParamType &&param) noexcept
        : ReplyableMessage(), m_param{ forward<ParamType>(param) }
    {
    }
};