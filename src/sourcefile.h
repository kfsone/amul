#pragma once
#ifndef AMUL_SOURCEFILE_H
#define AMUL_SOURCEFILE_H

#include <string>
#include <vector>

#include "amul.enum.h"
#include "buffer.h"
#include "logging.h"
#include "typedefs.h"

struct SourceFile {
    std::string filepath{};
    void *mapping{ nullptr };
    Buffer buffer{};
    uint16_t lineNo{ 0 };
    size_t size{ 0 };

    using Line = std::vector<string_view>;
    using iterator = Line::iterator;
    Line line{};
    iterator cur{ line.end() };
    std::vector<std::string> alterations;

    SourceFile(std::string filepath);
    error_t Open();
    void Close();
    std::string GetPosition() const;

    constexpr bool Eof() const noexcept { return buffer.Eof(); }
    bool Eol() const noexcept { return cur == line.end(); }

    bool GetLineTerms() noexcept;
    bool GetLine() noexcept;

    void SkipBlock() noexcept
    {
        while (GetLine() && !line.empty() && !line.front().empty())
            ;
    }

    bool GetIDLine(string_view prefix);
    void GetLines(std::string &into);
    string_view PopFront() noexcept { return cur != line.end() ? *(cur++) : ""; }

    template<typename... Args>
    void Error(Args &&... args) const
    {
        LogError(filepath, ":", lineNo, ": ", forward<Args>(args)...);
    }
};

#endif