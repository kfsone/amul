#ifndef SMUGL_SMUGLCOM_ERRORS_H
#define SMUGL_SMUGLCOM_ERRORS_H

#include "smuglcom.hpp"
#include <utility>

extern void errabort();

template<typename... Args>
void
report(const char *pfx, const char *msg, Args&&... args)
{
    if (needcr)
        printf("\n");
    needcr = FALSE;
    printf("%s> ", pfx);
    printf(msg, std::forward<Args>(args)...);
}

static inline void
error(const char *msg)
{
    report("#E#", "%s", msg);
    if (++err > 20)
        errabort();
}

template<typename... Args>
void
error(const char *msg, Args&&... args)
{
    report("#E#", msg, std::forward<Args>(args)...);
    if (++err > 20)
        errabort();
}

static inline void
warne(const char* msg)
{
    if (warn)
        report(" W ", "%s", msg);
}

template<typename... Args>
void
warne(const char *msg, Args&&... args)
{
    if (!warn)
        return;
    report(" W ", msg, std::forward<Args>(args)...);
}

extern void quit();

static inline void
quit(const char *msg)
{
    report("===", "%s", msg);
    quit();
}

template<typename... Args>
void
quit(const char *msg, Args&&... args)
{
    report("===", msg, std::forward<Args>(args)...);
    quit();
}

#endif
