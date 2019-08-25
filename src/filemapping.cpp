#include <cstring>

#include "amulconfig.h"

#include "h/amul.file.h"
#include "h/amul.type.h"
#include "h/filemapping.h"
#include "h/logging.h"

//#include <h/portable.hpp>

#if defined(HAVE_MMAN_H)
#include <mman.h>
#endif
#if defined(HAVE_SYS_MMAN_H)
#include <sys/mman.h>
#endif
#if defined(HAVE_MMAP_H)
#include <mmap.h>
#endif

#ifndef _MSC_VER
static const int MMAP_FLAGS = MAP_PRIVATE | MAP_FILE |
#ifdef MAP_POPULATE
                              MAP_POPULATE |
#endif
#ifdef MAP_DENYWRITE
                              MAP_DENYWRITE |
#endif
#ifdef MAP_NOCACHE
                              MAP_NOCACHE |
#endif
                              0;
#endif

#define REQUIRE(condition)                                                                         \
    if (!(condition))                                                                              \
    return EINVAL

error_t
NewFileMapping(std::string_view filepath, void **datap, size_t *sizep) noexcept
{
    REQUIRE(filepath.size() && datap);
    REQUIRE(*datap == nullptr);

    int fd = open(filepath.data(), O_RDONLY);
    if (fd < 0)
        return ENOENT;

    struct stat sb {};
    if (fstat(fd, &sb) < 0)
        return errno;

    const auto size = sb.st_size;
    if (sizep)
        *sizep = size;

#if defined(_MSC_VER)
    HANDLE osfh = (HANDLE) _get_osfhandle(fd);
    HANDLE maph = CreateFileMapping(osfh, nullptr, PAGE_READONLY, 0, 0, nullptr);
    close(fd);
    if (!maph) {
        LogFatal("unable to map file: ", filepath);
    }
    void *data = MapViewOfFile(maph, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(maph);
#else
    void *data = mmap(nullptr, size, PROT_READ, MMAP_FLAGS, fd, 0);
    if (data == MAP_FAILED) {
        LogFatal("failed to load file: ", filepath, ": ", strerror(errno));
    }
    close(fd);
#endif

    if (data == nullptr)
        LogFatal("unable to load file: ", filepath);

    *datap = data;
    LogDebug("mapped ", filepath);

    return 0;
}

void
CloseFileMapping(void **datap, size_t length) noexcept
{
    if (datap && *datap) {
#if defined(_MSC_VER)
        BOOL result = UnmapViewOfFile(*datap);
        if (!result)
            LogFatal("Error closing file mapping");
#else
        munmap(*datap, length);
#endif
    }
    if (datap)
        *datap = nullptr;
}
