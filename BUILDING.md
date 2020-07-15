# Compiling AMUL

To launch a game with AMUL you need four things:

- amulcom -- the compiler,
- aman -- the controller/manager/server,
- amul -- the game client,
- game files

# Current Status:

(Note [./] is supposed to be an ascii check mark)

## amulcom

[./] Basic C++ pass
[./] Compiles
[~~] Compiles without warnings
[./] Basic execution (compiles a game)
[na] Port of Amiga specific functionality
[  ] Eliminate TODOs
[  ] Executes accurately

There are several points at which it is highly likely that amulcom is
not correctly writing game data, but it compiles and runs under asan now
without errors.

## aman

[~~] Basic C++ pass
[  ] Compiles
	Missing some variables
[  ] Compiles without warnings
[  ] Basic execution (loads & hosts a game)
[~~] Port of Amiga specific functionality
	Created a stub implementation of MsgPorts,
	Need to review what the TimerDevice did and replace,
	Need to review whether to simulate MsgPorts
[  ] Eliminate TODOs
	Needs more TODOs
[  ] Executes accurately

## amul

[  ] Basic C++ pass
[  ] Compiles
[  ] Compiles without warnings
[  ] Basic execution
	[  ] Load game data
	[  ] Connect to manager
	[  ] Input/Output
	[  ] Parser
	[  ] VM
[  ] Port of Amiga specific functionality
	Needs MsgPorts and Messages to communicate with Aman,
	Has Amiga-specific full-screen code that needs ditching,
	Has Amiga-specific serial I/O code that needs ditching,
	Does not know anything about tcp/ip
[  ] Eliminate TODOs
[  ] Executes accurately
	Define accurately, I dare you

# Building

## Windows
### Visual Studio 2019

1. File > Open Directory,
2. Open the top level folder,
3. F7 to build,
4. ^R+A to Run all tests,
	- unit tests appear in the unit test explorer bar,
5. To build a specific target, change the view mode in the solution
   explorer from Directory View to CMake Targets view.

### Visual Studio Code

I forget.

## Docker

You can either build the docker image yourself or pull it from the Docker
registry.

```
	> docker build --tag kfsone/amulu --file Docker.ubuntu .
	> docker run --rm -it -v /path/to/this/dir:/amul kfsone/amulu
	$ mkdir build ; cd build ; cmake -G Ninja .. ; cp ../etc/Makefile . ; make
```

## Linuxes

As per Docker:

```
	$ mkdir build ; cd build ; cmake -G Ninja .. ; cp ../etc/Makefile . ; make
```

I'm using GCC and Clang versions >= 7, and also have it configured to use
asan and ubsan when building with `-DCMAKE_BUILD_TYPE=Debug`.


# Running tests

Trivial in visual studio, not sure in other IDEs. To do it by hand:

```
	$ cd ${topdir}  # not an actual command unless you defined $topdir
	$ cd build/tests
	$ ls *Test
	$ ./AtomTest && ./BufferTest && ..
```

## Compiling a game

Use 'amulcom' and give it the path to the game files. There are two sample
games: test-game and precious.

