// text processing routines
// This file is for 'stand alone' functions that don't otherwise fit into
// the object hierachy.

#include <cctype>
#include <cstring>

#include "actuals.hpp"
#include "parser.hpp"
#include "ranks.hpp"
#include "rooms.hpp"
#include "smugl.hpp"

extern char vername[];

char *out_buf;         // Generic output buffer
long out_buf_len = 0;  // Length of data in output buffer
long out_bufsz;        // Total size of outbuf memory allocation

const char *message(msgno_t id)  // Convert umsg id into a string pointer
{
    if (id & IWORD)  // It's actually a player-context word
    {
        if (id == -1 || id == -2)
            return "\0";
        return "<someword>";
    }
    return (char *) (data->msgbase[id]);  // De-reference a message id
}

const char *rightstr(const char *s, int len)  // Return EOS - len characters
{
    len = strlen(s) - len;
    if (len <= 0)
        return s;
    else
        return (s + len);
}

/* Case insensitive, word-based, 'considered' string match.
 * Returns: 0 on match, -1 on non-match.
 * . Consider space and underscore to be equals; so if either string
 *  contains either character in the same position, consider a match.
 * . The match must exhaust string 1, but it's still a match if
 *  string 2 is at end of word (i.e. a null or a space). Thus:
 *
 *#T   match("fred bloggs", "fred bloggs friend") == 0
 *#T   match("fred bloggs", "fred bloggsy") == -1
 *#T   match("to_day", "today") == -1
 *#T   match("to day", "to_day is the day") == 0
 */
int
match(const char *s1, const char *s2)
{
    while (*s1) {
        char c1 = *(s1++);
        char c2 = *(s2++);

        if (c1 == c2)  // Keep looping if we find an accepted match

            continue;
        if ((c1 == ' ' || c1 == '_') && (c2 == ' ' || c2 == '_'))
            continue;
        if (tolower(c1) == tolower(c2))
            continue;

        return -1;  // We didn't find a match, so we failed
    }
    /* Finally, make sure that both strings have 'ended'. 's2' should
     * now point to either a null byte or a space character */
    if (*s2 && *s2 != ' ')
        return -1;
    return 0;
}

#ifdef NEVER
char *
strcopy(char *to, char const *from)
{
}
#endif

