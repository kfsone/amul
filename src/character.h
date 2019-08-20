#pragma once
#ifndef AMUL_CHARACTER_H
#define AMUL_CHARACTER_H

#include "iotype.h"

// character.h defines player-character related types and functions.

// Character describes a human actor in the game world.
/// TODO: Share common properties with NPCs into structs.
struct Character {
    char name[NAMEL + 1];
    char passwd[23];      /// TODO: hash
    int32_t score;        // Total points scored
    RoomDescMode rdmode;  // Room Description mode (brief/verbose)
    stat_t strength;
    stat_t stamina;
    stat_t dext;
    stat_t wisdom;
    stat_t experience;
    stat_t magicpts;
    rankid_t rank;
    uint16_t plays;  // times played
    uint16_t tries;  // failed login attempts since last login
    char gender;     // gender
    uint32_t tasks;  // bitmask of tasks completed
    uint8_t flags;   // preferences, really, and unused
                     // client/terminal preferences
    char llen;       // preferred line length
    char slen;       // preferred screen length
    char rchar;      // 'redo' character (refresh)
};

// AVATAR is the live presence of a given player
struct Avatar {
    Avatar();

    slotid_t slotId{ -1 };            // Which of the available login slots we're using
    uint32_t flags{ 0 };              // Player flags
    roomid_t room{ WNONE };           // Current room
    char *m_outputBuffer{ nullptr };  // Points to output scratch buffer
    uint32_t sessionScore{ 0 };       // Score this session
    uint32_t rec{ 0 };                // Record number in the player db
    int16_t numobj{ 0 };              // Current inventory count
    int32_t weight{ 0 };              // Weight of inventory
    stat_t strength{ 0 };             // Current strength points
    stat_t stamina{ 0 };              // Current stamina points
    stat_t dext{ 0 };                 // Current dexterity
    stat_t dextadj{ 0 };              // Dexterity offset
    stat_t wisdom{ 0 };               // Current wisdom
    stat_t magicpts{ 0 };             // Magic points
    int16_t wield{ WNONE };           // Weapon being wielded
    SlotState state{ OFFLINE };       // Play state
    IoType ioType{ CONSOLE };         // IO used for connection
    uint16_t light{ true };           // Count of lit objects the player has
    uint16_t hadlight{ true };        // Rather silly count of how many they had "before"
    slotid_t IOlock{ -1 };            // -1 or the slotid of user locking the player's IO
    slotid_t following{ -1 };         // -1 or the slotid of user I'm following
    slotid_t helping{ -1 };           // Player I am helping
    slotid_t followed{ -1 };          // Which player is following me
    slotid_t helped{ -1 };            // Which player is helping me
    slotid_t fighting{ -1 };          // Who I am fighting
                                      /// TODO: normalize to stringids
    char pre[80]{};                   // Pre-rank description
    char post[80]{};                  // Post-rank description
    char arr[80]{};                   // When player arrives
    char dep[80]{};                   // When player leaves
};

extern thread_local Character *t_character;
extern thread_local Avatar *t_avatar;

#endif  // AMUL_CHARACTER_H