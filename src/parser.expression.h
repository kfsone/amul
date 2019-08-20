#pragma once

#include <optional>

#include "typedefs.h"
#include "parser.token.h"

namespace Parse
{

struct Term {
    std::optional<adjid_t> m_adj;
    Token m_token;
};

struct Expression {
    verbid_t m_verb;
    Term m_terms[2];
};

}