// Process an '@XX' escape code, and write the apropriate string
// into the 'to' area.
// XXX: currently no boundary checking implemented - worrisome?
char *
esc(const char *code, char *to)
{
    // All escape codes are two characters long; grab both.
    char first = tolower(*code);
    char second = tolower(*(code + 1));

    switch (first) {
        case 'a':
            if (second == 'd')  // @ad = ADventure name
                return strcopy(to, data->name);
            if (second == 'n')  // @an = Author Name (Oliver)
                return strcopy(to, "Oliver Smith <oliver@kfs.org>");
            if (second == 'v')  // @av = Adventure Version
                return strcopy(to, vername);
            break;

        case 'c':
            if (second == 'r')  // @cr = <return>
                return strcopy(to, "\n");
            break;

        case 'e':
            if (second == 'x')  // @ex = my experience
            {
                return (to + sprintf(to, "%ld", me->experience));
            }
            break;

        case 'f':  // @f codes are for <friend> - person you are helping
            //            if(c=='r' && me->helping!=-1) { strcpy(s,(usr+me->helping)->name); return
            //            1; } if(c=='m' && me->followed!=-1) { strcpy(s,(usr+me->followed)->name);
            //            return 1;
            //            }
            return strcopy(to, "no-one");

        case 'g':  // Gender words
            switch (second) {
                case 'n':  // @gn = gender name
                    return strcopy(to, (me->sex) ? "female" : "male");
                case 'e':  // @ge = he/she
                    return strcopy(to, (me->sex) ? "she" : "he");
                case 'o':  // @go = gender for owner, i.e. his/her
                    return strcopy(to, (me->sex) ? "her" : "his");
                case 'h':  // @gh = him/her
                    return strcopy(to, (me->sex) ? "her" : "him");
                case 'p':  // @gp = games player
                    return (to + sprintf(to, "%d", me->plays));
            }
            break;

        case 'h': /* The person helping you */
            //            if(c=='e' && me->helped!=-1) { strcpy(s,(usr+me->helped)->name); return 1;
            //            }
            break;

        case 'l':
            if (second == 'r')  // @lr = last reset time
                return strcopy(to, data->lastres);
            if (second == 'c')  // @lc = last compile time
                return strcopy(to, data->lastcrt);
            break;

        case 'm':
            switch (second) {
                case '!':  // @m! = padded version of my name
                    return (to + sprintf(to, "%-21s", me->name()));
                case 'e':  // @me = my name
                    return strcopy(to, me->name());
                //          case 'f': if(me->following==-1) strcpy(s,"no-one"); else
                //          strcpy(s,(usr+me->following)->name); return 1;
                case 'g':  // @mg = magic points
                    return (to + sprintf(to, "%ld", me->magicpts));
                case 'r':  // @mr = my rank
                    return myRank->copy(to);
            }
            break;

        case 'n':
            // if(c=='1' && inoun1>=0 && wtype[1]==WNOUN){
            // strcpy(s,(obtab+inoun1)->id); return 1;
            //}
            // if(c=='1' && wtype[1]==WTEXT){
            // strcpy(s,(char *)inoun1); return 1;
            //}
            // if(c=='1' && inoun1>=0 && wtype[1]==WPLAYER){
            // strcpy(s,(usr+inoun1)->name); return 1;
            //}
            // if(c=='2' && inoun2>=0 && wtype[3]==WNOUN){
            // strcpy(s,(obtab+inoun2)->id); return 1;
            //}
            // if(c=='2' && wtype[3]==WTEXT){
            // strcpy(s,(char *)inoun2); return 1;
            //}
            // if(c=='2' && inoun2>=0 && wtype[3]==WPLAYER){
            // strcpy(s,(usr+inoun2)->name); return 1;
            //}
            return strcopy(to, "something");

        case 'o':  // @o1 / @o2 = weapons
            // if(c=='1' && me->wield!=-1) { strcpy(s,(obtab+(me->wield))->id); return 1; }
            // if(c=='2' && (lstat+(me->fighting))->wield!=-1) {
            // strcpy(s,(obtab+((lstat+(me->fighting))->wield))->id); return 1; }
            return strcopy(to, "bare hands");

        case 'p':
            // if(c=='l') { strcpy(s,(usr+me->fighting)->name); return 1; }
            if (second == 'w')  // @pw = my password (bad idea really)
                return strcopy(to, me->passwd);
            if (second == 'o')  // @po = players online
            {
                // Capture the 'online' count so it doesn't change
                int online = data->connected;
                // Believe it or not, but 'data->connected' could change
                // during the sprintf below; and it seems pointless to
                // lock sem_DATA for *this*
                if (online == 1)
                    return strcopy(to, "are no other players");
                else
                    return (to + sprintf(to,
                                         "%s %d other %s",
                                         (online > 2) ? "are" : "is",
                                         online - 1,
                                         (online > 2) ? "players" : "player"));
            }
            break;

        case 'r':
            switch (second) {
                    // case 'e':	timeto(s,*rescnt); return 1;
                case 'd':  // @rd = room's short description
                    if (cur_loc->s_descrip != -1)
                        return strcopy(to, message(cur_loc->s_descrip));
                    else
                        return strcopy(to, "<somewhere>");
                case 'm':  // @rm = room id of my location
                    return strcopy(to, word(cur_loc->id));
            }
            break;

        case 's':
            switch (second) {
                case 'c':  // @sc = my score
                    return (to + sprintf(to, "%ld", me->score));
                    // case 'g':         // @sg = score this game
                    // sprintf(to, "%ld", me->sctg);
                    // return to + strlen(to);
                case 'r':  // @sr = my strength
                    return (to + sprintf(to, "%ld", me->strength));
                case 't':  // @st = my stamina
                    return (to + sprintf(to, "%ld", me->stamina));
            }
            break;

        case 't':
            if (second >= '0' && second <= '9') {
                int tok = second - '0';
                switch (token[tok].type) {
                    case tokSTRING:
                        return strcopy(to, token[tok].ptr);
                    case tokWORD:
                        return strcopy(to, word(token[tok].id));
                    case tokUNK:
                        return strcopy(to, "<empty token>");  // XXX
                    default:
                        return strcopy(to, "<broken token>");  // XXX
                }
            }
            break;

        case 'v':
            switch (second) {
                case 'b':
                    if (token[0].type == tokWORD)
                        return strcopy(to, word(token[0].id));
                    else if (token[0].type == tokSTRING)
                        return strcopy(to, token[0].ptr);
                    else
                        return strcopy(to, "<some word>");
            }
            // if(c=='e') { strcpy(s,(vbtab+iverb)->id); return 1; }
            // if(c=='1' && inoun1>=0 && wtype[1]==WNOUN) {
            // sprintf(s,"%ld",scaled(State(inoun1)->value,State(inoun1)->flags)); return 1; }
            // if(c=='2'
            // && inoun2>=0 && wtype[3]==WNOUN) {
            // sprintf(s,"%ld",scaled(State(inoun2)->value,State(inoun2)->flags)); return 1; }
            break;

        case 'w':
            // if(c=='1' && inoun1>=0 && wtype[1]==WNOUN) {
            // sprintf(s,"%ldg",((obtab+inoun1)->states+(long)(obtab+inoun1)->state)->weight);
            // return 1; } if(c=='2' && inoun2>=0 && wtype[3]==WNOUN) {
            // sprintf(s,"%ldg",((obtab+inoun2)->states+(long)(obtab+inoun2)->state)->weight);
            // return 1;
            // }
            if (second == 'i')  // @wi = my wisdom
                return (to + sprintf(to, "%ld", me->wisdom));
            break;

        case 'x':
            // if (c=='x')
            //    strcpy(s,mxx);
            // if (c=='y')
            //    strcpy(s,mxy);
            // return 1;
            break;
    }
    return nullptr;
}

