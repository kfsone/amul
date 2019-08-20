# TODO List

- Don't create txt files unless the user requests it.

## Stats Fix
- Move rank/player/avatar stats into a single struct or dictionary,
- Add "last reset" to player record,
- Change avatar stats to modifiers that last only for a given reset,
 - If a player loses 10 points of strength, they mustn't be able to restore it simply by logging out/in again,
 - If a player loses 10 points of strength, they mustn't retain it if they log in after a reset,
 - Store deltas and which "reset" session they happened in, and clear them when the player relogs,

## General
- Move player records to some kind of database rather than managing it on our own,
- Assign GUIDs to players,
- Replace 'slotid' with a session ID so we can prevent actions intended for A being
  applied to B because they logged in and got the same slot instantaneously,
- Reduce Avatar / Player to one class (inherit Avatar -> Player or make it a member),
- Create dictionary (registry of ALL words),
- Convert lang_proc to SourceFile,
- Use ParserContext for parsing/lang execution,
- Network IO,

## Const all the things:

- Restrict non-const versions of GetObject, GetRoom etc to the main thread,
- Move all modification work to the main thread,
