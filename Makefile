all:: amulcom

amulcom::  bin bin/amulcom

bin: bin
	mkdir bin
	
bin/amulcom: bin/amulcom src/amulcom.c src/extras.c src/logging.c h/*.h
	cc -std=c11 -I. -Wall -Wno-format -Wno-missing-braces -Wno-int-conversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-tautological-constant-out-of-range-compare \
			-o bin/amulcom src/amulcom.c src/extras.c src/logging.c \
			-O0 -g2 \
			-fsanitize=address,undefined \
			-fno-omit-frame-pointer
