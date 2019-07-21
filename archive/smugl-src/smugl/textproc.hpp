// $Id: textproc.hpp,v 1.4 1997/04/28 01:10:23 oliver Exp $
// protos/externs for textprocessing functions

#ifndef TEXTPROC_H
#define TEXTPROC_H 1

extern const char *message(msgno_t id);
extern const char *rightstr(const char *s, int len);
extern int match(const char *s1, const char *s2);
extern char *esc(const char *code, char *to);
extern void ioproc(const char *str);

// Copy a string, returning a pointer to the null byte in the copy
// Avoids strcpy(into, from); into += strlen(into); situations
static inline char *strcopy(char *to, char const *from)
    {
    while ((*to = (*(from++))))
        to++;
    return to;
    }

extern char *out_buf;
extern long out_bufsz;
extern long out_buf_len;

#endif /* TEXTPROC_H */