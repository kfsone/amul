#include "cl_mappedfile.hpp"
#include "config.h"
#include "fderror.hpp"

#if defined(HAVE_MMAN_H)
#include <mman.h>
#elif defined(HAVE_SYS_MMAN_H)
#include <sys/mman.h>
#elif defined(WIN32) || defined(_MSC_VER) || defined(_MSCVER)
#include <Windows.h>
#else
#error "MMap support required (MapViewOfFile not supported yet)"
#endif

#if defined(HAVE_MMAP)
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <stdexcept>

namespace Smugl
{

//////////////////////////////////////////////////////////////////////
// Posix version.

#if defined(HAVE_MMAP)
MappedFile::MappedFile(const char *const filename_, const std::string &dirname_)
    : m_filename(filename_), m_basePtr(nullptr), m_endPtr(nullptr), m_currentPtr(nullptr),
      m_size(0), m_lineNo(0), m_fd(-1)
{
    if (dirname_.empty() == false && dirname_[0] != PATH_SEP_CHAR) {
        if (dirname_.back() != PATH_SEP_CHAR)
            m_filename.insert(0, PATH_SEP);
        m_filename.insert(0, dirname_);
    }

    // Check we can open the file.
    const int fd = open(m_filename.c_str(), O_RDONLY);
    if (fd < 0)
        throw Smugl::FileError(filename_, errno);
    m_fd = fd;

    // Use fstat to quickly obtain the file size.
    struct stat stats;
    const int sr = fstat(fd, &stats);
    if (sr < 0)
        throw Smugl::FileError(filename_, errno);
    m_size = stats.st_size;

    // Empty file? Don't try and mmap it then.
    if (m_size == 0)
        return;

    // Try to memory map the data into memory.

    static const int flags =
            MAP_FILE | MAP_SHARED |
            MAP_POPULATE;  //| MAP_DENYWRITE | MAP_FILE | MAP_NORESERVE | MAP_POPULATE ;
    void *const ptr = mmap(nullptr, m_size + 1, PROT_READ, flags, m_fd, 0);
    if (ptr == MAP_FAILED)
        throw Smugl::FileError(filename_, errno);

    m_basePtr = ptr;
    m_currentPtr = static_cast<const char *>(m_basePtr);
    m_endPtr = m_currentPtr + m_size;

    // Skip ahead for any meaningful content.
    advanceToNextContent();
}

MappedFile::~MappedFile()
{
    if (m_basePtr != nullptr) {
        munmap(m_basePtr, m_size + 1);
        m_basePtr = nullptr;
        m_endPtr = m_currentPtr = nullptr;
        m_size = 0;
    }
    if (m_filename.empty() == false && m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
}

#else

MappedFile::MappedFile(const char *const filename_, const std::string dirname_)
    : m_filename(filename_), m_basePtr(nullptr), m_endPtr(nullptr), m_currentPtr(nullptr),
      m_size(0), m_lineNo(0), m_handle(INVALID_HANDLE_VALUE), m_mapping(INVALID_HANDLE_VALUE)
{
    if (dirname_.empty() == false && dirname_[0] != PATH_SEP_CHAR) {
        if (dirname_.back() != PATH_SEP_CHAR)
            m_filename.insert(0, PATH_SEP);
        m_filename.insert(0, dirname_);
        printf("extended file name, it's now %s\n", m_filename.c_str());
    }

    // Check we can open the file.
    const char *const filename = m_filename.c_str();
    m_handle = CreateFile(
            filename, FILE_GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
    if (m_handle == INVALID_HANDLE_VALUE)
        throw Smugl::FileError(filename, GetLastError());

    // Use fstat to quickly obtain the file size.
    LARGE_INTEGER size;
    if (!GetFileSizeEx(m_handle, &size))
        throw Smugl::FileError(filename, GetLastError());
    m_size = (size_t) size.QuadPart;

    // Empty file? Don't try and mmap it then.
    if (m_size == 0)
        return;

    // Try to memory map the data into memory.
    m_mapping = CreateFileMapping(m_handle, nullptr, PAGE_READONLY, 0, 0, nullptr);
    if (m_mapping == INVALID_HANDLE_VALUE)
        throw Smugl::FileError(filename, GetLastError());

    LPVOID const ptr = MapViewOfFileEx(m_mapping, FILE_MAP_READ, 0, 0, 0, nullptr);
    if (ptr == nullptr)
        throw Smugl::FileError(filename, GetLastError());

    m_basePtr = (void *) ptr;
    m_currentPtr = static_cast<const char *>(m_basePtr);
    m_endPtr = m_currentPtr + m_size;

    // Skip ahead for any meaningful content.
    advanceToNextContent();
}

MappedFile::~MappedFile()
{
    if (m_basePtr != nullptr) {
        UnmapViewOfFile(m_basePtr);
        m_basePtr = nullptr;
        m_endPtr = m_currentPtr = nullptr;
        m_size = 0;
    }
    if (m_mapping != INVALID_HANDLE_VALUE) {
        CloseHandle(m_mapping);
        m_mapping = INVALID_HANDLE_VALUE;
    }
    if (m_handle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_handle);
        m_handle = INVALID_HANDLE_VALUE;
    }
}
#endif  // HAVE_MMAP

//////////////////////////////////////////////////////////////////////
// Non-platform specific functions.
//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
// Find the last interesting character of the current line.

bool
MappedFile::advanceToEndOfLine()
{
    const char *ptr = m_currentPtr;
    while (ptr < m_endPtr) {
        const char cur = *(ptr++);
        if (cur == ';' || cur == '\r' || cur == '\n')
            return true;
        // Quoted comments ... skip them.
        if (cur == '\\' && ptr < m_endPtr && *ptr == ';')
            ++ptr;
        if (!isspace(cur))
            m_currentPtr = ptr;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// Skip to the start of the next line.

bool
MappedFile::advanceToNextLine()
{
    while (!atEof()) {
        const char cur = *m_currentPtr;
        // We support both \r and \n so that files work
        // across platforms too. If this isn't one of
        // them, then we're still on the line.
        if (cur != '\r' && cur != '\n') {
            ++m_currentPtr;
            continue;
        }

        // Now we've reached an end-of-line character.
        ++m_lineNo;
        ++m_currentPtr;

        // Did that put us at EOF?
        if (atEof())
            return false;

        // But what if it put us at a different EOL,
        // as happens on systems with \r\n?
        if (*m_currentPtr != cur && (*m_currentPtr == '\r' || *m_currentPtr == '\n')) {
            ++m_currentPtr;
            return !atEof();
        }

        // We reached the next line.
        return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Skip to the end of the current paragraph.

bool
MappedFile::advanceToEndOfParagraph()
{
    while (!atEof()) {
        // Seek to the start of the next line.
        if (!advanceToNextLine())
            return false;

        if (*m_currentPtr == '\r' || *m_currentPtr == '\n')
            return true;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Advance to the next non-whitespace character, with linecounts.

bool
MappedFile::advanceToNonWhitespace()
{
    while (!atEof()) {
        if (!isspace(*m_currentPtr))
            return true;
        if (*m_currentPtr == ';') {
            if (!advanceToNextLine())
                return false;
        }
        ++m_currentPtr;
    }
    return false;
}

//////////////////////////////////////////////////////////////////////
// Advance past the current word.

bool
MappedFile::advancePastCurrentWord()
{
    while (m_currentPtr < m_endPtr) {
        if (isspace(*m_currentPtr))
            return true;
        ++m_currentPtr;
    }

    return false;
}

//////////////////////////////////////////////////////////////////////
// Allow partial copying of mapped files; this creates a sub-view that
// doesn't own the file mapping itself.

MappedFile &
MappedFile::operator=(const MappedFile &rhs_)
{
    if (rhs_.m_filename.empty() == false || rhs_.m_basePtr != nullptr)
        throw std::runtime_error("illegal mapped-file operation");
    if (m_filename.empty() == false || m_basePtr != nullptr)
        throw std::runtime_error("illegal mapped-file overwrite operation");

    m_filename = "";
    m_basePtr = nullptr;
    m_endPtr = rhs_.m_endPtr;
    m_currentPtr = rhs_.m_currentPtr;
    m_size = m_endPtr - m_currentPtr;
    m_lineNo = rhs_.m_lineNo;

    return *this;
}

}  // namespace Smugl
