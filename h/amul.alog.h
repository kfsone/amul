#ifndef H_LIB_LOGGING_H
#define H_LIB_LOGGING_H 1

#include <inttypes.h>

enum LogLevel {
    AL_DEBUG,
    AL_INFO,
    AL_NOTE,
    AL_WARN,
    AL_ERROR,
    AL_FATAL,
    MAX_LOG_LEVEL,
};

extern uint32_t al_errorCount;
void            alog(enum LogLevel level, const char *fmt, ...);
void            alogLevel(enum LogLevel level);

#endif
