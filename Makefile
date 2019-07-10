a@ll:: bindir amulcom trie

amulcom::  bin/amulcom

trie:: bin/trie_test

bindir::
	if [ ! -d ./bin ] ; then mkdir -f bin; fi
	
bin/amulcom: src/amulcom.c src/extras.c src/logging.c h/*.h
	cc -std=c11 -I. -Wall -Wno-format -Wno-missing-braces -Wno-int-conversion -Wno-incompatible-pointer-types-discards-qualifiers -Wno-tautological-constant-out-of-range-compare \
			-DDEBUG -DDEBUG_ -D_DEBUG \
			-o bin/amulcom src/amulcom.c src/extras.c src/logging.c \
			-O0 -g2 \
			-fsanitize=address,undefined \
			-fno-omit-frame-pointer

bin/trie_test: bin src/trie.c src/trie.h
	cc -std=c11 -I. -Wall -O0 -g2 -o bin/trie_test src/trie.c \
			-DDEBUG -DDEBUG_ -D_DEBUG
