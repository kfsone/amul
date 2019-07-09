#include <stdarg.h>
#include <stdio.h>

#include "h/amul.alog.h"

extern void quit();

// Track the number of messages logged with AL_ERROR
uint32_t al_errorCount = 0;

static const char *errors[MAX_LOG_LEVEL] = {
        "Debug", "Info", "Note", "WARNING", "ERROR", "FATAL",
};

static enum LogLevel s_logLevel = AL_INFO;

void
alogLevel(enum LogLevel level)
{
    s_logLevel = level;
}

void
alog(enum LogLevel level, const char *fmt, ...)
{
    if (level >= s_logLevel) {
        if (level > MAX_LOG_LEVEL)
            level = MAX_LOG_LEVEL - 1;

        printf("%c] ", errors[s_logLevel][0]);

        va_list vl;
        va_start(vl, fmt);
        vprintf(fmt, vl);
        va_end(vl);
        printf("\n");
        fflush(stdout);

        switch (level) {
        case AL_ERROR: al_errorCount++; break;
        case AL_FATAL: quit(); break;
        default: break;
        }
    }
}
