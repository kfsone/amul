#ifndef AMUL_SRC_AMULCOM_CTXLOG_H
#define AMUL_SRC_AMULCOM_CTXLOG_H

#include <h/amul.alog.h>
#include <sstream>

extern void PushContext(const std::string &ctx);
extern void PopContext();

struct ScopeContext final {
    ScopeContext(const std::string &ctx) { PushContext(ctx); }
    ~ScopeContext() { PopContext(); }

    static const std::string &str() noexcept;
};

template<typename... Args>
void
ctxLog(const LogLevel level, Args&&... args)
{
    std::ostringstream str { ScopeContext::str() };
    str << ' ';
    (str << ... << args);

    alog(level, "%s", str.str().c_str());
}

#endif
