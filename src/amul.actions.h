#pragma once

#include "amul.defs.h"
#include "amul.typedefs.h"

// Prototypes for action operations

namespace Action
{
    void AbortParse();
    void CancelHelp();
    void DescribeInventory();
    void EndParse();
    void FailParse();
    void FinishParse();
    void LoseFollower();
    void MovePlayerTo(roomid_t roomId);
    void PrintText(amulid_t id);
    void QuitPlayer();
    void SavePlayerCharacter();
    void SetInteractingWith(slotid_t who);
    void SetPlayerFlags(flag_t flagsOn, flag_t flagsOff);
    void SetRoomDescMode(RoomDescMode newMode);
    void StopFollowing();
    void TreatAsVerb(verbid_t verbId);
    void Who(Verbosity verbosity);
}

