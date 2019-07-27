#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "scanner.h"

const char *atoms[MAX_ATOM] = {
	"AT_Illegal",
	"AT_End",
	"AT_Space",
	"AT_Letter",
	"AT_Digit",
	"AT_Symbol",
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

int main()
{
	char format[32];
	sprintf(format, "%%%zus, ", getMaxAtomLen());

	printf("#ifndef CHAR_ATOM_MAP_H\n");
	printf("#define CHAR_ATOM_MAP_H\n");
	printf("// Generated by atomcharmap.c\n");
	printf("static const enum Atom charAtomMap[128] = {\n");
	enum Atom kind;
	for (int i = 0; i < 32; ++i) {
		printf("    ");
		for (int j = 0; j < 8; ++j) {
			int cNo = i * 8 + j;
			if (cNo == 0 || cNo == '\n' || cNo == '\r') {
				kind = AT_End;
			} else if (isalpha(cNo)) {
				kind = AT_Letter;
			} else if (isdigit(cNo)) {
				kind = AT_Digit;
			} else if (isblank(cNo)) {
				kind = AT_Space;
			} else if (ispunct(cNo)) {
				kind = AT_Symbol;
			} else if (iscntrl(cNo) || isblank(cNo) || isspace(cNo) || !isprint(cNo)) {
				kind = AT_Illegal;
			}
			printf(format, atoms[kind]);
		}
		printf("\n");
	}
	printf("};\n");
	printf("#endif  // CHAR_ATOM_MAP_H\n");
}
