#pragma once
#include "include/fileerror.hpp"

namespace Smugl
{

struct FPError : public FileError {
    FPError(const char* const filename_, const int errno_, FILE* const fp_)
        : FileError(filename_, errno_), fp(fp_)
    {
    }
    ~FPError() override
    {
        if (fp)
            fclose(fp);
        fp = nullptr;
    }
    const char* type() const override { return "general FILE"; }
    FILE* fp;
};

struct FPReadError final : public FPError {
    FPReadError(const char* filename_, const int errno_, FILE* const fp_)
        : FPError(filename_, errno_, fp_)
    {
    }
    const char* type() const override { return "FILE read"; }
};

struct FPWriteError final : public FPError {
    FPWriteError(const char* filename_, const int errno_, FILE* const fp_)
        : FPError(filename_, errno_, fp_)
    {
    }
    const char* type() const override { return "FILE write"; }
};

}  // namespace Smugl
