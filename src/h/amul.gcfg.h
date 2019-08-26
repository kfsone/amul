#ifndef AMUL_GCFG_H
#define AMUL_GCFG_H

// Description of game configuration and properties (room count, etc)

#include <vector>

#include "h/amul.type.h"
#include "h/amul.stct.h"

enum { GAME_NAME_LENGTH = 64 };

struct GameConfig {
    enum Versions {
        CPPAmul = 1,

        CurrentVersion = CPPAmul
    };

    int32_t version { CurrentVersion };
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

    time_t compiled;        // When the game was compiled
    size_t numStrings;      // Number of strings in the string table
    size_t stringBytes;     // Total length of string data
    size_t numRanks;        // Size of rank table
    size_t numRooms;        // Size of room table
    size_t numTTEnts;       // Size of travel table
    size_t numObjects;      // Size of object table
    size_t numObjStates;    // Total object state count
    size_t numObjLocations; // Number of locations objects occupy
    size_t numVerbs;        // Size of verb table
    size_t numVerbSlots;    // size of the slot component of verbs
    size_t numVerbOps;      // Total count of verb C&A
    size_t numAdjectives;   // Size of adjective table
    size_t numSynonyms;     // Size of synonym table
    size_t numMobs;         // Size of mobile table
    size_t numMobPersonas;  // Size of mobile persona table
};

struct Game final : public GameConfig {
    void *m_dataPtr;  // raw pointer to the data
    /// TODO: Game Info and Game Config
    //
    std::vector<stringid_t>   m_stringIndex;
    std::vector<char>         m_strings;
    std::vector<_RANK_STRUCT> m_ranks;
    std::vector<_ROOM_STRUCT> m_rooms;
    std::vector<_ADJECTIVE>   m_adjectives;
    std::vector<_OBJ_STRUCT>  m_objects;
    std::vector<roomid_t>     m_objectLocations;
    std::vector<_OBJ_STATE>   m_objectStates;
    std::vector<_SYNONYM>     m_synonyms;

    struct _TT_ENT *     m_travel;

    struct _VERB_STRUCT *m_verbs;
    struct _SLOTTAB *    m_verbSlots;
    struct _VBTAB *      m_verbOps;

    ~Game() {}

    error_t Load();
    error_t Save();
};


extern Game g_game;

static inline _RANK_STRUCT &GetRank(int n) noexcept { return g_game.m_ranks[n]; }
static inline _ROOM_STRUCT &GetRoom(roomid_t n) noexcept { return g_game.m_rooms[n]; }
static inline _ADJECTIVE &GetAdj(adjid_t n) noexcept { return g_game.m_adjectives[n]; }
static inline _OBJ_STRUCT &GetObject(objid_t n) noexcept { return g_game.m_objects[n]; }
static inline const char *GetString(stringid_t n) noexcept {
    return g_game.m_strings.data() + g_game.m_stringIndex[n];
}

#endif
