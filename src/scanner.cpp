#include "scanner.h"
#include "buffer.h"

// Include the table.
#include "char-to-atom.h"

// nextAtom returns the next atom for the given buffer, and advances the
// cursor for the extent of that atom; for example, consumes '\r\n' as one
// atom, or consumes entire alpha sequences as 'letter'.
// 
AtomType
nextAtom(Buffer &buf)
{
    const auto firstc = buf.Peek();
    switch (AtomType at = charToAtom[firstc]; at) {
    case AT_Illegal:
    case AT_Symbol:
        buf.Skip();
        return at;
    case AT_End: {
        if (firstc == 0) {
		if (!buf.Eof())
			buf.Skip();
            return at;
	}
        const auto nextc = buf.SkipPeek();
        // skip \r or \n after it's compliment
        if (nextc && nextc != firstc && charToAtom[firstc] == AT_End) {
            buf.Skip();
        }
        return at;
    }
    case AT_Space:
    case AT_Letter:
    case AT_Digit:
        // check the *next* character
        while (charToAtom[buf.SkipPeek()] == at) {
            // continue
        }
        return at;
    }
}



/*
struct AtomInst
{
	AtomType	type;
	const char	*start;
	const char	*end;
};

using Atoms = std::vector<AtomInst>;

Atoms::const_iterator
_number(const Atoms& atoms)
{
	auto cur = atoms.cbegin();
	return (cur++)->type == AtomType::Digit ? ++cur: atoms.cbegin();
}

Atoms::const_iterator
Number(const Atoms& atoms)
{
	auto cur = atoms.cbegin();
	if (cur->type == AtomType::Symbol && (*cur->start == '-')) {
		++cur;
	}
	return _number(atoms);
}

Atoms::const_iterator
Word(const Atoms& atoms)
{
	auto cur = atoms.cbegin();
	if ((cur++)->type != AtomType::Letter)
		return atoms.cbegin();
	bool allowSymbol = true;
	while (cur != atoms.cend()) {
	switch ((*cur)->type) {
	case AtomType::Letter:
	case AtomType::Digit:
		allowSymbol = true;
		++cur;
		continue;
	case AtomType::Symbol:
		if (*cur->start == '-' || *cur->start == '_') {
			++cur;
			allowSymbol = false;
			continue;
		}
		// FALLTHROUGH
	default:
		return atoms.cbegin();
	}
	}
	return cur;
}
 */