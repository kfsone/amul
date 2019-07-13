#ifndef AMUL_H_AMUL_GCFG_H
#define AMUL_H_AMUL_GCFG_H

// Description of game configuration and properties (room count, etc)

#include <stdint.h>

enum { GAME_NAME_LENGTH = 64 };

struct GameConfig {
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
};

struct GameInfo {
    time_t compiled;        // When the game was compiled
    size_t numStrings;		// Number of strings in the string table
    size_t numRooms;        // Size of room table
    size_t numRanks;        // Size of rank table
    size_t numVerbs;        // Size of verb table
    size_t numSynonyms;     // Size of synonym table
    size_t numNouns;        // Size of noun table
    size_t numAdjectives;   // Size of adjective table
    size_t numTTEnts;       // Size of travel table
    size_t numMobs;         // Size of mobile table
    size_t numMobPersonas;  // Size of mobile persona table
};

#endif