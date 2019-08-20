#ifndef AMUL_ATOMTYPE_H
#define AMUL_ATOMTYPE_H

enum AtomType {
    A_INVALID,
    A_END,     // \r or \n or \n\r or \r\n
    A_SPACE,   // ' '
    A_LETTER,  // a-z A-Z
    A_DIGIT,   // 0-9
    A_PUNCT,   // punctuation
};

#endif  // AMUL_ATOMTYPE_H
