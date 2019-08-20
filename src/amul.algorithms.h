#pragma once

#include "typedefs.h"
#include "game.h"

constexpr auto ForEachPlayer = [](auto function) {
    for (slotid_t slotId = 0; slotId < MAXU; ++slotId) {
        if (GetAvatar(slotId).state == PLAYING) {
            function(slotId, GetCharacter(slotId), GetAvatar(slotId));
        }
    }
};

constexpr auto HasAny = [](const auto &container, auto predicate) noexcept -> bool {
    for (auto &item : container) {
        if (predicate(item))
            return true;
    }
    return false;
};