#ifndef AMUL_SOURCEFILE_LOG_H
#define AMUL_SOURCEFILE_LOG_H

#include <iostream>

#include "h/sourcefile.h"

std::ostream &operator << (std::ostream &os, const SourceFile &sf) noexcept
{
    os << sf.filepath << ":" << sf.lineNo << ":";
    return os;
}

#endif  // AMUL_SOURCEFILE_LOG_H
