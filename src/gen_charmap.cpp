#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "scanner.h"

const char *atoms[MAX_ATOM] = {
        "A_INVALID", "A_END", "A_SPACE", "A_LETTER", "A_DIGIT", "A_PUNCT",
};

size_t
getMaxAtomLen()
{
    size_t maxlen = 0;
    for (size_t i = 0; i < MAX_ATOM; ++i) {
        size_t len = strlen(atoms[i]);
        if (len > maxlen)
            maxlen = len;
    }
    return maxlen;
}

int
main()
{
    char format[32];
    sprintf(format, "%%%zus, ", getMaxAtomLen());

    printf("#ifndef AMUL_SRC_CHAR_ATOM_MAP_H\n");
    printf("#define AMUL_SRC_CHAR_ATOM_MAP_H\n");
    printf("// Generated by atomcharmap.c\n");
    printf("static const AtomType charAtomMap[128] = {\n");
    Atom kind;
    for (int i = 0; i < 32; ++i) {
        printf("    ");
        for (int j = 0; j < 8; ++j) {
            int cNo = i * 8 + j;
            if (cNo == 0 || cNo == '\n' || cNo == '\r') {
                kind = A_END;
            } else if (isalpha(cNo)) {
                kind = A_LETTER;
            } else if (isdigit(cNo)) {
                kind = A_DIGIT;
            } else if (isblank(cNo)) {
                kind = A_SPACE;
            } else if (ispunct(cNo)) {
                kind = A_PUNCT;
            } else if (iscntrl(cNo) || isblank(cNo) || isspace(cNo) || !isprint(cNo)) {
                kind = A_INVALID;
            }
            printf(format, atoms[kind]);
        }
        printf("\n");
    }
    printf("};\n");
    printf("#endif  // AMUL_SRC_CHAR_ATOM_MAP_H\n");
}
