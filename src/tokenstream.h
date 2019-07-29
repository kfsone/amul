#ifndef AMUL_SRC_TOKENSTREAM_H
#define AMUL_SRC_TOKENSTREAM_H

#include "array.h"
#include "atom.h"
#include "buffer.h"
#include "token.h"

struct TokenStream {
    Buffer &buffer;

    // The atoms we have available
    Array<Atom, 256>      atoms{};
    Array<TokenType, 256> tokens{};
    Array<TokenType, 64>  newTokens{};

    TokenStream(Buffer &_buffer)
        : buffer(_buffer)
    {
    }

    void *m_pattern{nullptr};
    void  SetPattern(void *pattern) { m_pattern = pattern; }

  protected:
    void Atomize();  // Transform next line into atoms
  public:
    void ScanLine();  // Transform next line into tokens
};

#endif  // AMUL_SRC_TOKENSTREAM_H
