# AMUL : Amiga Multi-User games Language

This is the source code for "AMUL" - my Amiga Multi-User [games] Language,
which I originally wrote on an Amiga 500 in the early 90s.

My code started out well organized, commented and clean, but quickly filled
the floppy drives I was working on. So I quickly began shortening things.

The Amiga used a "\n\r" line-ending convention, so putting multiple statements
on a single line *did* actually save one extra character.

Sadly, this snapshot dates from about 6-months prior to the last version of
AMUL I released, and so is missing a number of features (and probably bug-
fixes).

I'd eventually had to resort to using on-the-fly compression to try and make
enough room to have both the code AND compile, which ultimately lead to file
corruption, reducing my number of "fresh" backups to a set of disks that I
left in an attic in the town of Healing when I moved to the US, a house from
which my relatives subsequently moved out without the box...

And my only hard-disk copy was lost in a delightful little 1st-generation
CD-ROM too-close to hard-drive during backup experience that involved dancing
blue flames on the case of a hard disk :(

Most notably, this release is missing the full implementation of "daemons" and
the "mobile" system that is functional but buggy in AMUL 906, the last release.


# History

Every now and again I've dug this code up and poked/prodded at it, but usually
abandoned it for the fact not so much that it relied on the Amiga's IPC
systems like Message Ports but because it ultimately relied on the fact that
Amiga processes shared memory space and so could literally pass each other
pointers.

AMUL was my 3rd MUD system. The first and second were on the Atari ST. The
first was written in 68000 assembler and lead to my writing my own operating
system so that I didn't have to worry about multi-tasking in the game engine.

The second was written in C, in-fact it was how I taught myself C.

AMUL itself was written in ANSI K&R (C89) C, which mean't that by even the
late 90s, it made most compilers vomit walls of warnings and/or errors that
made it seem lightyears from operability.

My first attempts at an AMUL resurrection was "SMUGL" ("Simple Multi-User Game
Language") which was really just a C-with-Classes reworking of the original,
and it never was much more than a periodic hacking project.

28 years after the last AMUL release, I was inspired to try and see just how
far Visual Studio had come in terms of how feasible it would be to *quickly*
get some flavor of AMUL to compile, never mind run.

In the _master_ branch, I automatically started that process in C++ and I
also automatically began scope-creeping so that "compiling" continually became
further and further away. I just couldn't not deal with all the warnings.

But it did help me refresh myself on the code base.

Once I got amulcom compiling and largely compiling game code, I wondered:
Could I have gotten here by doing less? And I knew the answer was: Yes.

So I created this branch from the original commit, and I stuck to doing this
in C. I stuck to the focus of "make it compile before you make it pretty"
etc.

It took a handful of hours to get amulcom compiling and starting to compile
game code. Armed with knowledge of bug-fixes from master, I was quickly
able to pass the first obvious issues.

My big realization was the *why* of some of the organization, and I was
able to avoid many of my original refactoring mistakes by putting all of
amulcom into a single .c file instead of trying to shoe-horn it's component
sub .c files into behaving like stand-alone files. (In truth, they'd all
originally just been #included. I'm not sure why I gave them the .c suffix)


# Future

My goal, for now, is just to get it compiling the entire sample game. Perhaps
I'll go so far as to get the game manager and game engine compiling if not
running to some limited degree.

My main focus after that is likely to be refactoring amulcom to be token
based and to introduce some more 21st century features like dictionary
lookups etc.

I'm curious to see if I'll choose to stay with C or switch to C++. I have
half a mind to reimplement in golang - which is partly what's motivated me
to stick with basic C.

I also plan to replace the amulcom/aman/amul trio with a single binary
that translates the raw text files to memory and runs the game directly.

This was how AMUL was supposed to work originally but disk, memory and CPU
all hampered that goal.

