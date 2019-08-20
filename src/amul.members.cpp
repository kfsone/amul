#include <cassert>

#include "amul.stct.h"
#include "game.h"
#include "objflag.h"
#include "playerflag.h"
#include "typedefs.h"

constexpr roomid_t OwnerOffset = -5;
constexpr roomid_t MaxOwnerOffset = OwnerOffset - MAXU;

bool
Object::IsOwned() const noexcept
{
    return (nrooms == 1 && Room(0) <= OwnerOffset && Room(0) >= MaxOwnerOffset);
}

slotid_t
Object::Owner() const noexcept
{
    if (!IsOwned())
        return -1;
    return -(Room(0)) - OwnerOffset;
}

void
Object::SetOwner(slotid_t slot) noexcept
{
    assert(nrooms == 1);
    if (slot == -1) {
        rooms[0] = -1;
        return;
    }
    rooms[0] = -(OwnerOffset + slot);
}

bool
Object::IsVisibleTo(slotid_t who) const noexcept
{
    const auto &avatar = GetAvatar(who);
    // only blind players are given 'smell' descriptions.
    if ((flags & OF_SMELL) && !(avatar.flags & PFBLIND))
        return false;
    if ((avatar.flags & PFBLIND) && (!(flags & OF_SMELL) || Owner() != -(5 + who)))
        return false;
    if (flags & OF_INVIS)
        return true;
    if (isPINVIS(who) && pRANK(who) >= g_game.seeInvisRank - 1)
        return true;
    if (pRANK(who) < g_game.seeSuperInvisRank - 1)
        return false;
    return true;
}
