#ifndef AMUL_TOKENTYPE_H
#define AMUL_TOKENTYPE_H

// Enumerates the base kinds of token
enum TokenType {
    T_ATOM,
    T_SEPARATOR,   // A_SPACE+ | A_END -> subtype == atom type.
                   // note: comment -> A_END
    T_COMMENT,     // ; Any ~EOL
    T_NUMBER,      // digit+ ~
    T_WORD,        // letter {letter|underscore|hyphen|digit)+ ~
    T_SYMBOL,      // A_PUNCT(symbols)
    T_IDENTIFIER,  // T_SYMBOL Word
    T_DICTWORD,    // Word looked up from the dictionary table (TBD)
};

// Token SubType depends on the token type, so it's really just an opaque value.
// For T_ATOM and T_SEPARATOR it will be the atom type,
// For T_COMMENT it will be 0,
// For T_NUMBER it will be the value of the number,
// For T_WORD it will be 0,
// For T_SYMBOL it will be a symbol enumerator or the symbol's character (TBD),
// For T_DICTWORD it will be the dictionary id of the word,
using TokenSubType = uint64_t;

#endif  // AMUL_TOKENTYPE_H
