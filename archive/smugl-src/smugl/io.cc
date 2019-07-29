// base IO routines

static const char rcsid[] = "$Id: io.cc,v 1.12 1999/05/26 11:42:07 oliver Exp $";

#include "smugl.hpp"
#include "client.hpp"
#include "misc.hpp"
#include "ipc.hpp"
#include "io.hpp"

char lastc = 0;                 // Remember last character sent us
char in_input = FALSE;          // Initially we're not inputting
char txnest = FALSE;            // So tx can tell when it calls itself
char addcr = FALSE;             // When true, calls to 'tx()' are interrupts
char txed  = FALSE;             // So you know if you were interrupted
char xpos = 0;                  // Horizontal cursor position
char ypos = 0;                  // Vertical cursor position
long last_prompt = 0;           // Last string used to prompt user
char trailing_ws = FALSE;       // Trailing white space flag

char temp[10240];               // Space for output strings
 
// error() reports an error to stderr, and/or syslog
void
error(int pri, const char *msg, ...)
    {
    va_list va;
    va_start(va, msg);
    fprintf(stderr, ">> ");
#ifdef HAVE_VSYSLOG
    vfprintf(stderr, msg, va);
    vsyslog(pri, msg, va);
#else
    int bytes = vfprintf(stderr, msg, va);
    char *mem = (char *)malloc(bytes + 1);
    if (mem != NULL)
        {
        vsprintf(mem, msg, va);
        syslog(pri, mem);
        free(mem);
        }
    else
        syslog(LOG_CRIT, "out of memory for syslog(%d)", pri);
#endif
    fprintf(stderr, "\n");
    va_end(va);
    }

void
syslog_perror(const char *s)
    {
    syslog(LOG_ERR, "%s: %m", s);
    perror(s);
    }

/* ans(string) - sends an ansi string if the user has ansi capability */
void
ans(const char *s)
    {
    if (!(me->flags & ufANSI))
        return;
    int old_addcr = addcr;
    int old_xpos = xpos;
    xpos = 0;                   // So we don't do a line-wrap
    addcr = FALSE;              // So we don't add a cr to start
    txc(27);                    // Escape
    txc('[');                   // Open square bracket
    tx(s);                      // The string itself
    addcr = old_addcr;          // Put things back
    xpos = old_xpos;
    }

/* TX - process and print a string + optional character.
 * More often than not we want string + '\n', hence the paramters
 * to this function
 */
