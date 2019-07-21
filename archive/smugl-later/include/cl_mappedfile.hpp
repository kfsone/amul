#pragma once

#include <string>
#include "config.h"

#if defined(WIN32) || defined(WIN64) || defined(_MSCVER) || defined(_MSC_VER)
# include <windows.h>
#endif

namespace Smugl
{

//! @class Smugl::MappedFile
//! @brief Container for describing a memory mapped file.

class MappedFile
{
public:
	//! Create an in-memory view of a file from disk,
	//! while trying to avoid loading it from disk.
	//! @param[in] filename_ the file/sub-path of the file to open.
	//! @param[in] dirname_ [optional] prefix path.
	MappedFile(const char* const filename_, const std::string dirname_ = "") ;

	//! Create a viewport onto a portion of a file.
	MappedFile(const char* start_, const char* end_, size_t lineNo_)
		: m_filename("")		// it's not a file.
		, m_basePtr(NULL)
		, m_currentPtr(start_)
		, m_endPtr(end_)
		, m_size(end_ - start_)
		, m_lineNo(lineNo_)
		{}

	virtual ~MappedFile() ;						// Destructor.

private:
	MappedFile() {}
	MappedFile(const MappedFile& rhs_) {}

public:
	// Copying mapped files is only allowed for sub-views.
	MappedFile& operator = (const MappedFile& rhs_) ;

public:
	//! Determine if we have read all the data.
	bool atEof() const { return m_endPtr < m_currentPtr ; }

	//! Look at the next character.
	char nextCharacter() const { return atEof() ? 0 : *m_currentPtr ; }

	//! Advance to the end of the current line.
	bool advanceToEndOfLine() ;

	//! Advance to the start of the next line.
	bool advanceToNextLine() ;

	//! Find where this paragraph ends.
	bool advanceToEndOfParagraph() ;

	//! Skip current whitespace.
	//! @return true if we found a non-whitespace character,
	//! false if we reached eof.
	bool advanceToNonWhitespace() ;

	//! Advance to the next whitespace.
	bool advancePastCurrentWord() ;

	//! Skip ahead to first non-comment content.
	bool advanceToNextContent()
	{
		do
		{
			if ( advanceToNonWhitespace() )
				return false ;
			if ( *m_currentPtr != ';' )
				return true ;
		}
		while ( advanceToNextLine() ) ;

		return false ;
	}

public:
	// Accesors.

	const std::string& filename() const { return m_filename ; }
	const void* basePtr() const { return m_basePtr ; }
	const char* currentPtr() const { return m_currentPtr ; }
	const char* endPtr() const { return m_endPtr ; }
	size_t sizeBytes() const { return m_size ; }
	size_t remainingBytes() const { return (m_endPtr - m_currentPtr) ; }
	size_t lineNo() const { return m_lineNo ; }

protected:
	//! Name of the file.
	std::string		m_filename ;
	//! Where the file contents actually starts.
	void*			m_basePtr ;
	//! For convenience, where the file would end.
	const char*		m_endPtr ;
	//! Where we're currently at.
	const char*		m_currentPtr ;

	//! Size (in bytes) of the entire file.k
	size_t			m_size ;

	//! Current line number.
	size_t			m_lineNo ;

#if defined(HAVE_MMAP)
	int				m_fd ;
#else
	// For windows:
	HANDLE			m_handle ;
	HANDLE			m_mapping ;
#endif
} ;

}
