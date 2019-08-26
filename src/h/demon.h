#ifndef AMUL_DEMON_H
#define AMUL_DEMON_H

#include "h/amul.type.h"

struct Demon {
    demonid_t m_id{ -1 };
    slot_t m_owner{ -1 };   // Which connection owns this demon: replaces own[MAXD]
    time_t m_trigger{ 0 };  // Next event time: replaces count[MAXD]

    // Each demon has sufficient fields to associate it with a syntax line:
    // i.e verb, {wtype, wval}[2] but their use is opaque. Ultimately it would probably
    // make sense to have a base class and specific implementations.
    verbid_t m_action{ -1 };  // Demon's numeric parameter: replaces num[MAXD]
    struct Param {
        int64_t m_type{ -1 };   // replaces typ[MAXD][2]
        int64_t m_value{ -1 };  // replaces val[MAXD][2]
    } m_params[2];

  private:
    static demonid_t s_nextID;

  public:
    static constexpr slot_t GlobalOwner = -1;

  public:
    constexpr Demon() = default;
    constexpr Demon(demonid_t id,
                    slot_t owner,
                    time_t trigger,
                    verbid_t action,
                    Param param1,
                    Param param2)
        : m_id{ id }, m_owner{ owner }, m_trigger{ trigger }, m_action{ action }, m_params{ param1,
                                                                                            param2 }
    {
    }
    ~Demon();

    uint32_t GetSecondsRemaining() const noexcept;

    constexpr bool IsGlobal() const { return m_owner == GlobalOwner; }

    static demonid_t
    Start(slot_t owner, time_t seconds, verbid_t action, Param param1, Param param2);
    static void Kill(slot_t owner, verbid_t action);
};

#endif  // AMUL_DEMON_H
