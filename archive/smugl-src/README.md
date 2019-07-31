# SMUGL 
## Simple Multi-User Game Language

The original AMUL reflects the process by which I taught myself programming and
the C language in particular. At some point I tried rewriting large portions of
it in 68000 assembler as "AMUD" and then tried a different refactor as "AGL".

A handful of years later I used it as a Linux coding playground: SMUGL.

AMUL heavily relied on some of the caveats of Amiga programming, most important
being the IPC ("MsgPort"s) and the common memory space between processes, so it
was a significant amount of work to get the game more-or-less working again.

Before I finished, I succumbed to trying to optimize and improve the compiler,
and bit rot set in for the game proper.

# Building Executables

```shell
	$ ./bootstrap.sh
	$ ./develconfigure.sh
	$ make depend
	$ make -j9
```

# Compiling a game

A game is defined in a series of text files which, for this incarnation of
AMUL/SMUGL lives in the "test" folder.

```shell
	$ ./smuglcom/smuglcom --help
```

To compile the game in the 'test' folder:

```shell
	$ ./smuglcom/smuglcom test
```

This should get you something like:

```
	Successful: Rooms=283: Verbs=340 Objects=206. Total Items=6435
```

# Validating the compiled files

`smuglcom/verify` is supposed to do this, apparently it's broken.

# Launching a game

The server and client are the same executable, `smugl/smugl`, and this
version probably forks itself to handle players.

```shell
	$ ./smugl/smugl --help   # probably not very helpful
	$ ./smugl/smugl ./test	 # probably crashes
```

# Why is it broken?

AMUL treated rooms, objects and players as very different things. I was in the
process of making SMUGL have a core concept of a "basic object" struct so that
I could treat things more somewhat more homogenously.

