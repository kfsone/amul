#pragma once
#ifndef AMUL_PARSER_TOKEN_H
#define AMUL_PARSER_TOKEN_H

#include "parser.wtype.h"
#include "amul.typedefs.h"

namespace Parser
{

// Disambiguate the different wtype-specific values a token can have.
union TokenValue {
    objid_t um_object;
    slotid_t um_player;
    roomid_t um_room;
    amulid_t um_value;
    stringid_t um_stringid;
    string_view um_text;
};

// A token is a combination of a type and an (optional) value.
struct Token {
    WType m_wtype;
    optional<TokenValue> m_value;

    Token() : m_wtype{ WNONE }, m_value{} {}
    Token(WType wtype) : m_wtype{ wtype }, m_value{} {}
    Token(WType wtype, TokenValue value) : m_wtype{ wtype }, m_value{ value } {}

    bool operator()() const noexcept { return m_value.has_value(); }

    // Helper accessors
    bool HasWType() const noexcept { return m_wtype != WNONE; }
    bool HasValue() const noexcept { return m_value.has_value(); }
    objid_t ObjID() const noexcept { return m_value ? m_value->um_object : WNONE; }
    slotid_t SlotId() const noexcept { return m_value ? m_value->um_player : WNONE; }
    roomid_t RoomId() const noexcept { return m_value ? m_value->um_room : WNONE; }
    amulid_t Value() const noexcept { return m_value ? m_value->um_value : WNONE; }
    stringid_t StringId() const noexcept { return m_value ? m_value->um_stringid : WNONE; }
    string_view Text() const noexcept { return m_value ? m_value->um_text : string_view{}; }
};

}  // namespace Parser

#endif  // AMUL_PARSER_TOKEN_H

