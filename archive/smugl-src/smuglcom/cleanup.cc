/*
 * cleanup function; an important part of the processing mechanism,
 * it prepares and formats a text block to make it easier to
 * process (e.g. you don't have to say isspace(*p) or
 * if (*p == ' ' || *p == 9)
 *
 * $Log: cleanup.cc,v $
 * Revision 1.4  1997/05/22 02:21:34  oliver
 * Many and various: However, this implements a 'work in progress' commit --
 * the parser is still 'debug only':
 *
 * . All .C files renamed to .cc for DOS compatability
 * . All .h files renamed to .hpp for DOS compatability
 * . Changed all #include statements accordingly
 *  - breaks the cvs history a little :-(
 * . Begun adding 'masculine=', 'feminine=', 'neuter=' and 'self='
 *   options in system.txt -- these allow definition of 'special case'
 *   words for non-english users.
 *  - not implemented properly
 *  - the parser doesn't understand them yet
 *  + added to the 'default' system.txt generated by smuglcom
 *  + added to test/system.txt
 *  - need to add sensible defaults, otherwise these are a nuisance
 * . Added 'noise=' verb to system.txt; this allows user to define
 *   'noise words'. This is implemented using aliases with a 'meaning'
 *   of -1, which tells the parser to ignore them
 *  + modified synsproc.cc to cope with this
 *  + added to the 'default' system.txt generated by smuglcom
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
 * Revision 1.3  1997/04/26 21:43:23  oliver
 * . Converted all .c files into .C files
 * . Added room count flags
 * . Renamed all '_(.*)_STRUCT' structures to \1
 * . Various other cosmetic internal changes
 * . Some minor improvements
 *
 */

#include "smuglcom.hpp"

#define fQ (-1) /* Handle quotes */
#define fC (-2) /* Handle a comma */
#define fM (-3) /* Handle a comment */
#define fS (-4) /* Handle a line extension (usually 'slash') */
#define fR (-5) /* Handle \r */

/* List of replacement characters. For each possible character,
** the entry in this table specifies the replacement character.
** Negative values result in a function call, based on the 'f' defines
** above
*/
static const char repl[256] = {
    /*      0---1---2---3---4---5---6---7---8---9---10--11--12--13  */
    0,
    32,
    32,
    32,
    32,
    32,
    32,
    0,
    0,
    32,
    EOL,
    32,
    0,
    fR,
    /*      14--15--16--17--18--19--20--21--22--23--24--25--26--27  */
    /*      ....................................................esc */
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    32,
    0,
    /*      28--29--30--31--32--33--34--35--36--37--38--39--40--41  */
    /*      ................ ...!..."...#...$...%...&...'...(...)   */
    32,
    32,
    32,
    32,
    0,
    0,
    fQ,
    0,
    0,
    0,
    0,
    fQ,
    0,
    0,
    /*      42--43--44--45--46--47--48--49--50--51--52--53--54--55  */
    /*      *...+...,...-..'.'../...0...1...2...3...4...5...6...7   */
    0,
    fS,
    fC,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    /*      56--57--58--59--60--61--62--63--64--65--66--67--68--69  */
    /*      8...9...:...;...<...=...>...?...@...A...B...C...D...E   */
    0,
    0,
    0,
    fM,
    0,
    0,
    0,
    0,
    0,
    'a',
    'b',
    'c',
    'd',
    'e',
    /*      70--71--72--73--74--75--76--77--78--79--80--81--82--83  */
    /*      F...G...H...I...J...K...L...M...N...O...P...Q...R...S   */
    'f',
    'g',
    'h',
    'i',
    'j',
    'k',
    'l',
    'm',
    'n',
    'o',
    'p',
    'q',
    'r',
    's',
    /*      84--85--86--87--88--89--90--91--92--93--94--95--96--97  */
    /*      T...U...V...W...X...Y...Z...[...\       */
    't',
    'u',
    'v',
    'w',
    'x',
    'y',
    'z',
    0,
    fS
};

/* Turn a block of text from pretty much 'free format' into a fairly
** standardised format that reduces the number of conditionals that
** the compiler has to do when looking at text
*/
void
clean_up(char *p)
{
    char c, *s, *start, qt;
    if (!*p)
        return;
    start = s = p;
    while (*p) {
        c = (*s = *(p++));
        if (c > 93) {
            if ((u_char)(c) > 127)
                *s = ' ';
        } else {
            switch ((c = repl[(int) c])) {
                case 0: /* Do nothing */
                    break;

                case fQ:     /* Handle quoted text */
                    qt = *s; /* Quote character */
                    /* Find the close quote, or EOL */
                    while (*p && *p != qt && repl[(int) *p] != EOL)
                        *(++s) = *(p++);
                    if (*p == qt) /* If we ended on a quote char, skip it */
                        *(++s) = *(p++);
                    break;

                case fC: /* Handle a comma */
                    if (repl[(int) *p] == EOL)
                        s--; /* Comma at EOL is uneccesary, remove it */
                    else
                        *s = EOL; /* Otherwise make it an EOL */
                    break;

                case fM: /* Handle a comment */
                    s--; /* Remove the comment character */
                    if (s > start && *s == EOL)
                        s--; /* Remove comment-only lines completely */
                    while (*p && repl[(int) (*p)] != EOL)
                        p++;
                    break;

                case fS:                       /* Handle \ or + at EOL */
                    if (repl[(int) *p] == EOL) /* Useless unless at EOL */
                    {
                        *s = ' '; /* Remove the extend character */
                        p++;      /* But also forget about the EOL character */
                    }
                    break;

                case fR:            /* Handle a \r */
                    if (*p == '\n') /* Reduce \r\n to EOL */
                        p++;
                    *s = EOL;
                    break;

                default: /* Replace */
                    *s = c;
                    break;
            }
        }
        s++;
    }
    *s = 0;
}