void
tx(const char *s, char c)
    {
    char tstr[OBLIM * 2];	// YEUCH! on the stack!?!
    char *p = tstr;
    char *old_out_buf = NULL;
    long old_out_bufsz = 0;
    const char *working_ptr;
    char *out;
    int cur_x, cur_y;
    int i;
    int slen;
    int llen;

    if (!*s)                    // Anything to print?
	return;
    txed = TRUE;                // flag that tx was called
    if (addcr && xpos)          // If this is an interruption,
	txc('\n');              // we may need to add a cr
    addcr = FALSE;              // ... but now we're sorted

    if (manager)
        {                       // If we're the manager, don't be clever
        fprintf(stderr, "%s%c", s, c);
        return;
        }

    // Make local 'llen' and 'slen' variables; use defaults if
    // we don't have a player structure or it's not set up yet
    slen = (me && me->slen && me->llen) ? me->slen : DSLEN;
    llen = (me && me->slen && me->llen) ? me->llen : DLLEN;

    // We may recursively call 'tx' (e.g. during a long, paged output
    // the call to press_ret() will generate a call to tx)
    // In these instances, we need to remember the old buffers
    if (txnest)
        {
        old_out_buf = out_buf;
        old_out_bufsz = out_bufsz;
        }

    if (*s == 12)               // Allow form feed at start-of-string
	txc(*(s++));
    working_ptr = s;            // For working through the input string
    cur_y = 0;                  // Lines we've printed

    ioproc(working_ptr);        // Process the text for output
    out = out_buf;
    cur_x = xpos;               // Adopt cursor position
    while (*out)                // While there's anything to print
        {
        *(p = tstr) = 0;
        // Now copy from 'out' to 'p', looking for end-of-lines;
        char *last_break = NULL;
        do
            {
            while (*out && *out != 12 && (!llen || cur_x < llen))
                {
                trailing_ws = FALSE;
                if (*out == '\n')
                    {
                    out++;
                    cur_x = llen;
                    last_break = NULL;
                    break;
                    }
                if (*out == 8)
                    {
                    while (*out == 8)
                        {
                        if (cur_x)
                            cur_x--;
                        *(p++) = *(out++);
                        }
                    }
                    
                if (!last_break || isspace(*last_break))
                    last_break = p;
                if (*out == SPC || *out == TAB)
                    {
                    // See how far that takes us
                    while (*out == SPC || *out == TAB)
                        {
                        if (llen && cur_x >= llen)
                            {       // Ignore space past end-of-line
                            out++;
                            continue;
                            }
                        switch (*out)
                            {
                          case SPC:
                                *(p++) = *(out++);
                                cur_x++;
                                break;

                          case TAB:
				int bytes = cur_x % 8;
                                for (i = 0; i < bytes; i++)
                                    {
                                    *(p++) = SPC;
                                    cur_x++;
                                    }
                                out++;
                                break;
                            }
                        }
                    // We've finished seeing white space, we may have wrapped
                    if (llen && cur_x >= llen)
                        {
                        // Did we find EOL before wrapping?
                        if (*out == '\n')
                            out++;
                        break;
                        }
                    trailing_ws = TRUE;
                    continue;
                    }

                // Now we have an acceptable character; roll until we
                // find a 'boundary' character
                char *end = strpbrk(out, " \t,.\n;!?:-");
                // If we didn't find one, take the rest of the string
                if (!end)
                    end = out + strlen(out);
                else if (!isspace(*end))
                    end++;
                int bytes = (int)(end - out);
                // The favourite case; we don't wrap because of this
                if (!llen || cur_x + bytes < llen)
                    {
                    strncpy(p, out, bytes);
                    p += bytes;
                    cur_x += bytes;
                    out = end;
                    last_break = NULL;
                    continue;
                    }
                // Does this word, of it's own accord, wrap?
                if ((int)(end - out) > llen)
                    {
                    // If we're into the line, but only with white space
                    // Get rid of the white space and go to the beginning
                    // of the line
                    if (cur_x)
                        {
                        cur_x = llen;
                        continue;
                        }
                    bytes = llen - cur_x - 1;
                    strncpy(p, out, bytes);
                    p += bytes;
                    out += bytes;
                    cur_x = llen;
                    last_break = NULL;
                    break;
                    }
                // Otherwise, wrap...
                cur_x = llen;
                }
            if (cur_x >= llen)
                {
                trailing_ws = FALSE;
                if (last_break)
                    p = last_break;
                if (p != tstr)
                    {
                    *(p++) = '\r';
                    *(p++) = '\n';
                    }
                cur_x = 0;
                cur_y++;
                last_break = NULL;
                }
            }
        while (*out && *out != 12 && (slen && cur_y < (slen - 1)));

        if (cur_x == llen)
            cur_x = 0;          // Assuming terminal has automargins
        xpos = cur_x;           // Update cursor position

        write(conn_sock_fd, tstr, (long)(p - tstr)); // Print the text

        // If we have exceeded the screen length, or if we've reached
        // a form-feed mid-text, use a 'more' prompt
        if ((slen && cur_y >= (slen - 1)) || (*out == 12 && cur_y > 1))
            {
            txnest++;           // Flag that we're nesting tx's
            pressret();
            cur_y = 0;
            cur_x = 0;
            txnest--;
            xpos = cur_x;
            }
        if (*out == 12)           // Print form feeds on their own
            txc(*(out++));
        }

    // Print the character on it's lonesome. This is undesirable,
    // as it causes an additional call to 'write'.
    if (c == SPC || c == TAB)
        {
        if (llen && cur_x >= llen)
            addcr = TRUE;
        else if (cur_x && !trailing_ws)
            txc(c);
        }
    else if (c)
        txc(c);

    // Restore the old buffer state if we were nested
    if (txnest)
        {
        free(out_buf);
        out_buf = old_out_buf;
        out_bufsz = old_out_bufsz;
        }
    }

/* call TX with printf style arguments */
void
txprintf(const char *s, ...)
    {
    va_list va;
    va_start(va, s);
    vsprintf(temp, s, va);
    va_end(va);
    tx(temp, 0);
    }

/* print a single character */
void
txc(char c)
    {
    if (!c)                     // Don't send nulls
        return;
    if (manager)
        {                       // Nothing fancy for the manager
        putchar(c);
        return;
        }
    txed = TRUE;                // YES! We have SENT THE LIGHT!
    if (addcr && c != '\r' && c != '\n')
        {                       // When we need our own line
        txc('\n');
        addcr = FALSE;
        }
    if (c == 12)
        {                       // form feed
        write(conn_sock_fd, &c, 1);
        xpos = 0;
        ypos = 0;
        trailing_ws = FALSE;
        }
    else if (c == '\r' || c == '\n')
        {                       // CRLF
        // XXX: Should only send '\r' if player has ufCRLF
        // Really ought to deal with the situation where we get
        // called as: txc('\r'); txc('\n'); as this will generate
        // two CRLFs
        write(conn_sock_fd, "\r\n", 2);
        xpos = 0;
        ypos++;
        trailing_ws = FALSE;
        }
    else if (c == 8 || c == 127)
        {                       // Backspace / delete
        if (xpos == 0)
            return;
        write(conn_sock_fd, "\x08 \x08", 3);
        xpos--;
        trailing_ws = FALSE;
        }
    else if (!(trailing_ws && (c == SPC || c == TAB)))
        {                       // Any other character
        write(conn_sock_fd, &c, 1);
        xpos++;
        if (me && me->llen && xpos >= me->llen)
            txc('\n');          // Do we need to line-wrap?
        trailing_ws = FALSE;
        }
    }

