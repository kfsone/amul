#pragma once

#include <cstdio>
#include <utility>

[[noreturn]] void quit();

namespace AMUL::Logging
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-security"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"

struct Logger {
    ~Logger();

    // Hide warnings?
    bool m_quiet{false};

    size_t m_numErrors{0};

    template <typename... Args>
    void logf(const char *level, const char *fmt, Args &&... args)
    {
        printf("%c)%s: ", level[0], level + 1);
        printf(fmt, std::forward<Args>(args)...);
        printf("\n");
        fflush(stdout);
    }

    template <typename... Args>
    void infof(const char *fmt, Args &&... args)
    {
        if (!m_quiet)
            logf("Info", fmt, std::forward<Args>(args)...);
    }
    void info(const char *message) { logf("Info", message); }

    template <typename... Args>
    void errorf(const char *fmt, Args &&... args)
    {
        m_numErrors++;
        logf("ERROR", fmt, std::forward<Args>(args)...);
    }

    void error(const char *issue) { logf("ERROR", issue); }

    template <typename... Args>
    void warnf(const char *fmt, Args &&... args)
    {
        logf("Warning", fmt, std::forward<Args>(args)...);
    }
    void warn(const char *message) { logf("Warning", message); }

    template <typename... Args>
    [[noreturn]] void fatalf(const char *fmt, Args &&... args)
    {
        logf("FATAL", fmt, std::forward<Args>(args)...);
        quit();
    }

    [[noreturn]] void fatal(const char *issue) { fatalf("%s", issue); }

    [[noreturn]] void fatalop(const char *verb, const char *noun) { fatalf("Can't %s %s."); }
};
#pragma GCC diagnostic pop
#pragma clang diagnostic pop

Logger &GetLogger();

}  // namespace AMUL::Logging
