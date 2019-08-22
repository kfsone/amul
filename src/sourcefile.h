#ifndef AMUL_SRC_SOURCEFILE_H
#define AMUL_SRC_SOURCEFILE_H

#include <h/amul.enum.h>
#include <h/amul.type.h>

#include <string>
#include <string_view>

#include "buffer.h"

struct SourceFile {
	std::string filepath;
    void *   mapping;
    Buffer   buffer;
    uint16_t lineNo;
    size_t   size;

	SourceFile(std::string_view filepath);
	error_t Open();
	void Close();
};

#endif
