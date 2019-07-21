# AMUL : Amiga Multi-User games Language

# Development

AMUL is built using CMake and is developed for Windows and Linux. MacOS is
not officially supported but patches will be accepted. (Send me a Mac Mini
if you want me to directly support MacOS)

For the most part I do the bulk of my editing in Vim, VS Code and Studio.

## Windows

Just point VS 2019 or higher at this folder and the CMakeLists.txt.
TODO: VS Code howto.

## WSL

I use the Ubuntu base image with gcc, cmake and ninja, and create a build
directory called "wsl" from which I run `cmake -G Ninja ..`.

## Linux

Two Docker files are provided that will also serve as a description of the
packages required etc. I'm going to publish these as kfsone/amula and
kfsone/amulu respectively.

To launch: 

```sh
	docker run --rm -it -v /dir/to/amul:/amul kfsone/amula
	# mkdir linux
	# cd linux
	# cmake -G Ninja ..
	# cmake --build .
```

There is a copy of my vimrc file in etc/, in particular I use `:make` to do
my builds and then press the `=` key to skip between build lines.

# About

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

# AMUL Basics

AMUL is a language for writing text-based, multi-player advenure games (MUDs),
and it does it in an old-school way, so there are a lot of reinvented wheels.

For example: there's really no good reason, today, not just to throw the game
data into a database and be done with. Give it a good editor, maybe web based,
and you could do away with the text files too.

The structure of the data actually lends itself very well to such a thing.

Or, I could abandon the custom language and just use any of the extant forms,
because literally any of them would probably do OK: CSV, XML, JSON, YAML.

But that's all the shit we do day to day. What AMUL really is, at this point,
is an opportunity to dick around with stuff behind the scenes. Maybe when
the rest of it works, that'll be project #2.

# Adventure Games and MUDs

Adventure games were text-based puzzles where the world is described to you
in text, and you write out natural-language instructions to try to tell the
narrator (the game) what to do.

*	It is dark.
*	> turn the light on
*	It is no-longer dark. There is a door to the east.
*	> go east
*	To business! You rush eastward, only to be halted by the firmly closed door.
*	> open it
*	It's locked.
*	> unlock it
*	Well, why didn't you ask? Ok. It's unlocked.
*	> quit

MUDs range anywhere from being eclectic alternatives to IRC with themed chat
rooms, to being text-based world simulations.

The rooms in an adventure are typically just nodes in a form of graph. While
some MUD engines may have operated on a cell/grid basis, most MUDs just
let rooms link up however they like. The "size" of a room entirely depends
on it's description and connections.

*	West of the grand hall.
*	> east
*	Grand hall.
*	> east
*	East of the grand hall.
*	> southwest
*	Narrow passage south of the grand hall.
*	> west
*	Middle of a narrow passage, south of the grand hall.
*	> west
*	West end of a narrow passage, south of the grand hall.
*	> northwest
*	West of the grand hall.

In an adventure game, there is a clear, defined goal, and typically a fixed
path to achieving it. Most adventure games had fairly linear plots.

In a MUD, the multi-user nature makes things more dynamic, and in particular,
while there is a generally fixed "starting state" there is generally very
little expectation that any two sessions should be the same.

An adventure, then, is like having a single level that you can play once and
the game is over and that's it.

MUDs tended to have some criteria for a "reset", at which point the game world
reverted back to initial state. Since having your level randomly just kick
you out would be no fun, they introduced characters that persisted across
resets and followed the D&D model of letting you level and achieve ranks.

In many MUDs, the ultimate goal was to become the highest rank (Wizard/GM),
and have the ability to do matrix-like stuff to the non-wiz players.

PVP was a routine part of MUDs, usually in the form of permadeath.

# AMUL Structure

## Software

AMUL is broken into several components:

`amulcom`
: Compiler: text => data,
`aman`
: Manager: data -> in-memory representation,
`amul`
: Frame[^1]: the client

After compiling your game, you launch the manager, which then launches instances
of the 'frame' or client. It eventually also launched 2 special instances: one for
handling 'daemons' (background events) and one for NPCs.

## Game Components

A MUD is generally made up of several formal components:

