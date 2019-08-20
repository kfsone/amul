#ifndef AMUL_TOKENSTREAM_H
#define AMUL_TOKENSTREAM_H

#include <vector>

#include "atom.h"
#include "buffer.h"
#include "token.h"

struct TokenStream {
    Buffer &buffer;

    // The atoms we have available
    std::vector<Atom> atoms{};
    std::vector<TokenType> tokens{};
    std::vector<TokenType> newTokens{};

    TokenStream(Buffer &_buffer)
        : buffer(_buffer) 
    {
        atoms.reserve(256);
        tokens.reserve(256);
        newTokens.reserve(64);
    }

    void *m_pattern{ nullptr };
    void SetPattern(void *pattern) { m_pattern = pattern; }

  protected:
    void Atomize();  // Transform next line into atoms
  public:
    void ScanLine();  // Transform next line into tokens
};

#endif  // AMUL_TOKENSTREAM_H
