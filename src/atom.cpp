#include "atom.h"
#include "buffer.h"
#include "char-to-atom.h"

// nextAtom returns the next atom for the given buffer, and advances the
// cursor for the extent of that atom; for example, consumes '\r\n' as one
// atom, or consumes entire alpha sequences as 'letter'.
//
AtomType
NextAtomType(Buffer &buffer)
{
    const auto firstc = buffer.Read();
    switch (const AtomType at = charToAtom[firstc & 0xff]; at) {
        case A_INVALID:
        case A_PUNCT:
            return at;
        case A_END: {
            const auto nextc = buffer.Peek();
            if (firstc != 0) {
                // skip \r or \n after it's compliment
                if (nextc && nextc != firstc && charToAtom[nextc & 0xff] == A_END) {
                    buffer.Skip();
                }
            }
            return at;
        }
        case A_SPACE:
        case A_LETTER:
        case A_DIGIT:
            // check the *next* character
            while (charToAtom[buffer.Peek() & 0xff] == at) {
                buffer.Skip();
            }
            return at;
    }

    // unreachable
    return A_INVALID;
}

Atom::Atom(Buffer &buffer) noexcept
    : m_start(buffer.it()), m_type(NextAtomType(buffer)), m_end(buffer.end())
{
}
