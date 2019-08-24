#include <h/amul.defs.h>
#include <h/amul.type.h>

#include "buffer.h"
#include "filemapping.h"
#include "filesystem.h"
#include "logging.h"
#include "sourcefile.h"
#include "svparse.h"

SourceFile::SourceFile(std::string_view filepath)
	: filepath{filepath}
{}

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

bool
SourceFile::GetLineTerms() noexcept
{
    line.clear();
    bool continuation = false;
    do {
        auto cur = buffer.it();
        if (buffer.Eof())
            return false;
        ++lineNo;
        auto end = buffer.ReadLine();
        if (cur == end || end == nullptr)
            return false;
        LogDebug("reading: ", std::string_view{cur, size_t(end - cur)});

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
            if (*start == '\'' || *start == '\"') {
                while (cur != end) {
                    if (cur + 1 != end && (*cur == '\\' || *cur == *start)) {
                        if (*cur+1 == *start) {
                            cur += 2;
                            continue;
                        }
                    }
                    if (*(cur++) == *start)
                        break;
                }
            } else if (*start == ';') {
                break;
            } else {
                while (cur != end && *cur != ' ' && *cur != '\t')
                    ++cur;
            }
            line.emplace_back(start, cur - start);
        }
    } while (continuation || line.empty());

    return true;
}

bool
SourceFile::GetLine() noexcept
{
    line.clear();
    auto start = buffer.it();
    auto eol = buffer.ReadLine();
    if (eol == nullptr)
        return false;
    ++lineNo;
    line.emplace_back(start, eol - start);
    LogDebug("read line: ", line.back());
    return true;
}

bool
SourceFile::GetIDLine(std::string_view prefix)
{
    if (!GetLineTerms() || line.empty() || line.front().empty())
        return false;
    auto [_, id] = removePrefix(line.front(), prefix);
    if (id.find("=") != id.npos) {
        LogError(filepath, ":", lineNo, ": Invalid ", prefix, " id: ", id);
        SkipBlock();
        return false;
    }
    if (id.length() < 2 || id.length() > IDL) {
        LogError(filepath, ":", lineNo, ": Invalid ", prefix, " id (length): ", id);
        SkipBlock();
        return false;
    }

    LogDebug(prefix, id);

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
