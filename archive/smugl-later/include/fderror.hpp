#pragma once
#include "fileerror.hpp"
#include "fileio.hpp"
#include "portable.hpp"

namespace Smugl
{

struct FDError : public FileError {
    FDError(const char *const filename_, const int errno_, const int fd_)
        : FileError(filename_, errno_), fd(fd_)
    {
    }
    ~FDError() override
    {
        if (fd >= 0)
            close(fd);
        fd = -1;
    }
    const char *type() const override { return "general FD"; }
    int fd;
};

struct FDReadError final : public FDError {
    FDReadError(const char *filename_, const int errno_, const int fd_)
        : FDError(filename_, errno_, fd_)
    {
    }
    const char *type() const override { return "FD read"; }
};

struct FDWriteError final : public FDError {
    FDWriteError(const char *filename_, const int errno_, const int fd_)
        : FDError(filename_, errno_, fd_)
    {
    }
    const char *type() const override { return "FD write"; }
};

}  // namespace Smugl
