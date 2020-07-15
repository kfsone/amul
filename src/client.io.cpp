// high-level input/output operations for amul

#include <cstring>

#include "game.h"
#include "amul.msgs.h"
#include "amul.stringmanip.h"
#include "typedefs.h"
#include "amulinc.h"
#include "client.io.h"
#include "textline.h"

/// TODO: This should go away.
extern thread_local slotid_t t_slotId;
extern thread_local bool t_forced;

extern void iocheck();

thread_local IoType t_iosup;  // Whether we're using console/network/logging output
thread_local bool t_addCR, t_needCR;

#if defined(TEXTLINE_CODE)
thread_local std::string TextLine::tl_line;
#endif

void
Printc(char c)
{
    t_needCR = c != '\n';
    switch (t_iosup) {
        case LOGFILE:
            return;
        default:
            putchar(c);
            return;
    }
}

void
Print(const char *src)
{
    int i, l;
    char *p, *ls, *lp;

    if (t_iosup == LOGFILE)
        return;
    if (t_addCR && t_needCR)
        Printc('\n');
    t_addCR = false;
    t_needCR = false;
    char *s = ProcessString(src);
    if (t_iosup == CONSOLE) {
        printf("%s", s);
        return;
    }

    l = 0;
    while (*s != 0) {
        char spc[256];
        p = spc;
        i = 0;
        ls = lp = nullptr;
        do {
            if (*s == '\n')
                break;
            if (i < 79 && (*s == 9 || *s == 32)) {
                ls = s;
                lp = p;
            }
            if (*s == '\r') {
                s++;
                continue;
            }
            *(p++) = *(s++);
            i++;
        } while (*s != 0 && *s != '\n' && (t_character->llen < 8 || i < (t_character->llen - 1)) &&
                 *s != 12);

        if (i > 0)
            t_needCR = true;
        if (((t_character->llen - 1) >= 8 && i == (t_character->llen - 1)) && *s != '\n') {
            if (*s == ' ' || *s == 9)
                s++;
            else if (*s != 0 && ls != nullptr) {
                s = ls + 1;
                p = lp + 1;
            }
            *(p++) = '\n';
            t_needCR = false;
        }
        if (*s == '\n') {
            *(p++) = '\n';
            s++;
            t_needCR = false;
        }
        *p = 0;
        l++;
        if (t_character->slen > 0 && l >= (t_character->slen) && *s != 12) {
            pressret();
            l = 0;
        }
        if (*s == 12) {
            s++;
            l = 0;
        }
    }
}

void
Ansify(const char *ansiCode)
{
    if (t_character->flags & ufANSI)
        Printf("\033[%s", ansiCode);
}

// transmit a processed string to a particular slot
void
PrintSlot(slotid_t slotId, const char *text)
{
    auto out = ProcessString(text);
    if (slotId == t_slotId)
        Print(out);
#ifdef MESSAGE_CODE
    else
        interact(MMESSAGE, slotId, -1, out);
#endif
}

// transmit a string with a numeric format in it to a given slot
void
PrintSlot(slotid_t plyr, const char *format, int n)
{
    char result[512];
    snprintf(result, sizeof(result), format, n);
    PrintSlot(plyr, result);
}

///////////////////////////////////////////////////////////// Input
thread_local bool t_clientDoesntEcho;

// Read user input into s which has max length maxLength
/// TODO: Have this function provide the buffer, perhaps use
/// TODO: a std::string, or take a string_view
void
GetInput(char *s, size_t maxLength)
{
    char *p = s;
    int c = *p = 0;
    t_forced = false;

    do {
        iocheck();
        if (t_forced)
            return;
        switch (t_iosup) {
            case CONSOLE:
                c = getchar();
                break;
            default:
                return;
        }
        iocheck();
        if (t_forced)
            return;
        c = c & 255;
        if (c == 0)
            continue;
        if (maxLength == 0)
            return;
        if (c == 8) {
            if (p > s) {
                if (t_clientDoesntEcho) {
                    Printc(8);
                    Printc(32);
                    Printc(8);
                }
                *(--p) = 0;
            }
            continue;
        }
        if (c == 10 || c == 13) {
            *(p++) = 0;
            if (t_clientDoesntEcho)
				Printc('\n');
            continue;
        }
        if (c == 27 || (c > 0 && c < 23) || c == t_character->rchar) {
            Printc('\n');
            Print(GetRank(t_character->rank).prompt);
            Print(s);
            continue;
        }
        if (c == 24 || c == 23) {
            while (p != s) {
                if (t_clientDoesntEcho)
                    Print("\x08\x020\x08");
                p--;
            }
            *p = 0;
            continue;
        }
        if (c < 32 || c > 127)
            continue;
        if (p >= s + maxLength - 1)
            continue;
        *(p++) = static_cast<char>(c);
        *p = 0;
        if (t_clientDoesntEcho)
	        Printc(static_cast<char>(c));
        t_needCR = true;
    } while (c != '\n');
    if (isspace(*(s + strlen(s) - 1)))
        *(s + strlen(s) - 1) = 0;
    t_needCR = false;
    iocheck();
}

char
Prompt(stringid_t msg, const char *options)
{
    char response[2]{};
    for (;;) {
        Print(msg);
        SafeInput(response);
        if (strchr(options, tolower(response[0])) != nullptr) {
            return tolower(response[0]);
        }
        Print(GetNamedString("didnt-understand", "I didn't understand that, try again.\n"));
    }

    return tolower(response[0]);
}

void
pressret()
{
    Print(RETURN);
    char text[8]{};
    GetInput(text, 0);
    // Return to the beginning of the line and erase to the end of the line.
    if (t_character->flags & ufANSI)
        Print("\r\x1b[K");
}
