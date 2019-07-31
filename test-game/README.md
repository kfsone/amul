# Test Game

A quick introduction:

title.txt -- game config and title screen
sysmsg.txt -- all player-facing built-in responses/prompts,
ranks.txt -- describes the levels players can achieve
rooms.txt -- describes the locations (rooms) that make up the world
travel.txt -- describes connections (travel commands) that link rooms
objects.txt -- the items that can be found in the game world and nouns the player might interact with
lang.txt -- a functional-programming style description of the things a player can do in this world
obdescs.txt -- for long or recurring object descriptions (assign them ids)
umsg.txt -- for long/recurring user-defined messages (assign them meaningful ids)
mobiles.txt -- describe the "personality" for any npcs (the npc itsellf will be an object)
syns.txt -- aliases (synonyms)

pure text files:

reset.txt -- sent to players when the game restarts. why this isn't a message I don't know.
scenario.txt -- displayed the first time a new character is played
