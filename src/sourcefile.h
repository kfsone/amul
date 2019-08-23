#ifndef AMUL_SRC_SOURCEFILE_H
#define AMUL_SRC_SOURCEFILE_H

#include <h/amul.enum.h>
#include <h/amul.type.h>

#include <string>
#include <string_view>
#include <vector>

#include "buffer.h"

struct SourceFile {
	std::string filepath {};
    void *   mapping {nullptr};
    Buffer   buffer {};
    uint16_t lineNo {0};
    size_t   size {0};

    std::vector<std::string_view> line {};

	SourceFile(std::string_view filepath);
	error_t Open();
	void Close();
    constexpr bool Eof() const noexcept { return buffer.Eof(); }
    bool GetLineTerms() noexcept;
    bool GetLine() noexcept;
};

#endif
