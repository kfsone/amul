#include "h/amul.type.h"
#include "h/amul.gcfg.h"
#include "h/amul.stct.h"

roomid_t
_OBJ_STRUCT::Room(size_t idx) const noexcept
{
    if (idx < nrooms)
        return g_objectLocations[rooms + idx];
    return -1;
}

const _OBJ_STATE &
_OBJ_STRUCT::State() const noexcept
{
    return g_objectStates[state];
}

_OBJ_STATE &
_OBJ_STRUCT::State() noexcept
{
    return g_objectStates[state];
}

constexpr roomid_t OwnerOffset = -5;
constexpr roomid_t MaxOwnerOffset = OwnerOffset - MAXU;

bool
_OBJ_STRUCT::IsOwned() const noexcept
{
    return (nrooms == 1 && Room(0) <= OwnerOffset && Room(0) >= MaxOwnerOffset);
}

slot_t
_OBJ_STRUCT::Owner() const noexcept
{
    if (!IsOwned())
        return -1;
    return -(Room(0)) - OwnerOffset;
}

void
_OBJ_STRUCT::SetOwner(slot_t slot) noexcept
{
    auto &location = g_game.m_rooms[rooms];
    if (slot == -1) {
        location = -1;
        return;
    }
    location = -(OwnerOffset + slot);
}
