#pragma once
#include <cstring>
#include <stdexcept>

namespace Smugl
{

struct FileError : public std::exception
{
	FileError(const char* const filename_, const int errno_)
		: m_filename(filename_)
		, m_errno(errno_)
		, m_error("")
		{}
	virtual ~FileError() {}

	virtual const char* type() const { return "general" ; }
	std::string m_filename ;
	const int m_errno ;
	mutable std::string m_error ;

	virtual const char* what() const noexcept override
	{
		if ( m_error.empty() )
		{
			m_error = type() ;
			m_error += " error: " ;
			m_error = m_filename ;
			m_error += ": " ;
			m_error += strerror(m_errno) ;

		}

		return m_error.c_str() ;
	}
} ;
}

