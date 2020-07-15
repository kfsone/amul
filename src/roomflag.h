#pragma once
#ifndef AMUL_ROOMFLAG_H
#define AMUL_ROOMFLAG_H

// Room bit-flags
#define DMOVE 1        // When players die, move rooms to...
#define STARTL 2       // Players can start from this room
#define RANDOB 4       // Random objects can start here..
#define DARK 8         // Room has no lighting
#define SMALL 16       // Only 1 player at a time
#define DEATH 32       // Players die after reading descrip
#define NOLOOK 64      // Cannot look into this room
#define SILENT 128     // Cannot hear outside noises
#define HIDE 256       // Players cannot be seen from outside
#define SANCTRY 512    // Score points for dropped objects
#define HIDEWY 1024    // Objects in here cannot be seen
#define PEACEFUL 2048  // No fighting allowed here
#define NOEXITS 4096   // Can't list exits
#define ANTERM 8192    // Special Pre-Start start location
#define NOGORM 16834   // Can't random go to this room!

#endif  // AMUL_ROOMFLAG_H

