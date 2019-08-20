#ifndef AMUL_DEMON_H
#define AMUL_DEMON_H

#include "typedefs.h"
#include "parser.expression.h"
#include "parser.wtype.h"

struct Demon {
    demonid_t m_id{ WNONE };
    slotid_t m_owner{ WNONE };  // Which connection owns this demon: replaces own[MAXD]
    time_t m_trigger{ 0 };   // Next event time: replaces count[MAXD]
    Parse::Expression m_expression {};

  private:
    static demonid_t s_nextID;

  public:
    static constexpr slotid_t GlobalOwner = -1;

  public:
    Demon() = default;
    Demon(demonid_t id, slotid_t owner, time_t trigger, Parse::Expression expression)
        : m_id{ id }, m_owner{ owner }, m_trigger{ trigger }, m_expression{ expression }
    {
    }
    ~Demon();

    uint32_t GetSecondsRemaining() const noexcept;

    constexpr bool IsGlobal() const { return m_owner == GlobalOwner; }

    static demonid_t
    Start(slotid_t owner, time_t seconds, Parse::Expression expression);
    static void Kill(slotid_t owner, verbid_t action);
};

#endif  // AMUL_DEMON_H
