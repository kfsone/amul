#ifndef AMUL_LOGGING_H
#define AMUL_LOGGING_H

#include <cinttypes>
#include <cstdlib>
#include <iostream>

#include "h/amul.type.h"

enum LogLevel {
    LMORE,      // ultra debug
    LDEBUG,
    LINFO,
    LNOTE,
    LWARN,
    LERROR,
    LFATAL,
    MAX_LOG_LEVEL,
};

[[noreturn]]
void Terminate(error_t);

size_t GetLogErrorCount() noexcept;
void SetLogLevel(LogLevel level) noexcept;
LogLevel GetLogLevel() noexcept;
std::string_view GetLogLevelName(LogLevel level) noexcept;
void BumpLogErrorCount() noexcept;
void LogBreak();

void LogFinish(LogLevel);

template<typename... Args>
void
Log(LogLevel level, Args &&... args)
{
    if (level >= GetLogLevel()) {
        std::cout << GetLogLevelName(level) << ": ";
        (std::cout << ... << std::forward<Args>(args));
        LogFinish(level);
    }
}

template<typename... Args>
void
LogMore(Args &&... args)
{
    Log(LMORE, std::forward<Args>(args)...);
}

template<typename... Args>
void
LogDebug(Args &&... args)
{
    Log(LDEBUG, std::forward<Args>(args)...);
}

template<typename... Args>
void
LogInfo(Args &&... args)
{
    Log(LINFO, std::forward<Args>(args)...);
}

template<typename... Args>
void
LogNote(Args &&... args)
{
    Log(LNOTE, std::forward<Args>(args)...);
}

template<typename... Args>
void
LogWarn(Args &&... args)
{
    Log(LWARN, std::forward<Args>(args)...);
}

template<typename... Args>
void
LogError(Args &&... args)
{
    Log(LERROR, std::forward<Args>(args)...);
}

template<typename... Args>
[[noreturn]] void
LogFatal(Args &&... args)
{
    Log(LFATAL, std::forward<Args>(args)...);
    Terminate(1);
}

error_t InitLogging();

#endif  // AMUL_LOGGING_H
