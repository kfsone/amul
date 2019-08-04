#pragma once
#include "include/fileerror.hpp"
#include "include/portable.hpp"

namespace Smugl
{

struct FDError : public FileError {
    FDError(const char* const filename_, const int errno_, const int fd_)
        : FileError(filename_, errno_), fd(fd_)
    {
    }
    virtual ~FDError()
    {
        if (fd >= 0)
            _close(fd);
        fd = -1;
    }
    virtual const char* type() const { return "general FD"; }
    int fd;
};

struct FDReadError : public FDError {
    FDReadError(const char* filename_, const int errno_, const int fd_)
        : FDError(filename_, errno_, fd_)
    {
    }
    virtual const char* type() const { return "FD read"; }
};

struct FDWriteError : public FDError {
    FDWriteError(const char* filename_, const int errno_, const int fd_)
        : FDError(filename_, errno_, fd_)
    {
    }
    virtual const char* type() const { return "FD write"; }
};

}  // namespace Smugl
