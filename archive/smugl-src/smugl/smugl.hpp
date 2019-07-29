// This may look like C, but it's really -*- C++ -*-
// $Id: smugl.hpp,v 1.8 1999/09/10 15:57:31 oliver Exp $
// smugl defines
//

#include "includes.hpp"
#include "structs.hpp"
#include "vocab.hpp"
#include "data.hpp"
#include "textproc.hpp"

//// Configuration Options. Don't edit configuration values in other
//// files unless you really know what you're doing!

// Determine which address and port to listen for commands
#define COMMAND_ADDR INADDR_LOOPBACK
//#define COMMAND_ADDR INADDR_ANY // Use if you don't have a loopback interface

// Define the default terminal behaviour
// (uf* flags are defined elsewhere)
#define DLLEN 80                // Default line length (screen width)
#define DSLEN 24                // Default screen length (number of lines)
#define DFLAGS ufCRLF           // Default = cr/lf ON & auto-redo on

// Define the "Internal Error" message
#define INTERNAL_ERROR "Internal Error - Exiting\n"

//// OK - Stop editing now - don't edit below this line, unless
//// you really know what you're doing! :-)

#define HEAVYDEBUG(x) if (heavyDebug) tx(x)

#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME
#  include <sys/time.h>
# elese
#  include <time.h>
# endif
#endif

// For indecision stuff
union Indecision
    {
    class Room *Rm;
    class Mobile *Mob;
    class Object *Ob;
    class Verb *Vb;
    };

enum ExitCause { ecFalse, ecQuit, ecDied };

// Important globals
extern char dir[];              // from libsmugl
extern char input[];            // from smugl.C
extern char manager;            // boolean: are we the server?
extern char slot;               // if (!manager) - our slot number
extern char debug;              // Debugging level
extern char fork_on_load;       // if (fork_on_load), fork after initialising
extern int exiting;
extern int heavyDebug;

// Important global variables
extern void txprintf(const char *, ...);
extern void tx(const char *s, char c=0);
extern void txc(char);
extern void announce(long to, const char *msg);
extern void announce(long to, long msg);
extern void announce_into(long to, const char *msg);
static inline void announce_into(long to, long msg) { announce_into(to, message(msg)); }

// Macros
#define ALLBUT(i) ~(1<<i)       // Bit mask for that excludes 'i'
#define ONLY(i) (1<<i)          // Bit mask for *only* 'i'
#define BOTH(i, j) ((1<<i)|(1<<j)) // Bit mask for two people

// Misc enum's
enum { READfd, WRITEfd };       // For reading/writing pipe fd's

#define ufANSI  0x001           // ANSI bit
#define ufCRLF  0x002           // Add LineFeed

extern bool quit();
extern void look();

void set_version_string();
