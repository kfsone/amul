#ifndef AMUL_SRC_SOURCEFILE_LOG_H
#define AMUL_SRC_SOURCEFILE_LOG_H

#include <iostream>
#include "sourcefile.h"

std::ostream &operator << (std::ostream &os, const SourceFile &sf) noexcept
{
    os << sf.filepath << ":" << sf.lineNo << ":";
    return os;
}

#endif  // AMUL_SRC_SOURCEFILE_LOG_H
