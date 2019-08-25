#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "h/logging.h"
#include "h/modules.h"

// Support breakpoints instead of exits for errors
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#ifndef NDEBUG
#if !defined(DEBUG_BREAK) && defined(_MSC_VER)
#define DEBUG_BREAK __debugbreak()
#endif
#if !defined(DEBUG_BREAK) && !defined(_MSC_VER)
#if __has_builtin(__builtin_debugtrap)
#define DEBUG_BREAK __builtin_debugtrap()
#endif
#if !defined(DEBUG_BREAK) && defined(POSIX)
#include <signal.h>
#if defined(SIGTRAP)
#define DEBUG_BREAK raise(SIGTRAP)
#else
#define DEBUG_BREAK raise(SIGABRT)
#endif
#endif
#endif
#endif

#if !defined(DEBUG_BREAK)
#define DEBUG_BREAK (void) 0
#endif

[[noreturn]]
void Terminate(error_t err);

// Track the number of messages logged with AL_ERROR
static size_t s_errorCount{ 0 };

static LogLevel s_logLevel = LogLevel::LWARN;

constexpr std::string_view levelName[MAX_LOG_LEVEL + 1] = {
    // Names of the debugging levels
    "More", "Debug", "Info",  "Note",
    "WARNING", "ERROR", "FATAL", "DISABLED" };

error_t
loggingModuleInit(Module * /*module*/)
{
    s_logLevel = LNOTE;
    return 0;
}

error_t
loggingModuleStart(Module * /*module*/)
{
    if (!GetModule(MOD_CMDLINE)) {
        fprintf(stderr, "*** FATAL: Logging module started before cmdline was registered\n");
        fflush(stderr);
        return EINVAL;
    }

    LogDebug("Logging enabled");

    return 0;
}

error_t
loggingModuleClose(Module * /*module*/, error_t /*err*/)
{
    LogDebug("Logging disabled");
    s_logLevel = MAX_LOG_LEVEL;
    return 0;
}

error_t
InitLogging()
{
    return NewModule(MOD_LOGGING,
                     loggingModuleInit,
                     loggingModuleStart,
                     loggingModuleClose,
                     nullptr,
                     nullptr);
}

void LogBreak()
{
    DEBUG_BREAK;
}

size_t
GetLogErrorCount() noexcept
{
    return s_errorCount;
}

void
BumpLogErrorCount() noexcept
{
    ++s_errorCount;
}

LogLevel
GetLogLevel() noexcept
{
    return s_logLevel;
}

void
SetLogLevel(LogLevel level) noexcept
{
    if (level < MAX_LOG_LEVEL)
        s_logLevel = level;
}

std::string_view
GetLogLevelName(LogLevel level) noexcept
{
    return levelName[level];
}

void
LogFinish(LogLevel level)
{
    if (level >= LERROR) {
        std::cout << std::endl;
        ++s_errorCount;
    } else {
        std::cout << "\n";
    }
    if (level >= LFATAL) {
        std::cout << std::flush;
        fflush(stdout);
        LogBreak();
        exit(1);
    }
}
