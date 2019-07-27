#ifndef AMUL_ATOMTYPE_H
#define AMUL_ATOMTYPE_H

// Atom enumerates the subset of sub-token components.

enum AtomType {
    AT_Illegal,
    AT_End,
    AT_Space,
    AT_Letter,
    AT_Digit,
    AT_Symbol,

    MAX_ATOM
};

#endif  // AMUL_ATOMTYPE_H

