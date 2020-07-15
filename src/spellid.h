#pragma once
#ifndef AMUL_SPELLID_H
#define AMUL_SPELLID_H

// Really the built-in magic should be limited to
// things that have effects you can't implement with
// the game itself, and allow users to define their own
// player and object flags to manipulate.
enum SpellID {
    SGLOW = 1,  // Spell #1
    SINVIS,     // Spell #2
    SBLIND,     // Spell #3
    SCRIPPLE,   // Spell #4
    SDEAF,      // Spell #5
    SDUMB,      // Player cant speak
    SSLEEP,     // Puts a player to bedie byes
    SSINVIS,    // Super Invisible
};

#endif  // AMUL_SPELLID_H

