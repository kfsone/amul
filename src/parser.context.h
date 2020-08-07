#pragma once
#ifndef AMUL_PARSER_CONTEXT_H
#define AMUL_PARSER_CONTEXT_H

#include "amul.typedefs.h"
#include "parser.expression.h"
#include "parser.token.h"
#include "parser.wtype.h"

namespace Parser
{

struct Context {
    // The parser allows for a verb and two parameters which are nominally called nouns.
    // There is also allowance for adjectives and a preposition, and the "nouns" might be
    // text strings or player names or numbers.
    //
    // We also keep the "adjective" around separately because it is a participant in resolving
    // specific instances. E.g if you say "lit match", there could be several matches, but we
    // need to avoid resolving the *exact* match until as late as possible, so we don't just
    // look for the first "lit" match and take that.

    verbid_t m_originalVerb;

    // Current verb and it's arguments
    Expression m_expression;

    // What was the previous command.
    optional<verbid_t> m_lastVerb;

    // The parser actually needs to know this for certain contexts.
    optional<roomid_t> m_previousRoom;

    // The last direction you travelled in so it can be exposed to the condition system,
    // and people can do things like make one-way passages.
    //  verb=east
    //    if lastdir is west then error "You can't turn around"
    optional<verbid_t> m_lastDir;
};

}  // namespace Parser

extern thread_local verbid_t iverb;
extern thread_local verbid_t overb;
extern thread_local adjid_t iadj1, iadj2;
extern thread_local amulid_t inoun1, inoun2;
extern thread_local prepid_t iprep;
extern thread_local verbid_t lverb, ldir;
extern thread_local roomid_t lroom;
extern thread_local WType wtype[6];
extern thread_local amulid_t last_it;
extern thread_local slotid_t actor, last_him, last_her; /* People we talked about */

extern thread_local bool failed;            // Current parse failed/aborted
extern thread_local bool t_forced;          // current action was forced on us
extern thread_local bool t_follow;          // current action is result of a follow
extern thread_local char inc, died;         // For parsers use
extern thread_local char autoexits;         // General flags
extern thread_local short int donev, skip;  // No. of vb's/TT's done
extern thread_local char exeunt, more;
extern thread_local long ml, donet;  // Maximum lines

pair<WType, amulid_t> GetTokenType(char **s);
adjid_t IsAdjective(string_view name) noexcept;
objid_t IsANoun(string_view name) noexcept;
roomid_t IsRoomName(string_view name) noexcept;

#endif  // AMUL_PARSER_CONTEXT_H
