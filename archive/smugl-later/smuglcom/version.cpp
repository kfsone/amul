/* SMUGLCom version information
 * $Log: version.cc,v $
 * Revision 1.4  1997/05/22 02:21:44  oliver
 * Many and various: However, this implements a 'work in progress' commit --
 * the parser is still 'debug only':
 *
 * . All .C files renamed to .cc for DOS compatability
 * . All .h files renamed to .hpp for DOS compatability
 * . Changed all #include statements accordingly
 *  - breaks the cvs history a little :-(
 * . Begun adding 'masculine=', 'feminine=', 'neuter=' and 'self='
 *   options in system.smg -- these allow definition of 'special case'
 *   words for non-english users.
 *  - not implemented properly
 *  - the parser doesn't understand them yet
 *  + added to the 'default' system.smg generated by smuglcom
 *  + added to test/system.smg
 *  - need to add sensible defaults, otherwise these are a nuisance
 * . Added 'noise=' verb to system.smg; this allows user to define
 *   'noise words'. This is implemented using aliases with a 'meaning'
 *   of -1, which tells the parser to ignore them
 *  + modified synsproc.cc to cope with this
 *  + added to the 'default' system.smg generated by smuglcom
 *  - need to add sensible defaults, otherwise these are a nuisance
 * . Now use static member functions for the various "Idx" Classes,
 *   e.g. RoomIdx, such that calls have changed;
 *   RoomIdx.locate() becomes RoomIdx::locate()
 * . Parser is now capable of tokenising input:
 *  + understands aliases
 *  + understands (and ignores) aliases of -1
 *  - stops on too many symbols (doesn't do anything special)
 *  - no processing of tokens yet
 *  - nothing like old AMUL parser (means a lot of work)
 *  - old AMUL code is illegible - who wrote it, git?
 *
 * Revision 1.3  1997/04/26 21:43:30  oliver
 * . Converted all .c files into .C files
 * . Added room count flags
 * . Renamed all '_(.*)_STRUCT' structures to \1
 * . Various other cosmetic internal changes
 * . Some minor improvements
 *
 * Revision 1.2  1997/04/25 01:13:44  oliver
 * Reset ID tag so CVS assigns us real version numbers
 *
 * Revision 1.1.1.1  1997/04/25 00:16:50  oliver
 * rel_0_3_23
 *
 * Revision 1.3  1997/04/23 01:12:07  oliver
 * Whole stack of changes to make SMUGLCOM compile with a full set of
 * warning options enabled - hopefully to avoid any sneaky bugs
 *
 */

#include "revision.h"  // From the build directory.

#include "smuglcom.hpp"

extern const char vername[] = "SMUGLCom " Smugl_BUILD_BUILT;  // Version name string