// The smugl equivalent of 'printf'. Takes an input string and prepares
// it for writing. Because the string may contain '@' codes, we have
// to provide an output buffer for the string, so we build one on the fly
// and convert the text as we go, also handling '^x' sequences (control codes)
void
ioproc(const char *str)
{
    if (!out_buf)  // No output buffer currently
        out_buf = (char *) malloc((out_bufsz = OBLIM));
    char *p = out_buf;  // Local pointer
    // Figure out where we need to start looking at more memory.
    char *high_water = out_buf + (out_bufsz * 4 / 3);

    /* Read through until we get to an end of line mid-way thru buffer
     * We can afford to whack a lot of short lines in the buffer, but
     * we try to avoid filling the buffer */
    while (*str) {
        if (p > high_water)  // Do we need more memory?
        {
            long off = p - out_buf;                          // Offset of 'p' relative to out_buf
            out_bufsz *= 2;                                  // Twice as much memory
            out_buf = (char *) realloc(out_buf, out_bufsz);  // New memory chunk
            p = out_buf + off;  // Make p a pointer within new block again
            high_water = out_buf + (out_bufsz * 4 / 3);
        }
        if (*str == '\r') {  // Ignore cr's
            str++;
            continue;
        }
        char thisc = *(str++);
        char *advance;
        /* Escape code: Process it, and find the end of the output */
        if (thisc == '@' && (advance = esc(str, p)) != nullptr) {
            p = advance;  // Move to end of string
            str += 2;     // Skip the escape code (2 characters)
            continue;
        } else if (thisc == '^') {  // Control-character escapes
            char ctrl = 0;
            switch (tolower(*(str++))) {
                case 'g':  // CTRL-g
                    ctrl = '\007';
                    break;

                case 'h':  // Backspace
                    ctrl = '\010';
                    break;

                case 'i':  // Tab
                    ctrl = '\t';
                    break;

                case 'j':  // End-of-line
                    ctrl = EOL;
                    break;

                case 'l':  // Page feed
                    ctrl = '\014';
                    break;

                case 'm':  // End-of-line too. Bah
                    ctrl = EOL;
                    break;

                default:
                    ctrl = '^';
            }
            *(p++) = ctrl;
            continue;
        }
        *(p++) = thisc;
    }
    *p = 0;
    out_buf_len = p - out_buf;
}
