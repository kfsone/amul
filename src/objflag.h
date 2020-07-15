#pragma once
#ifndef AMUL_OBJFLAG_H
#define AMUL_OBJFLAG_H

// Object flag bits
#define OF_OPENS 1       // Object is openable
#define OF_SCENERY 2     // Object is scenery
#define OF_COUNTER 4     // Ignore me!
#define OF_FLAMABLE 8    // Can we set fire to it?
#define OF_SHINES 16     // Can it provide light?
#define OF_SHOWFIRE 32   // Say 'The <noun> is on fire' when lit
#define OF_INVIS 64      // Object is invisible
#define OF_SMELL 128     // Object has a smell not visual
#define OF_ZONKED 32768  // Object was zonked!

// Object parameter flag no.'s
enum ObjectParameter { OP_NONE = -1, OP_ADJ, OP_START, OP_HOLDS, OP_PUT, OP_MOB };

// Object/state flags
#define SF_LIT 1      // Object is lumious
#define SF_OPEN 2     // Object is open
#define SF_CLOSED 4   // Object is closed
#define SF_WEAPON 8   // Its a weapon
#define SF_OPAQUE 16  // Can see inside object
#define SF_SCALED 32  // Scale the value
#define SF_ALIVE 64   // Mobile/Animated

#endif  // AMUL_OBJFLAG_H

