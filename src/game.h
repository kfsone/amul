#ifndef AMUL_GCFG_H
#define AMUL_GCFG_H

// Description of game configuration and properties (room count, etc)

#include <array>
#include <atomic>
#include <unordered_map>
#include <vector>

#include "amul.stct.h"
#include "character.h"
#include "amul.typedefs.h"

enum { GAME_NAME_LENGTH = 64 };

enum class RunMode { Normal, Reset, Terminate };

struct GameConfig {
    enum Versions {
        CPPAmul = 1,

        CurrentVersion = CPPAmul
    };

    int32_t version{ CurrentVersion };
    /// TODO: Add amul version number
    char gameName[GAME_NAME_LENGTH];
    // This rank is required to see an invisible player/thing
    int32_t seeInvisRank;  // `amul1:"invis"`
    // This rank is required to see a "super-invis" player/thing (typically GM)
    int32_t seeSuperInvisRank;  // `amul1:"invis2"`
    // Rank required to be able to do "supergo" (it's a built-in)
    int32_t superGoRank;  // `amul1:"minsgo"`
    // How long a game runs before resetting
    int32_t gameDuration_m;  // `amul1:"mins"`
    // "swamp" score is scaled based on your progress to max rank
    int32_t rankScale;  // `amul1:"rscale"`
    // "swamp" score is scaled based on how long the game has been running
    int32_t timeScale;  // `amul1:"tscale"`

    time_t compiled;         // When the game was compiled
    size_t numStrings;       // Number of strings in the string table
    size_t stringBytes;      // Total length of string data
    size_t numRanks;         // Size of rank table
    size_t numRooms;         // Size of room table
    size_t numTTEnts;        // Size of travel table
    size_t numObjects;       // Size of object table
    size_t numObjStates;     // Total object state count
    size_t numObjLocations;  // Number of locations objects occupy
    size_t numVerbs;         // Size of verb table
    size_t numVerbSlots;     // size of the slot component of verbs
    size_t numVerbOps;       // Total count of verb C&A
    size_t numAdjectives;    // Size of adjective table
    size_t numSynonyms;      // Size of synonym table
    size_t numNPCs;          // Size of npc table
    size_t numNPCClasses;    // Size of npc persona table

	constexpr rankid_t MaxRank() const noexcept { return rankid_t(numRanks - 1); }
};

struct Game final : public GameConfig {
    std::atomic<RunMode> m_runMode{ RunMode::Normal };
    bool m_forcedReset{ false };

    std::array<Character, MAXNODE> m_players{};
    std::array<Avatar, MAXNODE> m_avatars{};

    std::vector<stringid_t> m_stringIndex{};
    std::vector<char> m_strings{};
    std::vector<Rank> m_ranks{};
    std::vector<Room> m_rooms{};
    std::vector<TravelLine> m_travel{};
    std::vector<Adjective> m_adjectives{};
    std::vector<Object> m_objects{};
    std::vector<roomid_t> m_objectLocations{};
    std::vector<ObjState> m_objectStates{};
    std::vector<Synonym> m_synonyms{};
    std::vector<Verb> m_verbs{};
    std::vector<Syntax> m_verbSlots{};
    std::vector<VMLine> m_vmLines{};
    std::vector<NPCClass> m_npcClasses{};
    std::vector<NPC> m_npcs{};

    std::vector<roomid_t> m_startRooms{};

    // the list of characters people have created
    std::vector<Character> m_characters;
    std::unordered_map<std::string, Character &> m_characterIndex;

    // Create a bit-array of fields representing whether or not
    // each player has visited given rooms.
    using VisitedFlags = std::array<std::vector<bool>, MAXU>;
    VisitedFlags m_visited{};

    // Timestamp of the last reset
    char lastResetTime[64]{};
    char lastStartupTime[64]{};
    char lastCompileTime[64]{};

    Game() = default;
    ~Game() = default;

    error_t Load();
    error_t Save();
};

extern Game g_game;

static inline Adjective &
GetAdjective(adjid_t n) noexcept
{
    return g_game.m_adjectives[n];
}
static inline Avatar &
GetAvatar(slotid_t slot) noexcept
{
    return g_game.m_avatars[slot];
}
static inline Object &
GetObject(objid_t n) noexcept
{
    return g_game.m_objects[n];
}
static inline Character &
GetCharacter(slotid_t slot) noexcept
{
    return g_game.m_players[slot];
}
static inline Rank &
GetRank(rankid_t rankNo) noexcept
{
    return g_game.m_ranks[rankNo];
}
static inline Room &
GetRoom(roomid_t n) noexcept
{
    return g_game.m_rooms[n];
}
static inline const char *
GetString(stringid_t n) noexcept
{
    return g_game.m_strings.data() + g_game.m_stringIndex[n];
}
static inline Rank &
GetTopRank() noexcept
{
    return g_game.m_ranks.back();
}
static inline Verb &
GetVerb(verbid_t verbId) noexcept
{
    return g_game.m_verbs[verbId];
}

extern thread_local Character *t_player;
extern thread_local Avatar *t_avatar;
#endif
