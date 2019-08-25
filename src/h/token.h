#ifndef AMUL_TOKEN_H
#define AMUL_TOKEN_H

#include "h/tokentype.h"

struct Token {
    TokenType    type;
    TokenSubType subtype;
    const Atom * start;
    const Atom * end;
};

#endif  // AMUL_TOKEN_H
