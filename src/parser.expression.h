#pragma once
#ifndef AMUL_PARSER_EXPRESSION_H
#define AMUL_PARSER_EXPRESSION_H

#include "parser.token.h"
#include "typedefs.h"

namespace Parser
{

struct Term {
    optional<adjid_t> m_adj;
    Token m_token;
};

struct Expression {
    verbid_t m_verb;
    Term m_terms[2];
};

}  // namespace Parser

#endif  // AMUL_PARSER_EXPRESSION_H

