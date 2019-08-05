#include <string>
#include <vector>

#include <h/amul.type.h>

#include "amulcom.ctxlog.h"

std::string s_contextString;
std::vector<size_t> s_contextOffsets;

const std::string &
ScopeContext::str() noexcept
{
    return s_contextString;
}

void
PushContext(const std::string &ctx)
{
    size_t offset = s_contextString.length();
    s_contextOffsets.push_back(offset);
    s_contextString += ctx;
    s_contextString += ':';
    alog(AL_DEBUG, "+ %s", s_contextString.c_str());
}

void
PopContext()
{
    size_t offset = s_contextString.back();
    s_contextOffsets.pop_back();
    s_contextString.resize(offset);
}

