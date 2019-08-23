#include <h/amul.type.h>

#include "buffer.h"
#include "filemapping.h"
#include "filesystem.h"
#include "sourcefile.h"

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
    auto skipspc = [](const char *&cur) {
        return (*cur == ' ' || *cur == '\t') ? (++cur, true) : (false);
    };
    line.clear();
    bool continuation = false;
    do {
        auto cur = buffer.it();
        auto end = buffer.ReadLine();
        if (cur == end || end == nullptr)
            return false;

        continuation = false;
        while (cur != end) {
            if (skipspc(cur))
                continue;
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
    return true;
}
