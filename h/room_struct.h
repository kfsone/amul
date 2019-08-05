#ifndef AMUL_SRC_ROOM_STRUCT_H
#define AMUL_SRC_ROOM_STRUCT_H

#include <deque>
#include <string>
#include <vector>

#include <h/amul.defs.h>
#include <h/amul.type.h>

#include "lang_struct.h"
#include "struct_index.h"

// Travel table entry, which you will note is a verb table entry with a verb.
/// TODO: See above.
struct TravelIns : public VMCnA {
    TravelIns(verbid_t vb, const VMCnA& vbtab)
        : VMCnA(vbtab), verb(vb) {}
    verbid_t verb;
};

// Describes a location in the game world
struct Room {
    std::string id {};
    std::string dmove {};
    flag_t flags{ 0 };     // static flags
    stringid_t descid{ 0 };
    std::vector<TravelIns> travel;
};

using RoomIdx = StructIndex<Room>;
extern RoomIdx g_rooms;

#endif  // AMUL_SRC_ROOM_STRUCT_H
