a@ll:: bindir amulcom hash

amulcom::  bin/amulcom

hash:: bin/hash_test

bindir::
	if [ ! -d ./bin ] ; then mkdir -f bin; fi
	
bin/amulcom: src/amulcom.c src/extras.c src/logging.c h/*.h
	cc -std=c11 -I. -Wall -Wno-format -Wno-missing-braces -Wno-int-conversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-tautological-constant-out-of-range-compare \
			-DDEBUG -DDEBUG_ -D_DEBUG \
			-o bin/amulcom src/amulcom.c src/extras.c src/logging.c \
			-O0 -g2 \
			-fsanitize=address,undefined \
			-fno-omit-frame-pointer

bin/hash_test: bin src/hashmap.c
	cc -std=c11 -I. -Wall -O0 -g2 -o bin/hash_test src/hashmap.c \
			-DDEBUG -DDEBUG_ -D_DEBUG \
			-fsanitize=address,undefined \
			-fno-omit-frame-pointer

test::	bin/hash_test
	bin/hash_test

