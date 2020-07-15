#pragma once

#include "message.base.h"
#include "parser.expression.h"

//amul1:MDAEMON
struct MsgExecDemon : public ParameterizedDispatch<Parser::Expression> {
    using ParameterizedDispatch::ParameterizedDispatch;
    void Dispatch() override;
};

