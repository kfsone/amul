#pragma once
#include "include/fileerror.hpp"

namespace Smugl
{

struct FPError : public FileError
{
	FPError(const char* const filename_, const int errno_, FILE* const fp_) : FileError(filename_, errno_), fp(fp_) {}
	virtual ~FPError() { if ( fp ) fclose(fp) ; fp = NULL ; }
	virtual const char* type() const { return "general FILE" ; }
	FILE* fp ;
} ;

struct FPReadError : public FPError
{
	FPReadError(const char* filename_, const int errno_, FILE* const fp_) : FPError(filename_, errno_, fp_) {}
	virtual const char* type() const { return "FILE read" ; }
} ;

struct FPWriteError : public FPError
{
	FPWriteError(const char* filename_, const int errno_, FILE* const fp_) : FPError(filename_, errno_, fp_) {}
	virtual const char* type() const { return "FILE write" ; }
} ;

}

