#include "stringmanip.h"
#include <string>
#include <string_view>

using std::string;
using std::string_view;


void
ReplaceAll(string &text, string_view pattern, string_view replacement) noexcept
{
    if (text.empty())
        return;

    size_t pos = text.find(pattern);
    if (pos == text.npos)
        return;

    size_t lastPos{ 0 };

    // Switch the string to a backup buffer so we can build it in-place.
    string oldText{};
    oldText.swap(text);
    text.reserve(oldText.size());

    while (pos != text.npos) {
        // If there's a segment of the old text we've skipped, copy it,
        if (pos != lastPos)
            text.append(oldText, lastPos, pos - lastPos);
        if (!replacement.empty())
            text.append(replacement);

        // Skip ahead in the original text
        lastPos = pos + pattern.size();

        // If replacement is empty, we're performing an erasure, and we
        // want to clean up if there is a whitespace after the pattern.
        // E.g. "The {adj} box"." becomes "The box."
        if (replacement.empty() && lastPos < oldText.length()) {
            if (oldText[lastPos] == ' ')
                ++lastPos;
        }

        pos = oldText.find(pattern, lastPos);
    }

    text.append(oldText, lastPos);
}
