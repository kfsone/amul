#ifndef AMUL_SVPARSE_H
#define AMUL_SVPARSE_H

#include <string_view>

using std::string_view;

constexpr bool
StartsWith(const string_view source, const string_view prefix)
{
    return (source.size() >= prefix.size() && source.compare(0, prefix.size(), prefix) == 0);
}

static inline bool
RemovePrefix(string_view &source, const string_view prefix)
{
    if (StartsWith(source, prefix)) {
        source.remove_prefix(prefix.length());
        return true;
    }
    return false;
}

#endif  // AMUL_SVPARSE_H