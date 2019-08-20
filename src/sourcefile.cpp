#include <algorithm>
#include <string_view>

#include "sourcefile.h"
#include "amul.defs.h"
#include "typedefs.h"
#include "buffer.h"
#include "filemapping.h"
#include "filesystem.h"
#include "logging.h"
#include "svparse.h"

SourceFile::SourceFile(std::string file) : filepath{ move(file) } {}

error_t
SourceFile::Open()
{
    error_t err = GetFilesSize(filepath, &size);
    if (err != 0)
        return err;
    if (size == 0)
        return ENODATA;

    err = NewFileMapping(filepath, &mapping, &size);
    if (err != 0) {
        Close();
        return err;
    }

    buffer.Assign(static_cast<const char *>(mapping), size);

    return 0;
}

void
SourceFile::Close()
{
    line.clear();
    buffer.Close();
    CloseFileMapping(&mapping, size);
}

constexpr bool
isQuote(char c) noexcept
{
    return c == '\"' || c == '\'';
}

bool
SourceFile::GetLineTerms() noexcept
{
    line.clear();
    alterations.clear();
    cur = line.end();
    bool result = true;
    bool continuation = false;
    do {
        auto cur = buffer.it();
        if (buffer.Eof()) {
            result = false;
            break;
        }
        ++lineNo;
        auto end = buffer.ReadLine();
        if (cur == end || end == nullptr) {
            result = false;
            break;
        }
        LogMore("reading: ", string_view{ cur, size_t(end - cur) });

        continuation = false;
        while (cur != end) {
            if (*cur == ' ' || *cur == '\t') {
                ++cur;
                continue;
            }
            const char *start = cur++;
            if (*start == '+' && cur == end) {
                continuation = true;
                break;
            }
            if (*start == ';')
                break;
            // Allow for something="..."
            char startChar = *start;
            bool alterCase = false;
            if (!isQuote(*start)) {
                while (cur != end && !isspace(*cur)) {
                    if (*cur == '=' && isQuote(*(cur + 1))) {
                        ++cur;
                        startChar = *(cur++);
                        break;
                    }
                    alterCase = alterCase || (isalpha(*cur) && isupper(*cur));
                    ++cur;
                }
            }
            const size_t prefixLength = cur - start;
            if (isQuote(startChar)) {
                while (cur != end) {
                    if (cur + 1 != end && (*cur == '\\' || *cur == startChar)) {
                        if (*cur + 1 == startChar) {
                            cur += 2;
                            continue;
                        }
                    }
                    if (*(cur++) == startChar)
                        break;
                }
            }
            if (alterCase) {
                alterations.emplace_back(start, cur - start);
                auto abegin = alterations.back().begin(), aend = alterations.back().begin() + prefixLength;
                std::transform(abegin, aend, abegin, [](unsigned char c) { return std::tolower(c); });
                line.emplace_back(alterations.back());
            } else {
                line.emplace_back(start, cur - start);
            }
        }
    } while (continuation || line.empty());

    cur = line.begin();

    return result;
}

bool
SourceFile::GetLine() noexcept
{
    line.clear();
    auto start = buffer.it();
    auto eol = buffer.ReadLine();
    if (eol == nullptr) {
        cur = line.end();
        return false;
    }
    ++lineNo;
    line.emplace_back(start, eol - start);
    LogMore("read line: ", line.back());
    cur = line.begin();
    return true;
}

bool
SourceFile::GetIDLine(string_view prefix)
{
    if (!GetLineTerms() || line.empty() || line.front().empty())
        return false;
    RemovePrefix(line.front(), prefix);
    auto id = line.front();
    if (id.find("=") != id.npos) {
        Error("Invalid ", prefix, " id: ", id);
        SkipBlock();
        return false;
    }
    if (id.length() < 2 || id.length() > IDL) {
        Error("Invalid ", prefix, " id (length): ", id);
        SkipBlock();
        return false;
    }

    LogDebug(prefix, "|", id, "|");

    return true;
}

void
SourceFile::GetLines(std::string &into)
{
    while (GetLine() && !line.empty() && !line.front().empty()) {
        auto srcLine = line[0];
        if (srcLine.front() == '\t')
            srcLine.remove_prefix(1);
        bool addNewline = srcLine.empty() || srcLine.back() != '{';
        if (!addNewline)
            srcLine.remove_suffix(1);
        into += srcLine;
        if (addNewline)
            into += '\n';
    }
}
