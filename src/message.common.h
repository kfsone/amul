#pragma once
#ifndef AMUL_CLIENTMESSAGES_H
#define AMUL_CLIENTMESSAGES_H

#include <string_view>

#include "message.base.h"

extern void akillme();

// Messages

//amul1:MCNCT
struct MsgConnectClient final : public ReplyableMessage {
    void Dispatch() override;
};

struct MsgDie : public DispatchMessage {
    void Dispatch() { akillme(); }
};

//amul1:MDISCNCT
struct MsgDisconnectClient final : public ReplyableMessage {
    void Dispatch() override;
};

struct MsgExecuteVmop : public ParameterizedDispatch<VMLine> {
    void Dispatch() override;
};

struct MsgFollow : public ParameterizedDispatch<std::string> {
    void Dispatch() override;
};

struct MsgForced : public ParameterizedDispatch<std::string> {
    void Dispatch() override;
};

struct MsgLockUser final : public ParameterizedReplyable<slotid_t> {
    using ParameterizedReplyable::ParameterizedReplyable;
    void Dispatch() override;
};

struct MsgLog final : public ParameterizedReplyable<string_view> {
    using ParameterizedReplyable::ParameterizedReplyable;
    void Dispatch() override;
};

//amul1:MLOGGED
struct MsgLoggedIn final : public ReplyableMessage {
    void Dispatch() override;
};

//amul1:MPING
struct MsgPingServer final : public ReplyableMessage {
	using ReplyableMessage::ReplyableMessage;
    void Dispatch() override;
};

//amul1:MESSAGE
struct MsgPrintText : public ParameterizedDispatch<std::string> {
    void Dispatch() override;
};

struct MsgResetting final : public DispatchMessage {
    void Dispatch() override;
};

struct MsgSendStringId final : public ParameterizedReplyable<stringid_t> {
    using ParameterizedReplyable::ParameterizedReplyable;
    void Dispatch() override;
};

struct MsgSendText final : public ParameterizedReplyable<string_view> {
    using ParameterizedReplyable::ParameterizedReplyable;
    void Dispatch() override;
};

struct MsgSetBusy final : public ParameterizedDispatch<bool> {
    using ParameterizedDispatch::ParameterizedDispatch;
    void Dispatch() override;
};

struct MsgSummoned : public ParameterizedReplyable<roomid_t> {
    void Dispatch() override;
};

struct MsgUnlockUser final : public ParameterizedReplyable<slotid_t> {
    using ParameterizedReplyable::ParameterizedReplyable;
    void Dispatch() override;
};

#endif  // AMUL_CLIENTMESSAGES_H