"Room"s
: 1d locations in the game world with no formal dimension or coordinate systems,
"Object"s
: Anything that can exist or have a presence inside a room that isn't a player, including the more tangible elements of the room. Examples include doors, bags, treasure, even the weather.
Object States
: Each object has, essentially, one general-purpose variable controlling its properties in the game world. For instance, a "torch" might be on and luminous in one state; the weather might simply change between "It is snowing" and "It is raining" and being hidden as it changes states.
"Descriptions"
: The text that describes rooms, objects, etc.
"Messages"
: Everything else that might be sent to a player.
"Verbs" / "Language"
: A way to describe the commands players can enter and how those commands affect the player/world. AMUL takes a function-programming approach, in that you provide patterns that describe this verb and this combination of nouns, etc.
"Travel"/"Map"/"Exits"
: MUDs - like AMUL based ones - that use the 1D system need to be told how rooms connect/don't. This is
generally called the "travel table".


## Game Definition

Game Source comprises a series of text files that use a white-space oriented syntax
based on paragraphs, or blocks. Comments are denoted by ';' or (until I remove it again)
'*' which causes the comment to print at compile time.

Typically a "block" comprises:

	<category>=<identifier> [<options>]
	[sub-heading]
	[<optional indentation>] [... values ...]

The text files are as follows:

`title.txt`
: Describes general game configuration and then provides the title splash
`ranks.txt`
: Lists the ranks players can attain[^2]
`sysmsg.txt`
: Provides the text messages used by the game itself[^3].
`umsg.txt`
: For "out-of-band" string literals that would be annoying in the language file
`obdesc.txt`
: Long or frequently-repeated object descriptions (umsg for objects)
`objects.txt`
: Anything that can be used as a noun that is not a player is described here.
`rooms.txt`
: Describes the properties of the locations in the game and their text descriptions.
`travel.txt`
: Uses AMULs "c&a" (condition & action) language to describe which verbs take players between rooms or the side-effects they cause
`lang.txt`
: The main "c&a" file where you essentially build the parser that will interpret what players type.
`mobiles.txt`
: Describes NPC attributes that can then be associated with an object to create a "mobile" NPC
`syns.txt`
: I basically copied this name (synonyms), "aliases" would have been much better


# Language basics

Not going to describe the entire thing here. At it's best, AMUL can look a bit like JSON, using
key=value pairs to describe things. But in most cases the values actually represent a list and the
keys are opional, because disk space & memory, see.

And because the Amiga used `\r\n` as it's end of line, putting multiple lines onto one saved space.

*	cup|tvroom|250 10 0 0 "On the floor lies a small, silver cup." scaled

Not very elegant? This is the terse version of

*	noun=cup	; state-independent flags would be here
*		location=tvroom
*		weight=250 value=10 strength=0 damage=0 description="On the floor lies a small, silver cup." flags=scaled

Although the longer prefixes were yanked during a disk-space recovery at some point making it:

*	noun=cup	; state-independent flags would be here
*		location=tvroom
*		weight=250 value=10 str=0 dam=0 desc="On the floor lies a small, silver cup." scaled

The downside to this approach is that, in order to have blank lines between logical blocks or in paragraphs of text, you have to have a non-empty line. To deal with this, I typically used tab indentation. Otherwise, the indentation in AMUL is optional. This is _not_ Python :)


## SysMsg, UMsg, ObDescs

The simplest files:

```
	[<type>=]<identifier>
	<paragraph of text>
```

sysmsg example:
```
	msgid=$1		; system message IDs are prefixed '$' and numbered
	Welcome to %s.
```

umsg example:
```
	msgid=cantdothat	; this is going to be invoked a lot.
	You can't do that!

	fool				; id.
	Fool!
```

obdescs example:
```
	msgid=torch_lit
	The torch is lit.
```

## Rooms

Rooms is another simple file:

```
	[room=]<shortname> [<flags>]
	<one line description>
	[<long description>]
```

e.g.

```
room=hroom5 dark small
	Room 5.
	Unremarkable as it is, this tiny room seems quite cosy. You can almost picture crashing out on the small, single bed that would once have taken up the bulk of the room. However, the bed and all the other fixtures and fittings have long since been removed. The only remarkable point of the room now is the doorway in the western wall.
```

Points of note from this:

- Rooms are `light` by default,
- `small` restricts entry by "travel" actions to 1 person; there's a gm power that lets you bypass this.
- Room identifiers are often known to players.
- By default, players see the short description every time they "look" at a room (inc entering a location),
  but only see the long description when the "look" action is used or they haven't visited it before
  during this session.
- The long description has several "fixtures" mentioned: bed, doorway, wall. When a player visits the room,
  they will be given a list of the items present. There will likely be "invisible" (no description)
  objects called "bed", "door", "doorway" and "wall" that have a presence here so that players can type
  'touch the wall', for example, and not have the parser say "whats a wall?"