// wrapper for 'read' system call, which copes with interrupted
// system calls, etc
static int
do_read(char *p)
    {
    int ret = 0;

    do
        {
        ret = read(conn_sock_fd, p, 1);
        }
    while (ret == -1 && (errno == EINTR || errno == EAGAIN));
    if (ret == -1)
        txprintf(">> read() call interrupted: %s\n", strerror(errno));

    return ret;
    }

/* prompt the user, but remember what we prompted with */
void
prompt(long msg)
    {
    last_prompt = msg;
    tx(message(msg));
    }

// fetch_input is the gory-guts of the user interface.
// It deals with fetching inputs from users, and echoing them, etc,
// i.e. providing 'command line' editing.
// Todo List:
//  . ^W = delete word
//  . ^D = end-of-file [quit]
//  . like amul: when interrupted, redisplay current input
void
fetch_input(char *to, int length)
    {
    char *p = to;
    char c = 0;
    char noecho;
    int max = conn_sock_fd;
    fd_set fds, rd_fds;

    FD_ZERO(&fds);
    FD_SET(conn_sock_fd, &fds);
    FD_SET(ipc_fd, &fds);
    if (ipc_fd > max)
        max = ipc_fd;
        
    *p = 0;
    forced = FALSE;

    if (length < 0)
        {                       // A negative length means 'hidden input'
        noecho = TRUE;
        length = -length;
        }
    else noecho = FALSE;        // A length of zero means 'hit a key'

    in_input = TRUE;

    do
        {
        int ret;                // Return value from select

        // Because we have to listen to the 'ipc_fd' as well as the
        // users input, so we use a select wrapper
        rd_fds = fds;
        while ((ret = select(max + 1, &rd_fds, NULL, NULL, NULL)) == -1 &&
               (errno == EINTR || errno == EAGAIN));
        if (ret == -1)
            {
            tx(INTERNAL_ERROR);
            error(LOG_ERR, "fetch_input::select(): %s", strerror(errno));
            exit(1);
            }

        if (FD_ISSET(ipc_fd, &rd_fds))
            {                   // IPC interruption
            while (xpos)
                txc(8);
            ipc_proc();
            *p = 0;
            if (forced)
                return;
            prompt(last_prompt);
            tx(to);
            continue;
            }

        // Because we have 'shit' happening, the read may fail,
        // it's very likely that it's not a problem though.
        if ((ret = do_read(p)) == -1)
            continue;
        c = *p;

        // Ignore spurious CRLF characters
        if ((c == '\n' && lastc == '\r') || (c == '\r' && lastc == '\n'))
            {
            c = lastc = 0;
            continue;
            }

        lastc = c;

        // Because of al the crap we did with telnet stuff, and because
        // we're assuming the user is using a telnet client, we need to
        // implement very rudimentary telnet support
        if ((u_char)c == 255)   // Telnet option?
            {
            u_char byte;
            // Telnet commands are at least two bytes long
            ret = do_read(p);
            byte = (u_char) *p;
            if (ret == -1 || byte == 255)
                continue;
            else if (byte >= 251)
                {
                do_read(p);     // 255 + OptNegotiation + Option
                continue;
                }
            // XXX: else if (*p == SB)
            continue;
            }

        if (c == '\r' || c == '\n') // Ignore \r's
            {
            *p = 0;
            txc('\n');
            break;
            }

        if (c == 8 || c == 127) // Backspace
            {
            if (p <= to)        // Beginning of string
		{
		txc(7);
                continue;
		}
            txc(8);
            *(--p) = 0;         // Rewind p
            continue;
            }

        if (c == 27 || c == 21) // Escape or CTRL-U
            {
            while (p > to)
                {
		txc(8);
                p--;
                }
            *p = 0;
            continue;
            }

        if (c < 32 || (c & 128) || p >= to + length - 1)
            continue;           // Illegal characters or end of buffer

        p++;
        *p = 0;
        if (noecho)             // For hidden input, eg. passwords
            txc('*');
        else txc(c);
        }
    while (c != '\r');
    while (p > to && isspace(*(p-1)))
        *(p--) = 0;
    in_input = FALSE;
    }

/* telnet_opt is a hack. SMUGL implements a very quick and nasty
 * understanding of the telnet protocol; just enough to allow it
 * to tell most telnet clients to behave properly, and enough
 * to ignore most responses or resulting commands from others.
 * If you want to know what this is, see the apropriate RFC
 */
void
telnet_opt(u_char neg_type, u_char opt)
    {
    u_char options[3];
    options[0] = 255;
    options[1] = neg_type;
    options[2] = opt;
    write(conn_sock_fd, options, 3);
    }
