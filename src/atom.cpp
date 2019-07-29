#include "atomtype.h"
#include "buffer.h"
#include "char-to-atom.h"

// nextAtom returns the next atom for the given buffer, and advances the
// cursor for the extent of that atom; for example, consumes '\r\n' as one
// atom, or consumes entire alpha sequences as 'letter'.
//
AtomType
NextAtomType(Buffer &buf)
{
    const auto firstc = buf.Read();
    switch (const AtomType at = charToAtom[firstc]; at) {
    case A_INVALID:
    case A_PUNCT:
        return at;
    case A_END: {
        const auto nextc = buf.Peek();
        if (firstc != 0) {
            // skip \r or \n after it's compliment
            if (nextc && nextc != firstc && charToAtom[nextc] == A_END) {
                buf.Skip();
            }
        }
        return at;
    }
    case A_SPACE:
    case A_LETTER:
    case A_DIGIT:
        // check the *next* character
        while (charToAtom[buf.Peek()] == at) {
            buf.Skip();
        }
        return at;
    }

	// unreachable
    return A_INVALID;
}
