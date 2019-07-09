bin: bin
	mkdir bin

amulcom::  bin
	cc -std=c11 -I. -Wall -Wno-format -Wno-missing-braces -Wno-int-conversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-tautological-constant-out-of-range-compare \
			-o bin/amulcom src/amulcom.c src/extras.c src/logging.c \
			-O0 -g3 \
			-fsanitize=address,undefined \
			-fno-omit-frame-pointer
