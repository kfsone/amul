#include <stdio.h>
#include <stdexcept>
#include <string.h>	// for strerror

#include "include/cl_mappedfile.hpp"
#include "include/fileerror.hpp"
#include "include/portable.hpp"

int
main(int argc, char* argv[])
{
	printf("SMUGL code testbed.\n") ;

	//// ---- Your code here ---- ////

	try
	{
		Smugl::MappedFile mf(PATH_SEP "this-file-shouldnt-exist") ;
		printf("FAIL: Mapped a non-existent file\n") ;
	}
	catch ( Smugl::FileError& /*e*/ )
	{
		printf("OK: Exception on non-existent file, that's good.\n") ;
	}
	catch ( std::exception& /*e*/ )
	{
		printf("FAIL: Unknown exception\n") ;
		return -1 ;
	}

	try
	{
		// When debugging under visual studios, set the working directory for
		// the executable to be $(OutDir), so that "..\test\testfile.txt" is
		// the correct relative path :)
		Smugl::MappedFile mf(".." PATH_SEP "test" PATH_SEP "testfile.txt") ;

		printf("We have mapped %s\n", mf.filename().c_str()) ;
		printf("It's size is %zu bytes\n", mf.sizeBytes()) ;
		printf("Bytes remaining are: %zu\n", mf.remainingBytes()) ;
		printf("basePtr=%p current=%p end=%p\n", mf.basePtr(), mf.currentPtr(), mf.endPtr()) ;

		if ( mf.atEof() )
		{
			printf("FAIL: We think we're at eof at the start of the file\n") ;
			return -1 ;
		}

		printf("first character is %d [%c]\n", (int)*mf.currentPtr(), *mf.currentPtr()) ;
		char c = mf.nextCharacter() ;
		printf("nextCharacter gives %d [%c]\n", (int)c, c) ;

		c = *(mf.currentPtr() + mf.sizeBytes()) ;
		printf("eof character is %d [%c]\n", (int)c, c) ;

		const char* line = mf.currentPtr() ;
		if ( !mf.advanceToNonWhitespace() )
		{
			printf("FAIL: EOF looking for non-whitespace\n") ;
			return -1 ;
		}
		if ( mf.currentPtr() != line )
		{
			printf("FAIL: We think there's some whitespace at the start of the file (there shouldn't be)\n") ;
			return -1 ;
		}

		if ( !mf.advanceToEndOfLine() )
		{
			printf("FAIL: EOF looking for start of next line\n") ;
			return -1 ;
		}
		if ( mf.currentPtr() == line )
		{
			printf("FAIL: Advance to end of line didn't advance.\n") ;
			return -1 ;
		}

		if ( mf.lineNo() > 0 )
		{
			printf("FAIL: lineNo shouldn't have advanced\n") ;
			return -1 ;
		}

		std::string lineContent ;
		
		lineContent.assign(line, (mf.currentPtr() - line)) ;
		printf("First line reads: |%s|\n", lineContent.c_str()) ;

		if ( !mf.advanceToNextLine() )
		{
			printf("FAIL: Advance to next line found nothing.\n") ;
			return -1 ;
		}
		if ( mf.lineNo() == 0 )
		{
			printf("FAIL: lineNo didn't advance\n") ;
			return -1 ;
		}
		if ( mf.lineNo() > 1 )
		{
			printf("FAIL: lineNo %zu over-advanced.\n", mf.lineNo()) ;
			return -1 ;
		}

		line = mf.currentPtr() ;
		if ( !mf.advanceToEndOfLine() )
		{
			printf("FAIL: advance to end of line failed.\n") ;
			return -1 ;
		}
		lineContent.assign(line, (mf.currentPtr() - line)) ;
		printf("Second line reads: |%s|\n", lineContent.c_str()) ;

		if ( !mf.advanceToNextLine() )
		{
			printf("FAIL: no next line\n") ;
			return -1 ;
		}

		line = mf.currentPtr() ;
		if ( isspace(*line) || *line == ';' )
		{
			printf("FAIL: Next character is '%c'\n", *line) ;
			return -1 ;
		}
		if ( !mf.advanceToEndOfParagraph() )
		{
			printf("FAIL: didn't find an end of paragraph.\n") ;
			return -1 ;
		}

		lineContent.assign(line, (mf.currentPtr() - line)) ;
		printf("--- snip --- rest of paragraph --- snip ---\n%s--- snip --- end of excerpt --- snip\n\n", lineContent.c_str()) ;
	}
	catch ( Smugl::FileError& e )
	{
		printf("FAIL: FileError %d: %s\n", e.m_errno, strerror(e.m_errno)) ;
		return -1 ;
	}
	catch ( std::exception& e )
	{
		printf("FAIL: Exception: %s\n", e.what()) ;
		return -1 ;
	}

	//// ---- End your code ---- ////

	return 0 ;
}
