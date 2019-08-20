#pragma once

// 'WType's are the identified types of input tokens. They actually have to line up
// exactly with the parameter types. Not all word types can be parameter types.
// Given that WType and PType are so close now, they should be collapsed into TokenType.
enum WType {
    WINVALID = -2,  // Something but it's not legal
    WNONE = -1,     // None!
    WANY,           // Anything!
    WNOUN,          // Word is a noun
    WADJ,           // Word is an adjective
    WPREP,          // Its a prep
    WPLAYER,        // Its a player
    WROOM,          // Its a room ID
    WSYN,           // Its a synonym
    WTEXT,          // Its text
    WVERB,          // Its a verb!
    WCLASS,         // Class name
    WNUMBER,        // A number
};
