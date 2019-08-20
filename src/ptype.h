#pragma once
#ifndef AMUL_PTYPE_H
#define AMUL_PTYPE_H

// Parameter types
enum PType {
    PREAL = -70,        // Noun or slot label
    PNOUN = WNOUN,      // Must be a noun
    PADJ = WADJ,        // Must be an adj
    PPREP = WPREP,      // Must be a preposition [unused]
    PPLAYER = WPLAYER,  // Must be a player
    PROOM = WROOM,      // Must be a room
    PSYN = WSYN,        // Must be a synonym
    PUMSG = WTEXT,      // Must be text
    PVERB = WVERB,      // Must be a verb
    PCLASS = WCLASS,    // Must be a class
    PNUM = WNUMBER,     // Must be a number
    PRFLAG = 11,        // Must be a room flag
    POFLAG = 12,        // Must be an obj flag
    PSFLAG = 13,        // Must be a stat flag
    PSEX = 14,          // Must be a gender
    PDAEMON = 15,       // Must be a daemon ID
};

#endif  // AMUL_PTYPE_H