#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "modules.h"

#include <h/amul.alog.h>

#ifndef __has_builtin
# define __has_builtin(x) 0
#endif

#if !defined(DEBUG_BREAK) && defined(_MSC_VER)
#    define DEBUG_BREAK __debugbreak()
#endif
#if !defined(DEBUG_BREAK) && !defined(_MSC_VER)
#    if __has_builtin(__builtin_debugtrap)
#        define DEBUG_BREAK __builtin_debugtrap
#    endif
#    if !defined(DEBUG_BREAK) && defined(POSIX)
#        include <signal.h>
#        if defined(SIGTRAP)
#            define DEBUG_BREAK raise(SIGTRAP)
#        else
#            define DEBUG_BREAK raise(SIGABRT)
#        endif
#    endif
#endif
#if !defined(DEBUG_BREAK)
#    define DEBUG_BREAK (void)0
#endif

extern void terminate(error_t err);

// Track the number of messages logged with AL_ERROR
uint32_t al_errorCount = 0;

static const char *levelName[MAX_LOG_LEVEL + 1] = {"Debug", "Info",  "Note",    "WARNING",
                                                   "ERROR", "FATAL", "DISABLED"};

static enum LogLevel s_logLevel = AL_DEBUG;

error_t
loggingModuleInit(struct Module *module)
{
    s_logLevel = AL_INFO;
    return 0;
}

error_t
loggingModuleStart(struct Module *module)
{
    if (!GetModule(MOD_CMDLINE)) {
        fprintf(stderr, "*** FATAL: Logging module started before cmdline was registered\n");
        fflush(stderr);
        return EINVAL;
    }

    alog(AL_DEBUG, "Logging enabled");

    return 0;
}

error_t
loggingModuleClose(struct Module *module, error_t err)
{
    alog(AL_DEBUG, "Logging disabled");
    s_logLevel = MAX_LOG_LEVEL;
    return 0;
}

void
InitLogging()
{
    error_t err = NewModule(
            true, MOD_LOGGING, loggingModuleInit, loggingModuleStart, loggingModuleClose, NULL,
            NULL);
}

void
alogLevel(enum LogLevel level)
{
    if (level < MAX_LOG_LEVEL)
        s_logLevel = level;
}

const char *
alogGetLevelName()
{
    return levelName[s_logLevel];
}

void
alog(enum LogLevel level, const char *fmt, ...)
{
    if (level < MAX_LOG_LEVEL && level >= s_logLevel) {
        if (level != AL_INFO)
            printf("%c] ", levelName[level][0]);

        va_list vl;
        va_start(vl, fmt);
        vprintf(fmt, vl);
        va_end(vl);
        printf("\n");
        fflush(stdout);

        switch (level) {
        case AL_ERROR: al_errorCount++; break;
        case AL_FATAL:
            DEBUG_BREAK;
            terminate(-1);
            break;
        default: break;
        }
    }
}
