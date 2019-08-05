#ifndef AMUL_SRC_LANG_STRUCT_H
#define AMUL_SRC_LANG_STRUCT_H

#include <array>
#include <deque>
#include <h/amul.vmop.h>
#include <h/structindex.h>

// _VBTAB represents an action (and optional condition) in the table of instructions
// associated with a verb + syntax block.
/// TODO: Allow multiple conditions, multiple actions.
/// TODO: Rename from 'vbtab' to 'VMOp', 'Instruction', something .. better

struct VMCnA {      ///TODO: Replace with vmins
    int32_t condition;
    std::array<opparam_t, MAX_VMOP_PARAMS> cparams;
    /// TODO: Replace negative actions with a GOTO action.

    int32_t action;   /* #>0=action, #<0=room  */
    std::array<opparam_t, MAX_VMOP_PARAMS> aparams;
};

// VerbForm represents a particular form (syntax) of a verb usage.
struct VerbForm {
    VerbForm(char i=0)
    : wtype { WANY, i, i, WANY, i }
    , slot  { WANY, WANY, WANY, WANY, WANY }
    , cna   {}
    {}

    char           wtype[5]; /* Type of word expected */
    long           slot[5];  /* wtype specific values	 */
    std::vector<VMCnA> cna;
};

// Verb describes the verbs that operate as runtime commands from
// players. Each verb has to describe the order in which it prefers to
// resolve conflicting matches for noun tokens: for example, if there is
// a "note" on the ground and a "note" in the player's inventory, then
// the "get" and "drop" verbs will want to try those in opposite orders.
//
// History: 'CHAE' is 'Carried, Here, Another (player's inventory) and
// Elsewhere (not in the room)'. This is really precedence or ordering,
//
struct Verb {
    std::string id;
    uint8_t flags;
    union {
        char precedence[2][5];
        char precedences[10];
    };
    std::vector<VerbForm> forms;
};

using VerbIdx = StructIndex<Verb>, verbid_t>;
extern VerbIdx g_verbs;

#endif  // AMUL_SRC_LANG_STRUCT_H