## Travel

The travel table is, effectively, a room-specific version of the language file, and is easier to explain first.

```
	[room=]<room id> [<room id...>]
	[verb=]<verb1 [... verbN]]>
	[<condition [<parameters>]>] <destination | action [<parameters>]>
```

Example:

```
	room=westend
	verb=east
		eastend			; the player will go east with no fuss
	verb=west
		; can only go west if the door is "open" (state 0)
		; 'respond' stops parsing here
		if the state of the door is 0 then respond "The door is closed"
		; which makes this an 'else'
		go outside      ; "go" is ignored

	room=eastend
	verb=east west
		respond "You're stuck here now, muahahaha"
```

## Language

The C&A language for is broken down into "verbs" and then further broken down into
pattern matching expressions I called "syntax"es.

```
	[verb=]<verb> [travel] [dream] [object predecende]
	[syns=[<synonym list -- not implemented>]]
	[usage="<help text>" -- not implemented]
	syntax=none | any | <pattern>
		[condition] <action | room>
	[syntax= ...]
```

The runtime parser is going to take what the user types and tokenize it. Some of the words will have more than one meaning (verb, noun, adjective) and it is going to try and find the best fit.

One of these components is the precedence ("chae" or "object sort"). If you say "get match", you probably mean the match on the floor vs the one in your inventory. Part of the runtime parser's job is to figure that out.

To save the parser a lot of back-tracking, you can provide it a hint: this verb looks for things nearby first. Whereas 'drop' most definitely wants to consider the one you're carrying first.

("CHAE" = Carried, Here, Another has it, and Elsewhere)

The "pattern" is actually fairly simple. Consider the sentence: `pot the plant in the plant pot`. What the parser sees is: `pot plant plant pot`, and it needs to correctly categorize the tokens as:

```
	verb:pot
	noun: plant
	adjective: plant
	noun: pot
```

AMUL has 6 placeholder slots for tokens, which are:

```
	verb adj1 noun1 prep adj1 noun2
```

and the syntax line patterns are all about various degrees of matching to each of those slots.

```
	verb=test
	syntax=none
		respond "You typed test all on its own."
	syntax=any
		reply "Ok, you said test <something>"  ; non-terminal
	syntax=verb noun
		reply "You want to test a thing"
	sytax=verb player
		reply "You want to test a person?"
	syntax=verb player=self
		respond "You want ME to test you?"		; terminal
	syntax=verb any noun
		respond "I'm guessing you want to test *with* the @n2?"
```

I snuck the @n2 in here to note that AMUL has a text-variable system that lets you have various placeholder words. In this case, you might expect the following:

*	> test
*	You typed test all on its own.
*	> test me
*	You want ME to test you?
*	> test me bro
*	I'm guessing you want me to test *with* the bro?

We can special case that by being more specific with a pattern:

```
	syntax=verb any noun=bro
		respond "I'm bringing it!"
	syntax=verb any noun
		respond "I'm guessing you want to test *with* the @n2?"
```

The other approach to this would be to use a condition. This is also actually an interesting insight into the parser. When AMUL finds a partial match, there are a number of idempotent tests it will do that can be used to force the parser to give-up backtracing and accept this branch. Or in other words, when the player does not type "test me bro", doing the test has not forced the parser to commit to this syntax block.

```
	syntax=verb any noun
		if noun1 is bro then respond "I'm bringing it!"
		else respond "I'm guessing you want to test *with* the @n2?"
```

Finishing with a demonstration of the "gloss" words that are syntactic sugar but entirely optional.



[^1]: 'Frame' is a term from the Bulletin-Board System era. BBS software typically
had direct control over the modem hardware, so in order for an external
application to communicate with the user, the BBS had to act as an intermediary.
The API that was first introduced for this was called 'Doors' (I guess whoever
coined it was a twilight zone fan) and the API layer itself was the 'Frame'.

'amul.c' was going to be a thin client that handled parsing and sent the
resulting token streams to the manager. Didn't play out that way, though.

[^2]: In 'SMUGL' I moved the title screen into 'title.text' and moved the
config elements along with the ranks into 'system.txt'. I really sucked at
naming things.

[^3]: Except where I was lazy and hardcoded it. Also, the player *has* to provide
sysmsg.txt, which was part laziness and part memory/disk pressure during my
original floppy-based development.