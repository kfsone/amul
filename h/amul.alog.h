#ifndef H_LIB_LOGGING_H
#define H_LIB_LOGGING_H 1

#include <inttypes.h>

#ifndef HAVE_ERROR_T
typedef int error_t;
#    define HAVE_ERROR_T
#endif

#ifndef __has_builtin
#    define __has_builtin(x) 0
#endif

#ifndef NDEBUG
#    if !defined(DEBUG_BREAK) && defined(_MSC_VER)
#        define DEBUG_BREAK __debugbreak()
#    endif
#    if !defined(DEBUG_BREAK) && !defined(_MSC_VER)
#        if __has_builtin(__builtin_debugtrap)
#            define DEBUG_BREAK __builtin_debugtrap()
#        endif
#        if !defined(DEBUG_BREAK) && defined(POSIX)
#            include <signal.h>
#            if defined(SIGTRAP)
#                define DEBUG_BREAK raise(SIGTRAP)
#            else
#                define DEBUG_BREAK raise(SIGABRT)
#            endif
#        endif
#    endif
#endif

#if !defined(DEBUG_BREAK)
#    define DEBUG_BREAK (void)0
#endif

enum LogLevel {
    AL_DEBUG,
    AL_INFO,
    AL_NOTE,
    AL_WARN,
    AL_ERROR,
    AL_FATAL,
    MAX_LOG_LEVEL,
};

extern uint32_t    al_errorCount;
extern void        alog(LogLevel level, const char *fmt, ...);
extern void        alogLevel(LogLevel level);
extern LogLevel    alogGetLevel();
extern const char *alogGetLevelName();

template <typename... Args>
[[noreturn]] void
afatal(const char *fmt, Args... args)
{
    alog(AL_FATAL, fmt, args...);
}

extern error_t InitLogging();

#endif
