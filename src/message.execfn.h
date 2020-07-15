#pragma once

#include <functional>

#include "message.base.h"

using ExecuteFn = std::function<void(void)>;
struct MsgExecuteFn final : public ParameterizedDispatch<ExecuteFn> {
  public:
    MsgExecuteFn(ExecuteFn &&fn, MsgPortPtr replyPort = t_replyPort)
        : ParameterizedDispatch(fn, replyPort)
    {
    }
	virtual ~MsgExecuteFn() noexcept;

    void Dispatch() { m_param(); }
};

