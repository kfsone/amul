#ifndef AMUL_SVPARSE_H
#define AMUL_SVPARSE_H

#include <string_view>

using std::string_view;

constexpr bool
starts_with(const string_view source, const string_view prefix)
{
    return (source.size() >= prefix.size() && source.compare(0, prefix.size(), prefix) == 0);
}

static inline std::pair<bool, string_view>
removePrefix(string_view &source, const string_view prefix)
{
    if (starts_with(source, prefix)) {
        source.remove_prefix(prefix.length());
        return std::make_pair(true, source);
    }
    return std::make_pair(false, source);
}

#endif  // AMUL_SVPARSE_H
