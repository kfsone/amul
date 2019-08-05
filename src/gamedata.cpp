#include <h/amul.defs.h>
#include <h/amul.type.h>
#include <h/amul.lcst.h>
#include <h/room_struct.h>

#include <deque>
#include <map>
#include <string>

using RoomTable = std::deque<Room>;
RoomTable g_rooms;

using RoomIndex = std::map<std::string, roomid_t>;
RoomIndex g_roomIndex;

roomid_t
RoomIdx::Register(Room &&room)
{
    roomid_t rid { g_rooms.size() };
    g_roomIndex[room.id] = rid;
    g_rooms.push_back(std::move(*this));
    return rid;
}

roomid_t
RoomIdx::Lookup(const std::string &name) noexcept
{
    auto it = g_roomIndex.lookup(name);
    return it != g_roomIndex.end() ? it->second : -1;
}

Room *
RoomIdx::Find(const std::string &name) noexcept
{
    roomid_t id = g_roomIndex.lookup(name);
    return id != -1 ? g_rooms[id] : nullptr;
}

Room *
RoomIdx::Get(roomid_t roomid) noexcept
{
    if (roomid >= 0 && roomid < g_rooms.size())
        return g_rooms[static_cast<uint32_t>(roomid)];
    return nullptr;
}

size_t
RoomIdx::Size() noexcept
{
    return g_rooms.size();
}
