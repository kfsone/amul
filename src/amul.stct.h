#ifndef AMUL_STCT_H
#define AMUL_STCT_H 1

#include "amul.defs.h"
#include "amul.vmop.h"  ///TODO: Move language structs to that file.
#include "typedefs.h"

// Room represents a room, or location, in the game world.
struct Room {
    char id[IDL + 1];
    roomid_t dmove{ -1 };        /// TODO: Rename cemetery
    uint32_t flags{ 0 };         // static flags
    stringid_t shortDesc{ -1 };  // short description id
    stringid_t longDesc{ -1 };   // long description id
    uint32_t ttlines{ 0 };       /// TODO: normalize 'count'
    union {
        size_t ttOffset{ 0 };
        struct TravelLine *ptr;
    };
};

// represents an instruction in the language table, along with a possible
// conditional operator ahead of it.
struct VMOper  // VM Operation: which may be a condition OR an action.
{
    vmopid_t m_op;
    oparg_t m_args[MAX_VMOP_PARAMS];
};

struct VMLine {
    bool notCondition;  // true :- negate result from condition
    VMOper condition;
    VMOper action;
};

// Syntax represents a syntaxes slot that expands to a series of instructions
struct Syntax {
    WType wtype[5];      /* Type of word expected */
    amulid_t slot[5];    /* wtype specific values	 */
    uint16_t ents{ 0 };  /// TODO: normalize 'count'
    union {
        size_t lineOffset{ 0 };
        VMLine *ptr;
    };
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
    char id[IDL + 1];
    uint8_t flags;
    union {
        char precedence[2][5];
        char precedences[10];
    };
    int16_t ents;  /// TODO: normalize 'count'
    union {
        Syntax *ptr;  /// TODO: variants[]
        size_t slotOffset;
    };
};

// Player ranks
struct Rank {
    char male[RANKL];    // Male title
    char female[RANKL];  // Female title
    char prompt[11];
    int32_t score;     // Score required to attain
    int16_t strength;  // Combat; does not affect carry capacity
    int16_t stamina;
    int16_t dext;
    int16_t wisdom;
    int16_t experience;
    int16_t magicpts;
    int32_t maxweight;
    int16_t numobj;
    int32_t minpksl;  // Base points for killing someone of this rank
    uint32_t tasks;   // Bitmask of required tasks to attain rank
};

// Object: State specific properties
struct ObjState {
    uint32_t weight{ 0 };    // In grammes
    int32_t value{ 0 };      // Base points for dropping in a swamp
    uint16_t strength{ 0 };  // } Unclear: May be health of the item,
    uint16_t damage{ 0 };    // } damage it does or damage done to it
    stringid_t description{ WNONE };
    uint16_t flags{ 0 };
};

// Object: Item or npc in the game world
struct Object {
    char id[IDL + 1];  /// TODO: noun id
    objid_t idno;      /* Object's ID no	 */
    adjid_t adj;       /* Adjective. -1 = none	 */
    int16_t inside;    /* No. objects inside	 */
    flag_t flags;      /* Fixed flags		 */
    int32_t contains;  /* How much it will hold */
    int8_t nstates;    /* No. of states	 */
    int8_t putto;      /* Where things go	 */
    int8_t state;      /* Current state	 */
    int8_t npc;        /* Mobile character	 */
    union {
        ObjState *states; /* Ptr to states!	 */
        size_t stateOffset;
    };
    int32_t nrooms; /* No. of rooms its in	 */
    union {
        roomid_t *rooms;
        size_t roomsOffset;
    };

    roomid_t Room(size_t idx) const noexcept { return idx < nrooms ? rooms[idx] : -1; }
    slotid_t Owner() const noexcept;
    void SetOwner(slotid_t slot) noexcept;
    bool IsOwned() const noexcept;
    const ObjState &State() const noexcept { return states[state]; }
    ObjState &State() noexcept { return states[state]; }
    bool IsVisibleTo(slotid_t who) const noexcept;
};

// Travel table entry, which you will note is a verb table entry with a verb.
/// TODO: See above.
struct TravelLine : public VMLine {
    verbid_t verb{ -1 };
};

// NPC is the "live" representation of a runtime NPC
struct NPC {
    int16_t dmove;                        /* Move to when it dies	*/
    char deadstate;                       /* State flags		*/
    char speed, travel, fight, act, wait; /* speed & %ages	*/
    char fear, attack;                    /* others		*/
    int8_t flags;                         /* -- none yet --	*/
    int16_t hitpower;
    uint16_t rank;                               /* Rank equiv		*/
    stringid_t arr, dep, flee, hit, miss, death; /* Various UMsgs	*/
    int16_t knows;                               /* Number of &		*/
    union {
        VMLine *cmds;
        size_t cmdsOffset;
    };
};

// NPCClass is the compile-time 'class' definition of an NPC
struct NPCClass {
    char id[IDL + 1]; /* Name of npc	*/
    struct NPC npc;
};

// NPCStats is a component of runtime association of a npc with its controller.
struct NPCStats {
    uint16_t obj;    /* Object no.		*/
    uint16_t speed;  /* Secs/turn		*/
    uint16_t count;  /* Time till next	*/
    uint16_t pflags; /* Player style flags	*/
};

struct Adjective {
    char word[IDL + 1];
    int32_t adjectiveId;
};

enum SynType { SYN_INVALID, SYN_VERB, SYN_NOUN };
struct Synonym {
    char word[IDL + 1]{};
    SynType type{ SYN_INVALID };
    int32_t aliases{ -1 };
};

#endif