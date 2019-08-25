#ifndef AMUL_TOKENSTREAM_H
#define AMUL_TOKENSTREAM_H

#include "h/array.h"
#include "h/atom.h"
#include "h/buffer.h"
#include "h/token.h"

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

#endif  // AMUL_TOKENSTREAM_H
