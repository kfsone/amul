#pragma once
// This may look like C, but it's really -*- C++ -*-
// $Id: smugl.hpp,v 1.8 1999/09/10 15:57:31 oliver Exp $
// smugl defines

#include "data.hpp"
#include "includes.hpp"
#include "structs.hpp"
#include "textproc.hpp"
#include "vocab.hpp"

//// Configuration Options. Don't edit configuration values in other
//// files unless you really know what you're doing!

// Determine which address and port to listen for commands
#define COMMAND_ADDR INADDR_LOOPBACK
//#define COMMAND_ADDR INADDR_ANY // Use if you don't have a loopback interface

// Define the default terminal behaviour
// (uf* flags are defined elsewhere)
#define DLLEN 80       // Default line length (screen width)
#define DSLEN 24       // Default screen length (number of lines)
#define DFLAGS ufCRLF  // Default = cr/lf ON & auto-redo on

// Define the "Internal Error" message
#define INTERNAL_ERROR "Internal Error - Exiting\n"

//// OK - Stop editing now - don't edit below this line, unless
//// you really know what you're doing! :-)

#define HEAVYDEBUG(x)                                                                              \
    if (g_heavyDebug)                                                                              \
    tx(x)

#include <time.h>

// For indecision stuff
union Indecision {
    class Room *Rm;
    class Mobile *Mob;
    class Object *Ob;
    class Verb *Vb;
};

enum ExitCause { ecFalse, ecQuit, ecDied };

// Important globals
extern char g_dir[];         // from libsmugl
extern char g_input[];       // from smugl.C
extern bool g_manager;       // boolean: are we the server?
extern int g_slot;           // if (!manager) - our slot number
extern int g_debug;          // Debugging level
extern char g_fork_on_load;  // if (fork_on_load), fork after initialising
extern int g_exiting;
extern bool g_heavyDebug;

// Important global variables
extern void txprintf(const char *, ...);
extern void tx(const char *s, char c = 0);
extern void txc(char);
extern void announce(long to, const char *msg);
extern void announce(long to, long msg);
extern void announce_into(long to, const char *msg);
static inline void
announce_into(long to, long msg)
{
    announce_into(to, message(msg));
}

// Macros
#define ALLBUT(i) ~(1 << (i))                 // Bit mask for that excludes 'i'
#define ONLY(i) (1 << (i))                    // Bit mask for *only* 'i'
#define BOTH(i, j) ((1 << (i)) | (1 << (j)))  // Bit mask for two people

// Misc enum's
enum { READfd, WRITEfd };  // For reading/writing pipe fd's

#define ufANSI 0x001  // ANSI bit
#define ufCRLF 0x002  // Add LineFeed
